// Fill out your copyright notice in the Description page of Project Settings.


#include "RCWorldCollectable.h"

#include "Async/TaskGraphInterfaces.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCWorldCollectable)

struct FInteractionQuery;

ARCWorldCollectable::ARCWorldCollectable()
{
}

void ARCWorldCollectable::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

FInventoryPickup ARCWorldCollectable::GetPickupInventory() const
{
	return StaticInventory;
}
