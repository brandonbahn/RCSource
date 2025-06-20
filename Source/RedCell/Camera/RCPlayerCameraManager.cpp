// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RCPlayerCameraManager.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/ActivateCameraRigFunctions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "Core/CameraRigAssetReference.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Character/RCPawnData.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerCameraManager)

class APlayerController;
class UCameraRigAsset;

ARCPlayerCameraManager::ARCPlayerCameraManager()
{
}

void ARCPlayerCameraManager::InitializeGameplayCamera(APawn* Pawn, const URCPawnData* PawnData)
{
	if (!Pawn || !PawnData || !PCOwner) return;
    
	const bool bUseGameplayCamera = UKismetSystemLibrary::GetConsoleVariableBoolValue(
		TEXT("DDCVar.NewGameplayCameraSystem.Enable"));
    
	if (!bUseGameplayCamera) 
	{
		// Fallback to traditional camera - call on PlayerController
		if (PCOwner)
		{
			PCOwner->SetViewTargetWithBlend(Pawn, 0.0f, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, false);
		}
		return;
	}

	// Find the GameplayCameraComponent
	if (UGameplayCameraComponent* GameplayCameraComp = Pawn->FindComponentByClass<UGameplayCameraComponent>())
	{
		// 1. ActivateCameraForPlayerController(GameplayCamera, PlayerController)
		GameplayCameraComp->ActivateCameraForPlayerController(PCOwner);
	}	
	if (UClass* FunctionLibraryClass = UActivateCameraRigFunctions::StaticClass())
	{
		if (UFunction* GlobalRigFunc = FunctionLibraryClass->FindFunctionByName(TEXT("ActivatePersistentGlobalCameraRig")))
		{
			if (PawnData->PersistentGlobalCameraRig)
			{
				struct { UObject* WorldContext; APlayerController* PC; UCameraRigAsset* Rig; } Params;
				Params.WorldContext = this;
				Params.PC = PCOwner;
				Params.Rig = PawnData->PersistentGlobalCameraRig;
				ProcessEvent(GlobalRigFunc, &Params);
			}
		}
        
		if (UFunction* BaseRigFunc = FunctionLibraryClass->FindFunctionByName(TEXT("ActivatePersistentBaseCameraRig")))
		{
			if (PawnData->PersistentBaseCameraRig)
			{
				struct { UObject* WorldContext; APlayerController* PC; UCameraRigAsset* Rig; } Params;
				Params.WorldContext = this;
				Params.PC = PCOwner;
				Params.Rig = PawnData->PersistentBaseCameraRig;
				ProcessEvent(BaseRigFunc, &Params);
			}
		}
	}
}


void ARCPlayerCameraManager::SetViewTargetWithBlend(AActor* NewViewTarget, float BlendWeight, bool bNoTransition)
{
	// Call SetViewTargetWithBlend on the PlayerController (PCOwner)
	if (PCOwner)
	{
		PCOwner->SetViewTargetWithBlend(NewViewTarget, BlendWeight, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, bNoTransition);
	}
}