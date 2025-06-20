// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/Interface.h"
#include "Engine/EngineTypes.h"
#include "Character/RCMovementModes.h"
#include "TraversableObstacleComponent.generated.h"

class USplineComponent;

UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	None = 0,
	// Traverse over a thin object and end on the ground at a similar level (Low fence)
	Hurdle = 1,
	// Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side)
	Vault = 2,
	// Traverse up and onto an object without passing over it
	Mantle = 3
};

USTRUCT(BlueprintType)
struct FTraversalCheckInputs
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal") 
		FVector TraceForwardDirection = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float TraceForwardDistance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal") 
		FVector TraceOriginOffset = FVector::ZeroVector;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal") 
		FVector TraceEndOffset = FVector::ZeroVector;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float TraceRadius = 0.0f;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float TraceHalfHeight = 0.0f;
};	

USTRUCT(BlueprintType)
struct FTraversalCheckResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		ETraversalActionType ActionType = ETraversalActionType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasFrontLedge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasFrontLedge", EditConditionHides))
		FVector FrontLedgeLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasFrontLedge", EditConditionHides))
		FVector FrontLedgeNormal = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasBackLedge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasBackLedge", EditConditionHides))
		FVector BackLedgeLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasBackLedge", EditConditionHides))
		FVector BackLedgeNormal = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasBackLedge", EditConditionHides))
		float BackLedgeHeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasBackFloor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(EditCondition="bHasBackFloor", EditConditionHides))
		FVector BackFloorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float ObstacleHeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float ObstacleDepth = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		TObjectPtr<const UAnimMontage> ChosenMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float StartTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float PlayRate = 0.0f;

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(
			TEXT("Has Front Ledge: %hhd\nHas Back Ledge: %hhd\nHas Back Floor:%hhd\nObstacle Height: %f\nObstacle Depth: %f\nBack Ledge Height: %f\nChosen Animation: %s\nAnimation Start Time: %f\nAnimation Play Rate: %f"),
			bHasFrontLedge, bHasBackLedge, bHasBackFloor, ObstacleHeight, ObstacleDepth, BackLedgeHeight, IsValid(ChosenMontage) ? *ChosenMontage->GetName() : TEXT("nullptr"), StartTime, PlayRate);
	}
};

USTRUCT(BlueprintType)
struct FTraversalChooserInputs
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		ETraversalActionType ActionType = ETraversalActionType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasFrontLedge = false;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasBackLedge = false;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		bool bHasBackFloor = false;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float ObstacleHeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float ObstacleDepth = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float BackLedgeHeight = 0.0f;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		TEnumAsByte<EMovementMode> MovementMode = MOVE_None; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		E_Gait Gait = E_Gait::Walk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float Speed = 0.0f;		
};

USTRUCT(BlueprintType)
struct FTraversalChooserOutputs
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		ETraversalActionType ActionType = ETraversalActionType::None;
		
};


UINTERFACE(Blueprintable)
class UTraversableObstacleInterface : public UInterface
{
	GENERATED_BODY()
};

class REDCELL_API ITraversableObstacleInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal")
	void GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation, UPARAM(ref, DisplayName = "TraversalTraceResult") FTraversalCheckResult& TraversalTraceResultOut);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REDCELL_API UTraversableObstacleComponent : public UActorComponent, public ITraversableObstacleInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
		TArray<USplineComponent*> Ledges = {};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
		TMap<USplineComponent*, USplineComponent*> OppositeLedges = {};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
		float MinLedgeWidth = 60.0f;

	UFUNCTION()
	USplineComponent* FindLedgeClosestToActor(const FVector& ActorLocation) const;

public:
	virtual void GetLedgeTransforms_Implementation(const FVector& HitLocation, const FVector& ActorLocation, FTraversalCheckResult& TraversalTraceResultOut) override;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void Initialize(UPARAM(DisplayName = "Ledges") const TArray<USplineComponent*>& NewLedges, UPARAM(DisplayName = "OppositeLedges") const TMap<USplineComponent*, USplineComponent*>& NewOppositeLedges);

	FORCEINLINE const TArray<USplineComponent*>& GetLedges() const { return Ledges; }
	FORCEINLINE const TMap<USplineComponent*, USplineComponent*>& GetOppositeLedges() const { return OppositeLedges; }
	FORCEINLINE float GetMinLedgeWidth() const { return MinLedgeWidth; }
};

