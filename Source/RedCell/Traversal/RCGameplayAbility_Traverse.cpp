// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameplayAbility_Traverse.h"

#include "Character/RCCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Traversal/RCTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayAbility_Traverse)

URCGameplayAbility_Traverse::URCGameplayAbility_Traverse(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool URCGameplayAbility_Traverse::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, 
    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }

    const ARCCharacter* RCCharacter = Cast<ARCCharacter>(ActorInfo->AvatarActor.Get());
    if (!RCCharacter)
    {
        return false;
    }

    const URCTraversalComponent* TraversalComp = RCCharacter->GetTraversalComponent();
    if (!TraversalComp)
    {
        return false;
    }

    // Don't allow if already doing traversal
    if (TraversalComp->IsDoingTraversalAction())
    {
        return false;
    }

    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    
    // ALWAYS do a live check for traversable obstacles
    // This is what enables the dynamic switching during jumps
    FTraversalCheckInputs TraversalInputs = RCCharacter->GetTraversalCheckInputs();
    bool bTraversalCheckFailed, bMontageSelectionFailed;
    
    return true;
    
}

void URCGameplayAbility_Traverse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (!TryTraversalAction(EDrawDebugTrace::ForOneFrame))
    {
        // No traversal found, end ability immediately
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void URCGameplayAbility_Traverse::EndAbility(const FGameplayAbilitySpecHandle Handle, 
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
                                             bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool URCGameplayAbility_Traverse::TryTraversalAction(EDrawDebugTrace::Type DebugType)
{
    ARCCharacter* RCCharacter = GetRCCharacterFromActorInfo();
    if (!RCCharacter)
    {
        return false;
    }

    URCTraversalComponent* TraversalComp = RCCharacter->GetTraversalComponent();
    if (!TraversalComp)
    {
        return false;
    }

    // Get traversal inputs from character
    FTraversalCheckInputs TraversalInputs = RCCharacter->GetTraversalCheckInputs();
    
    // Perform traversal check
    bool bTraversalCheckFailed, bMontageSelectionFailed;
    TraversalComp->TryTraversalAction(TraversalInputs, DebugType, bTraversalCheckFailed, bMontageSelectionFailed);
    
    if (!bTraversalCheckFailed && !bMontageSelectionFailed)
    {
        // Traversal succeeded - character will perform the action
        return true;
    }

    // Traversal failed
    return false;
}

void URCGameplayAbility_Traverse::OnMontageCompletedOrInterrupted(UPrimitiveComponent* HitComponent,
    UCharacterMovementComponent* MovementComp, ETraversalActionType ActionType)
{
    if (ARCCharacter* RCCharacter = GetRCCharacterFromActorInfo())
    {
        // Clean up capsule collision
        RCCharacter->GetCapsuleComponent()->IgnoreComponentWhenMoving(HitComponent, false);
        
        // Set movement mode based on action type
        EMovementMode NewMovementMode = (ActionType == ETraversalActionType::Vault) ? MOVE_Falling : MOVE_Walking;
        MovementComp->SetMovementMode(NewMovementMode);
    }
}
