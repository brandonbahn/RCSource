// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.cpp

#include "RCPlayerController.h"
#include "Character/RCCharacter.h"

ARCPlayerController::ARCPlayerController(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    // any default PC‐wide setup can go here
}

void ARCPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    UE_LOG(LogTemp, Log, TEXT("AcknowledgePossession called on controller %s, pawn = %s"),
           *GetName(), P ? *P->GetName() : TEXT("nullptr"));

    if (ARCCharacter* RCChar = Cast<ARCCharacter>(P))
    {
        // make sure the pawn’s Owner is _this_ controller
        RCChar->SetOwner(this);
    }
}

