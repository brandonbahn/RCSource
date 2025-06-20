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
enum class E_MovementSituation : uint8;
enum class E_Gait : uint8;
enum class E_MovementState : uint8;
enum class E_RotationMode : uint8;
enum class E_Stance : uint8;
enum class E_MovementDirection : uint8;


namespace RCGameplayTags
{
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Action_Traverse);
	
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_MoveWorldSpace);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Gamepad);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Run);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Walk);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Sprint);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Strafe);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Aim);

	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputState_WantsToWalk);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputState_WantsToSprint);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputState_WantsToStrafe);
	REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputState_WantsToAim);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mana_Empty);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mana_Restored);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

    // …add any more tags here…

    /** Optional runtime lookup helper */
    REDCELL_API   FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

    // These are mappings from MovementMode enums to GameplayTags associated with those enums (below)

	REDCELL_API   extern const TMap<E_MovementSituation, FGameplayTag> MovementSituationTagMap;
    REDCELL_API   extern const TMap<E_Gait,              FGameplayTag> GaitTagMap;
	REDCELL_API   extern const TMap<E_MovementState,     FGameplayTag> MovementStateTagMap;
	REDCELL_API   extern const TMap<E_RotationMode,      FGameplayTag> RotationModeTagMap;
	REDCELL_API   extern const TMap<E_Stance,            FGameplayTag> StanceTagMap;
    REDCELL_API   extern const TMap<E_MovementDirection, FGameplayTag> MovementDirectionTagMap;


	REDCELL_API   extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
    REDCELL_API   extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

    REDCELL_API   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};

