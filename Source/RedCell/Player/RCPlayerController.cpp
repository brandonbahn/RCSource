// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.cpp

#include "RCPlayerController.h"
#include "Character/RCCharacter.h"
#include "RCPlayerState.h"
#include "UI/RCHUD.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "RCGameplayTags.h"
#include "GameModes/RCGameState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerController)

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

ARCPlayerState* ARCPlayerController::GetRCPlayerState() const
{
    return CastChecked<ARCPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

URCAbilitySystemComponent* ARCPlayerController::GetRCAbilitySystemComponent() const
{
    const ARCPlayerState* RCPS = GetRCPlayerState();
    return (RCPS ? RCPS->GetRCAbilitySystemComponent() : nullptr);
}

ARCHUD* ARCPlayerController::GetRCHUD() const
{
    return CastChecked<ARCHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}
