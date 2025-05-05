// Fill out your copyright notice in the Description page of Project Settings.


#include "RCAbilitySystemGlobals.h"

#include "RCGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilitySystemGlobals)

struct FGameplayEffectContext;

URCAbilitySystemGlobals::URCAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FGameplayEffectContext* URCAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FRCGameplayEffectContext();
}
