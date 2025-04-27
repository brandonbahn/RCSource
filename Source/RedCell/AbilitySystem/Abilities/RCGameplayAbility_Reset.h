// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "RCGameplayAbility_Reset.generated.h"

UCLASS()
class REDCELL_API URCGameplayAbility_Reset : public URCGameplayAbility
{
    GENERATED_BODY()

public:
    URCGameplayAbility_Reset(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /** Optionally clean up or notify after reset */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
};
