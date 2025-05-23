// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StreamableManager.h"

DECLARE_DELEGATE_OneParam(FRCAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);

/** Handles reporting progress from streamable handles */
struct FRCAssetManagerStartupJob
{
	FRCAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;
	TFunction<void(const FRCAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;
	FString JobName;
	float JobWeight;
	mutable double LastUpdate = 0;

	/** Simple job that is all synchronous */
	FRCAssetManagerStartupJob(const FString& InJobName, const TFunction<void(const FRCAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc, float InJobWeight)
		: JobFunc(InJobFunc)
		, JobName(InJobName)
		, JobWeight(InJobWeight)
	{}

	/** Perform actual loading, will return a handle if it created one */
	TSharedPtr<FStreamableHandle> DoJob() const;

	void UpdateSubstepProgress(float NewProgress) const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
	{
		if (SubstepProgressDelegate.IsBound())
		{
			// StreamableHandle::GetProgress traverses() a large graph and is quite expensive
			double Now = FPlatformTime::Seconds();
			if (LastUpdate - Now > 1.0 / 60)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
};
