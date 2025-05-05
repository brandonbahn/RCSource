// RCPlayerState.cpp

#include "RCPlayerState.h"

#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCCombatSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameplayEffectTypes.h"
#include "RCPlayerController.h"
#include "GameModes/RCGameMode.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

// Need this for EGameplayEffectReplicationMode

const FName ARCPlayerState::NAME_RCAbilityReady("RedCellAbilitiesReady");

ARCPlayerState::ARCPlayerState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 1) Create the ASC subobject
    AbilitySystemComponent = CreateDefaultSubobject<URCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);        // Make sure it's replicated
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); 
    // Mixed mode so that GameplayEffects and attribute changes
    // Goes to *only* the owning client.
    
    // 2) Create the HealthSet
    HealthSet = CreateDefaultSubobject<URCHealthSet>(TEXT("HealthSet"));
    CombatSet = CreateDefaultSubobject<URCCombatSet>(TEXT("CombatSet"));
    
    // 3) Tell the ASC
    AbilitySystemComponent->AddAttributeSetSubobject(HealthSet.Get());
    
    // AbilitySystemComponent needs to be updated at a high frequency.
    SetNetUpdateFrequency(100.0f);
}

void ARCPlayerState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void ARCPlayerState::Reset()
{
    Super::Reset();
}

void ARCPlayerState::ClientInitialize(AController* C)
{
    Super::ClientInitialize(C);

    if (URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
    {
        PawnExtComp->CheckDefaultInitialization();
    }
}

void ARCPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
}

void ARCPlayerState::OnDeactivated()
{
    Super::OnDeactivated();
}

void ARCPlayerState::OnReactivated()
{
    Super::OnReactivated();
}

void ARCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    FDoRepLifetimeParams SharedParams;
    SharedParams.bIsPushBased = true;
    
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}


ARCPlayerController* ARCPlayerState::GetRCPlayerController() const
{
    return Cast<ARCPlayerController>(GetOwner());
}

UAbilitySystemComponent* ARCPlayerState::GetAbilitySystemComponent() const
{
    return GetRCAbilitySystemComponent();
}

void ARCPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    // Only on the server (authority)
    if (GetLocalRole() == ROLE_Authority)
    {
        if (ARCGameMode* RCGameMode = GetWorld()->GetAuthGameMode<ARCGameMode>())
        {
            if (const URCPawnData* NewPawnData = RCGameMode->GetPawnDataForController(GetOwningController()))
            {
                SetPawnData(NewPawnData);
            }
            else
            {
                UE_LOG(LogTemp, Error,
                    TEXT("ARCPlayerState::PostInitializeComponents(): No PawnData for PlayerState %s"),
                    *GetNameSafe(this));
            }
        }
    }
}

void ARCPlayerState::SetPawnData(const URCPawnData* InPawnData)
{
    check(InPawnData);

    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (PawnData)
    {
        UE_LOG(LogTemp, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
        return;
    }

    MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
    PawnData = InPawnData;

    for (const URCAbilitySet* AbilitySet : PawnData->AbilitySets)
    {
        if (AbilitySet)
        {
            AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
        }
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_RCAbilityReady);
    
    ForceNetUpdate();
}

void ARCPlayerState::OnRep_PawnData()
{
}

