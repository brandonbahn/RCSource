// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cosmetics/RCCosmeticAnimationTypes.h"
#include "Equipment/RCEquipmentInstance.h"
#include "GameFramework/InputDevicePropertyHandle.h"

#include "RCWeaponInstance.generated.h"

class UAnimInstance;
class UObject;
struct FFrame;
struct FGameplayTagContainer;
class UInputDeviceProperty;

/**
 * A piece of equipment representing a weapon spawned and applied to a pawn
 */
UCLASS()
class REDCELL_API URCWeaponInstance : public URCEquipmentInstance
{
	GENERATED_BODY()
	
public:
    URCWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~URCEquipmentInstance interface
    virtual void OnEquipped() override;
    virtual void OnUnequipped() override;
    //~End of URCEquipmentInstance interface

    UFUNCTION(BlueprintCallable)
    void UpdateFiringTime();

    // Returns how long it's been since the weapon was interacted with (fired or equipped)
    UFUNCTION(BlueprintPure)
    float GetTimeSinceLastInteractedWith() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
    FRCAnimLayerSelectionSet EquippedAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
    FRCAnimLayerSelectionSet UnequippedAnimSet;

    /**
     * Device properties that should be applied while this weapon is equipped.
     * These properties will be played in with the "Looping" flag enabled, so they will
     * play continuously until this weapon is unequipped!
     */
    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Input Devices")
    TArray<TObjectPtr<UInputDeviceProperty>> ApplicableDeviceProperties;
    
    // Choose the best layer from EquippedAnimSet or UneuippedAnimSet based on the specified gameplay tags
    UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Animation)
    TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const;

    /** Returns the owning Pawn's Platform User ID */
    UFUNCTION(BlueprintCallable)
    const FPlatformUserId GetOwningUserId() const;

    /** Callback for when the owning pawn of this weapon dies. Removes all spawned device properties. */
    UFUNCTION()
    void OnDeathStarted(AActor* OwningActor);

    /**
     * Apply the ApplicableDeviceProperties to the owning pawn of this weapon.
     * Populate the DevicePropertyHandles so that they can be removed later. This will
     * Play the device properties in Looping mode so that they will share the lifetime of the
     * weapon being Equipped.
     */
    void ApplyDeviceProperties();

    /** Remove any device proeprties that were activated in ApplyDeviceProperties. */
    void RemoveDeviceProperties();

private:

    /** Set of device properties activated by this weapon. Populated by ApplyDeviceProperties */
    UPROPERTY(Transient)
    TSet<FInputDevicePropertyHandle> DevicePropertyHandles;

    double TimeLastEquipped = 0.0;
    double TimeLastFired = 0.0;
};

