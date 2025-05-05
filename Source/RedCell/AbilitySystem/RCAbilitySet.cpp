// Fill out your copyright notice in the Description page of Project Settings.

#include "RCAbilitySet.h"

#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilitySet)

void FRCAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
    if (Handle.IsValid())
    {
        AbilitySpecHandles.Add(Handle);
    }
}

void FRCAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
    if (Handle.IsValid())
    {
        GameplayEffectHandles.Add(Handle);
    }
}

void FRCAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
    GrantedAttributeSets.Add(Set);
}

void FRCAbilitySet_GrantedHandles::TakeFromAbilitySystem(URCAbilitySystemComponent* RCASC)
{
    check(RCASC);

    if (!RCASC->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return;
    }

    for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
    {
        if (Handle.IsValid())
        {
            RCASC->ClearAbility(Handle);
        }
    }

    for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
    {
        if (Handle.IsValid())
        {
            RCASC->RemoveActiveGameplayEffect(Handle);
        }
    }

    for (UAttributeSet* Set : GrantedAttributeSets)
    {
        RCASC->RemoveSpawnedAttribute(Set);
    }

    AbilitySpecHandles.Reset();
    GameplayEffectHandles.Reset();
    GrantedAttributeSets.Reset();
}

URCAbilitySet::URCAbilitySet(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URCAbilitySet::GiveToAbilitySystem(URCAbilitySystemComponent* RCASC, FRCAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
    check(RCASC);

    if (!RCASC->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return;
    }
    
    // Grant the attribute sets.
    for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
    {
        const FRCAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

        if (!IsValid(SetToGrant.AttributeSet))
        {
            continue;
        }

        UAttributeSet* NewSet = NewObject<UAttributeSet>(RCASC->GetOwner(), SetToGrant.AttributeSet);
        RCASC->AddAttributeSetSubobject(NewSet);

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddAttributeSet(NewSet);
        }
    }

    // Grant the gameplay abilities.
    for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
    {
        const FRCAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

        if (!IsValid(AbilityToGrant.Ability))
        {
            continue;
        }

        URCGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<URCGameplayAbility>();

        FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
        AbilitySpec.SourceObject = SourceObject;
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

        const FGameplayAbilitySpecHandle AbilitySpecHandle = RCASC->GiveAbility(AbilitySpec);

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
        }
    }

    // Grant the gameplay effects.
    for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
    {
        const FRCAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

        if (!IsValid(EffectToGrant.GameplayEffect))
        {
            continue;
        }

        const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
        const FActiveGameplayEffectHandle GameplayEffectHandle = RCASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, RCASC->MakeEffectContext());

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
        }
    }
}

