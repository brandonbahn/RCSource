// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RCInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "InventoryFragment_EquippableItem.generated.h"

class URCEquipmentDefinition;
class UObject;

UCLASS()
class UInventoryFragment_EquippableItem : public URCInventoryItemFragment
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category=RC)
    TSubclassOf<URCEquipmentDefinition> EquipmentDefinition;
};

