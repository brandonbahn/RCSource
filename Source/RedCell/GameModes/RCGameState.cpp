// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RCGameState.h"

#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Player/RCPlayerState.h"
#include "Player/RCPlayerSpawningManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameState)

class APlayerState;
class FLifetimeProperty;

ARCGameState::ARCGameState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<URCAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    
    // create the component just like Lyra does
    SpawnManager = ObjectInitializer.CreateDefaultSubobject<URCPlayerSpawningManagerComponent>(this, TEXT("SpawnManager"));
}

void ARCGameState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void ARCGameState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}

UAbilitySystemComponent* ARCGameState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ARCGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ARCGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
