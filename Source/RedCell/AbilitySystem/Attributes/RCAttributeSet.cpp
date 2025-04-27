// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RCAttributeSet.h"

#include "AbilitySystem/RCAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAttributeSet)

URCAttributeSet::URCAttributeSet()
{
}

UWorld* URCAttributeSet::GetWorld() const
{
    if (const UObject* Outer = GetOuter())
    {
        return Outer->GetWorld();
    }
    return nullptr;
}

URCAbilitySystemComponent* URCAttributeSet::GetRCAbilitySystemComponent() const
{
    return Cast<URCAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}


