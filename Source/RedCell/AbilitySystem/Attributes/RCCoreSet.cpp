// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCoreSet)

UE_DEFINE_GAMEPLAY_TAG(TAG_Mana_Empty,    "Mana.Empty");
UE_DEFINE_GAMEPLAY_TAG(TAG_Mana_Restored, "Mana.Restored");

URCCoreSet::URCCoreSet()
    : Mana(500.f)
    , MaxMana(500.f)

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

    const float CurrentMana = GetMana();
    const float EstimatedMagnitude = CurrentMana - OldValue.GetCurrentValue();
	
    OnManaChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMana);

    if (!bOutOfMana && CurrentMana <= 0.0f)
    {
        OnOutOfMana.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMana);
    }

    bOutOfMana = (CurrentMana <= 0.0f);
}

void URCCoreSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCCoreSet, MaxMana, OldValue);

    OnMaxManaChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxMana() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxMana());
}

bool URCCoreSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }
    // Save the current mana
    ManaBeforeAttributeChange = GetMana();
    MaxManaBeforeAttributeChange = GetMaxMana();

    return true;
}

void URCCoreSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
    AActor* Instigator = EffectContext.GetOriginalInstigator();
    AActor* Causer = EffectContext.GetEffectCauser();

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
    // If Mana has actually changed activate callbacks
    if (GetMana() != ManaBeforeAttributeChange)
    {
        OnManaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, ManaBeforeAttributeChange, GetMana());
    }

    if ((GetMana() <= 0.0f) && !bOutOfMana)
    {
        OnOutOfMana.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, ManaBeforeAttributeChange, GetMana());
    }

    // Check Mana again in case an event above changed it.
    bOutOfMana = (GetMana() <= 0.0f);
}


void URCCoreSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
    ClampAttribute(Attribute, NewValue);
}

void URCCoreSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void URCCoreSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxManaAttribute())
    {
        // Make sure current Mana is not greater than the new max Mana.
        if (GetMana() > NewValue)
        {
            URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent();
            check(RCASC);

            RCASC->ApplyModToAttribute(GetManaAttribute(), EGameplayModOp::Override, NewValue);
        }
    }

    if (bOutOfMana && (GetMana() > 0.0f))
    {
        bOutOfMana = false;
    }
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