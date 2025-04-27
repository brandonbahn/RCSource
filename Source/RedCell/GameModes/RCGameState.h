// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include "RCGameState.generated.h"

UCLASS()
class REDCELL_API ARCGameState : public AGameStateBase
{
  GENERATED_BODY()

public:
  ARCGameState(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

  /** Our spawn manager, taken from Lyra’s pattern */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
  URCPlayerSpawningManagerComponent* SpawnManager;
};

