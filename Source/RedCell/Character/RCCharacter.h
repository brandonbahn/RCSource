// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Player/RCPlayerState.h"
#include "Character/RCHealthComponent.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"  // ← new include
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCMovementModes.h"
#include "Player/RCPlayerController.h"
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

/**
 * ARCCharacter
 *
 *    The base character pawn class used by this project.
 *    Responsible for sending events to pawn components.
 *    New behavior should be added via pawn components when possible.
 */

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class REDCELL_API ARCCharacter
  : public ACharacter
  , public IAbilitySystemInterface
  , public IGameplayTagAssetInterface
{
  GENERATED_BODY()


public:
  ARCCharacter(const FObjectInitializer& ObjInit = FObjectInitializer::Get());
    
  UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
  ARCPlayerController* GetRCPlayerController() const;

  UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
  ARCPlayerState* GetRCPlayerState() const;

  UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
  URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;
  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
  virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
  virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
  virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
  virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    
  UFUNCTION(BlueprintCallable, Category="Tags")
  void NotifyAllMovementTags(
     E_MovementMode      MovementMode,
     E_Gait              Gait,
     E_MovementState     MovementState,
     E_RotationMode      RotationMode,
     E_Stance            Stance,
     E_MovementDirection MovementDirection);

  /*
  // IAbilitySystemInterface
  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
  

   * Blueprint‐friendly getter that returns the RC subclass,
   * so you never have to cast in Blueprint./
  UFUNCTION(BlueprintCallable, Category="RedCell|Abilities")
  URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;
    
  
  UFUNCTION(BlueprintImplementableEvent, Category="Abilities")
  void OnAbilitySystemInitialized();
  */

  /** Blueprint‐callable accessor for GetHealthComponent */
  UFUNCTION(BlueprintCallable, Category="RedCell|Health")
  URCHealthComponent* GetHealthComponent() const { return HealthComponent; }

  // The PawnData Asset this Pawn will initialize itself from
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RedCell|Pawn")
  TObjectPtr<URCPawnData> PawnDataAsset;
  
/*
  UFUNCTION(BlueprintCallable, Category="RedCell|Pawn")
  void InitializeFromPawnData(URCPawnData* PawnData);
*/

protected:

  virtual void OnAbilitySystemInitialized();
  virtual void OnAbilitySystemUninitialized();

  virtual void PossessedBy(AController* NewController) override;
  virtual void UnPossessed() override;

  virtual void OnRep_Controller() override;
  virtual void OnRep_PlayerState() override;
    
  void InitializeGameplayTags();
    
  virtual void FellOutOfWorld(const class UDamageType& dmgType) override;
    
  // Begins the death sequence for the character (disables collision, disables movement, etc...)
  UFUNCTION()
  virtual void OnDeathStarted(AActor* OwningActor);
    
  // Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
  UFUNCTION()
  virtual void OnDeathFinished(AActor* OwningActor);
    
  void DisableMovementAndCollision();
  void DestroyDueToDeath();
  void UninitAndDestroy();
    
    
   /** Simple default team ID (everyone is on team 0) */
   UPROPERTY()
   uint8 MyTeamID = 0;
    
    
    
  // Called when the death sequence for the character has completed
  UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
  void K2_OnDeathFinished();
    
  // Declare this so BeginPlay() in .cpp actually matches
  virtual void BeginPlay() override;

  /** Which AbilitySet to grant on spawn (Death, Reset, etc) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
  URCAbilitySet* DefaultAbilitySet;
    
 


private:
    
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedCell|Pawn", meta=(AllowPrivateAccess="true"))
  TObjectPtr<URCPawnExtensionComponent> PawnExtensionComponent;
    
  /** The pure C++ health component that binds to the ASC */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RedCell|Health", meta=(AllowPrivateAccess="true"))
  URCHealthComponent* HealthComponent;
};
