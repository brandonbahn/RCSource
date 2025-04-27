// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RCGameState.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameState)

ARCGameState::ARCGameState(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
  // create the component just like Lyra does
  SpawnManager = ObjInit.CreateDefaultSubobject<URCPlayerSpawningManagerComponent>(this, TEXT("SpawnManager"));
}

