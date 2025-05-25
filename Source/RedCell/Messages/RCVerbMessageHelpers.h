// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "RCVerbMessageHelpers.generated.h"

struct FGameplayCueParameters;
struct FRCVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;

UCLASS()
class REDCELL_API URCVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "RedCell")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "RedCell")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "RedCell")
	static FGameplayCueParameters VerbMessageToCueParameters(const FRCVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "RedCell")
	static FRCVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};

