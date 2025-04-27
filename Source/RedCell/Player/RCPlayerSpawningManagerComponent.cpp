// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RCPlayerSpawningManagerComponent.h"
#include "GameModes/RCGameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Level.h"
#include "Engine/PlayerStartPIE.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerSpawningManagerComponent)

URCPlayerSpawningManagerComponent::URCPlayerSpawningManagerComponent(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    // This component is for server only, non-replicated
    SetIsReplicatedByDefault(false);
    bAutoRegister = true;
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void URCPlayerSpawningManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();

    // Listen for levels being added so we can cache their PlayerStarts
    FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

    if (UWorld* World = GetWorld())
    {
        // Also detect newly spawned PlayerStarts
        World->AddOnActorSpawnedHandler(
            FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned)
        );

        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            CachedPlayerStarts.Add(*It);
        }
    }
}

void URCPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
    if (InWorld == GetWorld())
    {
        for (AActor* Actor : InLevel->Actors)
        {
            if (APlayerStart* PS = Cast<APlayerStart>(Actor))
            {
                CachedPlayerStarts.Add(PS);
            }
        }
    }
}

void URCPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
    if (APlayerStart* PS = Cast<APlayerStart>(SpawnedActor))
    {
        CachedPlayerStarts.Add(PS);
    }
}

AActor* URCPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
    if (!Player)
    {
        return nullptr;
    }

#if WITH_EDITOR
    if (APlayerStart* PIEStart = FindPlayFromHereStart(Player))
    {
        return PIEStart;
    }
#endif

    // Gather valid start points
    TArray<APlayerStart*> Starts;
    for (auto& WeakPS : CachedPlayerStarts)
    {
        if (APlayerStart* PS = WeakPS.Get())
        {
            Starts.Add(PS);
        }
    }

    // Spectators can spawn anywhere
    if (APlayerState* PS = Player->GetPlayerState<APlayerState>())
    {
        if (PS->IsOnlyASpectator())
        {
            return Starts.Num() ? Starts[FMath::RandRange(0, Starts.Num() - 1)] : nullptr;
        }
    }

    // Ask subclass / Blueprint
    AActor* Chosen = OnChoosePlayerStart(Player, Starts);
    if (!Chosen)
    {
        Chosen = GetFirstRandomUnoccupiedPlayerStart(Player, Starts);
    }

    return Chosen;
}

bool URCPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player) const
{
    // Simple always-allow
    return true;
}

void URCPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
    K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

void URCPlayerSpawningManagerComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* URCPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(
    AController* Controller,
    const TArray<APlayerStart*>& Starts) const
{
    // Basic random selection
    return Starts.Num() ?
        Starts[FMath::RandRange(0, Starts.Num() - 1)] : nullptr;
}

#if WITH_EDITOR
APlayerStart* URCPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player) const
{
    if (Player->IsA<APlayerController>())
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<APlayerStart> It(World); It; ++It)
            {
                if (APlayerStart* PS = *It)
                {
                    if (PS->IsA<APlayerStartPIE>())
                    {
                        return PS;
                    }
                }
            }
        }
    }
    return nullptr;
}
#endif

