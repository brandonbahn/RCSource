// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "RCPlayerState.generated.h"

struct FRCVerbMessage;

class AController;
class ARCPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class URCExperienceDefinition;
class URCPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/**
 * ARCPlayerState
 *
 *    Base player state class used by this project.
 */
UCLASS(Config = Game)
class REDCELL_API ARCPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
  GENERATED_BODY()

public:
    ARCPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    ARCPlayerController* GetRCPlayerController() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    template <class T>
    const T* GetPawnData() const { return Cast<T>(PawnData); }
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetPawnData(const URCPawnData* InPawnData);

    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void PostInitializeComponents() override;
    //~End of AActor interface

    //~APlayerState interface
    virtual void Reset() override;
    virtual void ClientInitialize(AController* C) override;
    virtual void CopyProperties(APlayerState* PlayerState) override;
    virtual void OnDeactivated() override;
    virtual void OnReactivated() override;
    //~End of APlayerState interface
    
    static const FName NAME_RCAbilityReady;

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "RedCell|PlayerState")
	void ClientBroadcastMessage(const FRCVerbMessage Message);

    // Default AbilitySet to grant on possession (server only)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilitySystem")
    URCAbilitySet* DefaultAbilitySet;

private:
	void OnExperienceLoaded(const URCExperienceDefinition* CurrentExperience);
	
    
protected:
    UFUNCTION()
    void OnRep_PawnData();

protected:

    UPROPERTY(ReplicatedUsing = OnRep_PawnData)
    TObjectPtr<const URCPawnData> PawnData;
    
private:

    // The ability system component sub-object used by player characters.
    UPROPERTY(VisibleAnywhere, Category = "RedCell|PlayerState")
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;

    // Health attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<URCHealthSet> HealthSet;
    
    // Combat attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<const class URCCombatSet> CombatSet;
    
    // Core attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<URCCoreSet> CoreSet;
    
};
