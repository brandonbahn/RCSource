//
//  RCGameplayTags.cpp
//  RedCell
//
//  Created by Brandon Bahn on 4/23/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#include "RCGameplayTags.h"
#include "GameplayTagsManager.h"

namespace RCGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead,   "Ability.ActivateFail.IsDead",   "Failed activation because owner is dead.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Failed activation due to cooldown.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death,       "Status.Death",       "Actor is dead.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Actor is dying.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead,  "Status.Death.Dead",  "Actor is fully dead.");


    // …and any others you declared…

    FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
    {
        return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound=*/false);
    }
}

