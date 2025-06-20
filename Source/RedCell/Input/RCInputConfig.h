// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/RCMovementModes.h"
#include "GameplayTagContainer.h"

#include "RCInputConfig.generated.h"

class UInputAction;
class UObject;
struct FFrame;

/**
 * FRCInputAction
 *
 *    Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FRCInputAction
{
    GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UInputAction> InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};

/**
 * URCInputConfig
 *
 *    Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class URCInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:

    URCInputConfig(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "RedCell|Pawn")
    const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Pawn")
    const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

    // Helper function to get movement input scale based on mode
    UFUNCTION(BlueprintCallable, Category = "RedCell|Pawn")
    FVector2D GetMovementInputScaleValue(const FVector2D& Input) const;

public:
    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FRCInputAction> NativeInputActions;

    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FRCInputAction> AbilityInputActions;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Analog Stick Behavior", Meta = (DisplayName = "Movement Mode"))
    EMovementStickMode MovementStickMode = EMovementStickMode::FixedSpeedSingleGait;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Analog Stick Behavior", Meta = (DisplayName = "Walk / Run Threshold")) 
    float AnalogWalkRunThreshold = 0.7f;
};
