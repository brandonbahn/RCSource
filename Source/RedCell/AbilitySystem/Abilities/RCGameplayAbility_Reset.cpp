// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/RCGameplayAbility_Reset.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RCGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayAbility_Reset)

URCGameplayAbility_Reset::URCGameplayAbility_Reset(const FObjectInitializer& ObjInit)
  : Super(ObjInit)
{
    InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        FAbilityTriggerData TriggerData;
        TriggerData.TriggerTag    = RCGameplayTags::GameplayEvent_RequestReset;
        TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        AbilityTriggers.Add(TriggerData);
    }
}

void URCGameplayAbility_Reset::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    check(ActorInfo);

    URCAbilitySystemComponent* RCASC = CastChecked<URCAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

    FGameplayTagContainer AbilityTypesToIgnore;
    AbilityTypesToIgnore.AddTag(RCGameplayTags::Ability_Behavior_SurvivesDeath);

    // Cancel all abilities and block others from starting.
    RCASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

    SetCanBeCanceled(false);

    // Execute the reset from the character
    if (ARCCharacter* RCChar = Cast<ARCCharacter>(CurrentActorInfo->AvatarActor.Get()))
    {
        RCChar->Reset();
    }

    // Let others know a reset has occurred
    FRCPlayerResetMessage Message;
    Message.OwnerPlayerState = CurrentActorInfo->OwnerActor.Get();
    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(RCGameplayTags::GameplayEvent_Reset, Message);

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    const bool bReplicateEndAbility = true;
    const bool bWasCanceled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCanceled);
}

