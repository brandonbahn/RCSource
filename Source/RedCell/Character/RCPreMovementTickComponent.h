// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "RCPreMovementTickComponent.generated.h"

/**
 * This is a simple component which allows us to execute certain functions before the CMC, using the tick pre-requisite system.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REDCELL_API URCPreMovementTickComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:	
	URCPreMovementTickComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class URCCharacterMovementComponent* RCCharacterMovementComponent;
};
