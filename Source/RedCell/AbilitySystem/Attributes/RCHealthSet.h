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

REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RedCell_Damage_Message);

// Forward declaration of the callback data used in the execute hooks
struct FGameplayEffectModCallbackData;

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

    ATTRIBUTE_ACCESSORS(URCHealthSet, Health);
    ATTRIBUTE_ACCESSORS(URCHealthSet, MaxHealth);
    ATTRIBUTE_ACCESSORS(URCHealthSet, Healing);
    ATTRIBUTE_ACCESSORS(URCHealthSet, Damage);

    // Delegate when health changes due to damage/healing, some information may be missing on the client
    mutable FRCAttributeEvent OnHealthChanged;

    // Delegate when max health changes
    mutable FRCAttributeEvent OnMaxHealthChanged;

    // Delegate to broadcast when the health attribute reaches zero
    mutable FRCAttributeEvent OnOutOfHealth;

    /** Replicate health & max‐health and notify clients */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

    /** Hook before a damage/heal effect executes */
    virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

    /** Hook after a damage/heal effect executes */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    /** Clamp base (unmodified) attribute changes */
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

    /** Clamp new values for Health/MaxHealth */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    /** Handle final attribute clamping and “out of health” checks */
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    /** Utility to clamp Health vs MaxHealth */
    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

    // The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "RedCell|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
    FGameplayAttributeData Health;

    // The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "RedCell|Health", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData MaxHealth;

    /** Track whether we’ve already broadcasted “out of health” */
    bool bOutOfHealth = false;

    /** Temp storage of old values for messaging */
    float HealthBeforeAttributeChange = 0.f;
    float MaxHealthBeforeAttributeChange = 0.f;

    // Incoming healing. This is mapped directly to +Health
    UPROPERTY(BlueprintReadOnly, Category="RedCell|Health", Meta=(AllowPrivateAccess=true))
    FGameplayAttributeData Healing;

    // Incoming damage. This is mapped directly to -Health
    UPROPERTY(BlueprintReadOnly, Category="RedCell|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
    FGameplayAttributeData Damage;
};
