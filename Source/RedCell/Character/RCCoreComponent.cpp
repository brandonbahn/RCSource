// Fill out your copyright notice in the Description page of Project Settings.


#include "RCCoreComponent.h"
#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "RCLogChannels.h"
#include "System/RCAssetManager.h"
#include "System/RCGameData.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"               
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "Messages/RCVerbMessage.h"
#include "Messages/RCVerbMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCoreComponent)

URCCoreComponent::URCCoreComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    AbilitySystemComponent = nullptr;
    CoreSet = nullptr;
}

void URCCoreComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();
    Super::OnUnregister();
}

void URCCoreComponent::ClearGameplayTags()
{
    // Add any mana-related gameplay tags that need to be cleared here
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Mana_Empty, 0);
    }
}

void URCCoreComponent::InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC)
{
    AActor* Owner = GetOwner();
    check(Owner);

    if (AbilitySystemComponent)
    {
        UE_LOG(LogRC, Error, TEXT("RCCoreComponent: Core component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
        return;
    }

    AbilitySystemComponent = InASC;
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogRC, Error, TEXT("RCCoreComponent: Cannot initialize Core component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
        return;
    }

    CoreSet = AbilitySystemComponent->GetSet<URCCoreSet>();
    if (!CoreSet)
    {
        UE_LOG(LogRC, Error, TEXT("RCCoreComponent: Cannot initialize Core component for owner [%s] with NULL Core set on the ability system."), *GetNameSafe(Owner));
        return;
    }
	
    // Register to listen for attribute changes.
    CoreSet->OnManaChanged.AddUObject(this, &ThisClass::HandleManaChanged);
    CoreSet->OnMaxManaChanged.AddUObject(this, &ThisClass::HandleMaxManaChanged);
    CoreSet->OnOutOfMana.AddUObject(this, &ThisClass::HandleOutOfMana);

    // TEMP: Reset attributes to default values.  Eventually this will be driven by a spread sheet.
    AbilitySystemComponent->SetNumericAttributeBase(URCCoreSet::GetManaAttribute(), CoreSet->GetMaxMana());

    ClearGameplayTags();

    OnManaChanged.Broadcast(this, CoreSet->GetMana(), CoreSet->GetMana(), nullptr);
    OnMaxManaChanged.Broadcast(this, CoreSet->GetMana(), CoreSet->GetMana(), nullptr);
}

void URCCoreComponent::UninitializeFromAbilitySystem()
{
    ClearGameplayTags();

    if (CoreSet)
    {
        CoreSet->OnManaChanged.RemoveAll(this);
        CoreSet->OnMaxManaChanged.RemoveAll(this);
        CoreSet->OnOutOfMana.RemoveAll(this);
    }

    CoreSet = nullptr;
    AbilitySystemComponent = nullptr;
}

float URCCoreComponent::GetMana() const
{
    return CoreSet ? CoreSet->GetMana() : 0.f;
}

float URCCoreComponent::GetMaxMana() const
{
    return CoreSet ? CoreSet->GetMaxMana() : 0.f;
}

float URCCoreComponent::GetManaNormalized() const
{
    if (!CoreSet) return 0.f;
    const float MaxM = CoreSet->GetMaxMana();
    return MaxM > 0.f ? (CoreSet->GetMana() / MaxM) : 0.f;
}

void URCCoreComponent::HandleManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
    OnManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);

    // If we just got mana back after being empty, remove the tags
    if (OldValue <= 0.0f && NewValue > 0.0f && AbilitySystemComponent)
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Mana_Empty, 0);
    }
}
            
void URCCoreComponent::HandleMaxManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
    OnMaxManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URCCoreComponent::HandleOutOfMana(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
    if (AbilitySystemComponent && DamageEffectSpec)
    {
        // Send the "GameplayEvent.Mana.Empty" gameplay event through the owner's ability system
        // This can be used to trigger abilities that respond to being out of mana
        {
            FGameplayEventData Payload;
            Payload.EventTag = RCGameplayTags::GameplayEvent_Mana_Empty;
            Payload.Instigator = DamageInstigator;
            Payload.Target = AbilitySystemComponent->GetAvatarActor();
            Payload.OptionalObject = DamageEffectSpec->Def;
            Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
            Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
            Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
            Payload.EventMagnitude = DamageMagnitude;

            FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
            AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
        }

        // Optionally: Apply a gameplay tag to mark the character as out of mana
        // This could be used to prevent casting abilities, change UI state, etc.
        if (AbilitySystemComponent)
        {
            AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Mana_Empty, 1);
        }
    }
#endif // #if WITH_SERVER_CODE
}