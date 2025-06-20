// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Traversal/TraversableObstacleComponent.h"
#include "RCGameplayAbility_Traverse.generated.h"

class URCTraversalComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;
struct FTraversalCheckResult;

/**
 * Gameplay ability used for character traversal.
 */
UCLASS(Abstract)
class REDCELL_API URCGameplayAbility_Traverse : public URCGameplayAbility
{
	GENERATED_BODY()

public:
	URCGameplayAbility_Traverse(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
								   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
								const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
						   const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Blueprint callable function for trying traversal with debug options
	UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
	bool TryTraversalAction(EDrawDebugTrace::Type DebugType = EDrawDebugTrace::ForOneFrame);

	UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
	void OnMontageCompletedOrInterrupted(UPrimitiveComponent* HitComponent, UCharacterMovementComponent* MovementComp, ETraversalActionType ActionType);

private:
	FTimerHandle TraversalCheckTimer;
};
