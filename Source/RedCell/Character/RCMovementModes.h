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

/** Character gait movement */
UENUM(BlueprintType)
enum class E_Gait : uint8
{
  Walk    UMETA(DisplayName = "Walk"),
  Run     UMETA(DisplayName = "Run"),
  Sprint  UMETA(DisplayName = "Sprint"),
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

/** Is the Character on ground or in air */
UENUM(BlueprintType)
enum class E_MovementMode : uint8
{
  OnGround   UMETA(DisplayName = "OnGround"),
  InAir      UMETA(DisplayName = "InAir"),
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


