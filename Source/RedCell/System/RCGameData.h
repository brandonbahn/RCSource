// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RCGameData.generated.h"

class UGameplayEffect;
class UObject;

/**
 * Non-mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Redcell Game Data", ShortTooltip = "Data asset containing global game data."))
class REDCELL_API URCGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	URCGameData();

	// Returns the loaded game data.
	static const URCGameData& Get();

public:

	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};

