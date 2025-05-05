// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySet.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

#include "GameplayAbilitySpecHandle.h"
#include "RCAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;
class URCAbilitySystemComponent;
class URCGameplayAbility;
class UObject;

/**
 * FRCAbilitySet_GameplayAbility
 *
 *    Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FRCAbilitySet_GameplayAbility
{
    GENERATED_BODY()

public:

    // Gameplay ability to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<URCGameplayAbility> Ability = nullptr;

    // Level of ability to grant.
    UPROPERTY(EditDefaultsOnly)
    int32 AbilityLevel = 1;

    // Tag used to process input for the ability.
    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};


/**
 * FRCAbilitySet_GameplayEffect
 *
 *    Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FRCAbilitySet_GameplayEffect
{
    GENERATED_BODY()

public:

    // Gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

    // Level of gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    float EffectLevel = 1.0f;
};

/**
 * FRCAbilitySet_AttributeSet
 *
 *    Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FRCAbilitySet_AttributeSet
{
    GENERATED_BODY()

public:
    // Gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UAttributeSet> AttributeSet;

};

/**
 * FRCAbilitySet_GrantedHandles
 *
 *    Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FRCAbilitySet_GrantedHandles
{
    GENERATED_BODY()

public:

    void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
    void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
    void AddAttributeSet(UAttributeSet* Set);

    void TakeFromAbilitySystem(URCAbilitySystemComponent* RCASC);

protected:

    // Handles to the granted abilities.
    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

    // Handles to the granted gameplay effects.
    UPROPERTY()
    TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

    // Pointers to the granted attribute sets
    UPROPERTY()
    TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * URCAbilitySet
 *
 *    Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */
UCLASS(BlueprintType, Const)
class URCAbilitySet : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    URCAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Grants the ability set to the specified ability system component.
    // The returned handles can be used later to take away anything that was granted.
    void GiveToAbilitySystem(URCAbilitySystemComponent* RCASC, FRCAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:

    // Gameplay abilities to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
    TArray<FRCAbilitySet_GameplayAbility> GrantedGameplayAbilities;

    // Gameplay effects to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
    TArray<FRCAbilitySet_GameplayEffect> GrantedGameplayEffects;

    // Attribute sets to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
    TArray<FRCAbilitySet_AttributeSet> GrantedAttributes;
};
