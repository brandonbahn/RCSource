// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Player/RCPlayerState.h"
#include "Character/RCHealthComponent.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"  // ← new include
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "RCCharacter.generated.h"

class AActor;
class AController;
class ARCPlayerController;
class ARCPlayerState;
class FLifetimeProperty;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class URCHealthComponent;
class URCPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

UCLASS()
class REDCELL_API ARCCharacter
  : public ACharacter
  , public IAbilitySystemInterface
{
  GENERATED_BODY()

public:
  ARCCharacter(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

  // IAbilitySystemInterface
  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

  /**
   * Blueprint‐friendly getter that returns the RC subclass,
   * so you never have to cast in Blueprint.
   */
  UFUNCTION(BlueprintCallable, Category="RedCell|Abilities")
  URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;
    
  UFUNCTION(BlueprintImplementableEvent, Category="Abilities")
  void OnAbilitySystemInitialized();

  /** Blueprint‐callable accessor for GetHealthComponent */
  UFUNCTION(BlueprintCallable, Category="RedCell|Health")
  URCHealthComponent* GetHealthComponent() const { return HealthComponent; }
    
  UFUNCTION(BlueprintCallable, Category="RedCell|Pawn")
  void InitializeFromPawnData(URCPawnData* PawnData);

  // Called when this pawn is possessed by a controller (server)
  virtual void PossessedBy(AController* NewController) override;

  // Called on clients when PlayerState replicates in
  virtual void OnRep_PlayerState() override;
    
  /** Extension component that drives PawnData initialization */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RedCell|Pawn", meta=(AllowPrivateAccess="true"))
  URCPawnExtensionComponent* PawnExtensionComponent;
   
  // The PawnData Asset this Pawn will initialize itself from
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RedCell|Pawn")
  TObjectPtr<URCPawnData> PawnDataAsset;
    
   


protected:
  // Declare this so BeginPlay() in .cpp actually matches
  virtual void BeginPlay() override;

  /** Which AbilitySet to grant on spawn (Death, Reset, etc) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
  URCAbilitySet* DefaultAbilitySet;

private:
  /** The pure C++ health component that binds to the ASC */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RedCell|Health", meta=(AllowPrivateAccess="true"))
  URCHealthComponent* HealthComponent;
};


