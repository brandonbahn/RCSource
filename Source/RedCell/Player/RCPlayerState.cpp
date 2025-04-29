// RCPlayerState.cpp

#include "RCPlayerState.h"

#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameplayEffectTypes.h"
#include "RCPlayerController.h"
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

ARCPlayerController* ARCPlayerState::GetRCPlayerController() const
{
    return Cast<ARCPlayerController>(GetOwner());
}

UAbilitySystemComponent* ARCPlayerState::GetAbilitySystemComponent() const
{
    return GetRCAbilitySystemComponent();
}

void ARCPlayerState::ClientInitialize(AController* NewController)
{
    Super::ClientInitialize(NewController);
}

void ARCPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
}

void ARCPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void ARCPlayerState::OnReactivated()
{
    Super::OnReactivated();
}

void ARCPlayerState::OnDeactivated()
{
    Super::OnDeactivated();
}
