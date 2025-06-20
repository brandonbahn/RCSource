// Fill out your copyright notice in the Description page of Project Settings.

#include "RCPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPawnData)

URCPawnData::URCPawnData(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnClass = nullptr;
    InputConfig = nullptr;
    PersistentGlobalCameraRig = nullptr;
    PersistentBaseCameraRig = nullptr;
}
