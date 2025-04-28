// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/RCAbilityTagRelationshipMapping.h"
#include "AbilitySystem/RCAbilitySet.h"

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
