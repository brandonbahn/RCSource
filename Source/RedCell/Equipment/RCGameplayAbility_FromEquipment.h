// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"

#include "RCGameplayAbility_FromEquipment.generated.h"

/**
 * RCGameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 */
UCLASS()
class REDCELL_API URCGameplayAbility_FromEquipment : public URCGameplayAbility
{
    GENERATED_BODY()
    
public:
    
    URCGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category="RedCell|Ability")
    URCEquipmentInstance* GetAssociatedEquipment() const;
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    URCInventoryItemInstance* GetAssociatedItem() const;
    
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
    
};
