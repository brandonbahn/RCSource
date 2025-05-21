// Fill out your copyright notice in the Description page of Project Settings.


#include "RCEquipmentManagerComponent.h"

#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/ActorChannel.h"
#include "RCEquipmentDefinition.h"
#include "RCEquipmentInstance.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCEquipmentManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//////////////////////////////////////////////////////////////////////
// FRCAppliedEquipmentEntry

FString FRCAppliedEquipmentEntry::GetDebugString() const
{
    return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}

//////////////////////////////////////////////////////////////////////
// FRCEquipmentList

void FRCEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
     for (int32 Index : RemovedIndices)
     {
         const FRCAppliedEquipmentEntry& Entry = Entries[Index];
        if (Entry.Instance != nullptr)
        {
            Entry.Instance->OnUnequipped();
        }
     }
}

void FRCEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        const FRCAppliedEquipmentEntry& Entry = Entries[Index];
        if (Entry.Instance != nullptr)
        {
            Entry.Instance->OnEquipped();
        }
    }
}

void FRCEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
//     for (int32 Index : ChangedIndices)
//     {
//         const FGameplayTagStack& Stack = Stacks[Index];
//         TagToCountMap[Stack.Tag] = Stack.StackCount;
//     }
}

URCAbilitySystemComponent* FRCEquipmentList::GetAbilitySystemComponent() const
{
    check(OwnerComponent);
    AActor* OwningActor = OwnerComponent->GetOwner();
    return Cast<URCAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

URCEquipmentInstance* FRCEquipmentList::AddEntry(TSubclassOf<URCEquipmentDefinition> EquipmentDefinition)
{
    URCEquipmentInstance* Result = nullptr;

    check(EquipmentDefinition != nullptr);
     check(OwnerComponent);
    check(OwnerComponent->GetOwner()->HasAuthority());
    
    const URCEquipmentDefinition* EquipmentCDO = GetDefault<URCEquipmentDefinition>(EquipmentDefinition);

    TSubclassOf<URCEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
    if (InstanceType == nullptr)
    {
        InstanceType = URCEquipmentInstance::StaticClass();
    }
    
    FRCAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
    NewEntry.EquipmentDefinition = EquipmentDefinition;
    NewEntry.Instance = NewObject<URCEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);  //@TODO: Using the actor instead of component as the outer due to UE-127172
    Result = NewEntry.Instance;

    if (URCAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        for (const TObjectPtr<const URCAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
        {
            AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
        }
    }
    else
    {
        //@TODO: Warning logging?
    }

    Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);


    MarkItemDirty(NewEntry);

    return Result;
}

void FRCEquipmentList::RemoveEntry(URCEquipmentInstance* Instance)
{
    for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
    {
        FRCAppliedEquipmentEntry& Entry = *EntryIt;
        if (Entry.Instance == Instance)
        {
            if (URCAbilitySystemComponent* ASC = GetAbilitySystemComponent())
            {
                Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
            }

            Instance->DestroyEquipmentActors();
            

            EntryIt.RemoveCurrent();
            MarkArrayDirty();
        }
    }
}

//////////////////////////////////////////////////////////////////////
// URCEquipmentManagerComponent

URCEquipmentManagerComponent::URCEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , EquipmentList(this)
{
    SetIsReplicatedByDefault(true);
    bWantsInitializeComponent = true;
}

void URCEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, EquipmentList);
}

URCEquipmentInstance* URCEquipmentManagerComponent::EquipItem(TSubclassOf<URCEquipmentDefinition> EquipmentClass)
{
    URCEquipmentInstance* Result = nullptr;
    if (EquipmentClass != nullptr)
    {
        Result = EquipmentList.AddEntry(EquipmentClass);
        if (Result != nullptr)
        {
            Result->OnEquipped();

            if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
            {
                AddReplicatedSubObject(Result);
            }
        }
    }
    return Result;
}

void URCEquipmentManagerComponent::UnequipItem(URCEquipmentInstance* ItemInstance)
{
    if (ItemInstance != nullptr)
    {
        if (IsUsingRegisteredSubObjectList())
        {
            RemoveReplicatedSubObject(ItemInstance);
        }

        ItemInstance->OnUnequipped();
        EquipmentList.RemoveEntry(ItemInstance);
    }
}

bool URCEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (FRCAppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        URCEquipmentInstance* Instance = Entry.Instance;

        if (IsValid(Instance))
        {
            WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
        }
    }

    return WroteSomething;
}

void URCEquipmentManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void URCEquipmentManagerComponent::UninitializeComponent()
{
    TArray<URCEquipmentInstance*> AllEquipmentInstances;

    // gathering all instances before removal to avoid side effects affecting the equipment list iterator
    for (const FRCAppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        AllEquipmentInstances.Add(Entry.Instance);
    }

    for (URCEquipmentInstance* EquipInstance : AllEquipmentInstances)
    {
        UnequipItem(EquipInstance);
    }

    Super::UninitializeComponent();
}

void URCEquipmentManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing RCEquipmentInstances
    if (IsUsingRegisteredSubObjectList())
    {
        for (const FRCAppliedEquipmentEntry& Entry : EquipmentList.Entries)
        {
            URCEquipmentInstance* Instance = Entry.Instance;

            if (IsValid(Instance))
            {
                AddReplicatedSubObject(Instance);
            }
        }
    }
}

URCEquipmentInstance* URCEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<URCEquipmentInstance> InstanceType)
{
    for (FRCAppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        if (URCEquipmentInstance* Instance = Entry.Instance)
        {
            if (Instance->IsA(InstanceType))
            {
                return Instance;
            }
        }
    }

    return nullptr;
}

TArray<URCEquipmentInstance*> URCEquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<URCEquipmentInstance> InstanceType) const
{
    TArray<URCEquipmentInstance*> Results;
    for (const FRCAppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        if (URCEquipmentInstance* Instance = Entry.Instance)
        {
            if (Instance->IsA(InstanceType))
            {
                Results.Add(Instance);
            }
        }
    }
    return Results;
}


