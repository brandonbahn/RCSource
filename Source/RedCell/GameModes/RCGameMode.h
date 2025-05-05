// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/RCHUD.h"
#include "RCGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class URCPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
/**
 * ARCGameMode
 * The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class REDCELL_API ARCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    
    ARCGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Pawn")
    const URCPawnData* GetPawnDataForController(const AController* InController) const;
    
    //~AGameModeBase interface
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
    virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
    TArray<URCPawnData*> AvailablePawnData;
};
