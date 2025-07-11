// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Teams/RCTeamAgentInterface.h"
#include "RCPlayerController.generated.h"

class ARCHUD;
class ARCPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class URCAbilitySystemComponent;
class URCSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * ARCPlayerController
 * The base player controller class used by this project.
 */

UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class REDCELL_API ARCPlayerController : public ACommonPlayerController, public IRCTeamAgentInterface
{
    GENERATED_BODY()

public:
    // THIS is the signature Unreal will actually call
    ARCPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    ARCPlayerState* GetRCPlayerState() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    ARCHUD* GetRCHUD() const;

    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~End of AActor interface

    //~AController interface
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void InitPlayerState() override;
    virtual void CleanupPlayerState() override;
    virtual void OnRep_PlayerState() override;
    //~End of AController interface

    //~APlayerController interface
    virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
    virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
    virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
    //~End of APlayerController interface

    //~IRCTeamAgentInterface interface
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
    virtual FGenericTeamId GetGenericTeamId() const override;
    virtual FOnRCTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
    //~End of IRCTeamAgentInterface interface
    
protected:
    virtual void AcknowledgePossession(APawn* P) override;

private:
    UPROPERTY()
    FOnRCTeamIndexChangedDelegate OnTeamChangedDelegate;

    UPROPERTY()
    TObjectPtr<APlayerState> LastSeenPlayerState;

private:
    UFUNCTION()
    void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
    // Called when the player state is set or cleared
    virtual void OnPlayerStateChanged();

private:
    void BroadcastOnPlayerStateChanged();

};
