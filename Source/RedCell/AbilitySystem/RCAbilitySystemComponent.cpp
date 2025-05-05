// Fill out your copyright notice in the Description page of Project Settings.

#include "RCAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystem/RCAbilityTagRelationshipMapping.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilitySystemComponent)

URCAbilitySystemComponent::URCAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();

    FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void URCAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (!AbilitySpec.IsActive())
        {
            continue;
        }

        URCGameplayAbility* RCAbilityCDO = Cast<URCGameplayAbility>(AbilitySpec.Ability);
        if (!RCAbilityCDO)
        {
            UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Non-RCGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
            continue;
        }

PRAGMA_DISABLE_DEPRECATION_WARNINGS
        ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
            
        // Cancel all the spawned instances.
        TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
        for (UGameplayAbility* AbilityInstance : Instances)
        {
            URCGameplayAbility* RCAbilityInstance = CastChecked<URCGameplayAbility>(AbilityInstance);

            if (ShouldCancelFunc(RCAbilityInstance, AbilitySpec.Handle))
            {
                if (RCAbilityInstance->CanBeCanceled())
                {
                    RCAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), RCAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *RCAbilityInstance->GetName());
                }
            }
        }
    }
}


int32 URCAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    UE_LOG(LogTemp, Warning, TEXT("[ASC] HandleGameplayEvent: %s"), *EventTag.ToString());
    return Super::HandleGameplayEvent(EventTag, Payload);
}

/*
void URCAbilitySystemComponent::AddAbilitySet(URCAbilitySet* AbilitySet)
{
    if (AbilitySet)
    {
        UE_LOG(LogTemp, Log, TEXT("[ASC] Granting AbilitySet %s"), *AbilitySet->GetName());
        AbilitySet->GiveAbilities(this);
    }
}
*/

void URCAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

void URCAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}

bool URCAbilitySystemComponent::IsActivationGroupBlocked(ERCAbilityActivationGroup Group) const
{
    bool bBlocked = false;

    switch (Group)
    {
    case ERCAbilityActivationGroup::Independent:
        // Independent abilities are never blocked.
        bBlocked = false;
        break;

    case ERCAbilityActivationGroup::Exclusive_Replaceable:
    case ERCAbilityActivationGroup::Exclusive_Blocking:
        // Exclusive abilities can activate if nothing is blocking.
        bBlocked = (ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Blocking] > 0);
        break;

    default:
        checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
        break;
    }

    return bBlocked;
}

void URCAbilitySystemComponent::AddAbilityToActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility)
{
    check(RCAbility);
    check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

    ActivationGroupCounts[(uint8)Group]++;

    const bool bReplicateCancelAbility = false;

    switch (Group)
    {
    case ERCAbilityActivationGroup::Independent:
        // Independent abilities do not cancel any other abilities.
        break;

    case ERCAbilityActivationGroup::Exclusive_Replaceable:
    case ERCAbilityActivationGroup::Exclusive_Blocking:
        CancelActivationGroupAbilities(ERCAbilityActivationGroup::Exclusive_Replaceable, RCAbility, bReplicateCancelAbility);
        break;

    default:
        checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
        break;
    }

    const int32 ExclusiveCount = ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Blocking];
    if (!ensure(ExclusiveCount <= 1))
    {
        UE_LOG(LogTemp, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
    }
}

void URCAbilitySystemComponent::RemoveAbilityFromActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility)
{
    check(RCAbility);
    check(ActivationGroupCounts[(uint8)Group] > 0);

    ActivationGroupCounts[(uint8)Group]--;
}

void URCAbilitySystemComponent::CancelActivationGroupAbilities(ERCAbilityActivationGroup Group, URCGameplayAbility* IgnoreRCAbility, bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this, Group, IgnoreRCAbility](const URCGameplayAbility* RCAbility, FGameplayAbilitySpecHandle Handle)
    {
        return ((RCAbility->GetActivationGroup() == Group) && (RCAbility != IgnoreRCAbility));
    };

    CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}


void URCAbilitySystemComponent::SetTagRelationshipMapping(URCAbilityTagRelationshipMapping* NewMapping)
{
    TagRelationshipMapping = NewMapping;
}

void URCAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
    if (TagRelationshipMapping)
    {
        TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
    }
}


bool URCAbilitySystemComponent::IsAbilityActiveByTag(FGameplayTag AbilityTag) const
{
    for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (!Spec.IsActive())
        {
            continue;
        }
        // Use the ability's tags to identify it
        if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}

bool URCAbilitySystemComponent::IsAbilityGrantedByTag(FGameplayTag AbilityTag) const
{
    for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}
