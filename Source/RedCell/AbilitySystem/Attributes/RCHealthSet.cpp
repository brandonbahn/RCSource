// RCHealthSet.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"  // custom ASC
#include "NativeGameplayTags.h"                      // for UE_DEFINE_GAMEPLAY_TAG
#include "RCGameplayTags.h"                          // for RCGameplayTags::GameplayEvent_Death
#include "GameplayEffectExtension.h"                 // defines FGameplayEffectModCallbackData fully
#include "Messages/RCVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCHealthSet)

//――――――――――――――――――――――――――――――――――――――――――――――――
// Define native damage tags
//――――――――――――――――――――――――――――――――――――――――――――――――
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage,             "Gameplay.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageImmunity,     "Gameplay.DamageImmunity");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageSelfDestruct, "Gameplay.Damage.SelfDestruct");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_FellOutOfWorld,     "Gameplay.FellOutOfWorld");
UE_DEFINE_GAMEPLAY_TAG(TAG_RedCell_Damage_Message,      "RedCell.Damage.Message");

URCHealthSet::URCHealthSet()
    : Health(100.f)
    , MaxHealth(100.f)
{
    // match Lyra’s ctor initialization
    bOutOfHealth = false;
    MaxHealthBeforeAttributeChange = 0.f;
    HealthBeforeAttributeChange = 0.f;
}

void URCHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URCHealthSet, Health,    COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URCHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void URCHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCHealthSet, Health, OldValue);
}

void URCHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URCHealthSet, MaxHealth, OldValue);
}

void URCHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.f);
        if (Health.GetCurrentValue() > NewValue)
        {
            Health.SetCurrentValue(NewValue);
        }
    }
    else if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, MaxHealth.GetCurrentValue());
    }
}

bool URCHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
    // you can now cast to your ASC if needed:
    // auto* RCASC = Cast<URCAbilitySystemComponent>(Data.Target.GetOwningAbilitySystemComponent());
    return Super::PreGameplayEffectExecute(Data);
}

void URCHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // Only run this when Health actually changed
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        const float NewHealth = GetHealth();
        if (NewHealth <= 0.f && !bOutOfHealth)
        {
            bOutOfHealth = true;

            // Grab your custom ASC and fire off the Death event
            if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
            {
                FGameplayEventData EventData;
                ASC->HandleGameplayEvent(
                    RCGameplayTags::GameplayEvent_Death,
                    /*Payload=*/&EventData
                );
            }
        }
    }
}

void URCHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
    ClampAttribute(Attribute, NewValue);
}

void URCHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);
    // your Lyra‑style out‑of‑health handling can go here, using RCAbilitySystemComponent if desired
}

void URCHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.f);
    }
}
