// Fill out your copyright notice in the Description page of Project Settings.

#include "RCCombatSet.h"

#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCombatSet)

class FLifetimeProperty;


URCCombatSet::URCCombatSet()
    : BaseDamage(0.0f)
    , BaseHeal(0.0f)
{
}

void URCCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URCCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URCCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void URCCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCCombatSet, BaseDamage, OldValue);
}

void URCCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCCombatSet, BaseHeal, OldValue);
}

