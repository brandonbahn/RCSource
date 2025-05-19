// Fill out your copyright notice in the Description page of Project Settings.

// RCHUD.h

#pragma once

#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "RCHUD.generated.h"

UCLASS()
class REDCELL_API ARCHUD : public AHUD
{
  GENERATED_BODY()

public:
  ARCHUD();

  /**
   * Select default player HUD widget
   * (need to add StaminaBarClass, ManaBarClass, XPBarClass, etc.)
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
  TSubclassOf<UUserWidget> PlayerHUD_WidgetClass;

  /** Optional hook if designers want to do extra Blueprint logic */
  UFUNCTION(BlueprintImplementableEvent, Category="UI")
  void OnPlayerHUDCreated(UUserWidget* PlayerHUD);
    
    // *** Whenever add StaminaBar, ManaBar, etc. *** //
    
    // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
    // TSubclassOf<UUserWidget> StaminaBarWidgetClass;
    // UFUNCTION(BlueprintImplementableEvent, Category="UI")
    // void BP_OnStaminaBarCreated(UUserWidget* StaminaBar);


protected:
  virtual void BeginPlay() override;

private:
  /** Keep reference so we can unbind or remove if needed */
  UPROPERTY()
  UUserWidget* PlayerHUDWidget;
};
