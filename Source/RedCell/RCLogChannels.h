// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

class UObject;

REDCELL_API DECLARE_LOG_CATEGORY_EXTERN(LogRC, Log, All);
REDCELL_API DECLARE_LOG_CATEGORY_EXTERN(LogRCExperience, Log, All);
REDCELL_API DECLARE_LOG_CATEGORY_EXTERN(LogRCAbilitySystem, Log, All);
REDCELL_API DECLARE_LOG_CATEGORY_EXTERN(LogRCTeams, Log, All);

REDCELL_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

