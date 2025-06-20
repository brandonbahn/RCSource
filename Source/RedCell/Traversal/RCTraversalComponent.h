// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChooserFunctionLibrary.h"
#include "TraversableObstacleComponent.h"
#include "Character/RCCharacterMovementComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "RCTraversalComponent.generated.h"

#define TRAVERSABLE_TRACE_CHANNEL ECC_GameTraceChannel1
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPerformTraversalAction);

class UMotionWarpingComponent;
class ARCCharacter;

/**
 * Traversal component for traverse gameplay ability.
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class REDCELL_API URCTraversalComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	URCTraversalComponent(const FObjectInitializer& ObjInit);

	// Standard GFC pattern like your other components
	UFUNCTION(BlueprintPure, Category="RedCell|Traversal")
	static URCTraversalComponent* FindTraversalComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<URCTraversalComponent>() : nullptr;
	}

	// Main traversal function
	void TryTraversalAction(FTraversalCheckInputs TraversalCheckInputs, EDrawDebugTrace::Type DebugType, 
						   bool& bOutTraversalCheckFailed, bool& bOutMontageSelectionFailed);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void UpdateWarpTargets();

	bool IsDoingTraversalAction() const { return bDoingTraversalAction; }

	UPROPERTY(BlueprintAssignable, Category = "Traversal")
	FPerformTraversalAction PerformTraversalAction;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FTraversalCheckResult TraversalResult;

	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	bool bDoingTraversalAction = false;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Stored montages for Motion Matching
	UPROPERTY()
	TArray<UObject*> ValidMontages;

private:
	UPROPERTY()
	TObjectPtr<ARCCharacter> OwnerCharacter;
    
	UPROPERTY()
	TObjectPtr<URCCharacterMovementComponent> MovementComponent;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// Chooser data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UChooserTable> TraversalAnimationsChooserTable;
};
