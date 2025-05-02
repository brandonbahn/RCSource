// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "RCGameplayAbility.generated.h"

/**
 *
 */
UCLASS()
class REDCELL_API URCGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    URCGameplayAbility(const FObjectInitializer& ObjInit = FObjectInitializer::Get());
    
    /** Rich tag‐query in place of simple Required/Blocked lists */
    UPROPERTY(EditDefaultsOnly, Category="Tags")
    FGameplayTagQuery ActivationRequiredTagQuery;

    UPROPERTY(EditDefaultsOnly, Category="Tags")
    FGameplayTagQuery ActivationBlockedTagQuery;
    
    // Override the built-in activation check to run our tag queries
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    // You can add project‑wide helpers here, e.g.:
    // UFUNCTION(BlueprintCallable) void K2_OnAbilityFailedToActivate(const FGameplayTagContainer& FailureTags);

protected:
    // Override any of the UGameplayAbility hooks you need, e.g.
    // virtual void ActivateAbility(...) override;
};
