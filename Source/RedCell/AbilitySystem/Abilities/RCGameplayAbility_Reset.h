// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "RCGameplayAbility_Reset.generated.h"

class AActor;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * URCGameplayAbility_Reset
 *
 *	Gameplay ability used for handling quickly resetting the player back to initial spawn state.
 *	Ability is activated automatically via the "GameplayEvent.RequestReset" ability trigger tag (server only).
 */
UCLASS()
class REDCELL_API URCGameplayAbility_Reset : public URCGameplayAbility
{
    GENERATED_BODY()

public:
    URCGameplayAbility_Reset(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};

USTRUCT(BlueprintType)
struct FRCPlayerResetMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> OwnerPlayerState = nullptr;
};
