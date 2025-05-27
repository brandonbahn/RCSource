// Fill out your copyright notice in the Description page of Project Settings.
// RCHealthComponent.cpp

#include "Character/RCHealthComponent.h"

#include "AbilitySystem/Attributes/RCAttributeSet.h"
#include "RCLogChannels.h"
#include "System/RCAssetManager.h"
#include "System/RCGameData.h"
#include "RCGameplayTags.h"
#include "GameplayEffectExtension.h"                    // for FOnAttributeChangeData & FGameplayEffectModCallbackData
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "Messages/RCVerbMessage.h"
#include "Messages/RCVerbMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCHealthComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RedCell_Elimination_Message, "RedCell.Elimination.Message");

URCHealthComponent::URCHealthComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    AbilitySystemComponent = nullptr;
    HealthSet = nullptr;
    DeathState = ERCDeathState::NotDead;
}

void URCHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URCHealthComponent, DeathState);
}

void URCHealthComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();
    Super::OnUnregister();
}

void URCHealthComponent::InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC)
{
    AActor* Owner = GetOwner();
    check(Owner);

    if (AbilitySystemComponent)
    {
        UE_LOG(LogRC, Error, TEXT("RCHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
        return;
    }

    AbilitySystemComponent = InASC;
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogRC, Error, TEXT("RCHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
        return;
    }

    HealthSet = AbilitySystemComponent->GetSet<URCHealthSet>();
    if (!HealthSet)
    {
        UE_LOG(LogRC, Error, TEXT("RCHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
        return;
    }

    // Register to listen for attribute changes.
    HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
    HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
    HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

    // TEMP: Reset attributes to default values.  Eventually this will be driven by a spread sheet.
    AbilitySystemComponent->SetNumericAttributeBase(URCHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());

    ClearGameplayTags();

    OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
    OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void URCHealthComponent::UninitializeFromAbilitySystem()
{
    ClearGameplayTags();

    if (HealthSet)
    {
        HealthSet->OnHealthChanged.RemoveAll(this);
        HealthSet->OnMaxHealthChanged.RemoveAll(this);
        HealthSet->OnOutOfHealth.RemoveAll(this);
    }

    HealthSet = nullptr;
    AbilitySystemComponent = nullptr;
}

void URCHealthComponent::ClearGameplayTags()
{
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(RCGameplayTags::Status_Death_Dying, 0);
        AbilitySystemComponent->SetLooseGameplayTagCount(RCGameplayTags::Status_Death_Dead, 0);
    }
}

float URCHealthComponent::GetHealth() const
{
    return HealthSet ? HealthSet->GetHealth() : 0.f;
}

float URCHealthComponent::GetMaxHealth() const
{
    return HealthSet ? HealthSet->GetMaxHealth() : 0.f;
}

float URCHealthComponent::GetHealthNormalized() const
{
    if (HealthSet)
    {
        const float Health = HealthSet->GetHealth();
        const float MaxHealth = HealthSet->GetMaxHealth();

        return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
    }

    return 0.0f;
}

void URCHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URCHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URCHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = RCGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// Send a standardized verb message that other systems can observe
		{
			FRCVerbMessage Message;
			Message.Verb = TAG_RedCell_Elimination_Message;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Message.Target = URCVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}

		//@TODO: assist messages (could compute from damage dealt elsewhere)?
	}

#endif // #if WITH_SERVER_CODE
}

void URCHealthComponent::OnRep_DeathState(ERCDeathState OldDeathState)
{
	const ERCDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back but we've already predicted past the server state.
		UE_LOG(LogRC, Warning, TEXT("RCHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == ERCDeathState::NotDead)
	{
		if (NewDeathState == ERCDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == ERCDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogRC, Error, TEXT("RCHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == ERCDeathState::DeathStarted)
	{
		if (NewDeathState == ERCDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogRC, Error, TEXT("RCHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("RCHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}

void URCHealthComponent::StartDeath()
{
	if (DeathState != ERCDeathState::NotDead)
	{
		return;
	}

	DeathState = ERCDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(RCGameplayTags::Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void URCHealthComponent::FinishDeath()
{
	if (DeathState != ERCDeathState::DeathStarted)
	{
		return;
	}

	DeathState = ERCDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(RCGameplayTags::Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void URCHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	if ((DeathState == ERCDeathState::NotDead) && AbilitySystemComponent)
	{
		const TSubclassOf<UGameplayEffect> DamageGE = URCAssetManager::GetSubclass(URCGameData::Get().DamageGameplayEffect_SetByCaller);
		if (!DamageGE)
		{
			UE_LOG(LogRC, Error, TEXT("RCHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to find gameplay effect [%s]."), *GetNameSafe(GetOwner()), *URCGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
			return;
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1.0f, AbilitySystemComponent->MakeEffectContext());
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (!Spec)
		{
			UE_LOG(LogRC, Error, TEXT("RCHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to make outgoing spec for [%s]."), *GetNameSafe(GetOwner()), *GetNameSafe(DamageGE));
			return;
		}

		Spec->AddDynamicAssetTag(TAG_Gameplay_DamageSelfDestruct);

		if (bFellOutOfWorld)
		{
			Spec->AddDynamicAssetTag(TAG_Gameplay_FellOutOfWorld);
		}

		const float DamageAmount = GetMaxHealth();

		Spec->SetSetByCallerMagnitude(RCGameplayTags::SetByCaller_Damage, DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

