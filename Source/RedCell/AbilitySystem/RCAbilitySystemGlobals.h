// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"

#include "RCAbilitySystemGlobals.generated.h"

class UObject;
struct FGameplayEffectContext;

UCLASS(Config=Game)
class URCAbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_UCLASS_BODY()

    //~UAbilitySystemGlobals interface
    virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
    //~End of UAbilitySystemGlobals interface
};
