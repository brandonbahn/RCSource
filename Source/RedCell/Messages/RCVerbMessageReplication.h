// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RCVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "RCVerbMessageReplication.generated.h"

class UObject;
struct FRCVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FRCVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRCVerbMessageReplicationEntry()
	{}

	FRCVerbMessageReplicationEntry(const FRCVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FRCVerbMessageReplication;

	UPROPERTY()
	FRCVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FRCVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FRCVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FRCVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRCVerbMessageReplicationEntry, FRCVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FRCVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FRCVerbMessageReplicationEntry> CurrentMessages;
	
	// Owner (for a route to a world)
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FRCVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FRCVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
