// Fill out your copyright notice in the Description page of Project Settings.
// RCHealthComponent.cpp

#include "Character/RCHealthComponent.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"                    // for FOnAttributeChangeData & FGameplayEffectModCallbackData
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCHealthComponent)

URCHealthComponent::URCHealthComponent(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
    , AbilitySystemComponent(nullptr)
    , HealthSet(nullptr)
    , DeathState(ERCDeathState::NotDead)
{
    SetIsReplicatedByDefault(true);
}

void URCHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URCHealthComponent, DeathState);
}

void URCHealthComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();
    Super::OnUnregister();
}

void URCHealthComponent::InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC)
{
    if (!InASC || AbilitySystemComponent)
    {
        return;
    }

    AbilitySystemComponent = InASC;
    HealthSet = InASC->GetSet<URCHealthSet>();
    if (!HealthSet)
    {
        AbilitySystemComponent = nullptr;
        return;
    }

    // Bind the single‑param delegates
    InASC->GetGameplayAttributeValueChangeDelegate(URCHealthSet::GetHealthAttribute())
        .AddUObject(this, &URCHealthComponent::HandleHealthChanged);

    InASC->GetGameplayAttributeValueChangeDelegate(URCHealthSet::GetMaxHealthAttribute())
        .AddUObject(this, &URCHealthComponent::HandleMaxHealthChanged);
}

void URCHealthComponent::UninitializeFromAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    auto& HealthDel = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URCHealthSet::GetHealthAttribute());
    auto& MaxDel    = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URCHealthSet::GetMaxHealthAttribute());

    HealthDel.RemoveAll(this);
    MaxDel   .RemoveAll(this);

    AbilitySystemComponent = nullptr;
    HealthSet              = nullptr;
}

float URCHealthComponent::GetHealth() const
{
    return HealthSet ? HealthSet->GetHealth() : 0.f;
}

float URCHealthComponent::GetMaxHealth() const
{
    return HealthSet ? HealthSet->GetMaxHealth() : 0.f;
}

float URCHealthComponent::GetHealthNormalized() const
{
    if (!HealthSet) return 0.f;
    const float MaxH = HealthSet->GetMaxHealth();
    return MaxH > 0.f ? (HealthSet->GetHealth() / MaxH) : 0.f;
}

void URCHealthComponent::StartDeath()
{
    if (DeathState != ERCDeathState::NotDead) return;
    DeathState = ERCDeathState::DeathStarted;
    OnDeathStarted.Broadcast(GetOwner());
}

void URCHealthComponent::FinishDeath()
{
    if (DeathState != ERCDeathState::DeathStarted) return;
    DeathState = ERCDeathState::DeathFinished;
    OnDeathFinished.Broadcast(GetOwner());
}

void URCHealthComponent::DamageSelfDestruct(bool /*bFellOutOfWorld*/)
{
    if (AbilitySystemComponent && HealthSet)
    {
        AbilitySystemComponent->ApplyModToAttribute(
            URCHealthSet::GetHealthAttribute(),
            EGameplayModOp::Override,
            0.f
        );
    }
}

void URCHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    // Your existing broadcast
    AActor* InstigatorActor = nullptr;
    if (Data.GEModData)
    {
        InstigatorActor = Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator();
    }
    OnHealthChanged.Broadcast(this, Data.OldValue, Data.NewValue, InstigatorActor);

    // **New**: if we hit zero, start death **and** fire the ability event
    if (Data.NewValue <= 0.f && DeathState == ERCDeathState::NotDead)
    {
        // 1) flip your state & broadcast Blueprint event
        StartDeath();

        // 2) dispatch the GameplayEvent.Death tag so your Death ability fires
        if (AbilitySystemComponent)
        {
            FGameplayEventData EventData;
            AbilitySystemComponent->HandleGameplayEvent(
                RCGameplayTags::GameplayEvent_Death,
                /*Payload=*/&EventData
            );
        }
    }
}

void URCHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    AActor* InstigatorActor = nullptr;
    if (Data.GEModData)
    {
        InstigatorActor = Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator();
    }

    OnMaxHealthChanged.Broadcast(this, Data.OldValue, Data.NewValue, InstigatorActor);
}

void URCHealthComponent::HandleOutOfHealth(const FOnAttributeChangeData& /*Data*/)
{
    // No-op: zero‑health is handled in HandleHealthChanged
}

void URCHealthComponent::OnRep_DeathState(ERCDeathState OldState)
{
    if (DeathState == ERCDeathState::DeathStarted)
    {
        OnDeathStarted.Broadcast(GetOwner());
    }
    else if (DeathState == ERCDeathState::DeathFinished)
    {
        OnDeathFinished.Broadcast(GetOwner());
    }
}
