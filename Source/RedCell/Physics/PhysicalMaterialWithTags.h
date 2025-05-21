// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "PhysicalMaterialWithTags.generated.h"

/**
 * RCWeaponInstance
 *
 * A piece of equipment representing a weapon spawned and applied to a pawn
 */
UCLASS()
class REDCELL_API UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()
	
public:
    UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // A container of gameplay tags that game code can use to reason about this physical material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
    FGameplayTagContainer Tags;
};

