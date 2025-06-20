// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCMovementModes.h"
#include "RCCameraProperties.generated.h"

/**
 * Simple struct to hold camera properties for GameplayCamera
 */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	FreeCam     UMETA(DisplayName = "Free Camera"),
	Strafe      UMETA(DisplayName = "Strafe"),
	Aim         UMETA(DisplayName = "Aim"),
};

UENUM(BlueprintType)
enum class ECameraStyle : uint8
{
	Far         UMETA(DisplayName = "Far"),
	Balanced    UMETA(DisplayName = "Balanced"),
	Close       UMETA(DisplayName = "Close"),
};

USTRUCT(BlueprintType)
struct REDCELL_API FCharacterPropertiesForCamera
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	ECameraStyle CameraStyle = ECameraStyle::Balanced;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	ECameraMode CameraMode = ECameraMode::FreeCam;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	E_Stance Stance = E_Stance::Stand;

	// Constructor for initialization
	FCharacterPropertiesForCamera()
	{
		CameraStyle = ECameraStyle::Balanced;
		CameraMode = ECameraMode::FreeCam;
		Stance = E_Stance::Stand;
	}
};