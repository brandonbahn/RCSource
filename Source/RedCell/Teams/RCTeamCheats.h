// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "RCTeamCheats.generated.h"

class UObject;
struct FFrame;

/**
 * Cheats related to teams
 */
UCLASS()
class URCTeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()
	
public:
	// Moves this player to the next available team, wrapping around to the
	// first team if at the end of the list of teams
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CycleTeam();

	// Moves this player to the specified team
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamID);

	// Prints a list of all of the teams
	UFUNCTION(Exec)
	virtual void ListTeams();
};

