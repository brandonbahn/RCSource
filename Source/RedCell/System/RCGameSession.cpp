// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameSession)


ARCGameSession::ARCGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ARCGameSession::ProcessAutoLogin()
{
	// This is actually handled in RCGameMode::TryDedicatedServerLogin
	return true;
}

void ARCGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ARCGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

