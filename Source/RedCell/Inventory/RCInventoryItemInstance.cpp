// Fill out your copyright notice in the Description page of Project Settings.

#include "RCInventoryItemInstance.h"

#include "Inventory/RCInventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCInventoryItemInstance)

class FLifetimeProperty;

URCInventoryItemInstance::URCInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URCInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, StatTags);
    DOREPLIFETIME(ThisClass, ItemDef);
}

#if UE_WITH_IRIS
void URCInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
    using namespace UE::Net;

    // Build descriptors and allocate PropertyReplicationFragments for this object
    FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void URCInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.AddStack(Tag, StackCount);
}

void URCInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.RemoveStack(Tag, StackCount);
}

int32 URCInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
    return StatTags.GetStackCount(Tag);
}

bool URCInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
    return StatTags.ContainsTag(Tag);
}

void URCInventoryItemInstance::SetItemDef(TSubclassOf<URCInventoryItemDefinition> InDef)
{
    ItemDef = InDef;
}

const URCInventoryItemFragment* URCInventoryItemInstance::FindFragmentByClass(TSubclassOf<URCInventoryItemFragment> FragmentClass) const
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<URCInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }

    return nullptr;
}


