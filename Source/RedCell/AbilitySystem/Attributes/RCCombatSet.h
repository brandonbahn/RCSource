// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "RCCombatSet.generated.h"

class UObject;
struct FFrame;


/**
 * URCCombatSet
 *
 *  Class that defines attributes that are necessary for applying damage or healing.
 *  Attribute examples include: damage, healing, attack power, and shield penetrations.
 */

UCLASS(BlueprintType)
class URCCombatSet : public URCAttributeSet
{
	GENERATED_BODY()
    
public:

    URCCombatSet();

    ATTRIBUTE_ACCESSORS(URCCombatSet, BaseDamage);
    ATTRIBUTE_ACCESSORS(URCCombatSet, BaseHeal);

protected:

    UFUNCTION()
    void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

    // The base amount of damage to apply in the damage execution.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "RedCell|Combat", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData BaseDamage;

    // The base amount of healing to apply in the heal execution.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "RedCell|Combat", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData BaseHeal;
	
};
