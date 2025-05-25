//
//  RCGameplayTags.cpp
//  RedCell
//
//  Created by Brandon Bahn on 4/23/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#include "RCGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Character/RCMovementModes.h"

namespace RCGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead,   "Ability.ActivateFail.IsDead",   "Failed activation because owner is dead.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Failed activation due to cooldown.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Slide, "InputTag.Slide", "Slide input.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death,       "Status.Death",       "Actor is dead.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Actor is dying.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead,  "Status.Death.Dead",  "Actor is fully dead.");


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mana_Empty, "GameplayEvent.Mana.Empty", "Event that fires once player mana is empty.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mana_Restored, "GameplayEvent.Mana.Restored", "Event that fires once player mana is restored.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");

    // These are mapped to the movement modes inside GetMovementModeTagMap()
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

    // Create your own movement modes but you need to update GetCustomMovementModeTagMap()
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See RCGameplayTags::CustomMovementModeTagMap.");

    // Unreal Movement Modes
    const TMap<uint8, FGameplayTag> MovementModeTagMap =
    {
        { MOVE_Walking, Movement_Mode_Walking },
        { MOVE_NavWalking, Movement_Mode_NavWalking },
        { MOVE_Falling, Movement_Mode_Falling },
        { MOVE_Swimming, Movement_Mode_Swimming },
        { MOVE_Flying, Movement_Mode_Flying },
        { MOVE_Custom, Movement_Mode_Custom }
    };

    // Gait
    const TMap<E_Gait, FGameplayTag> GaitTagMap =
    {
        { E_Gait::Walk,   FGameplayTag::RequestGameplayTag("Movement.Gait.Walking") },
        { E_Gait::Run,    FGameplayTag::RequestGameplayTag("Movement.Gait.Running") },
        { E_Gait::Sprint, FGameplayTag::RequestGameplayTag("Movement.Gait.Sprinting") },
    };

    // MovementDirection
    const TMap<E_MovementDirection, FGameplayTag> MovementDirectionTagMap =
    {
        { E_MovementDirection::None,     FGameplayTag::RequestGameplayTag("Movement.Direction.None") },
        { E_MovementDirection::Forward,  FGameplayTag::RequestGameplayTag("Movement.Direction.Forward") },
        { E_MovementDirection::Backward, FGameplayTag::RequestGameplayTag("Movement.Direction.Backward") },
        { E_MovementDirection::Right,    FGameplayTag::RequestGameplayTag("Movement.Direction.Right") },
        { E_MovementDirection::Left,     FGameplayTag::RequestGameplayTag("Movement.Direction.Left") },
    };

    // MovementMode
    const TMap<E_MovementMode, FGameplayTag> MoveModeTagMap =
    {
        { E_MovementMode::OnGround,   FGameplayTag::RequestGameplayTag("Movement.Mode.OnGround") },
        { E_MovementMode::InAir, FGameplayTag::RequestGameplayTag("Movement.Mode.InAir") },
    };

    // MovementState
    const TMap<E_MovementState, FGameplayTag> MovementStateTagMap =
    {
        { E_MovementState::Idle,   FGameplayTag::RequestGameplayTag("Movement.State.Idle") },
        { E_MovementState::Moving, FGameplayTag::RequestGameplayTag("Movement.State.Moving") },
    };

    // RotationMode
    const TMap<E_RotationMode, FGameplayTag> RotationModeTagMap =
    {
        { E_RotationMode::OrientToMovement,   FGameplayTag::RequestGameplayTag("Movement.Rotation.OrientToMovement") },
        { E_RotationMode::Strafe, FGameplayTag::RequestGameplayTag("Movement.Rotation.Strafe") },
    };

    // Stance
    const TMap<E_Stance, FGameplayTag> StanceTagMap =
    {
        { E_Stance::Stand,  FGameplayTag::RequestGameplayTag("Movement.Stance.Standing") },
        { E_Stance::Crouch, FGameplayTag::RequestGameplayTag("Movement.Stance.Crouched") },
    };

    // Custom Movement Modes
    const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
    {
       // Fill these in with your custom modes
    };


    // …and any others you declared…

    FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
    {
        return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound=*/false);
    }
}
