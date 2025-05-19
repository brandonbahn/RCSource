// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URCCoreSet::URCCoreSet()
    : Mana(100.f)
    , MaxMana(100.f)

{
    bOutOfMana = false;
    MaxManaBeforeAttributeChange = 0.f;
    ManaBeforeAttributeChange = 0.f;
}


void URCCoreSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URCCoreSet, Mana,    COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URCCoreSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void URCCoreSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCCoreSet, Mana, OldValue);
}

void URCCoreSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCCoreSet, MaxMana, OldValue);
}

void URCCoreSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxManaAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.f);
        if (Mana.GetCurrentValue() > NewValue)
        {
            Mana.SetCurrentValue(NewValue);
        }
    }
    else if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, MaxMana.GetCurrentValue());
    }
}

void URCCoreSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // Only run this when Mana actually changed
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        const float NewMana = GetMana();
        if (NewMana <= 0.f && !bOutOfMana)
        {
            bOutOfMana = true;

            // Fire a custom "ManaEmpty" gameplay event (define a tag for this)
            if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
            {
                FGameplayEventData EventData;
                EventData.EventTag = RCGameplayTags::GameplayEvent_Mana_Empty;
                EventData.Instigator = GetOwningActor();
                ASC->HandleGameplayEvent(RCGameplayTags::GameplayEvent_Mana_Empty, &EventData);
            }
        }
        // Regained some mana again
        else if (NewMana > 0.f && bOutOfMana)
        {
            bOutOfMana = false;

            if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
            {
                FGameplayEventData EventData;
                EventData.EventTag = RCGameplayTags::GameplayEvent_Mana_Restored;
                EventData.Instigator = GetOwningActor();
                ASC->HandleGameplayEvent(RCGameplayTags::GameplayEvent_Mana_Restored, &EventData);
            }
        }
    }
}

void URCCoreSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
    ClampAttribute(Attribute, NewValue);
}

void URCCoreSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
    }
    else if (Attribute == GetMaxManaAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.f);
    }
}
