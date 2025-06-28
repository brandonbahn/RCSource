// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RCAbilityCost.h"
#include "ScalableFloat.h"
#include "RCAbilityCost_ItemTagStack.generated.h"

/**
*  Represents a cost that requires expending a quantity of a tag stack
 * on the associated item instance
 */
UCLASS(meta = (DisplayName = "Item Tag Stack"))
class URCAbilityCost_ItemTagStack : public URCAbilityCost
{
	GENERATED_BODY()
	
public:
	URCAbilityCost_ItemTagStack();

	//~URCAbilityCost interface
	virtual bool CheckCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of URCAbilityCost interface

protected:
	/** How much of the tag to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** Which tag to spend some of */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;

	/** Which tag to send back as a response if this cost cannot be applied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag FailureTag;
};
