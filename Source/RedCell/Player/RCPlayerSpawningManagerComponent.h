// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "RCPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerStart;
class AActor;

/**
 * Component on GameState responsible for tracking PlayerStarts
 * and handling respawn logic.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REDCELL_API URCPlayerSpawningManagerComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    URCPlayerSpawningManagerComponent(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

    // ~ Begin UActorComponent Interface
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~ End UActorComponent Interface

    /** Called by GameMode to choose a spawn point for a player */
    AActor* ChoosePlayerStart(AController* Player);

    /** Returns whether the given player controller can restart */
    bool ControllerCanRestart(AController* Player) const;

    /** Completes the restart process and notifies Blueprints */
    void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);

protected:
    /** Blueprint hook for additional logic when finishing restart */
    UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnFinishRestartPlayer"))
    void K2_OnFinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);

    /** Override to customize spawn selection logic */
    virtual AActor* OnChoosePlayerStart(AController* Player, const TArray<APlayerStart*>& Starts) { return nullptr; }

    /** Utility: pick a random unoccupied start */
    APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<APlayerStart*>& Starts) const;

private:
    /** Cache of all discovered PlayerStart actors */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<APlayerStart>> CachedPlayerStarts;

    /** Called when a new level is loaded */
    void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);

    /** Called whenever any actor spawns in the world */
    void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
    /** PIE helper: Find the "Play From Here" PlayerStart */
    APlayerStart* FindPlayFromHereStart(AController* Player) const;
#endif
};
