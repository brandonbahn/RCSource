// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
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

    // You can add project‑wide helpers here, e.g.:
    // UFUNCTION(BlueprintCallable) void K2_OnAbilityFailedToActivate(const FGameplayTagContainer& FailureTags);

protected:
    // Override any of the UGameplayAbility hooks you need, e.g.
    // virtual void ActivateAbility(...) override;
};
