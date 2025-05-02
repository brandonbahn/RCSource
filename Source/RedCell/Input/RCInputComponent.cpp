// Fill out your copyright notice in the Description page of Project Settings.


#include "RCInputComponent.h"

#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCInputComponent)

class URCInputConfig;

URCInputComponent::URCInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void URCInputComponent::AddInputMappings(const URCInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
    check(InputConfig);
    check(InputSubsystem);

    // Here you can handle any custom logic to add something from your input config if required
}

void URCInputComponent::RemoveInputMappings(const URCInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
    check(InputConfig);
    check(InputSubsystem);

    // Here you can handle any custom logic to remove input mappings that you may have added above
}

void URCInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
    for (uint32 Handle : BindHandles)
    {
        RemoveBindingByHandle(Handle);
    }
    BindHandles.Reset();
}
