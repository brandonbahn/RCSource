// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonGameViewportClient.h"
#include "RCGameViewportClient.generated.h"

class UGameInstance;
class UObject;

UCLASS(BlueprintType)
class URCGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()
	
public:
	URCGameViewportClient();

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};

