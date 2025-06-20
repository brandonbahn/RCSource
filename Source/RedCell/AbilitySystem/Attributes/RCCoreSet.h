// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RCAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "RCCoreSet.generated.h"

class UObject;
struct FFrame;

REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Mana_Empty);
REDCELL_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Mana_Restored);

// Forward declaration of the callback data used in the execute hooks
struct FGameplayEffectModCallbackData;

/**
 *  Defines core player related attributes and execution logic.
 */
UCLASS()
class REDCELL_API URCCoreSet : public URCAttributeSet
{
	GENERATED_BODY()
	
public:
    URCCoreSet();

    ATTRIBUTE_ACCESSORS(URCCoreSet, Mana);
    ATTRIBUTE_ACCESSORS(URCCoreSet, MaxMana);
    ATTRIBUTE_ACCESSORS(URCCoreSet, RegenMana);

    /** Current mana */
    // Delegate when health changes due to damage/healing, some information may be missing on the client
    mutable FRCAttributeEvent OnManaChanged;

    // Delegate when max health changes
    mutable FRCAttributeEvent OnMaxManaChanged;

    // Delegate to broadcast when the health attribute reaches zero
    mutable FRCAttributeEvent OnOutOfMana;
    
    /** Replicate core attributes and notify clients */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
protected:
    
    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

    /** Hook before a damage/heal effect executes */
    virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
    
    /** Hook after gameplay effect executes */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    
    /** Clamp base (unmodified) attribute changes */
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

    /** Clamp new values for Mana/MaxMana */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    /** Handle final attribute clamping and “out of health” checks */
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    /** Utility to clamp Mana vs MaxMana */
    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
    // The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "RedCell|Core", Meta = (HideFromModifiers, AllowPrivateAccess = true))
    FGameplayAttributeData Mana;

    // The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "RedCell|Core", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData MaxMana;

    
    /** Track whether we’ve already broadcasted “out of mana” */
    bool bOutOfMana = false;

    /** Temp storage of old values for messaging */
    float ManaBeforeAttributeChange = 0.f;
    float MaxManaBeforeAttributeChange = 0.f;

    // Incoming healing. This is mapped directly to +Health
    UPROPERTY(BlueprintReadOnly, Category="RedCell|Core", Meta=(AllowPrivateAccess=true))
    FGameplayAttributeData RegenMana;
};