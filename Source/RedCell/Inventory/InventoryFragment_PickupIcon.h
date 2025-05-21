// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RCInventoryItemDefinition.h"
#include "UObject/ObjectPtr.h"
#include "InventoryFragment_PickupIcon.generated.h"

class UObject;
class USkeletalMesh;


UCLASS()
class UInventoryFragment_PickupIcon : public URCInventoryItemFragment
{
	GENERATED_BODY()
	
public:
    UInventoryFragment_PickupIcon();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    TObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FLinearColor PadColor;
};
