// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"

#include "RCInventoryItemInstance.generated.h"

class FLifetimeProperty;

class URCInventoryItemDefinition;
class URCInventoryItemFragment;
struct FFrame;
struct FGameplayTag;

/**
 * URCInventoryItemInstance
 */
UCLASS(BlueprintType)
class REDCELL_API URCInventoryItemInstance : public UObject
{
	GENERATED_BODY()
    
public:
    URCInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UObject interface
    virtual bool IsSupportedForNetworking() const override { return true; }
    //~End of UObject interface

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
    void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    UFUNCTION(BlueprintCallable, Category=Inventory)
    int32 GetStatTagStackCount(FGameplayTag Tag) const;

    // Returns true if there is at least one stack of the specified tag
    UFUNCTION(BlueprintCallable, Category=Inventory)
    bool HasStatTag(FGameplayTag Tag) const;

    TSubclassOf<URCInventoryItemDefinition> GetItemDef() const
    {
        return ItemDef;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
    const URCInventoryItemFragment* FindFragmentByClass(TSubclassOf<URCInventoryItemFragment> FragmentClass) const;

    template <typename ResultClass>
    const ResultClass* FindFragmentByClass() const
    {
        return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
    }

private:
#if UE_WITH_IRIS
    /** Register all replication fragments */
    virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

    void SetItemDef(TSubclassOf<URCInventoryItemDefinition> InDef);

    friend struct FRCInventoryList;

private:
    UPROPERTY(Replicated)
    FGameplayTagStackContainer StatTags;

    // The item definition
    UPROPERTY(Replicated)
    TSubclassOf<URCInventoryItemDefinition> ItemDef;
};
