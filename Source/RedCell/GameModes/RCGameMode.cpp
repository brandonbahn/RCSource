// Fill out your copyright notice in the Description page of Project Settings.

// RCGameMode.cpp
#include "RCGameMode.h"
#include "Player/RCPlayerController.h"
#include "Player/RCPlayerState.h"
#include "GameModes/RCGameState.h"
#include "Character/RCCharacter.h"
#include "UI/RCHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameMode)

ARCGameMode::ARCGameMode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)

//ARCGameMode::ARCGameMode(const FObjectInitializer& FI)
//  : Super(FI)
{
  PlayerControllerClass = ARCPlayerController::StaticClass();
  PlayerStateClass      = ARCPlayerState     ::StaticClass();
  GameStateClass        = ARCGameState       ::StaticClass();
  DefaultPawnClass      = ARCCharacter       ::StaticClass();
  HUDClass              = ARCHUD             ::StaticClass();
}

