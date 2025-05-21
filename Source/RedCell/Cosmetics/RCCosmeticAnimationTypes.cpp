// Fill out your copyright notice in the Description page of Project Settings.


#include "Cosmetics/RCCosmeticAnimationTypes.h"

#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCCosmeticAnimationTypes)

TSubclassOf<UAnimInstance> FRCAnimLayerSelectionSet::SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const
{
    for (const FRCAnimLayerSelectionEntry& Rule : LayerRules)
    {
        if ((Rule.Layer != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
        {
            return Rule.Layer;
        }
    }

    return DefaultLayer;
}

USkeletalMesh* FRCAnimBodyStyleSelectionSet::SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const
{
    for (const FRCAnimBodyStyleSelectionEntry& Rule : MeshRules)
    {
        if ((Rule.Mesh != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
        {
            return Rule.Mesh;
        }
    }

    return DefaultMesh;
}
