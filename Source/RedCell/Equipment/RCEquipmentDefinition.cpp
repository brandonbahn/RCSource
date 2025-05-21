// Fill out your copyright notice in the Description page of Project Settings.


#include "RCEquipmentDefinition.h"
#include "RCEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCEquipmentDefinition)

URCEquipmentDefinition::URCEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstanceType = URCEquipmentInstance::StaticClass();
}

