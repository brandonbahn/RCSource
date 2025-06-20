// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"

#include "RCGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class URCExperienceDefinition;
class URCPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
enum class ECommonSessionOnlineMode : uint8;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRCGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);
/**
 * ARCGameMode
 * The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class REDCELL_API ARCGameMode : public AModularGameModeBase
{
    GENERATED_BODY()

public:
    
    ARCGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Pawn")
    const URCPawnData* GetPawnDataForController(const AController* InController) const;
    
    //~AGameModeBase interface
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
    virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
    virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
    virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
    virtual void InitGameState() override;
    virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
    virtual void FailedToRestartPlayer(AController* NewPlayer) override;
    //~End of AGameModeBase interface

    UFUNCTION(BlueprintCallable)
    void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

    // Agnostic version of PlayerCanRestart that can be used for both player bots and players
    virtual bool ControllerCanRestart(AController* Controller);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
    TArray<URCPawnData*> AvailablePawnData;

    // Delegate called on player initialization, described above 
    FOnRCGameModePlayerInitialized OnGameModePlayerInitialized;

protected:	
    void OnExperienceLoaded(const URCExperienceDefinition* CurrentExperience);
    bool IsExperienceLoaded() const;

    void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

    void HandleMatchAssignmentIfNotExpectingOne();

    bool TryDedicatedServerLogin();
    void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode);

    UFUNCTION()
    void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
};
