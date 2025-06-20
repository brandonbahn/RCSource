// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/RCCameraProperties.h"
#include "Core/CameraEvaluationContext.h"
#include "Engine/DataAsset.h"
#include "RCPawnData.generated.h"

class APawn;
class URCAbilitySet;
class URCAbilityTagRelationshipMapping;
class UCameraRigAsset;
class URCInputConfig;
class UObject;


/**
 * URCPawnData
 *
 * Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "RedCell Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class REDCELL_API URCPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
    
public:

    URCPawnData(const FObjectInitializer& ObjectInitializer);

public:

    // Class to instantiate for this pawn (should usually derive from ARCPawn (not yet created) or ARCCharacter).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pawn")
    TSubclassOf<APawn> PawnClass;

    // Ability sets to grant to this pawn's ability system.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Abilities")
    TArray<TObjectPtr<URCAbilitySet>> AbilitySets;
    
    // What mapping of ability tags to use for actions taking by this pawn
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Abilities")
    TObjectPtr<URCAbilityTagRelationshipMapping> TagRelationshipMapping;
    
    // Input configuration used by player controlled pawns to create input mappings and bind input actions.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Input")
    TObjectPtr<URCInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCamera")
    TObjectPtr<UCameraAsset> CameraAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCamera")
    TObjectPtr<UCameraRigAsset> PersistentGlobalCameraRig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCamera") 
    TObjectPtr<UCameraRigAsset> PersistentBaseCameraRig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCamera")
    ECameraStyle DefaultCameraStyle = ECameraStyle::Balanced;

    /** (Optional) Portrait, DisplayName, etc. for UI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
    FText DisplayName;
    
};
