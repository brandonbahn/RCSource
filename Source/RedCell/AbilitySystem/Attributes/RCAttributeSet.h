// Fill out your copyright notice in the Description page of Project Settings.

// RCAttributeSet.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RCAttributeSet.generated.h"

class URCAbilitySystemComponent;

/**
 * Same‐as Lyra’s macro, gives:
 *   static FGameplayAttribute GetXAttribute();
 *   float GetX() const;
 *   void SetX(float NewVal);
 *   void InitX(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base AttributeSet for RedCell.
 *  – so the URCAttributeSet_* classes can share common helpers
 */
UCLASS()
class REDCELL_API URCAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    URCAttributeSet();

    /** Allow attributes to safely call GetWorld() */
    UWorld* GetWorld() const override;

    /** Typed convenience cast */
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;
};

