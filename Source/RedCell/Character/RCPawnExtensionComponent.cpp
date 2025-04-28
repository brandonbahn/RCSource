// Fill out your copyright notice in the Description page of Project Settings.

#include "RCPawnExtensionComponent.h"

#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "RCGameplayTags.h"
#include "RCPawnData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPawnExtensionComponent)

class FLifetimeProperty;
class UActorComponent;

const FName URCPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

URCPawnExtensionComponent::URCPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    PawnData = nullptr;
    AbilitySystemComponent = nullptr;
}

void URCPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(URCPawnExtensionComponent, PawnData);
}

void URCPawnExtensionComponent::OnRegister()
{
    Super::OnRegister();

    const APawn* Pawn = GetPawn<APawn>();
    ensureAlwaysMsgf((Pawn != nullptr), TEXT("RCPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

    TArray<UActorComponent*> PawnExtensionComponents;
    Pawn->GetComponents(URCPawnExtensionComponent::StaticClass(), PawnExtensionComponents);
    ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one RCPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

    // Register with the init state system early, this will only work if this is a game world
    RegisterInitStateFeature();
}

void URCPawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for changes to all features
    BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
    
    // Notifies state manager that we have spawned, then try rest of default initialization
    ensure(TryToChangeInitState(RCGameplayTags::InitState_Spawned));
    CheckDefaultInitialization();
}

void URCPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UninitializeAbilitySystem();
    UnregisterInitStateFeature();

    Super::EndPlay(EndPlayReason);
}

void URCPawnExtensionComponent::SetPawnData(const URCPawnData* InPawnData)
{
    check(InPawnData);

    APawn* Pawn = GetPawnChecked<APawn>();

    if (Pawn->GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (PawnData)
    {
        return;
    }

    PawnData = InPawnData;

    Pawn->ForceNetUpdate();

    CheckDefaultInitialization();
}

void URCPawnExtensionComponent::OnRep_PawnData()
{
    CheckDefaultInitialization();
}

void URCPawnExtensionComponent::InitializeAbilitySystem(URCAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
    check(InASC);
    check(InOwnerActor);

    if (AbilitySystemComponent == InASC)
    {
        // The ability system component hasn't changed.
        return;
    }

    if (AbilitySystemComponent)
    {
        // Clean up the old ability system component.
        UninitializeAbilitySystem();
    }

    APawn* Pawn = GetPawnChecked<APawn>();
    AActor* ExistingAvatar = InASC->GetAvatarActor();

    if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
    {
        // There is already a pawn acting as the ASC's avatar, so we need to kick it out
        // This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
        ensure(!ExistingAvatar->HasAuthority());

        if (URCPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
        {
            OtherExtensionComponent->UninitializeAbilitySystem();
        }
    }

    AbilitySystemComponent = InASC;
    AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

    if (ensure(PawnData))
    {
        InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
    }

    OnAbilitySystemInitialized.Broadcast();
}

void URCPawnExtensionComponent::UninitializeAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
    {
        FGameplayTagContainer AbilityTypesToIgnore;
        AbilityTypesToIgnore.AddTag(RCGameplayTags::Ability_Behavior_SurvivesDeath);

        AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
      //AbilitySystemComponent->ClearAbilityInput();// NEED TO ADD //
        AbilitySystemComponent->RemoveAllGameplayCues();

        if (AbilitySystemComponent->GetOwnerActor() != nullptr)
        {
            AbilitySystemComponent->SetAvatarActor(nullptr);
        }
        else
        {
            // If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
            AbilitySystemComponent->ClearActorInfo();
        }

        OnAbilitySystemUninitialized.Broadcast();
    }

    AbilitySystemComponent = nullptr;
}

void URCPawnExtensionComponent::HandleControllerChanged()
{
    if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
    {
        ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
        if (AbilitySystemComponent->GetOwnerActor() == nullptr)
        {
            UninitializeAbilitySystem();
        }
        else
        {
            AbilitySystemComponent->RefreshAbilityActorInfo();
        }
    }

    CheckDefaultInitialization();
}

void URCPawnExtensionComponent::HandlePlayerStateReplicated()
{
    CheckDefaultInitialization();
}

void URCPawnExtensionComponent::CheckDefaultInitialization()
{
    // Before checking our progress, try progressing any other features we might depend on
    CheckDefaultInitializationForImplementers();

    static const TArray<FGameplayTag> StateChain = { RCGameplayTags::InitState_Spawned, RCGameplayTags::InitState_DataAvailable, RCGameplayTags::InitState_DataInitialized, RCGameplayTags::InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain(StateChain);
}

bool URCPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
    check(Manager);

    APawn* Pawn = GetPawn<APawn>();
    if (!CurrentState.IsValid() && DesiredState == RCGameplayTags::InitState_Spawned)
    {
        // As long as we are on a valid pawn, we count as spawned
        if (Pawn)
        {
            return true;
        }
    }
    if (CurrentState == RCGameplayTags::InitState_Spawned && DesiredState == RCGameplayTags::InitState_DataAvailable)
    {
        // Pawn data is required.
        if (!PawnData)
        {
            return false;
        }

        const bool bHasAuthority = Pawn->HasAuthority();
        const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

        if (bHasAuthority || bIsLocallyControlled)
        {
            // Check for being possessed by a controller.
            if (!GetController<AController>())
            {
                return false;
            }
        }

        return true;
    }
    else if (CurrentState == RCGameplayTags::InitState_DataAvailable && DesiredState == RCGameplayTags::InitState_DataInitialized)
    {
        // Transition to initialize if all features have their data available
        return Manager->HaveAllFeaturesReachedInitState(Pawn, RCGameplayTags::InitState_DataAvailable);
    }
    else if (CurrentState == RCGameplayTags::InitState_DataInitialized && DesiredState == RCGameplayTags::InitState_GameplayReady)
    {
        return true;
    }

    return false;
}

void URCPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
    if (DesiredState == RCGameplayTags::InitState_DataInitialized)
    {
        // This is currently all handled by other components listening to this state change
    }
}

void URCPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    // If another feature is now in DataAvailable, see if we should transition to DataInitialized
    if (Params.FeatureName != NAME_ActorFeatureName)
    {
        if (Params.FeatureState == RCGameplayTags::InitState_DataAvailable)
        {
            CheckDefaultInitialization();
        }
    }
}

void URCPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
    if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
    {
        OnAbilitySystemInitialized.Add(Delegate);
    }

    if (AbilitySystemComponent)
    {
        Delegate.Execute();
    }
}

void URCPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
    if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
    {
        OnAbilitySystemUninitialized.Add(Delegate);
    }
}
