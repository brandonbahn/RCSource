// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.cpp

#include "RCPlayerController.h"
#include "CommonInputTypeEnum.h"
#include "Character/RCCharacter.h"
#include "RCPlayerState.h"
#include "UI/RCHUD.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "RCGameplayTags.h"
#include "CommonInputSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "RCLogChannels.h"
#include "Camera/RCPlayerCameraManager.h"
#include "GameModes/RCGameState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerController)

namespace RedCell
{
    namespace Input
    {
        static int32 ShouldAlwaysPlayForceFeedback = 0;
        static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("LyraPC.ShouldAlwaysPlayForceFeedback"),
            ShouldAlwaysPlayForceFeedback,
            TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
    }
}

ARCPlayerController::ARCPlayerController(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    PlayerCameraManagerClass = ARCPlayerCameraManager::StaticClass();
}

void ARCPlayerController::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void ARCPlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void ARCPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ARCPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

void ARCPlayerController::OnPlayerStateChanged()
{
}

void ARCPlayerController::BroadcastOnPlayerStateChanged()
{
}

void ARCPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ARCPlayerController::OnUnPossess()
{
    Super::OnUnPossess();
}

void ARCPlayerController::InitPlayerState()
{
    Super::InitPlayerState();
    BroadcastOnPlayerStateChanged();
}

void ARCPlayerController::CleanupPlayerState()
{
    Super::CleanupPlayerState();
    BroadcastOnPlayerStateChanged();
}

void ARCPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BroadcastOnPlayerStateChanged();
}

void ARCPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    UE_LOG(LogRCPlayer, Log, TEXT("AcknowledgePossession called on controller %s, pawn = %s"),
           *GetName(), P ? *P->GetName() : TEXT("nullptr"));

    if (ARCCharacter* RCChar = Cast<ARCCharacter>(P))
    {
        // make sure the pawn’s Owner is _this_ controller
        RCChar->SetOwner(this);
    }
}

void ARCPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
    if (bForceFeedbackEnabled)
    {
        if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
        {
            const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
            if (RedCell::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
            {
                InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
                return;
            }
        }
    }
	
    InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

void ARCPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
    Super::PreProcessInput(DeltaTime, bGamePaused);
}

void ARCPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        RCASC->ProcessAbilityInput(DeltaTime, bGamePaused);
    }

    Super::PostProcessInput(DeltaTime, bGamePaused);
}



