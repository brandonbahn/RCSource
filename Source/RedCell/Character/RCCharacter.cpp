// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.cpp
// Fill out your copyright notice in the Description page of Project Settings.
#include "RCCharacter.h"

#include "Net/UnrealNetwork.h"

#include "Player/RCPlayerState.h"
#include "Player/RCPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCHealthComponent.h"
#include "Character/RCCoreComponent.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCMovementModes.h"
#include "RCGameplayTags.h"
#include "AbilitySystem/RCAbilitySet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCharacter)

ARCCharacter::ARCCharacter(const FObjectInitializer& ObjInit)
  : Super(ObjInit)
{
  
  PawnExtensionComponent = ObjInit.CreateDefaultSubobject<URCPawnExtensionComponent>(this, TEXT("PawnExtensionComponent"));
  PawnExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
  PawnExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
    
  HealthComponent = ObjInit.CreateDefaultSubobject<URCHealthComponent>(this, TEXT("RCHealthComponent"));
  HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
  HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
    
  HeroComponent = ObjInit.CreateDefaultSubobject<URCHeroComponent>(this, TEXT("RCHeroComponent"));
    
  CoreComponent = ObjInit.CreateDefaultSubobject<URCCoreComponent>(this, TEXT("RCCoreComponent"));
}

void ARCCharacter::BeginPlay()
{
  Super::BeginPlay();

  // Only do this once, on either server or standalone client
  if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
  {
      // OwnerActor = PlayerState, AvatarActor = this Character
      ASC->InitAbilityActorInfo(CastChecked<ARCPlayerState>(GetPlayerState()), this);
      
      HealthComponent->InitializeWithAbilitySystem(ASC);
      {
         const float Curr = HealthComponent->GetHealth();
         const float Max  = HealthComponent->GetMaxHealth();
         HealthComponent->OnMaxHealthChanged.Broadcast(HealthComponent, Max, Max, nullptr);
         HealthComponent->OnHealthChanged  .Broadcast(HealthComponent, Curr, Curr, nullptr);
      }
      
      CoreComponent->InitializeWithAbilitySystem(ASC);
      {
         const float Curr = CoreComponent->GetMana();
         const float Max  = CoreComponent->GetMaxMana();
         CoreComponent->OnMaxManaChanged.Broadcast(CoreComponent, Max, Max, nullptr);
         CoreComponent->OnManaChanged  .Broadcast(CoreComponent, Curr, Curr, nullptr);
      }
      
    // Grant default set of abilities & init-effects (Death, Reset, etc)
    //if (DefaultAbilitySet)
    //{
    //  ASC->AddAbilitySet(DefaultAbilitySet);
    //}

    // Seed any UI with the current health values
    
  }
}

void ARCCharacter:: PossessedBy(AController* NewController)
{
  Super::PossessedBy(NewController);
  SetOwner(NewController);
    if (PawnExtensionComponent)
    {
        // Log what the actor‐level PawnDataAsset is before we pass it in:
        UE_LOG(LogTemp, Warning, TEXT("PossessedBy: PawnDataAsset = %s"), *GetNameSafe(PawnDataAsset));
        PawnExtensionComponent->SetPawnData(PawnDataAsset);
    }
    
  // Same wiring for server‐side Possession
  if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
  {
    ASC->InitAbilityActorInfo(CastChecked<ARCPlayerState>(GetPlayerState()), this);
    HealthComponent->InitializeWithAbilitySystem(ASC);
    //if (DefaultAbilitySet)
    //{
    //  ASC->AddAbilitySet(DefaultAbilitySet);
    //}
  }
     
  OnAbilitySystemInitialized();
}

void ARCCharacter::UnPossessed()
{
    // Call parent to handle default unpossess behavior
    Super::UnPossessed();

    // Notify the pawn extension component of possession change
    if (PawnExtensionComponent)
    {
        PawnExtensionComponent->HandleControllerChanged();
    }

    // Reset team to default (everyone on team 0)
    MyTeamID = 0;
}


void ARCCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    PawnExtensionComponent->HandleControllerChanged();
}

void ARCCharacter::OnRep_PlayerState()
{
  Super::OnRep_PlayerState();
  SetOwner(GetController());

    if (PawnExtensionComponent)
    {
        PawnExtensionComponent->SetPawnData(PawnDataAsset);
    }
    
  // Same wiring for client clones
  if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
  {
    ASC->InitAbilityActorInfo(CastChecked<ARCPlayerState>(GetPlayerState()), this);
      
      HealthComponent->InitializeWithAbilitySystem(ASC);
      {
         const float Curr = HealthComponent->GetHealth();
         const float Max  = HealthComponent->GetMaxHealth();
         HealthComponent->OnMaxHealthChanged.Broadcast(HealthComponent, Max, Max, nullptr);
         HealthComponent->OnHealthChanged  .Broadcast(HealthComponent, Curr, Curr, nullptr);
      }
      
      CoreComponent->InitializeWithAbilitySystem(ASC);
      {
         const float Curr = CoreComponent->GetMana();
         const float Max  = CoreComponent->GetMaxMana();
         CoreComponent->OnMaxManaChanged.Broadcast(CoreComponent, Max, Max, nullptr);
         CoreComponent->OnManaChanged  .Broadcast(CoreComponent, Curr, Curr, nullptr);
      }
  }

  OnAbilitySystemInitialized();
}

UAbilitySystemComponent* ARCCharacter::GetAbilitySystemComponent() const
{
  // IAbilitySystemInterface requires this
  return GetRCAbilitySystemComponent();
}

URCAbilitySystemComponent* ARCCharacter::GetRCAbilitySystemComponent() const
{
  // Grab your PlayerState’s ASC and cast to your custom subclass
  if (const ARCPlayerState* PS = Cast<ARCPlayerState>(GetPlayerState()))
  {
    return Cast<URCAbilitySystemComponent>(PS->GetAbilitySystemComponent());
  }
  return nullptr;
}



/* added from RCCharacter*/
ARCPlayerController* ARCCharacter::GetRCPlayerController() const
{
    return CastChecked<ARCPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ARCPlayerState* ARCCharacter::GetRCPlayerState() const
{
    return CastChecked<ARCPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

void ARCCharacter::OnAbilitySystemInitialized()
{
    URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponent();
    check(RCASC);

    HealthComponent->InitializeWithAbilitySystem(RCASC);

    InitializeGameplayTags();
}

void ARCCharacter::OnAbilitySystemUninitialized()
{
    HealthComponent->UninitializeFromAbilitySystem();
}


// This connects our new PlayerInputConfigs //
void ARCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PawnExtensionComponent->SetupPlayerInputComponent();
}



void ARCCharacter::InitializeGameplayTags()
{
    if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
    {
        // 1) Clear any leftover tags
        for (auto& Pair : RCGameplayTags::MovementModeTagMap)
        {
            ASC->SetLooseGameplayTagCount(Pair.Value, 0);
        }
        for (auto& Pair : RCGameplayTags::CustomMovementModeTagMap)
        {
            ASC->SetLooseGameplayTagCount(Pair.Value, 0);
        }

        /*
        // 2) Bind to MovementModeChanged (no C++ sub‐class required)
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MovementModeChangedDelegate.AddDynamic(
                this, &ARCCharacter::OnMovementModeChanged);

            // 3) Seed the very first mode tag
            OnMovementModeChanged(this, MoveComp->MovementMode, MoveComp->CustomMovementMode);
        }
         */
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

//Function to receive DABP movement enums and tags for enums in Source code//
void ARCCharacter::NotifyAllMovementTags(
    E_MovementMode      NewMode,
    E_Gait              NewGait,
    E_MovementState     NewState,
    E_RotationMode      NewRotation,
    E_Stance            NewStance,
    E_MovementDirection NewDir)
{
    if (auto* ASC = GetRCAbilitySystemComponent())
    {
        // Clear every map
        for (auto& P : RCGameplayTags::MoveModeTagMap)         ASC->SetLooseGameplayTagCount(P.Value, 0);
        for (auto& P : RCGameplayTags::GaitTagMap)                 ASC->SetLooseGameplayTagCount(P.Value, 0);
        for (auto& P : RCGameplayTags::MovementStateTagMap)        ASC->SetLooseGameplayTagCount(P.Value, 0);
        for (auto& P : RCGameplayTags::RotationModeTagMap)         ASC->SetLooseGameplayTagCount(P.Value, 0);
        for (auto& P : RCGameplayTags::StanceTagMap)               ASC->SetLooseGameplayTagCount(P.Value, 0);
        for (auto& P : RCGameplayTags::MovementDirectionTagMap)    ASC->SetLooseGameplayTagCount(P.Value, 0);

        // Set the ones you care about now:
        if (auto* T = RCGameplayTags::MoveModeTagMap.Find(NewMode))       ASC->SetLooseGameplayTagCount(*T, 1);
        if (auto* T = RCGameplayTags::GaitTagMap.Find(NewGait))               ASC->SetLooseGameplayTagCount(*T, 1);
        if (auto* T = RCGameplayTags::MovementStateTagMap.Find(NewState))     ASC->SetLooseGameplayTagCount(*T, 1);
        if (auto* T = RCGameplayTags::RotationModeTagMap.Find(NewRotation))   ASC->SetLooseGameplayTagCount(*T, 1);
        if (auto* T = RCGameplayTags::StanceTagMap.Find(NewStance))           ASC->SetLooseGameplayTagCount(*T, 1);
        if (auto* T = RCGameplayTags::MovementDirectionTagMap.Find(NewDir))   ASC->SetLooseGameplayTagCount(*T, 1);
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
    
    // Disable all movement on the CharacterMovementComponent
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }
    
    // Ensure jumping is stopped
    StopJumping();

    // Disable collisions if desired (uncomment if needed)
    // if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    // {
    //     CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //     CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    // }

    /*
    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    check(CapsuleComp);
    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

    URCCharacterMovementComponent* RCMoveComp = CastChecked<URCCharacterMovementComponent>(GetCharacterMovement());
    RCMoveComp->StopMovementImmediately();
    RCMoveComp->DisableMovement();
     */
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
