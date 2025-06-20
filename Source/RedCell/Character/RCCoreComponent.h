// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayEffectExtension.h"
#include "RCCoreComponent.generated.h"

class URCAbilitySystemComponent;
class URCCoreSet;
struct FOnAttributeChangeData;

/** Delegate fired when an attribute changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FRCCore_AttributeChanged,
    URCCoreComponent*, CoreComponent,
    float, OldValue,
    float, NewValue,
    AActor*, Instigator
);

/**
 * Core player attributes component
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class REDCELL_API URCCoreComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()
	
public:
    URCCoreComponent(const FObjectInitializer& ObjInit);

    /** Find a core component on an actor */
    UFUNCTION(BlueprintPure, Category="RedCell|Core")
    static URCCoreComponent* FindCoreComponent(const AActor* Actor)
    {
        return Actor
            ? Actor->FindComponentByClass<URCCoreComponent>()
            : nullptr;
    }

    /** Hook up to an ASC (usually from PlayerState) */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    void InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC);

    /** Unhook delegates and clear references */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    void UninitializeFromAbilitySystem();

    /** Current mana */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    float GetMana() const;

    /** Maximum mana */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    float GetMaxMana() const;

    /** Normalized [0–1] mana */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    float GetManaNormalized() const;

    /** Fired when mana changes */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Core")
    FRCCore_AttributeChanged OnManaChanged;

    /** Fired when max mana changes */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Core")
    FRCCore_AttributeChanged OnMaxManaChanged;

protected:
    virtual void OnUnregister() override;

    void ClearGameplayTags();

    void HandleManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
    void HandleMaxManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
    void HandleOutOfMana(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
    
private:
    UPROPERTY()
    URCAbilitySystemComponent* AbilitySystemComponent;

    // Core set used by this component.
    UPROPERTY()
    TObjectPtr<const URCCoreSet> CoreSet;
    
};