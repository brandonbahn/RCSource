// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RCDamageExecution.generated.h"

class UObject;

/**
 * URCDamageExecution
 *
 * Execution used by gameplay effects to apply damage to the health attributes.
 */
UCLASS()
class REDCELL_API URCDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:

	URCDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

