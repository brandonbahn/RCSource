// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySet.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "RCAbilitySet.generated.h"

UCLASS()
class REDCELL_API URCAbilitySet : public UGameplayAbilitySet
{
    GENERATED_BODY()

public:
    /** All GameplayEffects to apply on spawn (e.g. attribute init) */
    UPROPERTY(EditDefaultsOnly, Category="Effects")
    TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

    /** Grant both Abilities (base) and our init‐Effects */
    virtual void GiveAbilities(UAbilitySystemComponent* ASC);
};
