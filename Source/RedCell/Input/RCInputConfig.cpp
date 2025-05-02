// Fill out your copyright notice in the Description page of Project Settings.

#include "RCInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCInputConfig)


URCInputConfig::URCInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* URCInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
    for (const FRCInputAction& Action : NativeInputActions)
    {
        if (Action.InputAction && (Action.InputTag == InputTag))
        {
            return Action.InputAction;
        }
    }
    

    return nullptr;
}

const UInputAction* URCInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
    for (const FRCInputAction& Action : AbilityInputActions)
    {
        if (Action.InputAction && (Action.InputTag == InputTag))
        {
            return Action.InputAction;
        }
    }

    return nullptr;
}
