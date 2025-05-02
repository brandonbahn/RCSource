//
//  RCGameplayTags.h
//  RedCell
//
//  Created by Brandon Bahn on 4/23/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "NativeGameplayTags.h"

/*current GASP enums for MotionMatching*/
enum class E_Gait : uint8;
enum class E_MovementDirection : uint8;
enum class E_MovementMode : uint8;
enum class E_MovementState : uint8;
enum class E_RotationMode : uint8;
enum class E_Stance : uint8;


namespace RCGameplayTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Slide);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);


    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

    // …add any more tags here…

    /** Optional runtime lookup helper */
    REDCELL_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

    // These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
    extern const TMap<uint8, FGameplayTag> MovementModeTagMap;

    extern const TMap<E_Gait,              FGameplayTag> GaitTagMap;
    extern const TMap<E_MovementDirection, FGameplayTag> MovementDirectionTagMap;
    extern const TMap<E_MovementMode,      FGameplayTag> MoveModeTagMap;
    extern const TMap<E_MovementState,     FGameplayTag> MovementStateTagMap;
    extern const TMap<E_RotationMode,      FGameplayTag> RotationModeTagMap;
    extern const TMap<E_Stance,            FGameplayTag> StanceTagMap;
    
    extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};

