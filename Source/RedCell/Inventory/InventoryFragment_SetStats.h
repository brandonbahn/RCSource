// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RCInventoryItemDefinition.h"

#include "InventoryFragment_SetStats.generated.h"

class URCInventoryItemInstance;
class UObject;
struct FGameplayTag;

UCLASS()
class UInventoryFragment_SetStats : public URCInventoryItemFragment
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TMap<FGameplayTag, int32> InitialItemStats;

public:
    virtual void OnInstanceCreated(URCInventoryItemInstance* Instance) const override;

    int32 GetItemStatByTag(FGameplayTag Tag) const;
};

