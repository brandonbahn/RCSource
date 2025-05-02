// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystem/RCAbilityTagRelationshipMapping.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"

int32 URCAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    UE_LOG(LogTemp, Warning, TEXT("[ASC] HandleGameplayEvent: %s"), *EventTag.ToString());
    return Super::HandleGameplayEvent(EventTag, Payload);
}

void URCAbilitySystemComponent::AddAbilitySet(URCAbilitySet* AbilitySet)
{
    if (AbilitySet)
    {
        UE_LOG(LogTemp, Log, TEXT("[ASC] Granting AbilitySet %s"), *AbilitySet->GetName());
        AbilitySet->GiveAbilities(this);
    }
}

void URCAbilitySystemComponent::SetTagRelationshipMapping(URCAbilityTagRelationshipMapping* NewMapping)
{
    TagRelationshipMapping = NewMapping;
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
