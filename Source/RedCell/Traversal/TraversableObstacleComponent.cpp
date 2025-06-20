// Fill out your copyright notice in the Description page of Project Settings.


#include "Traversal/TraversableObstacleComponent.h"

#include "Components/SplineComponent.h"

void UTraversableObstacleComponent::Initialize(const TArray<USplineComponent*>& NewLedges, const TMap<USplineComponent*, USplineComponent*>& NewOppositeLedges)
{
	Ledges = NewLedges;
	OppositeLedges = NewOppositeLedges;
}

USplineComponent* UTraversableObstacleComponent::FindLedgeClosestToActor(const FVector& ActorLocation) const
{
	if (Ledges.IsEmpty())
	{
		return nullptr;
	}

	// Iterate through all the closest points on all available ledges to find the closest one.
	// An offset is added away from the ledge using its normal to prevent corners from having the same location.
	// If not, traversing against a corner would be inconsistent.
	int32 ClosestIndex = INDEX_NONE;
	float ClosestDistance = MAX_flt;
	for (int32 Index = 0; Index < Ledges.Num(); Index++)
	{
		if (IsValid(Ledges[Index]))
		{
			const FVector ClosestPoint =
				Ledges[Index]->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
			const FVector ClosestUp =
				Ledges[Index]->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) * 10.0f;
			const float Distance = FVector::Distance(ClosestPoint + ClosestUp, ActorLocation);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestIndex = Index;
			}
		}
	}
	return Ledges.IsValidIndex(ClosestIndex) ? Ledges[ClosestIndex] : nullptr;
}

void UTraversableObstacleComponent::GetLedgeTransforms_Implementation(const FVector& HitLocation, const FVector& ActorLocation, FTraversalCheckResult& TraversalTraceResultOut)
{
	// Find the ledge closest to the actors' location.
	// Also make sure the ledge is wide enough. If not, the front ledge will not be valid.
	const USplineComponent* ClosestLedge = FindLedgeClosestToActor(ActorLocation);
	if (!IsValid(ClosestLedge) ||
		ClosestLedge->GetSplineLength() < MinLedgeWidth)
	{
		TraversalTraceResultOut.bHasFrontLedge = false;
		return;
	}

	// Get the closest point on the ledge to the actor,
	// and clamp the location so that it can't be too close to the end of the ledge.
	// This prevents the character from floating if they traverse near a corner.
	// The clamped location will always be half the "min ledge width" from the edge.
	// If the min ledge width is 60 units, the ledge location will always be at least 30 units away from a corner.
	const float HalfMinLedgeWidth = MinLedgeWidth * 0.5f;
	const FVector ClosestHitLocation =
		ClosestLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
	const float ClosestHitDistance =
		FMath::Clamp(ClosestLedge->GetDistanceAlongSplineAtLocation(ClosestHitLocation, ESplineCoordinateSpace::Local),
			HalfMinLedgeWidth, ClosestLedge->GetSplineLength() - HalfMinLedgeWidth);
	const FTransform FrontLedgeTransform =
		ClosestLedge->GetTransformAtDistanceAlongSpline(ClosestHitDistance, ESplineCoordinateSpace::World);
	TraversalTraceResultOut.FrontLedgeLocation = FrontLedgeTransform.GetLocation();
	TraversalTraceResultOut.FrontLedgeNormal = FrontLedgeTransform.GetRotation().GetUpVector();
	TraversalTraceResultOut.bHasFrontLedge = true;

	// Use a map to find the opposite ledge of the closest ledge.
	if (!OppositeLedges.Contains(ClosestLedge))
	{
		TraversalTraceResultOut.bHasBackLedge = false;
	}

	// Get the closest point on the back ledge from the front ledges' location.
	const USplineComponent* OppositeLedge = *OppositeLedges.Find(ClosestLedge);
	const FTransform BackLedgeTransform =
		OppositeLedge->FindTransformClosestToWorldLocation(TraversalTraceResultOut.FrontLedgeLocation,
			ESplineCoordinateSpace::World);
	TraversalTraceResultOut.BackLedgeLocation = BackLedgeTransform.GetLocation();
	TraversalTraceResultOut.BackLedgeNormal = BackLedgeTransform.GetRotation().GetUpVector();
	TraversalTraceResultOut.bHasBackLedge = true;
}
