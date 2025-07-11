// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "RCAbilityTagRelationshipMapping.generated.h"

class UObject;

/** Struct that defines the relationship between different ability tags */
USTRUCT()
struct FRCAbilityTagRelationship
{
    GENERATED_BODY()

    /** The tag that this container relationship is about. Single tag, but abilities can have multiple of these */
    UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
    FGameplayTag AbilityTag;

    /** The other ability tags that will be blocked by any ability using this tag */
    UPROPERTY(EditAnywhere, Category = Ability)
    FGameplayTagContainer AbilityTagsToBlock;

    /** The other ability tags that will be canceled by any ability using this tag */
    UPROPERTY(EditAnywhere, Category = Ability)
    FGameplayTagContainer AbilityTagsToCancel;

    /** If an ability has the tag, this is implicitly added to the activation required tags of the ability */
    UPROPERTY(EditAnywhere, Category = Ability)
    FGameplayTagContainer ActivationRequiredTags;

    /** If an ability has the tag, this is implicitly added to the activation blocked tags of the ability */
    UPROPERTY(EditAnywhere, Category = Ability)
    FGameplayTagContainer ActivationBlockedTags;
};

    
UCLASS()
class REDCELL_API URCAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()
	
private:
    /** The list of relationships between different gameplay tags (which ones block or cancel others) */
    UPROPERTY(EditAnywhere, Category = Ability, meta=(TitleProperty="AbilityTag"))
    TArray<FRCAbilityTagRelationship> AbilityTagRelationships;

public:
    /** Given a set of ability tags, parse the tag relationship and fill out tags to block and cancel */
    void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

    /** Given a set of ability tags, add additional required and blocking tags */
    void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

    /** Returns true if the specified ability tags are canceled by the passed in action tag */
    bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};
