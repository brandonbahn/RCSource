// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RCWeaponUserInterface.generated.h"

class URCWeaponInstance;
class UObject;
struct FGeometry;

/**
 * 
 */
UCLASS()
class URCWeaponUserInterface : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	URCWeaponUserInterface(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponChanged(URCWeaponInstance* OldWeapon, URCWeaponInstance* NewWeapon);

private:
	void RebuildWidgetFromWeapon();

	UPROPERTY(Transient)
	TObjectPtr<URCWeaponInstance> CurrentInstance;
};
