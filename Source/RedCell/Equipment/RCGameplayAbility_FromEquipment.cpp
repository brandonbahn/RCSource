// Fill out your copyright notice in the Description page of Project Settings.

#include "RCGameplayAbility_FromEquipment.h"
#include "RCEquipmentInstance.h"
#include "Inventory/RCInventoryItemInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayAbility_FromEquipment)

URCGameplayAbility_FromEquipment::URCGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

URCEquipmentInstance* URCGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
    if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
    {
        return Cast<URCEquipmentInstance>(Spec->SourceObject.Get());
    }

    return nullptr;
}

URCInventoryItemInstance* URCGameplayAbility_FromEquipment::GetAssociatedItem() const
{
    if (URCEquipmentInstance* Equipment = GetAssociatedEquipment())
    {
        return Cast<URCInventoryItemInstance>(Equipment->GetInstigator());
    }
    return nullptr;
}


#if WITH_EDITOR
EDataValidationResult URCGameplayAbility_FromEquipment::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
    if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
    {
        Context.AddError(NSLOCTEXT("RedCell", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}

#endif
