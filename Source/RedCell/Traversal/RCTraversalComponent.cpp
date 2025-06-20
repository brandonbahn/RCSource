// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTraversalComponent.h"

#include "AnimationWarpingLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "KismetAnimationLibrary.h"
#include "MotionWarpingComponent.h"
#include "RCGameplayAbility_Traverse.h"
#include "RCLogChannels.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Engine/EngineTypes.h"
#include "Character/RCCharacter.h"
#include "GameFramework/Pawn.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/RCMovementModes.h"
#include "Traversal/InteractionTransformInterface.h"
#include "Traversal/TraversableObstacleComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCTraversalComponent)

URCTraversalComponent::URCTraversalComponent(const FObjectInitializer& ObjInit)
	: Super(ObjInit)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void URCTraversalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(URCTraversalComponent, TraversalResult);
}

void URCTraversalComponent::OnRegister()
{
	Super::OnRegister();
    
	// Ensure we're on a character
	const ARCCharacter* Character = Cast<ARCCharacter>(GetOwner());
	ensureAlwaysMsgf((Character != nullptr), TEXT("RCTraversalComponent on [%s] can only be added to RCCharacter actors."), *GetNameSafe(GetOwner()));
}

void URCTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
    
	// Cache references
	OwnerCharacter = Cast<ARCCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MovementComponent = Cast<URCCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());
        MotionWarping = OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>();
	}
}

void URCTraversalComponent::TryTraversalAction(FTraversalCheckInputs TraversalCheckInputs, 
    EDrawDebugTrace::Type DebugType, bool& bOutTraversalCheckFailed, bool& bOutMontageSelectionFailed)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    // Step 1: Cache important values
    const double StartTime = FPlatformTime::Seconds();
    const FVector& ActorLocation = OwnerCharacter->GetActorLocation();
    const float& CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float& CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    
    static const FString DrawDebugLevelConsoleVariableName = TEXT("DDCvar.Traversal.DrawDebugLevel");
    const int32& DrawDebugLevel = UKismetSystemLibrary::GetConsoleVariableIntValue(DrawDebugLevelConsoleVariableName);
    static const FString DrawDebugDurationConsoleVariableName = TEXT("DDCvar.Traversal.DrawDebugDuration");
    const float& DrawDebugDuration = UKismetSystemLibrary::GetConsoleVariableFloatValue(DrawDebugDurationConsoleVariableName);
    
    FTraversalCheckResult TraversalCheckResult;

    // Step 2.1: Perform forward trace to find traversable obstacle
    FHitResult Hit;
    FVector TraceStart = ActorLocation + TraversalCheckInputs.TraceOriginOffset;
    FVector TraceEnd = ActorLocation + TraversalCheckInputs.TraceOriginOffset +
                       (TraversalCheckInputs.TraceForwardDirection * TraversalCheckInputs.TraceForwardDistance) +
                       TraversalCheckInputs.TraceEndOffset;

    UKismetSystemLibrary::CapsuleTraceSingle(OwnerCharacter, TraceStart, TraceEnd,
        TraversalCheckInputs.TraceRadius, TraversalCheckInputs.TraceHalfHeight,
        UEngineTypes::ConvertToTraceType(TRAVERSABLE_TRACE_CHANNEL), false, {},
        DrawDebugLevel >= 2 ? DebugType : EDrawDebugTrace::None, Hit, true,
        FLinearColor::Black, FLinearColor::Black, DrawDebugDuration);

    ITraversableObstacleInterface* Obstacle = Cast<ITraversableObstacleInterface>(Hit.GetActor());
    if (!Hit.bBlockingHit || (Obstacle == nullptr && !Hit.GetActor()->Implements<UTraversableObstacleInterface>()))
    {
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    TraversalCheckResult.HitComponent = Hit.GetComponent();

    // Step 2.2: Get ledge transforms from the obstacle
    if (Obstacle != nullptr)
    {
        UE_LOG(LogRC, Warning, TEXT("Using Obstacle->Execute_ path"));
    	Obstacle->Execute_GetLedgeTransforms(Hit.GetActor(), Hit.ImpactPoint, ActorLocation, TraversalCheckResult);
    }
    else
    {
        UE_LOG(LogRC, Warning, TEXT("Using ITraversableObstacleInterface::Execute_ path"));
        ITraversableObstacleInterface::Execute_GetLedgeTransforms(Hit.GetActor(), Hit.ImpactPoint, ActorLocation, TraversalCheckResult);
    }

    UE_LOG(LogRC, Warning, TEXT("Hit Actor: %s, Cast Result: %s, Implements: %s"), 
    *Hit.GetActor()->GetName(),
    Obstacle ? TEXT("SUCCESS") : TEXT("FAILED"),
    Hit.GetActor()->Implements<UTraversableObstacleInterface>() ? TEXT("TRUE") : TEXT("FALSE"));
    
    // DEBUG: Draw debug shapes at ledge locations
    if (DrawDebugLevel >= 1)
    {
        if (TraversalCheckResult.bHasFrontLedge)
        {
            DrawDebugSphere(GetWorld(), TraversalCheckResult.FrontLedgeLocation, 10.0f, 12,
                FLinearColor::Green.ToFColor(true), false, DrawDebugDuration, SDPG_World, 1.0f);
        }
        if (TraversalCheckResult.bHasBackLedge)
        {
            DrawDebugSphere(GetWorld(), TraversalCheckResult.BackLedgeLocation, 10.0f, 12,
                FLinearColor::Blue.ToFColor(true), false, DrawDebugDuration, SDPG_World, 1.0f);
        }
    }

    // Step 3.1: Check if front ledge is valid
    if (!TraversalCheckResult.bHasFrontLedge)
    {
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    // Step 3.2: Check if there's room to move to front ledge
    const FVector HasRoomCheckFrontLedgeLocation = TraversalCheckResult.FrontLedgeLocation +
        TraversalCheckResult.FrontLedgeNormal * (CapsuleRadius + 2.0f) +
        FVector::ZAxisVector * (CapsuleHalfHeight + 2.0f);
    
    UKismetSystemLibrary::CapsuleTraceSingle(OwnerCharacter, ActorLocation,
        HasRoomCheckFrontLedgeLocation, CapsuleRadius, CapsuleHalfHeight,
        UEngineTypes::ConvertToTraceType(ECC_Visibility), false, {},
        DrawDebugLevel >= 3 ? DebugType : EDrawDebugTrace::None, Hit, true,
        FLinearColor::Red, FLinearColor::Green, DrawDebugDuration);

    if (Hit.bBlockingHit || Hit.bStartPenetrating)
    {
        TraversalCheckResult.bHasFrontLedge = false;
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    // Step 3.3: Calculate obstacle height
    TraversalCheckResult.ObstacleHeight = FMath::Abs((ActorLocation - FVector::ZAxisVector * CapsuleHalfHeight -
                                                      TraversalCheckResult.FrontLedgeLocation).Z);

    // Step 3.4: Check room across obstacle (front to back ledge)
    const FVector HasRoomCheckBackLedgeLocation = TraversalCheckResult.BackLedgeLocation +
                                                  TraversalCheckResult.BackLedgeNormal * (CapsuleRadius + 2.0f) +
                                                  FVector::ZAxisVector * (CapsuleHalfHeight + 2.0f);
    
    if (UKismetSystemLibrary::CapsuleTraceSingle(OwnerCharacter, HasRoomCheckFrontLedgeLocation,
        HasRoomCheckBackLedgeLocation, CapsuleRadius, CapsuleHalfHeight,
        UEngineTypes::ConvertToTraceType(ECC_Visibility), false, {},
        DrawDebugLevel >= 3 ? DebugType : EDrawDebugTrace::None, Hit, true,
        FLinearColor::Red, FLinearColor::Green, DrawDebugDuration))
    {
        // 3.5 No room - calculate partial depth and invalidate back ledge
        TraversalCheckResult.ObstacleDepth = (Hit.ImpactPoint - TraversalCheckResult.FrontLedgeLocation).Size2D();
        TraversalCheckResult.bHasBackLedge = false;
    }
    else
    {
        // 3.5 Room available - calculate full depth
        TraversalCheckResult.ObstacleDepth = (TraversalCheckResult.FrontLedgeLocation - TraversalCheckResult.BackLedgeLocation).Size2D();

        // Step 3.6: Check for back floor
        const FVector EndTraceLocation = TraversalCheckResult.BackLedgeLocation +
                                         TraversalCheckResult.BackLedgeNormal * (CapsuleRadius + 2.0f) -
                                         FVector::ZAxisVector * 50.0f;
        
        UKismetSystemLibrary::CapsuleTraceSingle(OwnerCharacter, HasRoomCheckBackLedgeLocation,
            EndTraceLocation, CapsuleRadius, CapsuleHalfHeight,
            UEngineTypes::ConvertToTraceType(ECC_Visibility), false, {},
            DrawDebugLevel >= 3 ? DebugType : EDrawDebugTrace::None, Hit, true,
            FLinearColor::Red, FLinearColor::Green, DrawDebugDuration);
        
        if (Hit.bBlockingHit)
        {
            TraversalCheckResult.bHasBackFloor = true;
            TraversalCheckResult.BackFloorLocation = Hit.ImpactPoint;
            TraversalCheckResult.BackLedgeHeight = FMath::Abs((Hit.ImpactPoint - TraversalCheckResult.BackLedgeLocation).Z);
        }
        else
        {
            TraversalCheckResult.bHasBackFloor = false;
        }
    }

    // Step 5.3: Evaluate a chooser to select all montages that match the conditions of the traversal check.
    FTraversalChooserInputs ChooserInputs;
    ChooserInputs.ActionType = TraversalCheckResult.ActionType;
    ChooserInputs.bHasFrontLedge = TraversalCheckResult.bHasFrontLedge;
    ChooserInputs.bHasBackLedge = TraversalCheckResult.bHasBackLedge;
    ChooserInputs.bHasBackFloor = TraversalCheckResult.bHasBackFloor;
    ChooserInputs.ObstacleHeight = TraversalCheckResult.ObstacleHeight;
    ChooserInputs.ObstacleDepth = TraversalCheckResult.ObstacleDepth;
    ChooserInputs.BackLedgeHeight = TraversalCheckResult.BackLedgeHeight;
    ChooserInputs.MovementMode = MovementComponent->MovementMode;
    ChooserInputs.Gait = MovementComponent->GetDesiredGait();
    ChooserInputs.Speed = MovementComponent->Velocity.Size2D();

    // Evaluate the chooser with both input and output structures
    FChooserEvaluationContext Context = UChooserFunctionLibrary::MakeChooserEvaluationContext();
    Context.AddStructParam(ChooserInputs); // Input struct
    FTraversalChooserOutputs ChooserOutputs;
    Context.AddStructParam(ChooserOutputs); // Output struct (populated by chooser)

    // Evaluate chooser - this should now populate both montages AND the output struct
    TArray<UObject*> ChooserResults = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(Context,
        UChooserFunctionLibrary::MakeEvaluateChooser(TraversalAnimationsChooserTable.LoadSynchronous()),
        UAnimMontage::StaticClass());
	
    // Update the TraversalCheckResult with the chooser output ActionType
    TraversalCheckResult.ActionType = ChooserOutputs.ActionType;

    // Store as ValidMontages for Step 5.4
    ValidMontages = ChooserResults;
	
    // Step 5.1: Continue if there is a valid action type. If none of the conditions were met, no action can be performed, therefore exit the function.
    if (TraversalCheckResult.ActionType == ETraversalActionType::None)
    {
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    // Step 5.2: Send the front ledge location to the Anim BP using an interface. This transform will be used for a custom channel within the following Motion Matching search.
    IInteractionTransformInterface* InteractableObject =
        Cast<IInteractionTransformInterface>(OwnerCharacter->GetMesh()->GetAnimInstance());
    if (InteractableObject == nullptr && !OwnerCharacter->GetMesh()->GetAnimInstance()->Implements<UInteractionTransformInterface>())
    {
        bOutTraversalCheckFailed = true;
        bOutMontageSelectionFailed = false;
        return;
    }

    const FTransform InteractionTransform =
        FTransform(FRotationMatrix::MakeFromZ(TraversalCheckResult.FrontLedgeNormal).ToQuat(),
            TraversalCheckResult.FrontLedgeLocation, FVector::OneVector);
    if (InteractableObject != nullptr)
    {
        InteractableObject->Execute_SetInteractionTransform(OwnerCharacter->GetMesh()->GetAnimInstance(), InteractionTransform);
    }
    else
    {
        IInteractionTransformInterface::Execute_SetInteractionTransform(OwnerCharacter->GetMesh()->GetAnimInstance(),
            InteractionTransform);
    }
	
    // Step 5.4: Perform a Motion Match on all the montages that were chosen by the chooser to find the best result. This match will elect the best montage AND the best entry frame (start time) based on the distance to the ledge, and the current characters pose. If for some reason no montage was found (motion matching failed, perhaps due to an invalid database or issue with the schema), print a warning and exit the function.
    static
    const FName PoseHistoryName = TEXT("PoseHistory");
    FPoseSearchBlueprintResult Result;
    FPoseSearchContinuingProperties ContinuingProperties;

    UPoseSearchLibrary::MotionMatch(
        OwnerCharacter->GetMesh()->GetAnimInstance(),
        ValidMontages,              // Stored ValidMontages from Step 5.3
        PoseHistoryName,
        ContinuingProperties,       // New 4th parameter for UE5.5
        FPoseSearchFutureProperties(), // Future properties  
        Result                      // Motion match result
    );
	
    TObjectPtr <const UAnimMontage > AnimationMontage = Cast<UAnimMontage>(Result.SelectedAnimation);
    if (!IsValid(AnimationMontage))
    {
        UKismetSystemLibrary::PrintString(this, TEXT("Failed To Find Montage!"),
            true, false, FLinearColor::Red, DrawDebugDuration);
        bOutTraversalCheckFailed = false;
        bOutMontageSelectionFailed = true;
        return;
    }

    TraversalCheckResult.ChosenMontage = AnimationMontage;
    TraversalCheckResult.StartTime = Result.SelectedTime;
    TraversalCheckResult.PlayRate = Result.WantedPlayRate;

    TraversalResult = TraversalCheckResult;

	//UpdateWarpTargets();
	bDoingTraversalAction = true;
    PerformTraversalAction.Broadcast();
	//UpdateWarpTargets();
    
    bOutTraversalCheckFailed = false;
    bOutMontageSelectionFailed = false;

    // DEBUG: Print out the resulting conditions and parameters
    if (DrawDebugLevel >= 1)
    {
        UKismetSystemLibrary::PrintString(this, TraversalCheckResult.ToString(),
            true, false, FLinearColor(0.0f, 0.66f, 1.0f), DrawDebugDuration);
        UKismetSystemLibrary::PrintString(this, UEnum::GetValueAsString(TraversalCheckResult.ActionType),
            true, false, FLinearColor(1.0f, 0.0f, 0.824021f), DrawDebugDuration);
        const FString PerfString = FString::Printf(TEXT("Execution Time: %f seconds"),
            FPlatformTime::Seconds() - StartTime);
        UKismetSystemLibrary::PrintString(this, PerfString, true, true,
            FLinearColor(1.0f, 0.5f, 0.15f), DrawDebugDuration);
    }
}

void URCTraversalComponent::UpdateWarpTargets()
{
    if (!OwnerCharacter || !MotionWarping)
    {
        UE_LOG(LogRC, Warning, TEXT("UpdateWarpTargets called but there is no TraversalComponent or MotionWarping."));
        return;
    }

	//UMotionWarpingComponent* MotionWarping = OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>();
	//if (!MotionWarping)
	//{
	//	UE_LOG(LogRC, Warning, TEXT("UpdateWarpTargets called but there is no MotionWarpingComponent."));
	//	return;
	//}	

    // Warp Target Name declarations
    //const FTraversalCheckResult& CurrentTraversalResult = TraversalComponent->TraversalResult;
	static const FName FrontLedgeWarpTargetName = TEXT("FrontLedge");
	static const FName BackLedgeWarpTargetName = TEXT("BackLedge");
	static const FName BackFloorWarpTargetName = TEXT("BackFloor");
	static const FName DistanceFromLedgeCurveName = TEXT("Distance_From_Ledge");

	// Update the FrontLedge warp target using the front ledge's location and rotation.
	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FrontLedgeWarpTargetName,
		TraversalResult.FrontLedgeLocation + FVector::ZAxisVector * 0.5f,
		FRotationMatrix::MakeFromX(-TraversalResult.FrontLedgeNormal).Rotator());

	float AnimatedDistanceFromFrontLedgeToBackLedge = 0.0f;
	// If the action type was a hurdle or a vault, we need to also update the BackLedge target. If it is not a hurdle or vault, remove it.
	if (TraversalResult.ActionType == ETraversalActionType::Hurdle ||
		TraversalResult.ActionType == ETraversalActionType::Vault)
	{
		// Because the traversal animations move at different distances (no fixed metrics), we need to know how far the animation moves in order to warp it properly. Here we cache a curve value at the end of the Back Ledge warp window to determine how far the animation is from the front ledge once the character reaches the back ledge location in the animation.
		TArray<FMotionWarpingWindowData> MotionWarpingWindowData; 
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage,
			BackLedgeWarpTargetName, MotionWarpingWindowData);
		if (!MotionWarpingWindowData.IsEmpty())
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(TraversalResult.ChosenMontage,
				DistanceFromLedgeCurveName, MotionWarpingWindowData[0].EndTime,
				AnimatedDistanceFromFrontLedgeToBackLedge);

			// Update the BackLedge warp target.
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(BackLedgeWarpTargetName,
				TraversalResult.BackLedgeLocation, FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(BackLedgeWarpTargetName);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(BackLedgeWarpTargetName);
	}

	// If the action type was a hurdle, we need to also update the BackFloor target. If it is not a hurdle, remove it.
	if (TraversalResult.ActionType == ETraversalActionType::Hurdle)
	{
		// Caches a curve value at the end of the Back Floor warp window to determine how far the animation is from the front ledge once the character touches the ground.
		TArray<FMotionWarpingWindowData> MotionWarpingWindowData; 
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage,
			BackFloorWarpTargetName, MotionWarpingWindowData);
		if (!MotionWarpingWindowData.IsEmpty())
		{
			float AnimatedDistanceFromFrontLedgeToBackFloor = 0.0f;
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(TraversalResult.ChosenMontage,
			                                                     DistanceFromLedgeCurveName, MotionWarpingWindowData[0].EndTime,
			                                                     AnimatedDistanceFromFrontLedgeToBackFloor);

			// Since the animations may land on the floor at different distances (a run hurdle may travel further than a walk or stand hurdle), use the total animated distance away from the back ledge as the X and Y values of the BackFloor warp point. This could technically cause some collision issues if the floor is not flat, or there is an obstacle in the way, therefore having fixed metrics for all traversal animations would be an improvement.
			const FVector Vector1 = TraversalResult.BackLedgeNormal *
				FMath::Abs(AnimatedDistanceFromFrontLedgeToBackLedge - AnimatedDistanceFromFrontLedgeToBackFloor);
			const FVector Vector2 = TraversalResult.BackLedgeLocation + Vector1;
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(BackFloorWarpTargetName,
				FVector(Vector2.X ,Vector2.Y, TraversalResult.BackFloorLocation.Z),
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(BackFloorWarpTargetName);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(BackFloorWarpTargetName);
	}
}
