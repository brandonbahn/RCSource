//
//  RCCollisionChannels.h
//  RedCell
//
//  Created by Brandon Bahn on 5/21/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

/**
 * when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list
 **/

// Trace against Actors/Components which provide interactions.
#define RC_TraceChannel_Interaction                    ECC_GameTraceChannel2

// Trace used by weapons, will hit physics assets instead of capsules
#define RC_TraceChannel_Weapon                        ECC_GameTraceChannel3

// Trace used by by weapons, will hit pawn capsules instead of physics assets
#define RC_TraceChannel_Weapon_Capsule                ECC_GameTraceChannel4

// Trace used by by weapons, will trace through multiple pawns rather than stopping on the first hit
#define RC_TraceChannel_Weapon_Multi                    ECC_GameTraceChannel5

// Allocated to aim assist by the ShooterCore game feature
// ECC_GameTraceChannel5
