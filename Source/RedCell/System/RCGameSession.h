// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "RCGameSession.generated.h"

class UObject;

/**
 * 
 */
UCLASS(Config = Game)
class ARCGameSession : public AGameSession
{
	GENERATED_BODY()
	
public:

	ARCGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Override to disable the default behavior */
	virtual bool ProcessAutoLogin() override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
};

