// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "RCPlayerCameraManager.generated.h"

class UCameraAsset;
class URCPawnData;
class APawn;
class UBlueprintFunctionLibrary;
/**
 * 
 */
UCLASS()
class REDCELL_API ARCPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ARCPlayerCameraManager();

	UFUNCTION(BlueprintCallable, Category="RedCell")
	void InitializeGameplayCamera(APawn* Pawn, const URCPawnData* PawnData);

protected:
	void SetViewTargetWithBlend(AActor* NewViewTarget, float BlendWeight, bool bNoTransition);
	
};
