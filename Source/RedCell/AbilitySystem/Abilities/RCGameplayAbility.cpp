// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/RCGameplayAbility.h"

URCGameplayAbility::URCGameplayAbility(const FObjectInitializer& ObjInit)
: Super(ObjInit)
{
  // Make most abilities instanced per actor by default
  InstancingPolicy     = EGameplayAbilityInstancingPolicy::InstancedPerActor;

  // Default to server‐initiated (override in Blueprint/C++ for client abilities)
  NetExecutionPolicy   = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

  // (Optional) give every ability the same activation failure tag container,
  // or hook up a Blueprint event on failure, etc.
}
