// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RCTrajectoryLibraryExtension.generated.h"

/**
 * Pose Search Library Trajectory Extension
 */
UCLASS(meta = (BlueprintThreadSafe))
class REDCELL_API URCTrajectoryLibraryExtension : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Experimental: Thread-safe variation of UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions
	UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch|Experimental", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "TraceChannel,bTraceComplex,ActorsToIgnore,DrawDebugType,bIgnoreSelf,MaxObstacleHeight,TraceColor,TraceHitColor,DrawTime"))
	static void HandleTrajectoryWorldCollisionsThreadSafe(const UObject* WorldContextObject,
	const UAnimInstance* AnimInstance, const FPoseSearchQueryTrajectory& InTrajectory, bool bApplyGravity,
	float FloorCollisionsOffset, FPoseSearchQueryTrajectory& OutTrajectory,
	FPoseSearchTrajectory_WorldCollisionResults& CollisionResult, ETraceTypeQuery TraceChannel, bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, bool bIgnoreSelf,
	float MaxObstacleHeight, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);
	
	
};
