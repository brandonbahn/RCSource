// Fill out your copyright notice in the Description page of Project Settings.


#include "RCInventoryItemDefinition.h"

#include "Templates/SubclassOf.h"
#include "Uobject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCInventoryItemDefinition)

URCInventoryItemDefinition::URCInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

const URCInventoryItemFragment* URCInventoryItemDefinition::FindFragmentByClass(TSubclassOf<URCInventoryItemFragment> FragmentClass) const
{
    if (FragmentClass != nullptr)
    {
        for (URCInventoryItemFragment* Fragment : Fragments)
        {
            if (Fragment && Fragment->IsA(FragmentClass))
            {
                return Fragment;
            }
        }
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////
// URCInventoryItemDefinition

const URCInventoryItemFragment* URCInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<URCInventoryItemDefinition> ItemDef, TSubclassOf<URCInventoryItemFragment> FragmentClass)
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<URCInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }
    return nullptr;
}

