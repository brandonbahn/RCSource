// Fill out your copyright notice in the Description page of Project Settings.
// RCHealthSet.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"        // for FGameplayEffectModCallbackData, etc.
#include "RCAttributeSet.h"               // your base AttributeSet
#include "NativeGameplayTags.h"           // for the UE_DECLARE_GAMEPLAY_TAG macros
#include "RCHealthSet.generated.h"

class UObject;
struct FFrame;

// Forward declaration of the callback data used in the execute hooks
struct FGameplayEffectModCallbackData;

//――――――――――――――――――――――――――――――――――――――――――――――――
// Native GameplayTag declarations for damage logic
//――――――――――――――――――――――――――――――――――――――――――――――――
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RedCell_Damage_Message);

/**
 * URCHealthSet
 *
 *  Defines health‐related attributes and damage/heal execution logic.
 */
UCLASS(BlueprintType)
class REDCELL_API URCHealthSet : public URCAttributeSet
{
    GENERATED_BODY()

public:
    URCHealthSet();

    /** Current health */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes|Health")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(URCHealthSet, Health)
    
    /** Maximum health */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Attributes|Health")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(URCHealthSet, MaxHealth)
    
    /** Incoming healing (adds to Health) */
    UPROPERTY(BlueprintReadOnly, Category="Attributes|Health")
    FGameplayAttributeData Healing;
    ATTRIBUTE_ACCESSORS(URCHealthSet, Healing)
    
    /** Incoming damage (subtracts from Health) */
    UPROPERTY(BlueprintReadOnly, Category="Attributes|Health", meta=(HideFromModifiers))
    FGameplayAttributeData Damage;
    ATTRIBUTE_ACCESSORS(URCHealthSet, Damage)

    /** Replicate health & max‐health and notify clients */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Clamp new values for Health/MaxHealth */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    /** Hook before a damage/heal effect executes */
    virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

    /** Hook after a damage/heal effect executes */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    /** Clamp base (unmodified) attribute changes */
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

    /** Handle final attribute clamping and “out of health” checks */
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

private:
    /** Utility to clamp Health vs MaxHealth */
    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

    /** Track whether we’ve already broadcasted “out of health” */
    bool bOutOfHealth = false;

    /** Temp storage of old values for messaging */
    float HealthBeforeAttributeChange = 0.f;
    float MaxHealthBeforeAttributeChange = 0.f;
};
