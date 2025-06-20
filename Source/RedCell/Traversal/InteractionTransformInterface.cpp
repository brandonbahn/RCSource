// Fill out your copyright notice in the Description page of Project Settings.


#include "Traversal/InteractionTransformInterface.h"

bool UInteractionTransformBlueprintFunctionLibrary::SetInteractionTransform(UObject* InteractionTransformObject,
	const FTransform& InteractionTransform)
{
	if (IsValid(InteractionTransformObject) &&
		InteractionTransformObject->Implements<UInteractionTransformInterface>())
	{
		IInteractionTransformInterface::Execute_SetInteractionTransform(InteractionTransformObject, InteractionTransform);
		return true;
	}
	return false;
}

bool UInteractionTransformBlueprintFunctionLibrary::GetInteractionTransform(UObject* InteractionTransformObject,
	FTransform& OutInteractionTransform)
{
	if (IsValid(InteractionTransformObject) &&
		InteractionTransformObject->Implements<UInteractionTransformInterface>())
	{
		IInteractionTransformInterface::Execute_GetInteractionTransform(InteractionTransformObject, OutInteractionTransform);
		return true;
	}
	return false;
}

