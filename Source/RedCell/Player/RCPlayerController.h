// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RCPlayerController.generated.h"

class ARCHUD;
class ARCPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class URCAbilitySystemComponent;
class URCSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * ARCPlayerController
 * The base player controller class used by this project.
 */

UCLASS(Blueprintable)
class REDCELL_API ARCPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // THIS is the signature Unreal will actually call
    ARCPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    ARCPlayerState* GetRCPlayerState() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerController")
    ARCHUD* GetRCHUD() const;

protected:
    virtual void AcknowledgePossession(APawn* P) override;
};
