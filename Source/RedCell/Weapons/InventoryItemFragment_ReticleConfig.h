// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RCInventoryItemDefinition.h"
#include "InventoryItemFragment_ReticleConfig.generated.h"

class URCReticleWidgetBase;
class UObject;

UCLASS()
class UInventoryFragment_ReticleConfig : public URCInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
	TArray<TSubclassOf<URCReticleWidgetBase>> ReticleWidgets;
};
