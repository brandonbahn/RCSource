// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.cpp
// Fill out your copyright notice in the Description page of Project Settings.
#include "RCCharacter.h"

#include "Net/UnrealNetwork.h"

#include "Player/RCPlayerState.h"
#include "Player/RCPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponent.h"
#include "AnimationWarpingLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "MotionWarpingComponent.h"
#include "KismetAnimationLibrary.h"
#include "RCCharacterMovementComponent.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCHealthComponent.h"
#include "Character/RCCoreComponent.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCMovementModes.h"
#include "RCGameplayTags.h"
#include "RCLogChannels.h"
#include "RCPreMovementTickComponent.h"
#include "System/RCSignificanceManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Traversal/RCTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCharacter)

class AActor;
class FLifeSpanComponent;
class IRepChangedPropertyTracker;
class UInputComponent;

ARCCharacter::ARCCharacter(const FObjectInitializer& ObjInit)
  : Super(ObjInit.SetDefaultSubobjectClass<URCCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;

    UCapsuleComponent* CharacterCapsuleComponent = GetCapsuleComponent();
    CharacterCapsuleComponent->SetCapsuleHalfHeight(86.0f);
    CharacterCapsuleComponent->SetCapsuleRadius(30.0f);
    CharacterCapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    CharacterCapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    USkeletalMeshComponent* CharacterMeshComponent = GetMesh();
    CharacterMeshComponent->SetRelativeTransform(FTransform(
        FRotator(0.0f, -90.0f, 0.0f).Quaternion(),
        FVector(0.0f, 0.0f, -88.0f),
        FVector::OneVector));
    CharacterMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    URCCharacterMovementComponent* RCMoveComp = CastChecked<URCCharacterMovementComponent>(GetCharacterMovement());
    RCMoveComp->GravityScale = 1.0f;
    RCMoveComp->MaxAcceleration = 800.0f;
    RCMoveComp->BrakingFrictionFactor = 1.0f;
    RCMoveComp->GroundFriction = 5.0f;
    RCMoveComp->MaxWalkSpeed = 500.0f;
    RCMoveComp->MinAnalogWalkSpeed = 150.0f;
    RCMoveComp->BrakingDecelerationWalking = 1500.0f;
    RCMoveComp->bCanWalkOffLedgesWhenCrouching = true;
    RCMoveComp->PerchRadiusThreshold = 20.0f;
    RCMoveComp->bUseFlatBaseForFloorChecks = true;
    RCMoveComp->JumpZVelocity = 500.0f;
    RCMoveComp->AirControl = 0.25f;
    RCMoveComp->RotationRate = FRotator(0.0f, -1.0f, 0.0f);
    RCMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
    RCMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
    RCMoveComp->SetCrouchedHalfHeight(60.0f);
    RCMoveComp->bUseControllerDesiredRotation = true;
	
    PreMovementTickComponent = CreateDefaultSubobject<URCPreMovementTickComponent>(TEXT("PreCMCTick"));
    TraversalComponent = CreateDefaultSubobject<URCTraversalComponent>(TEXT("TraversalComponent"));
    MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
    SpringArm->SetRelativeTransform(FTransform(
        FRotator(0.0f, 0.0f, 0.0f).Quaternion(),
        FVector(0.0f, 0.0f, 20.0f),
        FVector::OneVector));
    SpringArm->ProbeSize = 0.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagMaxDistance = 200.0f;
    SpringArm->SetupAttachment(RootComponent);

    GameplayCamera = ObjInit.CreateDefaultSubobject<UGameplayCameraComponent>(this, TEXT("GameplayCameraComponent"));
    GameplayCamera->SetupAttachment(GetMesh());
    GameplayCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    GameplayCamera->SetRelativeRotation(FRotator(0.0f, 90.0, 0.0f));
    
    PawnExtensionComponent = ObjInit.CreateDefaultSubobject<URCPawnExtensionComponent>(this, TEXT("PawnExtensionComponent"));
    PawnExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
    PawnExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
    
    HealthComponent = ObjInit.CreateDefaultSubobject<URCHealthComponent>(this, TEXT("RCHealthComponent"));
    HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
    HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
    
    CoreComponent = ObjInit.CreateDefaultSubobject<URCCoreComponent>(this, TEXT("RCCoreComponent"));

    BaseEyeHeight = 100.0f;
    CrouchedEyeHeight = 32.0f;
}

void ARCCharacter::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void ARCCharacter::BeginPlay()
{
  Super::BeginPlay();

    UWorld* World = GetWorld();

    const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
    if (bRegisterWithSignificanceManager)
    {
        if (URCSignificanceManager* SignificanceManager = USignificanceManager::Get<URCSignificanceManager>(World))
        {
            //@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
        }
    }

    // Setup movement event handling for simulated proxies
    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        if (URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement())
        {
            OnCharacterMovementUpdated.AddDynamic(this, &ARCCharacter::OnCharacterMovementUpdated_Event);
            UE_LOG(LogRC, Log, TEXT("Simulated Proxy OnMovementUpdated Delegate Bound Successfully!"));
        }
    }
}

void ARCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    UWorld* World = GetWorld();

    const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
    if (bRegisterWithSignificanceManager)
    {
        if (URCSignificanceManager* SignificanceManager = USignificanceManager::Get<URCSignificanceManager>(World))
        {
            SignificanceManager->UnregisterObject(this);
        }
    }
}

void ARCCharacter::Reset()
{
    Super::Reset();
}

void ARCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ARCCharacter, CharacterInputState, COND_SkipOwner);
    DOREPLIFETIME(ARCCharacter, CurrentMovementInputMagnitude);
    DOREPLIFETIME(ARCCharacter, CurrentMovementInputVector);
    DOREPLIFETIME(ThisClass, MyTeamID)
}

void ARCCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);
}

void ARCCharacter::NotifyControllerChanged()
{
    const FGenericTeamId OldTeamId = GetGenericTeamId();

    Super::NotifyControllerChanged();

    // Update our team ID based on the controller
    if (HasAuthority() && (Controller != nullptr))
    {
        if (IRCTeamAgentInterface* ControllerWithTeam = Cast<IRCTeamAgentInterface>(Controller))
        {
            MyTeamID = ControllerWithTeam->GetGenericTeamId();
            ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
        }
    }
}

ARCPlayerController* ARCCharacter::GetRCPlayerController() const
{
    return CastChecked<ARCPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ARCPlayerState* ARCCharacter::GetRCPlayerState() const
{
    return CastChecked<ARCPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}


URCAbilitySystemComponent* ARCCharacter::GetRCAbilitySystemComponent() const
{
    return Cast<URCAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ARCCharacter::GetAbilitySystemComponent() const
{
    if (PawnExtensionComponent == nullptr)
    {
        return nullptr;
    }

    return PawnExtensionComponent->GetRCAbilitySystemComponent();
}

void ARCCharacter::OnAbilitySystemInitialized()
{
    URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent();
    check(RCASC);
    
    HealthComponent->InitializeWithAbilitySystem(RCASC);
    CoreComponent->InitializeWithAbilitySystem(RCASC);
    
    InitializeGameplayTags();
}

void ARCCharacter::OnAbilitySystemUninitialized()
{
    HealthComponent->UninitializeFromAbilitySystem();
    CoreComponent->UninitializeFromAbilitySystem();
}

void ARCCharacter:: PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    PawnExtensionComponent->HandleControllerChanged();
}

void ARCCharacter::UnPossessed()
{
    Super::UnPossessed();
    PawnExtensionComponent->HandleControllerChanged();
}

void ARCCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    PawnExtensionComponent->HandleControllerChanged();
}

void ARCCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    PawnExtensionComponent->HandlePlayerStateReplicated();
}

void ARCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PawnExtensionComponent->SetupPlayerInputComponent();
}

void ARCCharacter::InitializeGameplayTags()
{
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        // 1) Clear any leftover tags for EMovementMode
        for (const TPair<uint8, FGameplayTag>& TagMapping : RCGameplayTags::MovementModeTagMap)
        {
            if (TagMapping.Value.IsValid())
            {
                RCASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
            }
        }

        for (const TPair<uint8, FGameplayTag>& TagMapping : RCGameplayTags::CustomMovementModeTagMap)
        {
            if (TagMapping.Value.IsValid())
            {
                RCASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
            }
        }

        // 2) Clear any leftover tags for MovementState
        auto ClearTagMap = [RCASC](const auto& TagMap) 
        {
            for (const auto& Pair : TagMap)
            {
                RCASC->SetLooseGameplayTagCount(Pair.Value, 0);
            }
        };
        
        ClearTagMap(RCGameplayTags::MovementSituationTagMap);
        ClearTagMap(RCGameplayTags::GaitTagMap);
        ClearTagMap(RCGameplayTags::MovementStateTagMap);
        ClearTagMap(RCGameplayTags::RotationModeTagMap);
        ClearTagMap(RCGameplayTags::StanceTagMap);
        ClearTagMap(RCGameplayTags::MovementDirectionTagMap);

        URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement();
        if (RCMoveComp)
        {
            SetMovementModeTag(RCMoveComp->MovementMode, RCMoveComp->CustomMovementMode, true);
            
            // 4) Set initial movement state tags using your movement component
            SetMovementStateTags(
                E_MovementSituation::OnGround,           // Start on ground
                RCMoveComp->GetDesiredGait(),            // Get current gait from movement component
                E_MovementState::Idle,                   // Start idle
                E_RotationMode::Strafe,                  // Default rotation mode
                E_Stance::Stand,                         // Start standing
                E_MovementDirection::None                // No movement initially
            );

            // Set strafe input as default
            RCASC->SetLooseGameplayTagCount(RCGameplayTags::InputState_WantsToStrafe, 1);
        }
    }
}

void ARCCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    if (const URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        RCASC->GetOwnedGameplayTags(TagContainer);
    }
}

bool ARCCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
    if (const URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        return RCASC->HasMatchingGameplayTag(TagToCheck);
    }

    return false;
}

bool ARCCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
    if (const URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        return RCASC->HasAllMatchingGameplayTags(TagContainer);
    }

    return false;
}

bool ARCCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
    if (const URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        return RCASC->HasAnyMatchingGameplayTags(TagContainer);
    }

    return false;
}

void ARCCharacter::PreCMCTick()
{
    // Update input state on server
    if (HasAuthority())
    {
        UpdateCharacterInputState();
    }
    UpdateRotation_PreCMC();
    UpdateMovement_PreCMC();
}

void ARCCharacter::UpdateRotation_PreCMC()
{
    if (URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement())
    {
        // Read input state tags for rotation decisions
        bool bWantsToStrafe = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToStrafe);
        bool bWantsToAim = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToAim);
        
        // Use controller rotation if character wants to strafe OR aim
        bool bShouldUseControllerRotation = bWantsToStrafe || bWantsToAim;
        
        RCMoveComp->bUseControllerDesiredRotation = bShouldUseControllerRotation;
        RCMoveComp->bOrientRotationToMovement = !bShouldUseControllerRotation;
        
        // Set rotation rate based on strafe/aim state and falling state
        // TODO: Get these values from PawnData instead of hardcoded
        FRotator OrientedMovementFallingRotationRate = FRotator(0.0f, 200.0f, 0.0f);
        FRotator OrientedMovementNotFallingRotationRate = FRotator(0.0f, -1.0f, 0.0f);
        FRotator StrafeFallingRotationRate = FRotator(0.0f, -1.0f, 0.0f);
        FRotator StrafeNotFallingRotationRate = FRotator(0.0f, -1.0f, 0.0f);
        
        RCMoveComp->RotationRate = RCMoveComp->IsFalling() ?
            (bShouldUseControllerRotation ? StrafeFallingRotationRate : OrientedMovementFallingRotationRate) :
            (bShouldUseControllerRotation ? StrafeNotFallingRotationRate : OrientedMovementNotFallingRotationRate);
        
        // Set movement tags for rotation mode (for animation)
        if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
        {
            E_RotationMode RotationMode = bShouldUseControllerRotation ? 
                E_RotationMode::Strafe : E_RotationMode::OrientToMovement;
            
            // Clear rotation mode tags and set current one
            for (const auto& [RotEnum, RotTag] : RCGameplayTags::RotationModeTagMap)
            {
                RCASC->SetLooseGameplayTagCount(RotTag, 0);
            }
            if (const FGameplayTag* CurrentRotTag = RCGameplayTags::RotationModeTagMap.Find(RotationMode))
            {
                RCASC->SetLooseGameplayTagCount(*CurrentRotTag, 1);
            }
        }
    }
}

void ARCCharacter::UpdateMovement_PreCMC()
{
    if (URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement())
    {
        EGait = RCMoveComp->GetDesiredGait();
        
        // Get the desired gait from CMC (which has all the calculation logic)
        E_Gait DesiredGait = RCMoveComp->GetDesiredGait();
        
        // Set the movement state tags using your existing enum map system
        if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
        {
            // Clear all gait tags first
            for (const auto& [GaitEnum, GaitTag] : RCGameplayTags::GaitTagMap)
            {
                RCASC->SetLooseGameplayTagCount(GaitTag, 0);
            }
            
            // Set the current gait tag
            if (const FGameplayTag* CurrentGaitTag = RCGameplayTags::GaitTagMap.Find(DesiredGait))
            {
                RCASC->SetLooseGameplayTagCount(*CurrentGaitTag, 1);
            }
            
            // Set movement state (Idle vs Moving)
            bool bIsMoving = CurrentMovementInputMagnitude > 0.1f;
            E_MovementState MovementState = bIsMoving ? E_MovementState::Moving : E_MovementState::Idle;
            
            for (const auto& [StateEnum, StateTag] : RCGameplayTags::MovementStateTagMap)
            {
                RCASC->SetLooseGameplayTagCount(StateTag, 0);
            }
            if (const FGameplayTag* CurrentStateTag = RCGameplayTags::MovementStateTagMap.Find(MovementState))
            {
                RCASC->SetLooseGameplayTagCount(*CurrentStateTag, 1);
            }
        }
        RCMoveComp->GroundFriction = RCMoveComp->CalculateGroundFriction();
        RCMoveComp->MaxWalkSpeedCrouched = RCMoveComp->CalculateMaxCrouchSpeed();
    }
}

void ARCCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
    HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

void ARCCharacter::OnDeathStarted(AActor*)
{
    DisableMovementAndCollision();
}

void ARCCharacter::OnDeathFinished(AActor*)
{
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

void ARCCharacter::DisableMovementAndCollision()
{
    if (Controller)
    {
        Controller->SetIgnoreMoveInput(true);
        
        // If this is a player controller, also disable input entirely
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            DisableInput(PC);
        }
    }

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    check(CapsuleComp);
    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    
    // Disable all movement on the CharacterMovementComponent
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->StopMovementImmediately();
    MoveComp->DisableMovement();
    
    // Ensure jumping is stopped
    StopJumping();
}

void ARCCharacter::DestroyDueToDeath()
{
    K2_OnDeathFinished();

    UninitAndDestroy();
}


void ARCCharacter::UninitAndDestroy()
{
    if (GetLocalRole() == ROLE_Authority)
    {
        DetachFromControllerPendingDestroy();
        SetLifeSpan(0.1f);
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        if (RCASC->GetAvatarActor() == this)
        {
            PawnExtensionComponent->UninitializeAbilitySystem();
        }
    }

    SetActorHiddenInGame(true);
}

void ARCCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement();

    SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
    SetMovementModeTag(RCMoveComp->MovementMode, RCMoveComp->CustomMovementMode, true);

    // Update movement situation based on new movement mode
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        E_MovementSituation NewSituation = E_MovementSituation::OnGround;
        
        // Match your ABP logic
        switch (RCMoveComp->MovementMode)
        {
        case MOVE_None:
        case MOVE_Walking:
        case MOVE_NavWalking:
        case MOVE_Flying:
            NewSituation = E_MovementSituation::OnGround;
            break;
            
        case MOVE_Falling:
            NewSituation = E_MovementSituation::InAir;
            break;
            
        default:
            NewSituation = E_MovementSituation::OnGround;
            break;
        }
        
        // Update only the movement situation tag
        for (const auto& [SituationEnum, SituationTag] : RCGameplayTags::MovementSituationTagMap)
        {
            RCASC->SetLooseGameplayTagCount(SituationTag, SituationEnum == NewSituation ? 1 : 0);
        }
    }
}

void ARCCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        const FGameplayTag* MovementModeTag = nullptr;
        if (MovementMode == MOVE_Custom)
        {
            MovementModeTag = RCGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
        }
        else
        {
            MovementModeTag = RCGameplayTags::MovementModeTagMap.Find(MovementMode);
        }
        if (MovementModeTag && MovementModeTag->IsValid())
        {
            RCASC->SetLooseGameplayTagCount(*MovementModeTag, bTagEnabled ? 1 : 0);       
        }    
    }   
}

void ARCCharacter::SetMovementStateTags(
    E_MovementSituation NewSituation,
    E_Gait              NewGait,
    E_MovementState     NewState,
    E_RotationMode      NewRotation,
    E_Stance            NewStance,
    E_MovementDirection NewDirection)
{
    if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
    {
        // Helper lambda to update only if changed
        auto UpdateTagMap = [ASC](const auto& TagMap, auto NewValue) 
        {
            for (const auto& Pair : TagMap)
            {
                ASC->SetLooseGameplayTagCount(Pair.Value, Pair.Key == NewValue ? 1 : 0);
            }
        };
        
        UpdateTagMap(RCGameplayTags::MovementSituationTagMap, NewSituation);   
        UpdateTagMap(RCGameplayTags::GaitTagMap, NewGait);
        UpdateTagMap(RCGameplayTags::MovementStateTagMap, NewState);
        UpdateTagMap(RCGameplayTags::RotationModeTagMap, NewRotation);
        UpdateTagMap(RCGameplayTags::StanceTagMap, NewStance);
        UpdateTagMap(RCGameplayTags::MovementDirectionTagMap, NewDirection);
    }
}

void ARCCharacter::ToggleCrouch()
{
    const URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement();

    if (bIsCrouched || (RCMoveComp && RCMoveComp->bWantsToCrouch))
    {
        UnCrouch();
    }
    else if (RCMoveComp && RCMoveComp->IsMovingOnGround())
    {
        Crouch();
    }
}

void ARCCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        // Set crouch status tag and stance tag
        RCASC->SetLooseGameplayTagCount(RCGameplayTags::StanceTagMap[E_Stance::Crouch], 1);
        RCASC->SetLooseGameplayTagCount(RCGameplayTags::StanceTagMap[E_Stance::Stand], 0);
    }

    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ARCCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    if (URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent())
    {
        // Set stand status tag and clear crouch
        RCASC->SetLooseGameplayTagCount(RCGameplayTags::StanceTagMap[E_Stance::Stand], 1);
        RCASC->SetLooseGameplayTagCount(RCGameplayTags::StanceTagMap[E_Stance::Crouch], 0);
    }

    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool ARCCharacter::CanJumpInternal_Implementation() const
{
    // same as ACharacter's implementation but without the crouch check
    return JumpIsAllowedInternal();
}

void ARCCharacter::UpdateCharacterInputState()
{
    {
        if (!HasAuthority())
            return;
        
        // Update from gameplay tags
        FCharacterInputState NewState;
        NewState.bWantsToSprint = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToSprint);
        NewState.bWantsToWalk = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToWalk);
        NewState.bWantsToStrafe = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToStrafe);
        NewState.bWantsToAim = HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToAim);
    
        // Only update if changed
        if (NewState != CharacterInputState)
        {
            PreviousCharacterInputState = CharacterInputState;
            CharacterInputState = NewState;
        
            // Force net update
            ForceNetUpdate();
        }
    }

}

void ARCCharacter::OnRep_CharacterInputState()
{
    // On clients, update the gameplay tags from the replicated state
    if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
    {
        ASC->SetLooseGameplayTagCount(RCGameplayTags::InputState_WantsToSprint, 
            CharacterInputState.bWantsToSprint ? 1 : 0);
        ASC->SetLooseGameplayTagCount(RCGameplayTags::InputState_WantsToWalk, 
            CharacterInputState.bWantsToWalk ? 1 : 0);
        ASC->SetLooseGameplayTagCount(RCGameplayTags::InputState_WantsToStrafe, 
            CharacterInputState.bWantsToStrafe ? 1 : 0);
        ASC->SetLooseGameplayTagCount(RCGameplayTags::InputState_WantsToAim, 
            CharacterInputState.bWantsToAim ? 1 : 0);
    }
}


void ARCCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    
    float GroundSpeedBeforeJump = GetRCCharacterMovement()->Velocity.Size2D();
    K2_OnJumpedEvent(GroundSpeedBeforeJump);
}

void ARCCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    
    LandVelocity = GetRCCharacterMovement()->Velocity;
    bJustLanded = true;
    K2_OnLandedEvent(LandVelocity);
}

void ARCCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
    const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
    Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation,
                                            TimeDelta);
    UnCrouch();
}

void ARCCharacter::OnCharacterMovementUpdated_Event(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
    UpdatedMovementSimulated(OldVelocity);
}

void ARCCharacter::UpdatedMovementSimulated(FVector OldVelocity)
{
    URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement();
    
    // Check if we transitioned from air to ground or vice versa
    bool bIsMovingOnGround = RCMoveComp->IsMovingOnGround();
    if (bIsMovingOnGround != bWasMovingOnGroundLastFrameSimulated)
    {
        if (bIsMovingOnGround)
        {
            // Just landed
            LandVelocity = OldVelocity;
            bJustLanded = true;
            K2_OnLandedEvent(OldVelocity);
        }
        else
        {
            // Just jumped
            float GroundSpeedBeforeJump = OldVelocity.Size2D();
            K2_OnJumpedEvent(GroundSpeedBeforeJump);
        }
    }
    
    bWasMovingOnGroundLastFrameSimulated = bIsMovingOnGround;
}

void ARCCharacter::PlayAudioEvent_Implementation(const FGameplayTag& Value, float VolumeMultiplier,
    float PitchMultiplier)
{
}


FTraversalCheckInputs ARCCharacter::GetTraversalCheckInputs() const
{
    FTraversalCheckInputs CheckInputs;

    URCCharacterMovementComponent* RCMoveComp = GetRCCharacterMovement();
    if (!RCMoveComp) return CheckInputs;
    
    // Switch on Movement Mode (matching Blueprint logic)
    EMovementMode CurrentMovementMode = RCMoveComp->MovementMode;
    
    switch (CurrentMovementMode)
    {
    case MOVE_None:
    case MOVE_Walking:
    case MOVE_NavWalking:
    case MOVE_Swimming:
    case MOVE_Custom:
        {
            static const FVector2D RangeA = FVector2D(0.0f, 500.0f);
            static const FVector2D RangeB = FVector2D(75.0f, 350.0f);
            CheckInputs.TraceForwardDistance = FMath::GetMappedRangeValueClamped(RangeA, RangeB,
               GetActorRotation().Quaternion().UnrotateVector(GetCharacterMovement()->Velocity).X);
            CheckInputs.TraceRadius = 30.0f;
            CheckInputs.TraceHalfHeight = 60.0f;
            CheckInputs.TraceEndOffset = FVector::ZeroVector;
            break;
        }
    case MOVE_Flying:
    case MOVE_Falling:
        {
            CheckInputs.TraceForwardDistance = 75.0f;
            CheckInputs.TraceRadius = 30.0f;
            CheckInputs.TraceHalfHeight = 86.0f;
            CheckInputs.TraceEndOffset = FVector(0.0f, 0.0f, 50.0f);
            break;
        }
    default:
        {
            UE_LOG(LogRC, Warning, TEXT("Unknown MovementMode in GetTraversalCheckInputs: %d"), (int32)CurrentMovementMode);
            // Handle any other unexpected values
            CheckInputs.TraceForwardDistance = 0.0f;
            CheckInputs.TraceRadius = 0.0f;
            CheckInputs.TraceHalfHeight = 0.0f;
            CheckInputs.TraceEndOffset = FVector::ZeroVector;
            break;
        }
    }
    // Common settings for all modes
    CheckInputs.TraceForwardDirection = GetActorForwardVector();
    CheckInputs.TraceOriginOffset = FVector::ZeroVector;
    
    return CheckInputs;
}

void ARCCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (GetController() == nullptr)
    {
        if (HasAuthority())
        {
            const FGenericTeamId OldTeamID = MyTeamID;
            MyTeamID = NewTeamID;
            ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
        }
        else
        {
            UE_LOG(LogRCTeams, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
        }
    }
    else
    {
        UE_LOG(LogRCTeams, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
    }
}

FGenericTeamId ARCCharacter::GetGenericTeamId() const
{
    return MyTeamID;
}

FOnRCTeamIndexChangedDelegate* ARCCharacter::GetOnTeamIndexChangedDelegate()
{
    return &OnTeamChangedDelegate;
}

void ARCCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    const FGenericTeamId MyOldTeamID = MyTeamID;
    MyTeamID = IntegerToGenericTeamId(NewTeam);
    ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ARCCharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}


FCharacterPropertiesForCamera ARCCharacter::GetCharacterPropertiesForCamera_Implementation() const
{
    FCharacterPropertiesForCamera Properties;
    
    // Get camera style from PawnData
    if (PawnExtensionComponent)
    {
        if (const URCPawnData* PawnData = PawnExtensionComponent->GetPawnData<URCPawnData>())
        {
            Properties.CameraStyle = PawnData->DefaultCameraStyle;
        }
    } 
    
    // Determine camera mode from current state
    if (HasMatchingGameplayTag(RCGameplayTags::InputState_WantsToAim))
    {
        Properties.CameraMode = ECameraMode::Aim;
    }
    else if (GetRCCharacterMovement()->bUseControllerDesiredRotation)
    {
        Properties.CameraMode = ECameraMode::Strafe;
    }
    else
    {
        Properties.CameraMode = ECameraMode::FreeCam;
    }
    
    // Current stance
    Properties.Stance = bIsCrouched ? E_Stance::Crouch : E_Stance::Stand;
    
    return Properties;
}



#if 0
void ARCCharacter::UpdateWarpTargets() const
{
    if (!TraversalComponent)
    {
        UE_LOG(LogRC, Warning, TEXT("UpdateWarpTargets called but there is no TraversalComponent."));
        return;
    }

    // Warp Target Name declarations
    const FTraversalCheckResult& CurrentTraversalResult = TraversalComponent->TraversalResult;
	static const FName FrontLedgeWarpTargetName = TEXT("FrontLedge");
	static const FName BackLedgeWarpTargetName = TEXT("BackLedge");
	static const FName BackFloorWarpTargetName = TEXT("BackFloor");
	static const FName DistanceFromLedgeCurveName = TEXT("Distance_From_Ledge");

	// Update the FrontLedge warp target using the front ledge's location and rotation.
	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FrontLedgeWarpTargetName,
		CurrentTraversalResult.FrontLedgeLocation + FVector::ZAxisVector * 0.5f,
		FRotationMatrix::MakeFromX(-CurrentTraversalResult.FrontLedgeNormal).Rotator());

	float AnimatedDistanceFromFrontLedgeToBackLedge = 0.0f;
	// If the action type was a hurdle or a vault, we need to also update the BackLedge target. If it is not a hurdle or vault, remove it.
	if (CurrentTraversalResult.ActionType == ETraversalActionType::Hurdle ||
		CurrentTraversalResult.ActionType == ETraversalActionType::Vault)
	{
		// Because the traversal animations move at different distances (no fixed metrics), we need to know how far the animation moves in order to warp it properly. Here we cache a curve value at the end of the Back Ledge warp window to determine how far the animation is from the front ledge once the character reaches the back ledge location in the animation.
		TArray<FMotionWarpingWindowData> MotionWarpingWindowData; 
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(CurrentTraversalResult.ChosenMontage,
			BackLedgeWarpTargetName, MotionWarpingWindowData);
		if (!MotionWarpingWindowData.IsEmpty())
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(CurrentTraversalResult.ChosenMontage,
				DistanceFromLedgeCurveName, MotionWarpingWindowData[0].EndTime,
				AnimatedDistanceFromFrontLedgeToBackLedge);

			// Update the BackLedge warp target.
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(BackLedgeWarpTargetName,
				CurrentTraversalResult.BackLedgeLocation, FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(BackLedgeWarpTargetName);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(BackLedgeWarpTargetName);
	}

	// If the action type was a hurdle, we need to also update the BackFloor target. If it is not a hurdle, remove it.
	if (CurrentTraversalResult.ActionType == ETraversalActionType::Hurdle)
	{
		// Caches a curve value at the end of the Back Floor warp window to determine how far the animation is from the front ledge once the character touches the ground.
		TArray<FMotionWarpingWindowData> MotionWarpingWindowData; 
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(CurrentTraversalResult.ChosenMontage,
			BackFloorWarpTargetName, MotionWarpingWindowData);
		if (!MotionWarpingWindowData.IsEmpty())
		{
			float AnimatedDistanceFromFrontLedgeToBackFloor = 0.0f;
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(CurrentTraversalResult.ChosenMontage,
			                                                     DistanceFromLedgeCurveName, MotionWarpingWindowData[0].EndTime,
			                                                     AnimatedDistanceFromFrontLedgeToBackFloor);

			// Since the animations may land on the floor at different distances (a run hurdle may travel further than a walk or stand hurdle), use the total animated distance away from the back ledge as the X and Y values of the BackFloor warp point. This could technically cause some collision issues if the floor is not flat, or there is an obstacle in the way, therefore having fixed metrics for all traversal animations would be an improvement.
			const FVector Vector1 = CurrentTraversalResult.BackLedgeNormal *
				FMath::Abs(AnimatedDistanceFromFrontLedgeToBackLedge - AnimatedDistanceFromFrontLedgeToBackFloor);
			const FVector Vector2 = CurrentTraversalResult.BackLedgeLocation + Vector1;
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(BackFloorWarpTargetName,
				FVector(Vector2.X ,Vector2.Y, CurrentTraversalResult.BackFloorLocation.Z),
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(BackFloorWarpTargetName);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(BackFloorWarpTargetName);
	}
}
#endif