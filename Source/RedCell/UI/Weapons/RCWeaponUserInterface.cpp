// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Weapons/RCWeaponUserInterface.h"

#include "Equipment/RCEquipmentManagerComponent.h"
#include "Weapons/RCWeaponInstance.h"

URCWeaponUserInterface::URCWeaponUserInterface(const FObjectInitializer& ObjectInitializer)
{
}

void URCWeaponUserInterface::NativeConstruct()
{
	Super::NativeConstruct();
}

void URCWeaponUserInterface::NativeDestruct()
{
	Super::NativeDestruct();
}

void URCWeaponUserInterface::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (URCEquipmentManagerComponent* EquipmentManger = Pawn->FindComponentByClass<URCEquipmentManagerComponent>())
		{
			if (URCWeaponInstance* NewInstance = EquipmentManger->GetFirstInstanceOfType<URCWeaponInstance>())
			{
				if (NewInstance != CurrentInstance && NewInstance->GetInstigator() != nullptr)
				{
					URCWeaponInstance* OldWeapon = CurrentInstance;
					CurrentInstance = NewInstance;
					RebuildWidgetFromWeapon();
					OnWeaponChanged(OldWeapon, CurrentInstance);
				}
			}
		}
	}
}

void URCWeaponUserInterface::RebuildWidgetFromWeapon()
{
}
