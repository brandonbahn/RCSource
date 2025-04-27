//
//  RCGameplayTags.h
//  RedCell
//
//  Created by Brandon Bahn on 4/23/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "NativeGameplayTags.h"

namespace RCGameplayTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

    // …add any more tags here…

    /** Optional runtime lookup helper */
    REDCELL_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);
}
