// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RCCameraProperties.h"
#include "RCGameplayCameraInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType, MinimalAPI, meta = (DisplayName = "GameplayCameraInterface"))
class URCGameplayCameraInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Gets the current camera properties for this actor
 */
class REDCELL_API IRCGameplayCameraInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera", meta = (DisplayName = "GetCharacterPropertiesForCamera"))
	FCharacterPropertiesForCamera GetCharacterPropertiesForCamera() const;
};
