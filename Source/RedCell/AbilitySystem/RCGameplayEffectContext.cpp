//
//  RCGameplayEffectContext.cpp
//  RedCell
//
//  Created by Brandon Bahn on 5/4/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#include "RCGameplayEffectContext.h"

#include "AbilitySystem/RCAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayEffectContext)

class FArchive;

FRCGameplayEffectContext* FRCGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
    FGameplayEffectContext* BaseEffectContext = Handle.Get();
    if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FRCGameplayEffectContext::StaticStruct()))
    {
        return (FRCGameplayEffectContext*)BaseEffectContext;
    }

    return nullptr;
}

bool FRCGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

    // Not serialized for post-activation use:
    // CartridgeID

    return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
    // Forward to FGameplayEffectContextNetSerializer
    // Note: If FRCGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
    UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(RCGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FRCGameplayEffectContext::SetAbilitySource(const IRCAbilitySourceInterface* InObject, float InSourceLevel)
{
    AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
    //SourceLevel = InSourceLevel;
}

const IRCAbilitySourceInterface* FRCGameplayEffectContext::GetAbilitySource() const
{
    return Cast<IRCAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FRCGameplayEffectContext::GetPhysicalMaterial() const
{
    if (const FHitResult* HitResultPtr = GetHitResult())
    {
        return HitResultPtr->PhysMaterial.Get();
    }
    return nullptr;
}

