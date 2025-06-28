// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RCAbilityCost.h"
#include "ScalableFloat.h"

#include "RCAbilityCost_PlayerTagStack.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;

class URCGameplayAbility;
class UObject;
struct FGameplayAbilityActorInfo;

/**
* Represents a cost that requires expending a quantity of a tag stack on the player state
*/
UCLASS(meta = (DisplayName = "Player Tag Stack"))
class URCAbilityCost_PlayerTagStack : public URCAbilityCost
{
	GENERATED_BODY()
	
public:
	URCAbilityCost_PlayerTagStack();

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
};

