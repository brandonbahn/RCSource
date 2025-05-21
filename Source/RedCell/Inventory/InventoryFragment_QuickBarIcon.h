// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RCInventoryItemDefinition.h"
#include "Styling/SlateBrush.h"

#include "InventoryFragment_QuickBarIcon.generated.h"

class UObject;

UCLASS()
class UInventoryFragment_QuickBarIcon : public URCInventoryItemFragment
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FSlateBrush Brush;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FSlateBrush AmmoBrush;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FText DisplayNameWhenEquipped;
};
