// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RCHealExecution.generated.h"

/**
 * Execution used by gameplay effects to apply healing to the health attributes.
 */
UCLASS()
class URCHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	URCHealExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
