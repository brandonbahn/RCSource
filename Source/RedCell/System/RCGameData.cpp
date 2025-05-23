// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameData.h"
#include "RCAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameData)

URCGameData::URCGameData()
{
}

const URCGameData& URCGameData::URCGameData::Get()
{
	return URCAssetManager::Get().GetGameData();
}
