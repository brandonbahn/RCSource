// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameplayCueManager.h"
#include "Engine/AssetManager.h"
#include "RCLogChannels.h"
#include "GameplayCueSet.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "UObject/UObjectThreadContext.h"
#include "Async/Async.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayCueManager)

//////////////////////////////////////////////////////////////////////

enum class ERCEditorLoadMode
{
	// Loads all cues upfront; longer loading speed in the editor but short PIE times and effects never fail to play
	LoadUpfront,

	// Outside of editor: Async loads as cue tag are registered
	// In editor: Async loads when cues are invoked
	//   Note: This can cause some 'why didn't I see the effect for X' issues in PIE and is good for iteration speed but otherwise bad for designers
	PreloadAsCuesAreReferenced_GameOnly,

	// Async loads as cue tag are registered
	PreloadAsCuesAreReferenced
};

namespace RCGameplayCueManagerCvars
{
	static FAutoConsoleCommand CVarDumpGameplayCues(
		TEXT("RC.DumpGameplayCues"),
		TEXT("Shows all assets that were loaded via RCGameplayCueManager and are currently in memory."),
		FConsoleCommandWithArgsDelegate::CreateStatic(URCGameplayCueManager::DumpGameplayCues));

	static ERCEditorLoadMode LoadMode = ERCEditorLoadMode::LoadUpfront;
}

const bool bPreloadEvenInEditor = true;

//////////////////////////////////////////////////////////////////////

struct FGameplayCueTagThreadSynchronizeGraphTask : public FAsyncGraphTaskBase
{
	TFunction<void()> TheTask;
	FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) : TheTask(MoveTemp(Task)) { }
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) { TheTask(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
};

//////////////////////////////////////////////////////////////////////

URCGameplayCueManager::URCGameplayCueManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

URCGameplayCueManager* URCGameplayCueManager::Get()
{
	return Cast<URCGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

void URCGameplayCueManager::OnCreated()
{
	Super::OnCreated();

	UpdateDelayLoadDelegateListeners();
}

void URCGameplayCueManager::LoadAlwaysLoadedCues()
{
	if (ShouldDelayLoadGameplayCues())
	{
		UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	
		//@TODO: Try to collect these by filtering GameplayCue. tags out of native gameplay tags?
		TArray<FName> AdditionalAlwaysLoadedCueTags;

		for (const FName& CueTagName : AdditionalAlwaysLoadedCueTags)
		{
			FGameplayTag CueTag = TagManager.RequestGameplayTag(CueTagName, /*ErrorIfNotFound=*/ false);
			if (CueTag.IsValid())
			{
				ProcessTagToPreload(CueTag, nullptr);
			}
			else
			{
				UE_LOG(LogRC, Warning, TEXT("URCGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"), *CueTagName.ToString());
			}
		}
	}
}

bool URCGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	switch (RCGameplayCueManagerCvars::LoadMode)
	{
	case ERCEditorLoadMode::LoadUpfront:
		return true;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return false;
		}
#endif
		break;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	return !ShouldDelayLoadGameplayCues();
}

bool URCGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
	return false;
}

bool URCGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
	return true;
}

void URCGameplayCueManager::DumpGameplayCues(const TArray<FString>& Args)
{
	URCGameplayCueManager* GCM = Cast<URCGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
	if (!GCM)
	{
		UE_LOG(LogRC, Error, TEXT("DumpGameplayCues failed. No URCGameplayCueManager found."));
		return;
	}

	const bool bIncludeRefs = Args.Contains(TEXT("Refs"));

	UE_LOG(LogRC, Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
	for (UClass* CueClass : GCM->AlwaysLoadedCues)
	{
		UE_LOG(LogRC, Log, TEXT("  %s"), *GetPathNameSafe(CueClass));
	}

	UE_LOG(LogRC, Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
	for (UClass* CueClass : GCM->PreloadedCues)
	{
		TSet<FObjectKey>* ReferencerSet = GCM->PreloadedCueReferencers.Find(CueClass);
		int32 NumRefs = ReferencerSet ? ReferencerSet->Num() : 0;
		UE_LOG(LogRC, Log, TEXT("  %s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);
		if (bIncludeRefs && ReferencerSet)
		{
			for (const FObjectKey& Ref : *ReferencerSet)
			{
				UObject* RefObject = Ref.ResolveObjectPtr();
				UE_LOG(LogRC, Log, TEXT("    ^- %s"), *GetPathNameSafe(RefObject));
			}
		}
	}

	UE_LOG(LogRC, Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
	int32 NumMissingCuesLoaded = 0;
	if (GCM->RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const FGameplayCueNotifyData& CueData : GCM->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
		{
			if (CueData.LoadedGameplayCueClass && !GCM->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass) && !GCM->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
			{
				NumMissingCuesLoaded++;
				UE_LOG(LogRC, Log, TEXT("  %s"), *CueData.LoadedGameplayCueClass->GetPathName());
			}
		}
	}

	UE_LOG(LogRC, Log, TEXT("=========== Gameplay Cue Notify summary ==========="));
	UE_LOG(LogRC, Log, TEXT("  ... %d cues in always loaded list"), GCM->AlwaysLoadedCues.Num());
	UE_LOG(LogRC, Log, TEXT("  ... %d cues in preloaded list"), GCM->PreloadedCues.Num());
	UE_LOG(LogRC, Log, TEXT("  ... %d cues loaded on demand"), NumMissingCuesLoaded);
	UE_LOG(LogRC, Log, TEXT("  ... %d cues in total"), GCM->AlwaysLoadedCues.Num() + GCM->PreloadedCues.Num() + NumMissingCuesLoaded);
}

void URCGameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
	FScopeLock ScopeLock(&LoadedGameplayTagsToProcessCS);
	bool bStartTask = LoadedGameplayTagsToProcess.Num() == 0;
	FUObjectSerializeContext* LoadContext = FUObjectThreadContext::Get().GetSerializeContext();
	UObject* OwningObject = LoadContext ? LoadContext->SerializedObject : nullptr;
	LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);
	if (bStartTask)
	{
		TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady([]()
			{
				if (GIsRunning)
				{
					if (URCGameplayCueManager* StrongThis = Get())
					{
						// If we are garbage collecting we cannot call StaticFindObject (or a few other static uobject functions), so we'll just wait until the GC is over and process the tags then
						if (IsGarbageCollecting())
						{
							StrongThis->bProcessLoadedTagsAfterGC = true;
						}
						else
						{
							StrongThis->ProcessLoadedTags();
						}
					}
				}
			});
	}
}

void URCGameplayCueManager::HandlePostGarbageCollect()
{
	if (bProcessLoadedTagsAfterGC)
	{
		ProcessLoadedTags();
	}
	bProcessLoadedTagsAfterGC = false;
}

void URCGameplayCueManager::ProcessLoadedTags()
{
	TArray<FLoadedGameplayTagToProcessData> TaskLoadedGameplayTagsToProcess;
	{
		// Lock LoadedGameplayTagsToProcess just long enough to make a copy and clear
		FScopeLock TaskScopeLock(&LoadedGameplayTagsToProcessCS);
		TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
		LoadedGameplayTagsToProcess.Empty();
	}

	// This might return during shutdown, and we don't want to proceed if that is the case
	if (GIsRunning)
	{
		if (RuntimeGameplayCueObjectLibrary.CueSet)
		{
			for (const FLoadedGameplayTagToProcessData& LoadedTagData : TaskLoadedGameplayTagsToProcess)
			{
				if (RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(LoadedTagData.Tag))
				{
					if (!LoadedTagData.WeakOwner.IsStale())
					{
						ProcessTagToPreload(LoadedTagData.Tag, LoadedTagData.WeakOwner.Get());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogRC, Warning, TEXT("URCGameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing."));
		}
	}
}

void URCGameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
	switch (RCGameplayCueManagerCvars::LoadMode)
	{
	case ERCEditorLoadMode::LoadUpfront:
		return;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	check(RuntimeGameplayCueObjectLibrary.CueSet);

	int32* DataIdx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag);
	if (DataIdx && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
	{
		const FGameplayCueNotifyData& CueData = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx];

		UClass* LoadedGameplayCueClass = FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString());
		if (LoadedGameplayCueClass)
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
		}
		else
		{
			bool bAlwaysLoadedCue = OwningObject == nullptr;
			TWeakObjectPtr<UObject> WeakOwner = OwningObject;
			StreamableManager.RequestAsyncLoad(CueData.GameplayCueNotifyObj, FStreamableDelegate::CreateUObject(this, &ThisClass::OnPreloadCueComplete, CueData.GameplayCueNotifyObj, WeakOwner, bAlwaysLoadedCue), FStreamableManager::DefaultAsyncLoadPriority, false, false, TEXT("GameplayCueManager"));
		}
	}
}

void URCGameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue)
{
	if (bAlwaysLoadedCue || OwningObject.IsValid())
	{
		if (UClass* LoadedGameplayCueClass = Cast<UClass>(Path.ResolveObject()))
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject.Get());
		}
	}
}

void URCGameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
	check(LoadedGameplayCueClass);

	const bool bAlwaysLoadedCue = OwningObject == nullptr;
	if (bAlwaysLoadedCue)
	{
		AlwaysLoadedCues.Add(LoadedGameplayCueClass);
		PreloadedCues.Remove(LoadedGameplayCueClass);
		PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
	}
	else if ((OwningObject != LoadedGameplayCueClass) && (OwningObject != LoadedGameplayCueClass->GetDefaultObject()) && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass))
	{
		PreloadedCues.Add(LoadedGameplayCueClass);
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass);
		ReferencerSet.Add(OwningObject);
	}
}

void URCGameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
	if (RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (UClass* CueClass : AlwaysLoadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}

		for (UClass* CueClass : PreloadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}
	}

	for (auto CueIt = PreloadedCues.CreateIterator(); CueIt; ++CueIt)
	{
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindChecked(*CueIt);
		for (auto RefIt = ReferencerSet.CreateIterator(); RefIt; ++RefIt)
		{
			if (!RefIt->ResolveObjectPtr())
			{
				RefIt.RemoveCurrent();
			}
		}
		if (ReferencerSet.Num() == 0)
		{
			PreloadedCueReferencers.Remove(*CueIt);
			CueIt.RemoveCurrent();
		}
	}
}

void URCGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	switch (RCGameplayCueManagerCvars::LoadMode)
	{
	case ERCEditorLoadMode::LoadUpfront:
		return;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;
	case ERCEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

bool URCGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
	const bool bClientDelayLoadGameplayCues = true;
	return !IsRunningDedicatedServer() && bClientDelayLoadGameplayCues;
}

const FPrimaryAssetType UFortAssetManager_GameplayCueRefsType = TEXT("GameplayCueRefs");
const FName UFortAssetManager_GameplayCueRefsName = TEXT("GameplayCueReferences");
const FName UFortAssetManager_LoadStateClient = FName(TEXT("Client"));

void URCGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
	TArray<FSoftObjectPath> CuePaths;
	UGameplayCueSet* RuntimeGameplayCueSet = GetRuntimeCueSet();
	if (RuntimeGameplayCueSet)
	{
		RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
	}

	FAssetBundleData BundleData;
	BundleData.AddBundleAssetsTruncated(UFortAssetManager_LoadStateClient, CuePaths);

	FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(UFortAssetManager_GameplayCueRefsType, UFortAssetManager_GameplayCueRefsName);
	UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}


