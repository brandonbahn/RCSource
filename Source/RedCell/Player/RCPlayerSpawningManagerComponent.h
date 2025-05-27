// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "RCPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ARCPlayerStart;
class AActor;

/**
 * Component on GameState responsible for tracking PlayerStarts
 * and handling respawn logic.
 */
UCLASS()
class REDCELL_API URCPlayerSpawningManagerComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    URCPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

    /** UActorComponent */
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    /** ~UActorComponent */

protected:
    // Utility
    APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ARCPlayerStart*>& FoundStartPoints) const;
	
    virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ARCPlayerStart*>& PlayerStarts) { return nullptr; }
    virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

    UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
    void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

    /** We proxy these calls from ARCGameMode, to this component so that each experience can more easily customize the respawn system they want. */
    AActor* ChoosePlayerStart(AController* Player);
    bool ControllerCanRestart(AController* Player);
    void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
    friend class ARCGameMode;
    /** ~ARCGameMode */

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<ARCPlayerStart>> CachedPlayerStarts;

private:
    void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
    void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
    APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};
