// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "RCAnimInstance.generated.h"

class UAbilitySystemComponent;

/**
 * URCAnimInstance
 *
 * The base game animation instance class used by this project.
 */
UCLASS(Config = Game)
class URCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	URCAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual void NativeInitializeAnimation() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
	
};
