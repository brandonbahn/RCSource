// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RCCoreComponent.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URCCoreComponent::URCCoreComponent(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
    , AbilitySystemComponent(nullptr)
    , CoreSet(nullptr)
{
    SetIsReplicatedByDefault(true);
}

void URCCoreComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();
    Super::OnUnregister();
}

void URCCoreComponent::InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC)
{
    if (!InASC || AbilitySystemComponent)
    {
        return;
    }

    AbilitySystemComponent = InASC;
    CoreSet = InASC->GetSet<URCCoreSet>();
    if (!CoreSet)
    {
        AbilitySystemComponent = nullptr;
        return;
    }

    // Bind the single‑param delegates
    InASC->GetGameplayAttributeValueChangeDelegate(URCCoreSet::GetManaAttribute())
        .AddUObject(this, &URCCoreComponent::HandleManaChanged);

    InASC->GetGameplayAttributeValueChangeDelegate(URCCoreSet::GetMaxManaAttribute())
        .AddUObject(this, &URCCoreComponent::HandleMaxManaChanged);
}

void URCCoreComponent::UninitializeFromAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    auto& ManaDel = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URCCoreSet::GetManaAttribute());
    auto& MaxDel    = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URCCoreSet::GetMaxManaAttribute());

    ManaDel.RemoveAll(this);
    MaxDel .RemoveAll(this);

    AbilitySystemComponent = nullptr;
    CoreSet                = nullptr;
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

void URCCoreComponent::HandleManaChanged(const FOnAttributeChangeData& Data)
{
    // Your existing broadcast
    AActor* InstigatorActor = nullptr;
    if (Data.GEModData)
    {
        InstigatorActor = Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator();
    }
    OnManaChanged.Broadcast(this, Data.OldValue, Data.NewValue, InstigatorActor);
    
    if (Data.NewValue <= 0.f && Data.OldValue > 0.f)
    {
        if (AbilitySystemComponent)
        {
            FGameplayEventData EventData;
            AbilitySystemComponent->HandleGameplayEvent(RCGameplayTags::GameplayEvent_Mana_Empty, &EventData);
        }
    }
    else if (Data.NewValue > 0.f && Data.OldValue <= 0.f)
    {
        if (AbilitySystemComponent)
        {
            FGameplayEventData EventData;
            AbilitySystemComponent->HandleGameplayEvent(RCGameplayTags::GameplayEvent_Mana_Restored, &EventData);
        }
    }
}
            
void URCCoreComponent::HandleMaxManaChanged(const FOnAttributeChangeData& Data)
{
    AActor* InstigatorActor = nullptr;
    if (Data.GEModData)
    {
        InstigatorActor = Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator();
    }

    OnMaxManaChanged.Broadcast(this, Data.OldValue, Data.NewValue, InstigatorActor);
}

void URCCoreComponent::HandleOutOfMana(const FOnAttributeChangeData& /*Data*/)
{
    // No-op: zero‑health is handled in HandleHealthChanged
}

