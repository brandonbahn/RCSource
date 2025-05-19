// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RCCoreSet.generated.h"

class UObject;
struct FFrame;

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

    /** Current mana */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="Attributes|Core")
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(URCCoreSet, Mana)
    
    /** Maximum mana */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="Attributes|Core")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(URCCoreSet, MaxMana)
    
    /** Replicate core attributes and notify clients */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
protected:
    
    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
    
    /** Clamp new values for core attributes */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    
    /** Hook after gameplay effect executes */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    
    /** Clamp base (unmodified) attribute changes */
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    
    
private:
    /** Utility to clamp Mana vs MaxMana */
    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

    /** Track whether we’ve already broadcasted “out of mana” */
    bool bOutOfMana = false;

    /** Temp storage of old values for messaging */
    float ManaBeforeAttributeChange = 0.f;
    float MaxManaBeforeAttributeChange = 0.f;
};
