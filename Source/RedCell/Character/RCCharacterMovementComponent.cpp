// Fill out your copyright notice in the Description page of Project Settings.


#include "RCCharacterMovementComponent.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCMovementModes.h"
#include "Character/RCCharacter.h"
#include "KismetAnimationLibrary.h"
#include "Input/RCInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCharacterMovementComponent)

URCCharacterMovementComponent::URCCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    InputConfig = nullptr;
}

void URCCharacterMovementComponent::GetInputConfig()
{
    if (!InputConfig)
    {
        if (ARCCharacter* Character = Cast<ARCCharacter>(GetOwner()))
        {
            if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Character))
            {
                if (const URCPawnData* PawnData = PawnExtComp->GetPawnData<URCPawnData>())
                {
                    InputConfig = PawnData->InputConfig;
                }
            }
        }
    }
}

float URCCharacterMovementComponent::GetMaxSpeed() const
{
	return CalculateMaxSpeed();
}

float URCCharacterMovementComponent::GetMaxAcceleration() const
{
	return CalculateMaxAcceleration();
}

float URCCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	return CalculateBrakingDeceleration();
}

E_Gait URCCharacterMovementComponent::GetDesiredGait() const
{
    float MovementInputLength = GetMovementInputMagnitude();
    
    // Get movement stick mode and threshold from InputConfig
    bool bFullMovementInput = IsFullMovementInput();
    float AnalogWalkRunThreshold = GetAnalogWalkRunThreshold();
    bool bAboveAnalogThreshold = MovementInputLength >= AnalogWalkRunThreshold;
    
    bool bWantsToSprint = HasInputStateTag(RCGameplayTags::InputState_WantsToSprint);
    bool bWantsToWalk = HasInputStateTag(RCGameplayTags::InputState_WantsToWalk);
    
    E_Gait Result;
    
    if (CanSprint() && bAboveAnalogThreshold)
    {
        Result = bFullMovementInput ? E_Gait::Sprint : E_Gait::Run;
    }
    else
    {
        Result = (!bWantsToSprint && bWantsToWalk) || 
                 (!bWantsToSprint && !bWantsToWalk && !bFullMovementInput) ?
                 E_Gait::Walk : E_Gait::Run;
    }
    
    return Result;
}

bool URCCharacterMovementComponent::CanSprint() const
{
    // Replace CharacterInputState.bWantsToSprint with input state tag
    if (!HasInputStateTag(RCGameplayTags::InputState_WantsToSprint)) return false;
    
    // Skip direction check if using orient to movement
    if (bOrientRotationToMovement) return true;
    
    // Get movement vector - use character's stored input vector
    const FVector MovementVector = GetMovementInputVector();
    
    // No movement = no sprint
    if (MovementVector.IsNearlyZero()) return false;
    
    // Check if moving forward (within 50 degree cone)
    const float YawDelta = FMath::Abs(FRotator::NormalizeAxis(
        FRotationMatrix::MakeFromX(MovementVector).Rotator().Yaw - UpdatedComponent->GetComponentRotation().Yaw));
    
    const bool bCanSprintDirection = YawDelta < 50.0f;
    
    return bCanSprintDirection;
}

float URCCharacterMovementComponent::CalculateMaxAcceleration() const
{
    E_Gait CurrentGait = GetDesiredGait();
    
    switch (CurrentGait)
    {
    case E_Gait::Walk:
    case E_Gait::Run:
        return 800.0f;
            
    case E_Gait::Sprint:
        {
            float CurrentSpeed = Velocity.Size2D();
            return FMath::GetMappedRangeValueClamped(
                FVector2D(300.0f, 700.0f),  // Input
                FVector2D(800.0f, 300.0f),  // Output  
                CurrentSpeed
            );
        }
        
    default:
        return 800.0f;
    }
}

float URCCharacterMovementComponent::CalculateBrakingDeceleration() const
{
    if (IsFalling())
    {
        return GetPendingInputVector().IsNearlyZero() ? 100.0f : 50.0f;   
    } 
    return GetPendingInputVector().IsNearlyZero() ? 2000.0f : 500.0f;
}

float URCCharacterMovementComponent::CalculateGroundFriction() const
{
    E_Gait CurrentGait = GetDesiredGait();
    
    switch (CurrentGait)
    {
    case E_Gait::Walk:
    case E_Gait::Run:
        return 5.0f;
            
    case E_Gait::Sprint:
        {
            float CurrentSpeed = Velocity.Size2D();
            return FMath::GetMappedRangeValueClamped(
                FVector2D(0.0f, 500.0f),
                FVector2D(5.0f, 3.0f), 
                CurrentSpeed
            );
        }
        
    default:
        return 5.0f;
    }
}

float URCCharacterMovementComponent::CalculateMaxSpeed() const
{
    E_Gait CurrentGait = GetDesiredGait();

    if (IsCrouching())
    {
        return CalculateMaxCrouchSpeed();
    } 
    
    if (!StrafeSpeedMapCurve.IsNull())
    {
        // Use bUseControllerDesiredRotation instead of bOrientRotationToMovement
        float StrafeSpeedMap = bUseControllerDesiredRotation ? 
            StrafeSpeedMapCurve.LoadSynchronous()->GetFloatValue(
                FMath::Abs(UKismetAnimationLibrary::CalculateDirection(Velocity, UpdatedComponent->GetComponentRotation()))) : 0.0f;
        
        const bool bStrafe = StrafeSpeedMap < 1.0f;
        const FVector& Speeds =
            CurrentGait == E_Gait::Walk ? WalkSpeeds :
            CurrentGait == E_Gait::Run ? RunSpeeds :
            CurrentGait == E_Gait::Sprint ? SprintSpeeds :
            FVector::ZeroVector;
        
        static const FVector2D RangeA1(0.0f, 1.0f), RangeA2(1.0f, 2.0f);
        return FMath::GetMappedRangeValueClamped(bStrafe ? RangeA1 : RangeA2,
            FVector2D(bStrafe ? Speeds.X : Speeds.Y, bStrafe ? Speeds.Y : Speeds.Z), StrafeSpeedMap);
    }
    return INDEX_NONE;
}

float URCCharacterMovementComponent::CalculateMaxCrouchSpeed() const
{
    if (!StrafeSpeedMapCurve.IsNull())
    {
        // FIXED: Use bUseControllerDesiredRotation instead of bOrientRotationToMovement
        float StrafeSpeedMap = bUseControllerDesiredRotation ? 
            StrafeSpeedMapCurve.LoadSynchronous()->GetFloatValue(
                FMath::Abs(UKismetAnimationLibrary::CalculateDirection(Velocity, UpdatedComponent->GetComponentRotation()))) : 
            0.0f;
        
        const bool bStrafe = StrafeSpeedMap < 1.0f;
        const FVector& Speeds = CrouchSpeeds;
        
        static const FVector2D RangeA1(0.0f, 1.0f), RangeA2(1.0f, 2.0f);
        return FMath::GetMappedRangeValueClamped(bStrafe ? RangeA1 : RangeA2,
            FVector2D(bStrafe ? Speeds.X : Speeds.Y, bStrafe ? Speeds.Y : Speeds.Z), StrafeSpeedMap);
    }
    return INDEX_NONE;
}

bool URCCharacterMovementComponent::CanAttemptJump() const
{
    // Same as UCharacterMovementComponent's implementation but without the crouch check
    return IsJumpAllowed() &&
        (IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

void URCCharacterMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

// Helper functions to get data from character
bool URCCharacterMovementComponent::HasInputStateTag(const FGameplayTag& Tag) const
{
    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
    {
        return ASC->HasMatchingGameplayTag(Tag);
    }
    return false;
}

float URCCharacterMovementComponent::GetMovementInputMagnitude() const
{
    if (ARCCharacter* Character = Cast<ARCCharacter>(GetOwner()))
    {
        return Character->GetMovementInputMagnitude();
    }
    return 0.0f;
}

FVector URCCharacterMovementComponent::GetMovementInputVector() const
{
    if (ARCCharacter* Character = Cast<ARCCharacter>(GetOwner()))
    {
        return Character->GetMovementInputVector();
    }
    return FVector::ZeroVector;
}

bool URCCharacterMovementComponent::IsFullMovementInput() const
{
    const_cast<URCCharacterMovementComponent*>(this)->GetInputConfig();
    
    if (InputConfig)
    {
        float MovementInputLength = GetMovementInputMagnitude();
        
        // Your exact GASP interface logic
        return InputConfig->MovementStickMode == EMovementStickMode::FixedSpeedSingleGait ||
               InputConfig->MovementStickMode == EMovementStickMode::VariableSpeedSingleGait ||
               ((InputConfig->MovementStickMode == EMovementStickMode::FixedSpeedWalkRun ||
                 InputConfig->MovementStickMode == EMovementStickMode::VariableSpeedWalkRun) &&
                MovementInputLength >= InputConfig->AnalogWalkRunThreshold);
    }
    return GetMovementInputMagnitude() > 0.1f;
}

float URCCharacterMovementComponent::GetAnalogWalkRunThreshold() const
{
    const_cast<URCCharacterMovementComponent*>(this)->GetInputConfig();
    
    return InputConfig ? InputConfig->AnalogWalkRunThreshold : 0.7f;
}