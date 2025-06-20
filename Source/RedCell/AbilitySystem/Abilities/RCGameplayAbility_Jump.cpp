// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameplayAbility_Jump.h"

#include "Character/RCCharacter.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Traversal/RCTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayAbility_Jump)

URCGameplayAbility_Jump::URCGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool URCGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, 
    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }

    const ARCCharacter* RCCharacter = Cast<ARCCharacter>(ActorInfo->AvatarActor.Get());
    if (!RCCharacter || !RCCharacter->CanJump())
    {
        return false;
    }

    // Don't allow jump if traversal is active
    if (const URCTraversalComponent* TraversalComp = RCCharacter->GetTraversalComponent())
    {
        if (TraversalComp->IsDoingTraversalAction())
        {
            return false;
        }
    }

    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    
    return true;
}

void URCGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateEndAbility, bool bWasCancelled)
{
    // Stop jumping in case the ability blueprint doesn't call it
    CharacterJumpStop();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URCGameplayAbility_Jump::CharacterJumpStart()
{
    if (ARCCharacter* RCCharacter = GetRCCharacterFromActorInfo())
    {
        if (RCCharacter->IsLocallyControlled() && !RCCharacter->bPressedJump)
        {
            RCCharacter->UnCrouch();
            RCCharacter->Jump();
        }
    }
}

void URCGameplayAbility_Jump::CharacterJumpStop()
{
    if (ARCCharacter* RCCharacter = GetRCCharacterFromActorInfo())
    {
        if (RCCharacter->IsLocallyControlled() && RCCharacter->bPressedJump)
        {
            RCCharacter->StopJumping();
        }
    }
}