// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RCAbilityCost.h"
#include "ScalableFloat.h"
#include "Templates/SubclassOf.h"

#include "RCAbilityCost_InventoryItem.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;

class URCGameplayAbility;
class URCInventoryItemDefinition;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * Represents a cost that requires expending a quantity of an inventory item
 */
UCLASS(meta = (DisplayName = "Inventory Item"))
class URCAbilityCost_InventoryItem : public URCAbilityCost
{
	GENERATED_BODY()
	
public:
	URCAbilityCost_InventoryItem();

	//~URCAbilityCost interface
	virtual bool CheckCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of URCAbilityCost interface

protected:
	/** How much of the item to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	FScalableFloat Quantity;

	/** Which item to consume */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	TSubclassOf<URCInventoryItemDefinition> ItemDefinition;
};

