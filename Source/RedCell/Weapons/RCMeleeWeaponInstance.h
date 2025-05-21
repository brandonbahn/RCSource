// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"

#include "Weapons/RCWeaponInstance.h"
#include "AbilitySystem/RCAbilitySourceInterface.h"

#include "RCMeleeWeaponInstance.generated.h"

/**
 * URCMeleeWeaponInstance
 *
 * A piece of equipment representing a melee weapon spawned and applied to a pawn
 */
UCLASS()
class REDCELL_API URCMeleeWeaponInstance : public URCWeaponInstance, public IRCAbilitySourceInterface
{
    GENERATED_BODY()
    
public:
    
    // A curve that maps the distance (in cm) to a multiplier on the base damage from the associated gameplay effect
    // If there is no data in this curve, then the weapon is assumed to have no falloff with distance
    UPROPERTY(EditAnywhere, Category = "Weapon Config")
    FRuntimeFloatCurve DistanceDamageFalloff;

    // List of special tags that affect how damage is dealt
    // These tags will be compared to tags in the physical material of the thing being hit
    // If more than one tag is present, the multipliers will be combined multiplicatively
    UPROPERTY(EditAnywhere, Category = "Weapon Config")
    TMap<FGameplayTag, float> MaterialDamageMultiplier;
    
public:
    void Tick(float DeltaSeconds);

    //~URCEquipmentInstance interface
    virtual void OnEquipped();
    virtual void OnUnequipped();
    //~End of URCEquipmentInstance interface
    
    //~IRCAbilitySourceInterface interface
    virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
    virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
    //~End of IRCAbilitySourceInterface interface

public:
   /** Perform a melee attack (sweep or projectile) */
   UFUNCTION(BlueprintCallable, Category="Weapon Functions")
   void EventAttack();
    
};
