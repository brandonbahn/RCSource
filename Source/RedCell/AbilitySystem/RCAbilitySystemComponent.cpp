// Fill out your copyright notice in the Description page of Project Settings.

#include "RCAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/RCGameplayAbility.h"
#include "AbilitySystem/RCAbilityTagRelationshipMapping.h"
#include "Animation/RCAnimInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "RCGlobalAbilitySystem.h"
#include "RCLogChannels.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "System/RCAssetManager.h"
#include "System/RCGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCAbilitySystemComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

URCAbilitySystemComponent::URCAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();

    FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void URCAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (URCGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<URCGlobalAbilitySystem>(GetWorld()))
    {
        GlobalAbilitySystem->UnregisterASC(this);
    }

    Super::EndPlay(EndPlayReason);
}

void URCAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
    FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
    check(ActorInfo);
    check(InOwnerActor);

    const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

    Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

    if (bHasNewPawnAvatar)
    {
        // Notify all abilities that a new pawn avatar has been set
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            PRAGMA_DISABLE_DEPRECATION_WARNINGS
                        ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
            PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
                        TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
            for (UGameplayAbility* AbilityInstance : Instances)
            {
                URCGameplayAbility* RCAbilityInstance = Cast<URCGameplayAbility>(AbilityInstance);
                if (RCAbilityInstance)
                {
                    // Ability instances may be missing for replays
                    RCAbilityInstance->OnPawnAvatarSet();
                }
            }
        }

        // Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
        if (URCGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<URCGlobalAbilitySystem>(GetWorld()))
        {
            GlobalAbilitySystem->RegisterASC(this);
        }

        if (URCAnimInstance* RCAnimInst = Cast<URCAnimInstance>(ActorInfo->GetAnimInstance()))
        {
            RCAnimInst->InitializeWithAbilitySystem(this);
        }

        TryActivateAbilitiesOnSpawn();
    }
}

void URCAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (const URCGameplayAbility* RCAbilityCDO = Cast<URCGameplayAbility>(AbilitySpec.Ability))
        {
            RCAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
        }
    }
}

void URCAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (!AbilitySpec.IsActive())
        {
            continue;
        }

        URCGameplayAbility* RCAbilityCDO = Cast<URCGameplayAbility>(AbilitySpec.Ability);
        if (!RCAbilityCDO)
        {
            UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Non-RCGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
            continue;
        }

PRAGMA_DISABLE_DEPRECATION_WARNINGS
        ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
            
        // Cancel all the spawned instances.
        TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
        for (UGameplayAbility* AbilityInstance : Instances)
        {
            URCGameplayAbility* RCAbilityInstance = CastChecked<URCGameplayAbility>(AbilityInstance);

            if (ShouldCancelFunc(RCAbilityInstance, AbilitySpec.Handle))
            {
                if (RCAbilityInstance->CanBeCanceled())
                {
                    RCAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), RCAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *RCAbilityInstance->GetName());
                }
            }
        }
    }
}

void URCAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const URCGameplayAbility* RCAbility, FGameplayAbilitySpecHandle Handle)
	{
		const ERCAbilityActivationPolicy ActivationPolicy = RCAbility->GetActivationPolicy();
		return ((ActivationPolicy == ERCAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == ERCAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void URCAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void URCAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

int32 URCAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    UE_LOG(LogTemp, Warning, TEXT("[ASC] HandleGameplayEvent: %s"), *EventTag.ToString());
    return Super::HandleGameplayEvent(EventTag, Payload);
}

/*
void URCAbilitySystemComponent::AddAbilitySet(URCAbilitySet* AbilitySet)
{
    if (AbilitySet)
    {
        UE_LOG(LogTemp, Log, TEXT("[ASC] Granting AbilitySet %s"), *AbilitySet->GetName());
        AbilitySet->GiveAbilities(this);
    }
}
*/

void URCAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

void URCAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}

void URCAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const URCGameplayAbility* RCAbilityCDO = Cast<URCGameplayAbility>(AbilitySpec->Ability);
				if (RCAbilityCDO && RCAbilityCDO->GetActivationPolicy() == ERCAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const URCGameplayAbility* RCAbilityCDO = Cast<URCGameplayAbility>(AbilitySpec->Ability);

					if (RCAbilityCDO && RCAbilityCDO->GetActivationPolicy() == ERCAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void URCAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void URCAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (URCGameplayAbility* RCAbility = Cast<URCGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(RCAbility->GetActivationGroup(), RCAbility);
	}
}

void URCAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void URCAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (URCGameplayAbility* RCAbility = Cast<URCGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(RCAbility->GetActivationGroup(), RCAbility);
	}
}

void URCAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

void URCAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}

void URCAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void URCAbilitySystemComponent::SetTagRelationshipMapping(URCAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void URCAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void URCAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	//UE_LOG(LogRCAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const URCGameplayAbility* RCAbility = Cast<const URCGameplayAbility>(Ability))
	{
		RCAbility->OnAbilityFailedToActivate(FailureReason);
	}	
}

bool URCAbilitySystemComponent::IsActivationGroupBlocked(ERCAbilityActivationGroup Group) const
{
    bool bBlocked = false;

    switch (Group)
    {
    case ERCAbilityActivationGroup::Independent:
        // Independent abilities are never blocked.
        bBlocked = false;
        break;

    case ERCAbilityActivationGroup::Exclusive_Replaceable:
    case ERCAbilityActivationGroup::Exclusive_Blocking:
        // Exclusive abilities can activate if nothing is blocking.
        bBlocked = (ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Blocking] > 0);
        break;

    default:
        checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
        break;
    }

    return bBlocked;
}

void URCAbilitySystemComponent::AddAbilityToActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility)
{
    check(RCAbility);
    check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

    ActivationGroupCounts[(uint8)Group]++;

    const bool bReplicateCancelAbility = false;

    switch (Group)
    {
    case ERCAbilityActivationGroup::Independent:
        // Independent abilities do not cancel any other abilities.
        break;

    case ERCAbilityActivationGroup::Exclusive_Replaceable:
    case ERCAbilityActivationGroup::Exclusive_Blocking:
        CancelActivationGroupAbilities(ERCAbilityActivationGroup::Exclusive_Replaceable, RCAbility, bReplicateCancelAbility);
        break;

    default:
        checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
        break;
    }

    const int32 ExclusiveCount = ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::Exclusive_Blocking];
    if (!ensure(ExclusiveCount <= 1))
    {
        UE_LOG(LogTemp, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
    }
}

void URCAbilitySystemComponent::RemoveAbilityFromActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility)
{
    check(RCAbility);
    check(ActivationGroupCounts[(uint8)Group] > 0);

    ActivationGroupCounts[(uint8)Group]--;
}

void URCAbilitySystemComponent::CancelActivationGroupAbilities(ERCAbilityActivationGroup Group, URCGameplayAbility* IgnoreRCAbility, bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this, Group, IgnoreRCAbility](const URCGameplayAbility* RCAbility, FGameplayAbilitySpecHandle Handle)
    {
        return ((RCAbility->GetActivationGroup() == Group) && (RCAbility != IgnoreRCAbility));
    };

    CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void URCAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = URCAssetManager::GetSubclass(URCGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogRCAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *URCGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (!Spec)
	{
		UE_LOG(LogRCAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
		return;
	}

	Spec->DynamicGrantedTags.AddTag(Tag);

	ApplyGameplayEffectSpecToSelf(*Spec);
}

void URCAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = URCAssetManager::GetSubclass(URCGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogRCAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *URCGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;

	RemoveActiveEffects(Query);
}

void URCAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}



bool URCAbilitySystemComponent::IsAbilityActiveByTag(FGameplayTag AbilityTag) const
{
    for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (!Spec.IsActive())
        {
            continue;
        }
        // Use the ability's tags to identify it
        if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}

bool URCAbilitySystemComponent::IsAbilityGrantedByTag(FGameplayTag AbilityTag) const
{
    for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}
