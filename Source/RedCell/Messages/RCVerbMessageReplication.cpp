// Fill out your copyright notice in the Description page of Project Settings.


#include "RCVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/RCVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FRCVerbMessageReplicationEntry

FString FRCVerbMessageReplicationEntry::GetDebugString() const
{
	return Message.ToString();
}

//////////////////////////////////////////////////////////////////////
// FRCVerbMessageReplication

void FRCVerbMessageReplication::AddMessage(const FRCVerbMessage& Message)
{
	FRCVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);
	MarkItemDirty(NewStack);
}

void FRCVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// 	for (int32 Index : RemovedIndices)
	// 	{
	// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
	// 		TagToCountMap.Remove(Tag);
	// 	}
}

void FRCVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FRCVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FRCVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FRCVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FRCVerbMessageReplication::RebroadcastMessage(const FRCVerbMessage& Message)
{
	check(Owner);
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}

