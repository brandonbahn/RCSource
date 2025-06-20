// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTrajectoryLibraryExtension.h"

#include "Engine/EngineTypes.h"
#include "PoseSearch/PoseSearchLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCTrajectoryLibraryExtension)

void URCTrajectoryLibraryExtension::HandleTrajectoryWorldCollisionsThreadSafe(const UObject* WorldContextObject, const UAnimInstance* AnimInstance, const FPoseSearchQueryTrajectory& InTrajectory, bool bApplyGravity, float FloorCollisionsOffset, FPoseSearchQueryTrajectory& OutTrajectory, FPoseSearchTrajectory_WorldCollisionResults& CollisionResult, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, bool bIgnoreSelf, float MaxObstacleHeight, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions(WorldContextObject, AnimInstance, InTrajectory,
		bApplyGravity, FloorCollisionsOffset, OutTrajectory, CollisionResult, TraceChannel, bTraceComplex,
		ActorsToIgnore, DrawDebugType, bIgnoreSelf, MaxObstacleHeight, TraceColor, TraceHitColor, DrawTime);
}
