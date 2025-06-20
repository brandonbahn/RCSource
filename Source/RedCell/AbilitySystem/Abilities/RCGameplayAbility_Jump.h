// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "RCGameplayAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;

/**
 * URCGameplayAbility_Jump
 *
 * Gameplay ability used for character jumping.
 * Also handles continuous traversal checking while jump key is held.
 */
UCLASS(Abstract)
class REDCELL_API URCGameplayAbility_Jump : public URCGameplayAbility
{
	GENERATED_BODY()

public:
	URCGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintReadWrite, Category = "Jump")
	bool bJumpInitiated = false;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
								   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
						   const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
	void CharacterJumpStop();
};
