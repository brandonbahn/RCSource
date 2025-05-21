// Fill out your copyright notice in the Description page of Project Settings.


#include "RCInventoryManagerComponent.h"

#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RCInventoryItemDefinition.h"
#include "RCInventoryItemInstance.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RedCell_Inventory_Message_StackChanged, "RedCell.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FRCInventoryEntry

FString FRCInventoryEntry::GetDebugString() const
{
    TSubclassOf<URCInventoryItemDefinition> ItemDef;
    if (Instance != nullptr)
    {
        ItemDef = Instance->GetItemDef();
    }

    return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

//////////////////////////////////////////////////////////////////////
// FRCInventoryList

void FRCInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
    for (int32 Index : RemovedIndices)
    {
        FRCInventoryEntry& Stack = Entries[Index];
        BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
        Stack.LastObservedCount = 0;
    }
}

void FRCInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        FRCInventoryEntry& Stack = Entries[Index];
        BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
        Stack.LastObservedCount = Stack.StackCount;
    }
}

void FRCInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    for (int32 Index : ChangedIndices)
    {
        FRCInventoryEntry& Stack = Entries[Index];
        check(Stack.LastObservedCount != INDEX_NONE);
        BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
        Stack.LastObservedCount = Stack.StackCount;
    }
}

void FRCInventoryList::BroadcastChangeMessage(FRCInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
    FRCInventoryChangeMessage Message;
    Message.InventoryOwner = OwnerComponent;
    Message.Instance = Entry.Instance;
    Message.NewCount = NewCount;
    Message.Delta = NewCount - OldCount;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
    MessageSystem.BroadcastMessage(TAG_RedCell_Inventory_Message_StackChanged, Message);
}

URCInventoryItemInstance* FRCInventoryList::AddEntry(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 StackCount)
{
    URCInventoryItemInstance* Result = nullptr;

    check(ItemDef != nullptr);
     check(OwnerComponent);

    AActor* OwningActor = OwnerComponent->GetOwner();
    check(OwningActor->HasAuthority());


    FRCInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
    NewEntry.Instance = NewObject<URCInventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
    NewEntry.Instance->SetItemDef(ItemDef);
    for (URCInventoryItemFragment* Fragment : GetDefault<URCInventoryItemDefinition>(ItemDef)->Fragments)
    {
        if (Fragment != nullptr)
        {
            Fragment->OnInstanceCreated(NewEntry.Instance);
        }
    }
    NewEntry.StackCount = StackCount;
    Result = NewEntry.Instance;

    //const URCInventoryItemDefinition* ItemCDO = GetDefault<URCInventoryItemDefinition>(ItemDef);
    MarkItemDirty(NewEntry);

    return Result;
}

void FRCInventoryList::AddEntry(URCInventoryItemInstance* Instance)
{
    unimplemented();
}

void FRCInventoryList::RemoveEntry(URCInventoryItemInstance* Instance)
{
    for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
    {
        FRCInventoryEntry& Entry = *EntryIt;
        if (Entry.Instance == Instance)
        {
            EntryIt.RemoveCurrent();
            MarkArrayDirty();
        }
    }
}

TArray<URCInventoryItemInstance*> FRCInventoryList::GetAllItems() const
{
    TArray<URCInventoryItemInstance*> Results;
    Results.Reserve(Entries.Num());
    for (const FRCInventoryEntry& Entry : Entries)
    {
        if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
        {
            Results.Add(Entry.Instance);
        }
    }
    return Results;
}

//////////////////////////////////////////////////////////////////////
// URCInventoryManagerComponent

URCInventoryManagerComponent::URCInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , InventoryList(this)
{
    SetIsReplicatedByDefault(true);
}

void URCInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, InventoryList);
}

bool URCInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 StackCount)
{
    //@TODO: Add support for stack limit / uniqueness checks / etc...
    return true;
}

URCInventoryItemInstance* URCInventoryManagerComponent::AddItemDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 StackCount)
{
    URCInventoryItemInstance* Result = nullptr;
    if (ItemDef != nullptr)
    {
        Result = InventoryList.AddEntry(ItemDef, StackCount);
        
        if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
        {
            AddReplicatedSubObject(Result);
        }
    }
    return Result;
}

void URCInventoryManagerComponent::AddItemInstance(URCInventoryItemInstance* ItemInstance)
{
    InventoryList.AddEntry(ItemInstance);
    if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
    {
        AddReplicatedSubObject(ItemInstance);
    }
}

void URCInventoryManagerComponent::RemoveItemInstance(URCInventoryItemInstance* ItemInstance)
{
    InventoryList.RemoveEntry(ItemInstance);

    if (ItemInstance && IsUsingRegisteredSubObjectList())
    {
        RemoveReplicatedSubObject(ItemInstance);
    }
}

TArray<URCInventoryItemInstance*> URCInventoryManagerComponent::GetAllItems() const
{
    return InventoryList.GetAllItems();
}

URCInventoryItemInstance* URCInventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef) const
{
    for (const FRCInventoryEntry& Entry : InventoryList.Entries)
    {
        URCInventoryItemInstance* Instance = Entry.Instance;

        if (IsValid(Instance))
        {
            if (Instance->GetItemDef() == ItemDef)
            {
                return Instance;
            }
        }
    }

    return nullptr;
}

int32 URCInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef) const
{
    int32 TotalCount = 0;
    for (const FRCInventoryEntry& Entry : InventoryList.Entries)
    {
        URCInventoryItemInstance* Instance = Entry.Instance;

        if (IsValid(Instance))
        {
            if (Instance->GetItemDef() == ItemDef)
            {
                ++TotalCount;
            }
        }
    }

    return TotalCount;
}

bool URCInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
    AActor* OwningActor = GetOwner();
    if (!OwningActor || !OwningActor->HasAuthority())
    {
        return false;
    }

    //@TODO: N squared right now as there's no acceleration structure
    int32 TotalConsumed = 0;
    while (TotalConsumed < NumToConsume)
    {
        if (URCInventoryItemInstance* Instance = URCInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
        {
            InventoryList.RemoveEntry(Instance);
            ++TotalConsumed;
        }
        else
        {
            return false;
        }
    }

    return TotalConsumed == NumToConsume;
}

void URCInventoryManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing URCInventoryItemInstance
    if (IsUsingRegisteredSubObjectList())
    {
        for (const FRCInventoryEntry& Entry : InventoryList.Entries)
        {
            URCInventoryItemInstance* Instance = Entry.Instance;

            if (IsValid(Instance))
            {
                AddReplicatedSubObject(Instance);
            }
        }
    }
}

bool URCInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (FRCInventoryEntry& Entry : InventoryList.Entries)
    {
        URCInventoryItemInstance* Instance = Entry.Instance;

        if (Instance && IsValid(Instance))
        {
            WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
        }
    }

    return WroteSomething;
}
