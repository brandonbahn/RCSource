// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RCAbilitySourceInterface.generated.h"

class UPhysicalMaterial;
struct FGameplayTagContainer;

UINTERFACE(BlueprintType)
class REDCELL_API URCAbilitySourceInterface : public UInterface
{
  GENERATED_UINTERFACE_BODY()
};

class IRCAbilitySourceInterface
{
  GENERATED_IINTERFACE_BODY()

public:
  virtual float GetDistanceAttenuation(
      float Distance,
      const FGameplayTagContainer* SourceTags = nullptr,
      const FGameplayTagContainer* TargetTags = nullptr
  ) const = 0;

  virtual float GetPhysicalMaterialAttenuation(
      const UPhysicalMaterial* PhysMat,
      const FGameplayTagContainer* SourceTags = nullptr,
      const FGameplayTagContainer* TargetTags = nullptr
  ) const = 0;
};
