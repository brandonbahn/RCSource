// Fill out your copyright notice in the Description page of Project Settings.

#include "RCMeleeWeaponInstance.h"
#include "NativeGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Weapons/RCWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCMeleeWeaponInstance)

void URCMeleeWeaponInstance::OnEquipped()
{
    Super::OnEquipped();
}

void URCMeleeWeaponInstance::OnUnequipped()
{
    Super::OnUnequipped();
}

float URCMeleeWeaponInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
    const FRichCurve* Curve = DistanceDamageFalloff.GetRichCurveConst();
    return Curve->HasAnyData() ? Curve->Eval(Distance) : 1.0f;
}

float URCMeleeWeaponInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
    float CombinedMultiplier = 1.0f;
    if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
    {
        for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
        {
            if (const float* pTagMultiplier = MaterialDamageMultiplier.Find(MaterialTag))
            {
                CombinedMultiplier *= *pTagMultiplier;
            }
        }
    }

    return CombinedMultiplier;
}

void URCMeleeWeaponInstance::EventAttack()
{
    // TODO: your melee‐sweep or projectile logic, cooldown check, damage application, VFX/SFX, etc.
}
