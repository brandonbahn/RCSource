// Fill out your copyright notice in the Description page of Project Settings.


#include "RCAbilityCost_PlayerTagStack.h"

#include "GameFramework/Controller.h"
#include "RCGameplayAbility.h"
#include "Player/RCPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilityCost_PlayerTagStack)

URCAbilityCost_PlayerTagStack::URCAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool URCAbilityCost_PlayerTagStack::CheckCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (ARCPlayerState* PS = Cast<ARCPlayerState>(PC->PlayerState))
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return PS->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void URCAbilityCost_PlayerTagStack::ApplyCost(const URCGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (ARCPlayerState* PS = Cast<ARCPlayerState>(PC->PlayerState))
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				PS->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}

