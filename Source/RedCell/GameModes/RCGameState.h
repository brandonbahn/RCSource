// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AbilitySystemInterface.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include "RCGameState.generated.h"

struct FRCVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class URCExperienceManagerComponent;
class UObject;
struct FFrame;

UCLASS(Config = Game)
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

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;	
    
    //~IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    //~End of IAbilitySystemInterface

    // Gets the ability system component used for game wide things
    UFUNCTION(BlueprintCallable, Category = "RedCell|GameState")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }

	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "RedCell|GameState")
	void MulticastMessageToClients(const FRCVerbMessage Message);
	
	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "RedCell|GameState")
	void MulticastReliableMessageToClients(const FRCVerbMessage Message);

	// Gets the server's FPS, replicated to clients
	float GetServerFPS() const;
	
    /** Our spawn manager, taken from Lyra’s pattern */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
    URCPlayerSpawningManagerComponent* SpawnManager;
    
private:
	// Handles loading and managing the current gameplay experience
	UPROPERTY()
	TObjectPtr<URCExperienceManagerComponent> ExperienceManagerComponent;
	
    // The ability system component subobject for game-wide things (primarily gameplay cues)
    UPROPERTY(VisibleAnywhere, Category = "RedCell|GameState")
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;

protected:
	UPROPERTY(Replicated)
	float ServerFPS;
};

