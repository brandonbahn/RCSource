// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RCAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilityTagRelationshipMapping)

void URCAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FRCAbilityTagRelationship& Tags = AbilityTagRelationships[i];
        if (AbilityTags.HasTag(Tags.AbilityTag))
        {
            if (OutTagsToBlock)
            {
                OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
            }
            if (OutTagsToCancel)
            {
                OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
            }
        }
    }
}

void URCAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FRCAbilityTagRelationship& Tags = AbilityTagRelationships[i];
        if (AbilityTags.HasTag(Tags.AbilityTag))
        {
            if (OutActivationRequired)
            {
                OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
            }
            if (OutActivationBlocked)
            {
                OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
            }
        }
    }
}

bool URCAbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FRCAbilityTagRelationship& Tags = AbilityTagRelationships[i];

        if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
        {
            return true;
        }
    }

    return false;
}

