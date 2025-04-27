// Fill out your copyright notice in the Description page of Project Settings.

#include "RCHUD.h"

ARCHUD::ARCHUD()
{
  // you could set a C++ default here if you want:
  // static ConstructorHelpers::FClassFinder<UUserWidget> HealthBP(TEXT("/Game/UI/W_HealthBar"));
  // HealthBarWidgetClass = HealthBP.Class;
}

void ARCHUD::BeginPlay()
{
  Super::BeginPlay();

  //if (HealthBarWidgetClass)
  //{
    // Create and add the widget
    //HealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), HealthBarWidgetClass);
    //if (HealthBarWidget)
    //{
    //  HealthBarWidget->AddToViewport();
      
      //    Top‑Center anchor (0.5,0.0) with 0.5 alignment means it will center around the top edge
    //  HealthBarWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.0f));
    //  HealthBarWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.0f));

      //    Offset down by 20 pixels, and give it a desired size of 400×32
    //  HealthBarWidget->SetPositionInViewport(FVector2D(125.0f, 20.0f));
    //  HealthBarWidget->SetDesiredSizeInViewport(FVector2D(450.0f, 40.0f));
        
      // Let any Blueprint subclass run extra setup (e.g. binding delegates)
    //  BP_OnHealthBarCreated(HealthBarWidget);
    //}
  //}

  // …and later, when you add Stamina/Mana, just repeat the pattern:
  // if (StaminaBarWidgetClass) { … CreateWidget → AddToViewport → BP_OnStaminaBarCreated(…) }
    if (HealthBarWidgetClass)
      {
        // 1) create the widget instance
        HealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), HealthBarWidgetClass);

        // 2) hand it off to Blueprint so we can call Add Widget for Player there
        BP_OnHealthBarCreated(HealthBarWidget);
      }
}
