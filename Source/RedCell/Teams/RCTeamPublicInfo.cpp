// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTeamPublicInfo.h"

#include "Net/UnrealNetwork.h"
#include "Teams/RCTeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCTeamPublicInfo)

class FLifetimeProperty;

ARCTeamPublicInfo::ARCTeamPublicInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ARCTeamPublicInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

void ARCTeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<URCTeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	TryRegisterWithTeamSubsystem();
}

void ARCTeamPublicInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsystem();
}
