// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

URCGameplayAbility::URCGameplayAbility(const FObjectInitializer& ObjInit)
: Super(ObjInit)
{
  // Make most abilities instanced per actor by default
  InstancingPolicy     = EGameplayAbilityInstancingPolicy::InstancedPerActor;

  // Default to server‐initiated (override in Blueprint/C++ for client abilities)
  NetExecutionPolicy   = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

  // (Optional) give every ability the same activation failure tag container,
  // or hook up a Blueprint event on failure, etc.
}

bool URCGameplayAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    // 1) Always call the parent first to get cost/cooldown/tag-array checks:
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    // 2) Grab the ASC’s current tags:
    const FGameplayTagContainer& OwnedTags = ActorInfo->AbilitySystemComponent->GetOwnedGameplayTags();

    // 3) Required-tag query must pass:
    if (!ActivationRequiredTagQuery.IsEmpty() &&
        !ActivationRequiredTagQuery.Matches(OwnedTags))
    {
        // Optionally append to OptionalRelevantTags here for debugging
        return false;
    }

    // 4) Block-tag query must *not* match:
    if (!ActivationBlockedTagQuery.IsEmpty() &&
        ActivationBlockedTagQuery.Matches(OwnedTags))
    {
        return false;
    }

    // 5) All checks passed:
    return true;
}
