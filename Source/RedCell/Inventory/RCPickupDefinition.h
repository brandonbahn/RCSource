// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RCPickupDefinition.generated.h"

class URCInventoryItemDefinition;
class UNiagaraSystem;
class UObject;
class USoundBase;
class UStaticMesh;

/**
 * Data asset used to configure pickups
 */
UCLASS(Blueprintable, BlueprintType, Const, Meta = (DisplayName = "RedCell Pickup Data", ShortTooltip = "Data asset used to configure pickups."))
class REDCELL_API URCPickupDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:

    //Defines the pickup's actors to spawn, abilities to grant, and tags to add
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup|Equipment")
    TSubclassOf<URCInventoryItemDefinition> InventoryItemDefinition;

    //Visual representation of the pickup as static mesh
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup|Mesh")
    TObjectPtr<UStaticMesh> DisplayMesh;

    //Cool down time between pickups in seconds
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup")
    int32 SpawnCoolDownSeconds;

    //Sound to play when picked up
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup")
    TObjectPtr<USoundBase> PickedUpSound;

    //Sound to play when pickup is respawned
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup")
    TObjectPtr<USoundBase> RespawnedSound;

    //Particle FX to play when picked up
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup")
    TObjectPtr<UNiagaraSystem> PickedUpEffect;

    //Particle FX to play when pickup is respawned
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup")
    TObjectPtr<UNiagaraSystem> RespawnedEffect;
};


UCLASS(Blueprintable, BlueprintType, Const, Meta = (DisplayName = "RedCell Weapon Pickup Data", ShortTooltip = "Data asset used to configure weapon pickups."))
class REDCELL_API URCWeaponPickupDefinition : public URCPickupDefinition
{
    GENERATED_BODY()

public:

    //Sets the height of the display mesh above the Weapon spawner
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup|Mesh")
    FVector WeaponMeshOffset;

    //Sets the height of the display mesh above the Weapon spawner
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Pickup|Mesh")
    FVector WeaponMeshScale = FVector(1.0f, 1.0f, 1.0f);
};
