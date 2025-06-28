// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "RCInventoryItemDefinition.generated.h"

template <typename T> class TSubclassOf;

class URCInventoryItemInstance;
struct FFrame;

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class REDCELL_API URCInventoryItemFragment : public UObject
{
    GENERATED_BODY()
public:
    /** Called when an instance is created from this definition. */
    virtual void OnInstanceCreated(class URCInventoryItemInstance* Instance) const {}
};

/**
 * URCInventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class REDCELL_API URCInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
    URCInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
    TArray<TObjectPtr<URCInventoryItemFragment>> Fragments;

public:
    const URCInventoryItemFragment* FindFragmentByClass(TSubclassOf<URCInventoryItemFragment> FragmentClass) const;
};

//@TODO: Make into a subsystem instead?
UCLASS()
class URCInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
    static const URCInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<URCInventoryItemDefinition> ItemDef, TSubclassOf<URCInventoryItemFragment> FragmentClass);
    
};
