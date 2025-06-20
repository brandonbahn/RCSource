// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemGlobals.h"
#include "RCMovementModes.h"
#include "RCGameplayTags.h"
#include "RCCharacterMovementComponent.generated.h"

class URCInputConfig;
class UCurveFloat;

/**
 * The base character movement component class used by this project.
 */
UCLASS(Config = Game)
class REDCELL_API URCCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	URCCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	//~Begin UMovementComponent interface
    virtual float GetMaxSpeed() const override;
    virtual float GetMaxAcceleration() const override;
    virtual float GetMaxBrakingDeceleration() const override;
    //~End UMovementComponent interface
	
    UFUNCTION(BlueprintPure, Category="Movement")
		virtual E_Gait GetDesiredGait() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
		bool CanSprint() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
		virtual float CalculateMaxAcceleration() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
		virtual float CalculateBrakingDeceleration() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
		virtual float CalculateGroundFriction() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
		virtual float CalculateMaxSpeed() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
		virtual float CalculateMaxCrouchSpeed() const;

	virtual bool CanAttemptJump() const override;

protected:

	virtual void InitializeComponent() override;
	
    // Speed values for different gaits (from your GASP character)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gait Speeds")
		FVector WalkSpeeds = FVector(200.0f, 180.0f, 150.0f);  // Forward, Strafe, Backward
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gait Speeds")
		FVector RunSpeeds = FVector(500.0f, 350.0f, 300.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gait Speeds")
		FVector SprintSpeeds = FVector(750.0f, 750.0f, 750.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gait Speeds")
		FVector CrouchSpeeds = FVector(225.0f, 200.0f, 180.0f);

    // Strafe speed curve (from your GASP character)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    TSoftObjectPtr<UCurveFloat> StrafeSpeedMapCurve;

private:
    // Helper functions
    bool HasInputStateTag(const FGameplayTag& Tag) const;
    float GetMovementInputMagnitude() const;
    FVector GetMovementInputVector() const;

	// Cached InputConfig for performance
	UPROPERTY()
	TObjectPtr<const URCInputConfig> InputConfig;
    
	// Initialize cached data
	void GetInputConfig();
    
	// Simple accessors using cached data
	bool IsFullMovementInput() const;
	float GetAnalogWalkRunThreshold() const;
};


