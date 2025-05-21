// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RCEquipmentDefinition.generated.h"

class AActor;
class URCAbilitySet;
class URCEquipmentInstance;

USTRUCT()
struct FRCEquipmentActorToSpawn
{
    GENERATED_BODY()

    FRCEquipmentActorToSpawn()
    {}

    UPROPERTY(EditAnywhere, Category=Equipment)
    TSubclassOf<AActor> ActorToSpawn;

    UPROPERTY(EditAnywhere, Category=Equipment)
    FName AttachSocket;

    UPROPERTY(EditAnywhere, Category=Equipment)
    FTransform AttachTransform;
};


/**
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, BlueprintType)
class URCEquipmentDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    URCEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Class to spawn
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TSubclassOf<URCEquipmentInstance> InstanceType;

    // Gameplay ability sets to grant when this is equipped
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TArray<TObjectPtr<const URCAbilitySet>> AbilitySetsToGrant;

    // Actors to spawn on the pawn when this is equipped
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TArray<FRCEquipmentActorToSpawn> ActorsToSpawn;
};
