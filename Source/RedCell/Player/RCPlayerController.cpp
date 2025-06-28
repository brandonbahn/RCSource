// Fill out your copyright notice in the Description page of Project Settings.

// RCPlayerController.cpp

#include "RCPlayerController.h"
#include "CommonInputTypeEnum.h"
#include "Character/RCCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "RCPlayerState.h"
#include "UI/RCHUD.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "RCGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "RCLocalPlayer.h"
#include "Development/RCDeveloperSettings.h"
#include "Net/UnrealNetwork.h"
#include "GameMapsSettings.h"
#include "RCLogChannels.h"
#include "Camera/RCPlayerCameraManager.h"
#include "GameModes/RCGameState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerController)

namespace RedCell
{
    namespace Input
    {
        static int32 ShouldAlwaysPlayForceFeedback = 0;
        static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("RedCellPC.ShouldAlwaysPlayForceFeedback"),
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

void ARCPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

void ARCPlayerController::OnPlayerStateChanged()
{
}

void ARCPlayerController::BroadcastOnPlayerStateChanged()
{
    OnPlayerStateChanged();

    // Unbind from the old player state, if any
    FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
    if (LastSeenPlayerState != nullptr)
    {
        if (IRCTeamAgentInterface* PlayerStateTeamInterface = Cast<IRCTeamAgentInterface>(LastSeenPlayerState))
        {
            OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
            PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
        }
    }

    // Bind to the new player state, if any
    FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
    if (PlayerState != nullptr)
    {
        if (IRCTeamAgentInterface* PlayerStateTeamInterface = Cast<IRCTeamAgentInterface>(PlayerState))
        {
            NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
            PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
        }
    }

    // Broadcast the team change (if it really has)
    ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

    LastSeenPlayerState = PlayerState;
}

void ARCPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ARCPlayerController::OnUnPossess()
{
    // Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
    if (APawn* PawnBeingUnpossessed = GetPawn())
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
        {
            if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
            {
                ASC->SetAvatarActor(nullptr);
            }
        }
    }

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

void ARCPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    UE_LOG(LogRCTeams, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

FGenericTeamId ARCPlayerController::GetGenericTeamId() const
{
    if (const IRCTeamAgentInterface* PSWithTeamInterface = Cast<IRCTeamAgentInterface>(PlayerState))
    {
        return PSWithTeamInterface->GetGenericTeamId();
    }
    return FGenericTeamId::NoTeam;
}

FOnRCTeamIndexChangedDelegate* ARCPlayerController::GetOnTeamIndexChangedDelegate()
{
    return &OnTeamChangedDelegate;
}


