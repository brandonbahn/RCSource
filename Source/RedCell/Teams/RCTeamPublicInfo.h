// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Teams/RCTeamInfoBase.h"
#include "RCTeamPublicInfo.generated.h"

class URCTeamCreationComponent;
class URCTeamDisplayAsset;
class UObject;
struct FFrame;

UCLASS()
class ARCTeamPublicInfo : public ARCTeamInfoBase
{
	GENERATED_BODY()
	
	friend URCTeamCreationComponent;

public:
	ARCTeamPublicInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	URCTeamDisplayAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

private:
	UFUNCTION()
	void OnRep_TeamDisplayAsset();

	void SetTeamDisplayAsset(TObjectPtr<URCTeamDisplayAsset> NewDisplayAsset);

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
	TObjectPtr<URCTeamDisplayAsset> TeamDisplayAsset;
};
