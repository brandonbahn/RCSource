// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameViewportClient.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameViewportClient)

class UGameInstance;

namespace GameViewportTags
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_HardwareCursor, "Platform.Trait.Input.HardwareCursor");
}

URCGameViewportClient::URCGameViewportClient()
	: Super(FObjectInitializer::Get())
{
}

void URCGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);
	
	// We have software cursors set up in our project settings for console/mobile use, but on desktop we're fine with
	// the standard hardware cursors
	const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameViewportTags::TAG_Platform_Trait_Input_HardwareCursor);
	SetUseSoftwareCursorWidgets(!UseHardwareCursor);
}
