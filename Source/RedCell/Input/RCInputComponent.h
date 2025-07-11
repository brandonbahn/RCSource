// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "RCInputConfig.h"
#include "RCInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;


/**
 * URCInputComponent
 *
 *    Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class URCInputComponent : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:

    URCInputComponent(const FObjectInitializer& ObjectInitializer);

    void AddInputMappings(const URCInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
    void RemoveInputMappings(const URCInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

    template<class UserClass, typename FuncType>
    void BindNativeAction(const URCInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

    template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindAbilityActions(const URCInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

    void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
void URCInputComponent::BindNativeAction(const URCInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
    check(InputConfig);
    if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
    {
        BindAction(IA, TriggerEvent, Object, Func);
    }
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void URCInputComponent::BindAbilityActions(const URCInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
    check(InputConfig);

    for (const FRCInputAction& Action : InputConfig->AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            if (PressedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
            }

            if (ReleasedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
            }
        }
    }
}
