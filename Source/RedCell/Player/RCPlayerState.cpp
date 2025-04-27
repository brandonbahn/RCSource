// RCPlayerState.cpp

#include "RCPlayerState.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "GameplayEffectTypes.h"
// Need this for EGameplayEffectReplicationMode

ARCPlayerState::ARCPlayerState()
{
    // 1) Create the ASC subobject
    AbilitySystemComponent = CreateDefaultSubobject<URCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    // Make sure it's replicated
    AbilitySystemComponent->SetIsReplicated(true);
    // Mixed mode so that GameplayEffects and attribute changes
    // Goes to *only* the owning client.
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // 2) Create the HealthSet
    HealthSet = CreateDefaultSubobject<URCHealthSet>(TEXT("HealthSet"));

    // 3) Tell the ASC
    AbilitySystemComponent->AddAttributeSetSubobject(HealthSet);
}
