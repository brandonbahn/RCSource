// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/RCGameplayAbility_Reset.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "RCGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

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
    // --- DEBUG ---
    APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    const FString AvatarName = AvatarPawn ? AvatarPawn->GetName() : TEXT("NonePawn");
    UE_LOG(LogTemp, Warning, TEXT("[ResetAbility] ActivateAbility on %s"), *AvatarName);

    if (AvatarPawn)
    {
        if (APlayerController* PC = Cast<APlayerController>(AvatarPawn->GetController()))
        {
            PC->ClientMessage(TEXT("ResetAbility Activated"), NAME_None, 5.f);
        }
    }
    // --- END DEBUG ---

    // Actual reset logic: use the pawn’s controller
    if (AvatarPawn)
    {
        if (AController* C = AvatarPawn->GetController())
        {
            if (AGameModeBase* GM = GetWorld()->GetAuthGameMode<AGameModeBase>())
            {
                GM->RestartPlayer(C);
            }
        }
    }

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}

void URCGameplayAbility_Reset::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
