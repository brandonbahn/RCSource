//
//  RCMovementModes.h
//  RedCell
//
//  Created by Brandon Bahn on 4/29/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "RCMovementModes.generated.h"

/** Character fixed or variable speed mode */
UENUM(BlueprintType)
enum class EMovementStickMode : uint8
{
  // Character will move at a fixed speed regardless of stick deflection.
  FixedSpeedSingleGait = 0 UMETA(DisplayName="Fixed Speed - Single Gait"),
  // The Character will move at a fixed walking speed with slight stick deflection, and a fixed running speed at full stick deflection.
  FixedSpeedWalkRun = 1 UMETA(DisplayName="Fixed Speed - Walk / Run"),
  // Full analog movement control with stick, character will remain walking or running based on gait input.
  VariableSpeedSingleGait = 2 UMETA(DisplayName="Variable Speed - Single Gait"),
  // Full analog movement control with stick, character will switch from walk to run gait based on stick deflection.
  VariableSpeedWalkRun = 3 UMETA(DisplayName="Variable Speed - Walk / Run")
};

/** Is the Character on ground or in air */
UENUM(BlueprintType)
enum class E_MovementSituation : uint8
{
  OnGround   UMETA(DisplayName = "OnGround"),
  InAir      UMETA(DisplayName = "InAir"),
};

/** Character gait movement */
UENUM(BlueprintType)
enum class E_Gait : uint8
{
  Walk    UMETA(DisplayName = "Walk"),
  Run     UMETA(DisplayName = "Run"),
  Sprint  UMETA(DisplayName = "Sprint"),
};

/** Is the character idle or moving */
UENUM(BlueprintType)
enum class E_MovementState : uint8
{
  Idle    UMETA(DisplayName = "Idle"),
  Moving  UMETA(DisplayName = "Moving"),
};

/** Character's rotation mode */
UENUM(BlueprintType)
enum class E_RotationMode : uint8
{
  OrientToMovement    UMETA(DisplayName = "OrientToMovement"),
  Strafe  UMETA(DisplayName = "Strafe"),
};

/** Is the character standing or crouching */
UENUM(BlueprintType)
enum class E_Stance : uint8
{
  Stand   UMETA(DisplayName = "Stand"),
  Crouch  UMETA(DisplayName = "Crouch"),
};

/** Direction the character is moving */
UENUM(BlueprintType)
enum class E_MovementDirection : uint8
{
  None     UMETA(DisplayName = "None"),
  Forward  UMETA(DisplayName = "Forward"),
  Backward UMETA(DisplayName = "Backward"),
  Right    UMETA(DisplayName = "Right"),
  Left     UMETA(DisplayName = "Left"),
};

