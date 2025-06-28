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
	// Stop listening for changes from the old controller
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (IRCTeamAgentInterface* ControllerAsTeamProvider = Cast<IRCTeamAgentInterface>(LastBoundPC.Get()))
	{
		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	// Grab the current team ID and listen for future changes
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (IRCTeamAgentInterface* ControllerAsTeamProvider = Cast<IRCTeamAgentInterface>(NewController))
	{
		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
		LastBoundPC = NewController;
	}

	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

void URCLocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	// Do nothing, we merely observe the team of our associated player controller
}

FGenericTeamId URCLocalPlayer::GetGenericTeamId() const
{
	if (IRCTeamAgentInterface* ControllerAsTeamProvider = Cast<IRCTeamAgentInterface>(PlayerController))
	{
		return ControllerAsTeamProvider->GetGenericTeamId();
	}
	else
	{
		return FGenericTeamId::NoTeam;
	}
}

FOnRCTeamIndexChangedDelegate* URCLocalPlayer::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void URCLocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}
