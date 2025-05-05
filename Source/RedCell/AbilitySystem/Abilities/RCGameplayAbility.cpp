// Fill out your copyright notice in the Description page of Project Settings.


#include "RCGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Player/RCPlayerController.h"
#include "Character/RCCharacter.h"
#include "RCGameplayTags.h"
#include "Character/RCHeroComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/RCAbilitySourceInterface.h"
#include "AbilitySystem/RCGameplayEffectContext.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameplayAbility)

URCGameplayAbility::URCGameplayAbility(const FObjectInitializer& ObjInit)
: Super(ObjInit)
{
    ReplicationPolicy  = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    NetSecurityPolicy  = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

    ActivationPolicy   = ERCAbilityActivationPolicy::OnInputTriggered;
    ActivationGroup    = ERCAbilityActivationGroup::Independent;
}

URCAbilitySystemComponent* URCGameplayAbility::GetRCAbilitySystemComponentFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<URCAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

ARCPlayerController* URCGameplayAbility::GetRCPlayerControllerFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<ARCPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* URCGameplayAbility::GetControllerFromActorInfo() const
{
    if (CurrentActorInfo)
    {
        if (AController* PC = CurrentActorInfo->PlayerController.Get())
        {
            return PC;
        }

        // Look for a player controller or pawn in the owner chain.
        AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
        while (TestActor)
        {
            if (AController* C = Cast<AController>(TestActor))
            {
                return C;
            }

            if (APawn* Pawn = Cast<APawn>(TestActor))
            {
                return Pawn->GetController();
            }

            TestActor = TestActor->GetOwner();
        }
    }

    return nullptr;
}

ARCCharacter* URCGameplayAbility::GetRCCharacterFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<ARCCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

URCHeroComponent* URCGameplayAbility::GetHeroComponentFromActorInfo() const
{
    return (CurrentActorInfo ? URCHeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

bool URCGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return false;
    }

    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    
    // 2) Grab the ASC’s current tags:
    const FGameplayTagContainer& OwnedTags = ActorInfo->AbilitySystemComponent->GetOwnedGameplayTags();

    // 3) Required-tag query must pass:
    if (!ActivationRequiredTagQuery.IsEmpty() &&
        !ActivationRequiredTagQuery.Matches(OwnedTags))
    {
        // Optionally append to OptionalRelevantTags here for debugging
        return false;
    }

    // 4) Block-tag query must *not* match:
    if (!ActivationBlockedTagQuery.IsEmpty() &&
        ActivationBlockedTagQuery.Matches(OwnedTags))
    {
        return false;
    }

    //@TODO Possibly remove after setting up tag relationships
    URCAbilitySystemComponent* RCASC = CastChecked<URCAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
    if (RCASC->IsActivationGroupBlocked(ActivationGroup))
    {
        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(RCGameplayTags::Ability_ActivateFail_ActivationGroup);
        }
        return false;
    }

    return true;
}

void URCGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
    // The ability can not block canceling if it's replaceable.
    if (!bCanBeCanceled && (ActivationGroup == ERCAbilityActivationGroup::Exclusive_Replaceable))
    {
        return;
    }

    Super::SetCanBeCanceled(bCanBeCanceled);
}

void URCGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);

    K2_OnAbilityAdded();

    TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void URCGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    K2_OnAbilityRemoved();

    Super::OnRemoveAbility(ActorInfo, Spec);
}

void URCGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URCGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//* Ability Cost would be applied here!!!! *//


FGameplayEffectContextHandle URCGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
    FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

    FRCGameplayEffectContext* EffectContext = FRCGameplayEffectContext::ExtractEffectContext(ContextHandle);
    check(EffectContext);

    check(ActorInfo);

    AActor* EffectCauser = nullptr;
    const IRCAbilitySourceInterface* AbilitySource = nullptr;
    float SourceLevel = 0.0f;
    GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

    UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

    AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

    EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
    EffectContext->AddInstigator(Instigator, EffectCauser);
    EffectContext->AddSourceObject(SourceObject);

    return ContextHandle;
}


#if 0 //
void URCGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
    Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

    if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
    {
        if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
        {
            Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
        }
    }
}
#endif //


bool URCGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    // Specialized version to handle death exclusion and AbilityTags expansion via ASC

    bool bBlocked = false;
    bool bMissing = false;

    UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
    const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
    const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

    // Check if any of this ability's tags are currently blocked
    if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
    {
        bBlocked = true;
    }

    const URCAbilitySystemComponent* RCASC = Cast<URCAbilitySystemComponent>(&AbilitySystemComponent);
    static FGameplayTagContainer AllRequiredTags;
    static FGameplayTagContainer AllBlockedTags;

    AllRequiredTags = ActivationRequiredTags;
    AllBlockedTags = ActivationBlockedTags;

    // Expand our ability tags to add additional required/blocked tags
    if (RCASC)
    {
        RCASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
    }

    // Check to see the required/blocked tags for this ability
    if (AllBlockedTags.Num() || AllRequiredTags.Num())
    {
        static FGameplayTagContainer AbilitySystemComponentTags;
        
        AbilitySystemComponentTags.Reset();
        AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

        if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
        {
            if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(RCGameplayTags::Status_Death))
            {
                // If player is dead and was rejected due to blocking tags, give that feedback
                OptionalRelevantTags->AddTag(RCGameplayTags::Ability_ActivateFail_IsDead);
            }

            bBlocked = true;
        }

        if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
        {
            bMissing = true;
        }
    }

    if (SourceTags != nullptr)
    {
        if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
        {
            if (SourceTags->HasAny(SourceBlockedTags))
            {
                bBlocked = true;
            }

            if (!SourceTags->HasAll(SourceRequiredTags))
            {
                bMissing = true;
            }
        }
    }

    if (TargetTags != nullptr)
    {
        if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
        {
            if (TargetTags->HasAny(TargetBlockedTags))
            {
                bBlocked = true;
            }

            if (!TargetTags->HasAll(TargetRequiredTags))
            {
                bMissing = true;
            }
        }
    }

    if (bBlocked)
    {
        if (OptionalRelevantTags && BlockedTag.IsValid())
        {
            OptionalRelevantTags->AddTag(BlockedTag);
        }
        return false;
    }
    if (bMissing)
    {
        if (OptionalRelevantTags && MissingTag.IsValid())
        {
            OptionalRelevantTags->AddTag(MissingTag);
        }
        return false;
    }

    return true;
}

void URCGameplayAbility::OnPawnAvatarSet()
{
    K2_OnPawnAvatarSet();
}

void URCGameplayAbility::GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IRCAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const
{
    OutSourceLevel = 0.0f;
    OutAbilitySource = nullptr;
    OutEffectCauser = nullptr;

    OutEffectCauser = ActorInfo->AvatarActor.Get();

    // If we were added by something that's an ability info source, use it
    UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

    OutAbilitySource = Cast<IRCAbilitySourceInterface>(SourceObject);
}

void URCGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
    // Try to activate if activation policy is on spawn.
    if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == ERCAbilityActivationPolicy::OnSpawn))
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate until we get the new one.
        if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
        {
            const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
            const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

            const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
            const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

            if (bClientShouldActivate || bServerShouldActivate)
            {
                ASC->TryActivateAbility(Spec.Handle);
            }
        }
    }
}

bool URCGameplayAbility::CanChangeActivationGroup(ERCAbilityActivationGroup NewGroup) const
{
    if (!IsInstantiated() || !IsActive())
    {
        return false;
    }

    if (ActivationGroup == NewGroup)
    {
        return true;
    }

    URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponentFromActorInfo();
    check(RCASC);

    if ((ActivationGroup != ERCAbilityActivationGroup::Exclusive_Blocking) && RCASC->IsActivationGroupBlocked(NewGroup))
    {
        // This ability can't change groups if it's blocked (unless it is the one doing the blocking).
        return false;
    }

    if ((NewGroup == ERCAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
    {
        // This ability can't become replaceable if it can't be canceled.
        return false;
    }

    return true;
}

bool URCGameplayAbility::ChangeActivationGroup(ERCAbilityActivationGroup NewGroup)
{
    // Lyra’s “ensure instantiated” guard inlined:
    if (!IsInstantiated())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ChangeActivationGroup called on uninstantiated ability %s"), *GetName());
        return false;
    }

    if (!CanChangeActivationGroup(NewGroup))
    {
        return false;
    }

    if (ActivationGroup != NewGroup)
    {
        URCAbilitySystemComponent* RCASC = GetRCAbilitySystemComponentFromActorInfo();
        check(RCASC);

        RCASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
        RCASC->AddAbilityToActivationGroup(NewGroup, this);

        ActivationGroup = NewGroup;
    }

    return true;
}

