//
//  RCGameplayEffectContext.h
//  RedCell
//
//  Created by Brandon Bahn on 5/4/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "GameplayEffectTypes.h"

#include "RCGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class IRCAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct FRCGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

    FRCGameplayEffectContext()
        : FGameplayEffectContext()
    {
    }

    FRCGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
        : FGameplayEffectContext(InInstigator, InEffectCauser)
    {
    }

    /** Returns the wrapped FRCGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
    static REDCELL_API FRCGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

    /** Sets the object used as the ability source */
    void SetAbilitySource(const IRCAbilitySourceInterface* InObject, float InSourceLevel);

    /** Returns the ability source interface associated with the source object. Only valid on the authority. */
    const IRCAbilitySourceInterface* GetAbilitySource() const;

    virtual FGameplayEffectContext* Duplicate() const override
    {
        FRCGameplayEffectContext* NewContext = new FRCGameplayEffectContext();
        *NewContext = *this;
        if (GetHitResult())
        {
            // Does a deep copy of the hit result
            NewContext->AddHitResult(*GetHitResult(), true);
        }
        return NewContext;
    }

    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FRCGameplayEffectContext::StaticStruct();
    }

    /** Overridden to serialize new fields */
    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

    /** Returns the physical material from the hit result if there is one */
    const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
    /** ID to allow the identification of multiple bullets that were part of the same cartridge */
    UPROPERTY()
    int32 CartridgeID = -1;

protected:
    /** Ability Source object (should implement IRCAbilitySourceInterface). NOT replicated currently */
    UPROPERTY()
    TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template<>
struct TStructOpsTypeTraits<FRCGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FRCGameplayEffectContext>
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true
    };
};

