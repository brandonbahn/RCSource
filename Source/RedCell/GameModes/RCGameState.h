// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AbilitySystemInterface.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include "RCGameState.generated.h"

class APlayerState;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class UObject;

UCLASS()
class REDCELL_API ARCGameState : public AGameStateBase, public IAbilitySystemInterface
{
  GENERATED_BODY()

public:
    ARCGameState(const FObjectInitializer& ObjInitializer = FObjectInitializer::Get());
    
    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void PostInitializeComponents() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    //~End of AActor interface
    
    //~IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    //~End of IAbilitySystemInterface

    // Gets the ability system component used for game wide things
    UFUNCTION(BlueprintCallable, Category = "RedCell|GameState")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }

    /** Our spawn manager, taken from Lyra’s pattern */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
    URCPlayerSpawningManagerComponent* SpawnManager;
    
private:
    // The ability system component subobject for game-wide things (primarily gameplay cues)
    UPROPERTY(VisibleAnywhere, Category = "RedCell|GameState")
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;
};

