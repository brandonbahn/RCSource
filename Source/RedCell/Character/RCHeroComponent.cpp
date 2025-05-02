// Fill out your copyright notice in the Description page of Project Settings.

#include "RCHeroComponent.h"
#include "Input/RCInputConfig.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "EnhancedInputSubsystems.h"
#include "Player/RCPlayerController.h"
#include "Player/RCPlayerState.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCCharacter.h"
#include "Input/RCInputComponent.h"
#include "RCGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCHeroComponent)

const FName URCHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName URCHeroComponent::NAME_ActorFeatureName("Hero");

URCHeroComponent::URCHeroComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReadyToBindInputs = false;
}

void URCHeroComponent::OnRegister()
{
    Super::OnRegister();

    // We assume this component is always placed on a Pawn,
    // so just hook into the init‐state system.
    RegisterInitStateFeature();
}

bool URCHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
    check(Manager);

    APawn* Pawn = GetPawn<APawn>();

    if (!CurrentState.IsValid() && DesiredState == RCGameplayTags::InitState_Spawned)
    {
        // As long as we have a real pawn, let us transition
        if (Pawn)
        {
            return true;
        }
    }
    else if (CurrentState == RCGameplayTags::InitState_Spawned && DesiredState == RCGameplayTags::InitState_DataAvailable)
    {
        // The player state is required.
        if (!GetPlayerState<ARCPlayerState>())
        {
            return false;
        }

        // If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
        if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
        {
            AController* Controller = GetController<AController>();

            const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
                (Controller->PlayerState != nullptr) && \
                (Controller->PlayerState->GetOwner() == Controller);

            if (!bHasControllerPairedWithPS)
            {
                return false;
            }
        }

        const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
        const bool bIsBot = Pawn->IsBotControlled();

        if (bIsLocallyControlled && !bIsBot)
        {
            ARCPlayerController* RCPC = GetController<ARCPlayerController>();

            // The input component and local player is required when locally controlled.
            if (!Pawn->InputComponent || !RCPC || !RCPC->GetLocalPlayer())
            {
                return false;
            }
        }

        return true;
    }
    else if (CurrentState == RCGameplayTags::InitState_DataAvailable && DesiredState == RCGameplayTags::InitState_DataInitialized)
    {
        // Wait for player state and extension component
        ARCPlayerState* RCPS = GetPlayerState<ARCPlayerState>();

        return RCPS && Manager->HasFeatureReachedInitState(Pawn, URCPawnExtensionComponent::NAME_ActorFeatureName, RCGameplayTags::InitState_DataInitialized);
    }
    else if (CurrentState == RCGameplayTags::InitState_DataInitialized && DesiredState == RCGameplayTags::InitState_GameplayReady)
    {
        // TODO add ability initialization checks?
        return true;
    }

    return false;
}

void URCHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
    if (CurrentState == RCGameplayTags::InitState_DataAvailable && DesiredState == RCGameplayTags::InitState_DataInitialized)
    {
        APawn* Pawn = GetPawn<APawn>();
        ARCPlayerState* RCPS = GetPlayerState<ARCPlayerState>();
        if (!ensure(Pawn && RCPS))
        {
            return;
        }

        const URCPawnData* PawnData = nullptr;

        if (URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            PawnData = PawnExtComp->GetPawnData<URCPawnData>();

            // The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
            // The ability system component and attribute sets live on the player state.
            PawnExtComp->InitializeAbilitySystem(RCPS->GetRCAbilitySystemComponent(), RCPS);
        }

        if (ARCPlayerController* RCPC = GetController<ARCPlayerController>())
        {
            if (Pawn->InputComponent != nullptr)
            {
                InitializePlayerInput(Pawn->InputComponent);
            }
        }

    }
}

void URCHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    if (Params.FeatureName == URCPawnExtensionComponent::NAME_ActorFeatureName)
    {
        if (Params.FeatureState == RCGameplayTags::InitState_DataInitialized)
        {
            // If the extension component says all all other components are initialized, try to progress to next state
            CheckDefaultInitialization();
        }
    }
}

void URCHeroComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> StateChain = { RCGameplayTags::InitState_Spawned, RCGameplayTags::InitState_DataAvailable, RCGameplayTags::InitState_DataInitialized, RCGameplayTags::InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain(StateChain);
}

void URCHeroComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for when the pawn extension component changes init state
    BindOnActorInitStateChanged(URCPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

    // Notifies that we are done spawning, then try the rest of initialization
    ensure(TryToChangeInitState(RCGameplayTags::InitState_Spawned));
    CheckDefaultInitialization();
}

void URCHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterInitStateFeature();

    Super::EndPlay(EndPlayReason);
}


void URCHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (const URCPawnData* PawnData = PawnExtComp->GetPawnData<URCPawnData>())
        {
            if (const URCInputConfig* InputConfig = PawnData->InputConfig)
            {
                // The RC Input Component has some additional functions to map Gameplay Tags to an Input Action.
                // If you want this functionality but still want to change your input component class, make it a subclass
                // of the URCInputComponent or modify this component accordingly.
                URCInputComponent* RCIC = Cast<URCInputComponent>(PlayerInputComponent);
                if (ensureMsgf(RCIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to URCInputComponent or a subclass of it.")))
                {

                    // This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
                    // be triggered directly by these input actions Triggered events.
                    TArray<uint32> BindHandles;
                    RCIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
                }
            }
        }
    }

    if (ensure(!bReadyToBindInputs))
    {
        bReadyToBindInputs = true;
    }
 
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}


void URCHeroComponent::AddAdditionalInputConfig(const URCInputConfig* InputConfig)
{
    TArray<uint32> BindHandles;

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }
    
    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    const ULocalPlayer* LP = PC->GetLocalPlayer();
    check(LP);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    check(Subsystem);

    if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        URCInputComponent* RCIC = Pawn->FindComponentByClass<URCInputComponent>();
        if (ensureMsgf(RCIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to URCInputComponent or a subclass of it.")))
        {
            RCIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
        }
    }
}

void URCHeroComponent::RemoveAdditionalInputConfig(const URCInputConfig* InputConfig)
{
    //@TODO: Implement me!
}

bool URCHeroComponent::IsReadyToBindInputs() const
{
    return bReadyToBindInputs;
}

void URCHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (const APawn* Pawn = GetPawn<APawn>())
    {
        if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            if (URCAbilitySystemComponent* RCASC = PawnExtComp->GetRCAbilitySystemComponent())
            {
                RCASC->AbilityInputTagPressed(InputTag);
            }
        }
    }
}

void URCHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (URCAbilitySystemComponent* RCASC = PawnExtComp->GetRCAbilitySystemComponent())
        {
            RCASC->AbilityInputTagReleased(InputTag);
        }
    }
}
