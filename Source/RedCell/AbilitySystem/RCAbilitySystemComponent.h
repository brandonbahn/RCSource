// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/RCGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "RCAbilitySet.h"
#include "RCAbilitySystemComponent.generated.h"

class AActor;
class UGameplayAbility;
class URCAbilityTagRelationshipMapping;
class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;
/**
 * Custom ASC for RedCell that can receive AbilitySets and log incoming events.
 */
UCLASS( ClassGroup=(Abilities)
       , Blueprintable
       , meta=(BlueprintSpawnableComponent) )
class REDCELL_API URCAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    /** Grant all Abilities and init-Effects in this set */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    void AddAbilitySet(URCAbilitySet* AbilitySet);
    
    void AbilityInputTagPressed(const FGameplayTag& InputTag);
    void AbilityInputTagReleased(const FGameplayTag& InputTag);

    /**
     * Override to log every gameplay event the ASC receives,
     * so you can see when Death/Reset tags are handled.
     */
    virtual int32 HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) override;
    
    /** Instantly override any attribute to a new value (e.g. refill Health to MaxHealth) */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    void OverrideAttribute(FGameplayAttribute Attribute, float NewValue)
    {
      ApplyModToAttribute(Attribute, EGameplayModOp::Override, NewValue);
    }

    
    /** Sets the current tag relationship mapping, if null it will clear it out */
    void SetTagRelationshipMapping(URCAbilityTagRelationshipMapping* NewMapping);

    /** Returns true if any currently-active ability has the given tag */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsAbilityActiveByTag(FGameplayTag AbilityTag) const;
    
    /** Returns true if any currently-granted ability has the given tag */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsAbilityGrantedByTag(FGameplayTag AbilityTag) const;

protected:

    // If set, this table is used to look up tag relationships for activate and cancel
    UPROPERTY()
    TObjectPtr<URCAbilityTagRelationshipMapping> TagRelationshipMapping;
    
    // Handles to abilities that had their input pressed this frame.
    TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

    // Handles to abilities that had their input released this frame.
    TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

    // Handles to abilities that have their input held.
    TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

};
