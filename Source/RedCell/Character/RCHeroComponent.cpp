// Fill out your copyright notice in the Description page of Project Settings.

#include "RCHeroComponent.h"
#include "Input/RCInputConfig.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/RCPlayerController.h"
#include "Player/RCPlayerState.h"
#include "Player/RCLocalPlayer.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCCharacter.h"
#include "Input/RCInputComponent.h"
#include "RCGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"
#include "RCLogChannels.h"

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

    	// Cache the character here
    	CachedCharacter = Cast<ARCCharacter>(Pawn);
    	
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
	UE_LOG(LogRC, Error, TEXT("=== InitializePlayerInput called on %s for pawn %s ==="), 
		GetNetMode() == NM_DedicatedServer ? TEXT("SERVER") : TEXT("CLIENT"),
		*GetNameSafe(GetOwner()));
	
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const URCLocalPlayer* LP = Cast<URCLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const URCPawnData* PawnData = PawnExtComp->GetPawnData<URCPawnData>())
		{
			if (const URCInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}
							
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}

				// The RC Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the URCInputComponent or modify this component accordingly.
				URCInputComponent* RCIC = Cast<URCInputComponent>(PlayerInputComponent);
				if (ensureMsgf(RCIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to URCInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					RCIC->AddInputMappings(InputConfig, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					RCIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					UE_LOG(LogRC, Error, TEXT("Bound Move action for client"));
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_MoveWorldSpace, ETriggerEvent::Triggered, this, &ThisClass::Input_MoveWorldSpace, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Look_Gamepad, ETriggerEvent::Triggered, this, &ThisClass::Input_LookGamepad, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);	
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Run, ETriggerEvent::Triggered, this, &ThisClass::Input_Run, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Walk, ETriggerEvent::Triggered, this, &ThisClass::Input_Walk, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Sprint, ETriggerEvent::Triggered, this, &ThisClass::Input_Sprint, /*bLogIfNotFound=*/ false);
					UE_LOG(LogTemp, Error, TEXT("Bound Sprint action for client"));
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Strafe, ETriggerEvent::Triggered, this, &ThisClass::Input_Strafe, /*bLogIfNotFound=*/ false);
					RCIC->BindNativeAction(InputConfig, RCGameplayTags::InputTag_Aim, ETriggerEvent::Triggered, this, &ThisClass::Input_Aim, /*bLogIfNotFound=*/ false);					
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

	UE_LOG(LogRC, Error, TEXT("InitializePlayerInput completed - %s"), 
		   GetNetMode() == NM_DedicatedServer ? TEXT("SERVER") : TEXT("CLIENT"));
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

void URCHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		// Get the input value (equivalent to GASP's Instance.GetValue().Get<FVector2D>())
		FVector2D MoveInput = InputActionValue.Get<FVector2D>();
		float InputMagnitude = MoveInput.Size();

		// Calculate world-space input vector (same logic as GASP)
		const FRotator YawRotation(0.0f, GetController<AController>()->GetControlRotation().Yaw, 0.0f);
		FVector WorldInputVector = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::X) * MoveInput.Y +
								   FRotationMatrix(YawRotation).GetScaledAxis(EAxis::Y) * MoveInput.X;

		
			CachedCharacter->SetMovementInputMagnitude(InputMagnitude);
			CachedCharacter->SetMovementInputVector(WorldInputVector);
		

		// Apply movement input to pawn using InputConfig's movement scale logic
		FVector2D InputScale = MoveInput.GetSafeNormal(); // Default fallback
		
		// Get the InputConfig from PawnData to determine movement scale
		if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (const URCPawnData* PawnData = PawnExtComp->GetPawnData<URCPawnData>())
			{
				if (const URCInputConfig* InputConfig = PawnData->InputConfig)
				{
					InputScale = InputConfig->GetMovementInputScaleValue(MoveInput);
				}
			}
		}
		
		const FRotator Rotator = FRotator(0.0f, GetController<AController>()->GetControlRotation().Yaw, 0.0f);
		Pawn->AddMovementInput(FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y), InputScale.X);
		Pawn->AddMovementInput(Rotator.Vector(), InputScale.Y);

		if (CachedCharacter)
		{
			CachedCharacter->SetMovementInputMagnitude(InputMagnitude);
			CachedCharacter->SetMovementInputVector(WorldInputVector);
        
			// Tell server about movement input
			if (!Pawn->HasAuthority())
			{
				ServerUpdateMovementInput(InputMagnitude, WorldInputVector);
			}
		}
	}
}

void URCHeroComponent::Input_MoveWorldSpace(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		const FVector2D InputScale = InputActionValue.Get<FVector2D>().GetSafeNormal();
		Pawn->AddMovementInput(FVector::RightVector, InputScale.X);
		Pawn->AddMovementInput(FVector::ForwardVector, InputScale.Y);
	}
}

void URCHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		const FVector2D InputScale = InputActionValue.Get<FVector2D>();
		Pawn->AddControllerYawInput(InputScale.X);
		Pawn->AddControllerPitchInput(InputScale.Y);
	}
}

void URCHeroComponent::Input_LookGamepad(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		const FVector2D InputScale = InputActionValue.Get<FVector2D>() * GetWorld()->GetDeltaSeconds();
		Pawn->AddControllerYawInput(InputScale.X);
		Pawn->AddControllerPitchInput(InputScale.Y);
	}
}

void URCHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (ARCCharacter* Character = Cast<ARCCharacter>(GetPawn<APawn>()))
	{
		Character->ToggleCrouch();
	}
}

void URCHeroComponent::Input_Run(const FInputActionValue& InputActionValue)
{
	// Clear walk and sprint to make Run the default
	SetInputStateTag(RCGameplayTags::InputState_WantsToWalk, false);
	SetInputStateTag(RCGameplayTags::InputState_WantsToSprint, false);
}

void URCHeroComponent::Input_Walk(const FInputActionValue& InputActionValue)
{
	// Toggle walk intent (only if not sprinting)
	bool bCurrentlyWantsWalk = HasInputStateTag(RCGameplayTags::InputState_WantsToWalk);
	bool bCurrentlyWantsSprint = HasInputStateTag(RCGameplayTags::InputState_WantsToSprint);
    
	if (!bCurrentlyWantsSprint)
	{
		SetInputStateTag(RCGameplayTags::InputState_WantsToWalk, !bCurrentlyWantsWalk);
	}
}

void URCHeroComponent::Input_Sprint(const FInputActionValue& InputActionValue)
{
	bool bPressed = InputActionValue.Get<bool>();
    
	// Set input intent tag - let Character logic handle validation
	SetInputStateTag(RCGameplayTags::InputState_WantsToSprint, bPressed);
    
	// Clear walk intent when sprinting 
	if (bPressed)
	{
		SetInputStateTag(RCGameplayTags::InputState_WantsToWalk, false);
	}
	if (ARCCharacter* Character = Cast<ARCCharacter>(GetPawn<APawn>()))
	{
		Character->UnCrouch();
	}
}

void URCHeroComponent::Input_Strafe(const FInputActionValue& InputActionValue)
{
	// Toggle strafe intent
	bool bCurrentlyWantsStrafe = HasInputStateTag(RCGameplayTags::InputState_WantsToStrafe);
	SetInputStateTag(RCGameplayTags::InputState_WantsToStrafe, !bCurrentlyWantsStrafe);
}

void URCHeroComponent::Input_Aim(const FInputActionValue& InputActionValue)
{
	bool bPressed = InputActionValue.Get<bool>();
    
	// Set aim intent and enable strafing when aiming
	SetInputStateTag(RCGameplayTags::InputState_WantsToAim, bPressed);
    
	// Aiming automatically enables strafing intent
	if (bPressed)
	{
		SetInputStateTag(RCGameplayTags::InputState_WantsToStrafe, true);
	}
}

void URCHeroComponent::SetInputStateTag(const FGameplayTag& Tag, bool bEnabled)
{
	if (URCAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(Tag, bEnabled ? 1 : 0);
		if (GetOwner() && !GetOwner()->HasAuthority())
		{
			ServerSetInputStateTag(Tag, bEnabled);
		}
	}
}

bool URCHeroComponent::HasInputStateTag(const FGameplayTag& Tag) const
{
	if (URCAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	return false;
}

void URCHeroComponent::ServerSetInputStateTag_Implementation(FGameplayTag Tag, bool bEnabled)
{
	// Server needs to set the tag on the character's ASC, not through HeroComponent
	if (CachedCharacter)
	{
		if (URCAbilitySystemComponent* ASC = CachedCharacter->GetRCAbilitySystemComponent())
		{
			ASC->SetLooseGameplayTagCount(Tag, bEnabled ? 1 : 0);
		}
	}
}

void URCHeroComponent::ServerUpdateMovementInput_Implementation(float Magnitude, FVector InputVector)
{
	if (CachedCharacter)
	{
		CachedCharacter->SetMovementInputMagnitude(Magnitude);
		CachedCharacter->SetMovementInputVector(InputVector);
	}
}

URCAbilitySystemComponent* URCHeroComponent::GetAbilitySystemComponent() const
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			return PawnExtComp->GetRCAbilitySystemComponent();
		}
	}
	return nullptr;
}
