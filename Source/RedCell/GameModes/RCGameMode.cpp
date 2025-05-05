// Fill out your copyright notice in the Description page of Project Settings.

// RCGameMode.cpp
#include "RCGameMode.h"
#include "Player/RCPlayerController.h"
#include "Player/RCPlayerState.h"
#include "GameModes/RCGameState.h"
#include "Character/RCCharacter.h"
#include "UI/RCHUD.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"

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

const URCPawnData* ARCGameMode::GetPawnDataForController(const AController* InController) const
{
    // See if pawn data is already set on the player state
    if (InController != nullptr)
    {
        if (const ARCPlayerState* RCPS = InController->GetPlayerState<ARCPlayerState>())
        {
            if (const URCPawnData* PawnData = RCPS->GetPawnData<URCPawnData>())
            {
                return PawnData;
            }
        }
    }
    
    return nullptr;
}

UClass* ARCGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (const URCPawnData* PawnData = GetPawnDataForController(InController))
    {
        if (PawnData->PawnClass)
        {
            return PawnData->PawnClass;
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ARCGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient;    // Never save the default player pawns into a map.
    SpawnInfo.bDeferConstruction = true;

    if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
    {
        if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
        {
            if (URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
            {
                if (const URCPawnData* PawnData = GetPawnDataForController(NewPlayer))
                {
                    PawnExtComp->SetPawnData(PawnData);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
                }
            }

            SpawnedPawn->FinishSpawning(SpawnTransform);

            return SpawnedPawn;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
    }

    return nullptr;
}
