// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameState.h"

#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/Experience/RCExperienceManagerComponent.h"
#include "Messages/RCVerbMessage.h"
#include "Player/RCPlayerState.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include "RCLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameState)

class APlayerState;
class FLifetimeProperty;

extern ENGINE_API float GAverageFPS;

ARCGameState::ARCGameState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<URCAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    ExperienceManagerComponent = CreateDefaultSubobject<URCExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
    
    // create the component just like Lyra does
    SpawnManager = ObjectInitializer.CreateDefaultSubobject<URCPlayerSpawningManagerComponent>(this, TEXT("SpawnManager"));

    ServerFPS = 0.0f;
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

void ARCGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ARCGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
}

void ARCGameState::RemovePlayerState(APlayerState* PlayerState)
{
    //@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
    // Need to at least comment the engine code, and possibly move things around
    Super::RemovePlayerState(PlayerState);
}

void ARCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, ServerFPS);
}

void ARCGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (GetLocalRole() == ROLE_Authority)
    {
        ServerFPS = GAverageFPS;
    }
}


void ARCGameState::MulticastMessageToClients_Implementation(const FRCVerbMessage Message)
{
    if (GetNetMode() == NM_Client)
    {
        UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
    }
}

void ARCGameState::MulticastReliableMessageToClients_Implementation(const FRCVerbMessage Message)
{
    MulticastMessageToClients_Implementation(Message);
}

float ARCGameState::GetServerFPS() const
{
    return ServerFPS;
}
