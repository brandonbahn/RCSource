// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"

void URCAbilitySet::GiveAbilities(UAbilitySystemComponent* ASC)
{
    Super::GiveAbilities(ASC);
    for (TSubclassOf<UGameplayEffect> GEClass : GameplayEffects)
    {
        if (GEClass && ASC)
        {
            auto Context = ASC->MakeEffectContext();
            auto Spec    = ASC->MakeOutgoingSpec(GEClass, 1.f, Context);
            if (Spec.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
}
