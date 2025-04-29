// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "RCPlayerState.generated.h"

class AController;
class ARCPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class URCPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/**
 * ARCPlayerState
 *
 *    Base player state class used by this project.
 */
UCLASS()
class REDCELL_API ARCPlayerState
  : public APlayerState
  , public IAbilitySystemInterface
{
  GENERATED_BODY()

public:
    ARCPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    ARCPlayerController* GetRCPlayerController() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

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

    // IAbilitySystemInterface
  //  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
  //{
  //  return AbilitySystemComponent;
  //}

    // The ability system component sub-object used by player characters.
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystemComponent")
    //URCAbilitySystemComponent* AbilitySystemComponent;

    // Health attribute set used by this actor.
    //UPROPERTY()
    //URCHealthSet*           HealthSet;
    
  // If using Stamina/Mana, add them here as well:
  // UPROPERTY() URCAttributeSet_Stamina* StaminaSet;
  // UPROPERTY() URCAttributeSet_Mana*     ManaSet;

    // Default AbilitySet to grant on possession (server only)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilitySystem")
    URCAbilitySet* DefaultAbilitySet;
    
private:

    // The ability system component sub-object used by player characters.
    UPROPERTY(VisibleAnywhere, Category = "RedCell|PlayerState")
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;

    // Health attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<URCHealthSet> HealthSet;
    
    //***Combat attribute set used by this actor once set-up***//
  //UPROPERTY()
  //TObjectPtr<const class ULyraCombatSet> CombatSet;
    
};
