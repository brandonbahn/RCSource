// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "RCGameplayAbility_Death.generated.h"

/**
 * 
 */
UCLASS()
class REDCELL_API URCGameplayAbility_Death : public URCGameplayAbility
{
    GENERATED_BODY()

public:
    URCGameplayAbility_Death(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

protected:
    // Triggered when the Death event fires
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

    /** Kick off the death flow (e.g. disable input, play anim) */
    UFUNCTION(BlueprintCallable, Category="RedCell|Ability|Death")
    void StartDeath();

    /** Complete the death flow (e.g. notify Reset ability) */
    UFUNCTION(BlueprintCallable, Category="RedCell|Ability|Death")
    void FinishDeath();

protected:
    /** If true, StartDeath() is called automatically when activated */
    UPROPERTY(EditDefaultsOnly, Category="RedCell|Ability|Death")
    bool bAutoStartDeath = true;
};
