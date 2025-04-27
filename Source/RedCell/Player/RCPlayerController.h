// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RCPlayerController.generated.h"

UCLASS(Blueprintable)
class REDCELL_API ARCPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // THIS is the signature Unreal will actually call
    ARCPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void AcknowledgePossession(APawn* P) override;
};
