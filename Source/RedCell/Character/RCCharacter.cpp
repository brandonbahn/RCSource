// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/RCCharacter.h"
#include "Player/RCPlayerState.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCHealthComponent.h"
#include "AbilitySystem/RCAbilitySet.h"

ARCCharacter::ARCCharacter(const FObjectInitializer& ObjInit)
  : Super(ObjInit)
{
  HealthComponent = ObjInit.CreateDefaultSubobject<URCHealthComponent>(this, TEXT("RCHealthComponent"));
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

    // Grant default set of abilities & init-effects (Death, Reset, etc)
    if (DefaultAbilitySet)
    {
      ASC->AddAbilitySet(DefaultAbilitySet);
    }

    // Seed any UI with the current health values
    const float Curr = HealthComponent->GetHealth();
    const float Max  = HealthComponent->GetMaxHealth();
    HealthComponent->OnMaxHealthChanged.Broadcast(HealthComponent, Max, Max, nullptr);
    HealthComponent->OnHealthChanged  .Broadcast(HealthComponent, Curr, Curr, nullptr);
  }
}

void ARCCharacter::PossessedBy(AController* NewController)
{
  Super::PossessedBy(NewController);
  SetOwner(NewController);

  // Same wiring for server‐side Possession
  if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
  {
    ASC->InitAbilityActorInfo(CastChecked<ARCPlayerState>(GetPlayerState()), this);
    HealthComponent->InitializeWithAbilitySystem(ASC);
    if (DefaultAbilitySet)
    {
      ASC->AddAbilitySet(DefaultAbilitySet);
    }
  }
  OnAbilitySystemInitialized();
}

void ARCCharacter::OnRep_PlayerState()
{
  Super::OnRep_PlayerState();
  SetOwner(GetController());

  // Same wiring for client clones
  if (URCAbilitySystemComponent* ASC = GetRCAbilitySystemComponent())
  {
    ASC->InitAbilityActorInfo(CastChecked<ARCPlayerState>(GetPlayerState()), this);
    HealthComponent->InitializeWithAbilitySystem(ASC);
    if (DefaultAbilitySet)
    {
      ASC->AddAbilitySet(DefaultAbilitySet);
    }

    // Re‐seed UI on clients
    const float Curr = HealthComponent->GetHealth();
    const float Max  = HealthComponent->GetMaxHealth();
    HealthComponent->OnMaxHealthChanged.Broadcast(HealthComponent, Max, Max, nullptr);
    HealthComponent->OnHealthChanged  .Broadcast(HealthComponent, Curr, Curr, nullptr);
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
