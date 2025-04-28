// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySet.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

#include "GameplayAbilitySpecHandle.h"
#include "RCAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;
class URCAbilitySystemComponent;
class URCGameplayAbility;
class UObject;

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
