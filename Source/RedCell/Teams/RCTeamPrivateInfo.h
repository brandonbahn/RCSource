// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Teams/RCTeamInfoBase.h"
#include "RCTeamPrivateInfo.generated.h"

class UObject;

UCLASS()
class ARCTeamPrivateInfo : public ARCTeamInfoBase
{
	GENERATED_BODY()
	
public:
	ARCTeamPrivateInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

