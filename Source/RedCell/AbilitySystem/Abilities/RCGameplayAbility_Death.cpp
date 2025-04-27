// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/RCGameplayAbility_Death.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "RCGameplayTags.h"


URCGameplayAbility_Death::URCGameplayAbility_Death(const FObjectInitializer& ObjInit)
  : Super(ObjInit)
{
    InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    bAutoStartDeath    = true;

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        FAbilityTriggerData TriggerData;
        TriggerData.TriggerTag    = RCGameplayTags::GameplayEvent_Death;
        TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        AbilityTriggers.Add(TriggerData);
    }
}

void URCGameplayAbility_Death::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // --- BEGIN DEBUG ---
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,                    // Key (-1 = new message)
            5.f,                   // Display for 5 seconds
            FColor::Red,           // Text color
            TEXT("DeathAbility Activated")
        );
    }
    // --- END DEBUG SNIPPET ---

    URCAbilitySystemComponent* RCASC = CastChecked<URCAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

    // Cancel everything except abilities marked to survive death
    FGameplayTagContainer IgnoreTags;
    IgnoreTags.AddTag(RCGameplayTags::Ability_Behavior_SurvivesDeath);
    RCASC->CancelAbilities(nullptr, &IgnoreTags, this);

    SetCanBeCanceled(false);

    if (bAutoStartDeath)
    {
        StartDeath();
    }

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URCGameplayAbility_Death::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // Once the ability ends, queue the reset
    FinishDeath();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URCGameplayAbility_Death::StartDeath()
{
    // TODO: if you implement a HealthComponent with StartDeath(), call it here.
    // For now, we simply move immediately to FinishDeath().
}

void URCGameplayAbility_Death::FinishDeath()
{
    // Fire the reset event on the ASC
    if (URCAbilitySystemComponent* RCASC = Cast<URCAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
    {
        FGameplayEventData EventData;
        RCASC->HandleGameplayEvent(
            RCGameplayTags::GameplayEvent_RequestReset,
            /*Payload=*/&EventData
        );
    }
}
