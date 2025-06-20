// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RCLocalPlayer.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "CommonUserSubsystem.h"
#include "RCLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCLocalPlayer)

class UObject;

URCLocalPlayer::URCLocalPlayer()
{
}

void URCLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();
}	

void URCLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);
	OnPlayerControllerChanged(PlayerController);
}

bool URCLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);
	OnPlayerControllerChanged(PlayerController);
	return bResult;
}

void URCLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);
	Super::InitOnlineSession();
}

void URCLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
	if (!bForceLoad && CurrentNetId == NetIdForSharedSettings)
	{
		// Already loaded once, don't reload
		return;
	}
	UE_LOG(LogRCPlayer, Log, TEXT("Skipping settings load - not implemented"));
}

void URCLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	UE_LOG(LogTemp, Error, TEXT("LocalPlayer OnPlayerControllerChanged: %s"), 
		   NewController ? *NewController->GetName() : TEXT("NULL"));
}
