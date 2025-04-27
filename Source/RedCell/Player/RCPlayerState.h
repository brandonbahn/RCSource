// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "RCPlayerState.generated.h"

UCLASS()
class REDCELL_API ARCPlayerState
  : public APlayerState
  , public IAbilitySystemInterface
{
  GENERATED_BODY()

public:
  ARCPlayerState();

  // IAbilitySystemInterface
  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
  {
    return AbilitySystemComponent;
  }

  // The ability system component sub-object used by player characters.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystemComponent")
  URCAbilitySystemComponent* AbilitySystemComponent;

  // Health attribute set used by this actor.
  UPROPERTY()
  URCHealthSet*           HealthSet;
    
  // If using Stamina/Mana, add them here as well:
  // UPROPERTY() URCAttributeSet_Stamina* StaminaSet;
  // UPROPERTY() URCAttributeSet_Mana*     ManaSet;
};
