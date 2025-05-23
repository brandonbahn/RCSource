// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/RCGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "RCAbilitySet.h"
#include "NativeGameplayTags.h"
#include "RCAbilitySystemComponent.generated.h"

class AActor;
class UGameplayAbility;
class URCAbilityTagRelationshipMapping;
class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;
/**
 * Custom ASC for RedCell that can receive AbilitySets and log incoming events.
 */
UCLASS( ClassGroup=(Abilities)
       , Blueprintable
       , meta=(BlueprintSpawnableComponent) )
class REDCELL_API URCAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:

    URCAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UActorComponent interface
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of UActorComponent interface

    virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
    
    typedef TFunctionRef<bool(const URCGameplayAbility* RCAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
    void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

    void CancelInputActivatedAbilities(bool bReplicateCancelAbility);
    
    virtual int32 HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) override;
    
    void AbilityInputTagPressed(const FGameplayTag& InputTag);
    void AbilityInputTagReleased(const FGameplayTag& InputTag);

    void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
    void ClearAbilityInput();
    
    bool IsActivationGroupBlocked(ERCAbilityActivationGroup Group) const;
    void AddAbilityToActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility);
    void RemoveAbilityFromActivationGroup(ERCAbilityActivationGroup Group, URCGameplayAbility* RCAbility);
    void CancelActivationGroupAbilities(ERCAbilityActivationGroup Group, URCGameplayAbility* IgnoreRCAbility, bool bReplicateCancelAbility);

    /** Instantly override any attribute to a new value (e.g. refill Health to MaxHealth) */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    void OverrideAttribute(FGameplayAttribute Attribute, float NewValue)
    {
      ApplyModToAttribute(Attribute, EGameplayModOp::Override, NewValue);
    }

    // Uses a gameplay effect to add the specified dynamic granted tag.
    void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

    // Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
    void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

    /** Gets the ability target data associated with the given ability handle and activation info */
    void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);
    
    /** Sets the current tag relationship mapping, if null it will clear it out */
    void SetTagRelationshipMapping(URCAbilityTagRelationshipMapping* NewMapping);
    
    /** Looks at ability tags and gathers additional required and blocking tags */
    void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

    /** Returns true if any currently-active ability has the given tag */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsAbilityActiveByTag(FGameplayTag AbilityTag) const;
    
    /** Returns true if any currently-granted ability has the given tag */
    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsAbilityGrantedByTag(FGameplayTag AbilityTag) const;

protected:

    void TryActivateAbilitiesOnSpawn();

    virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
    virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

    virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
    virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
    virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
    virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
    virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

    /** Notify client that an ability failed to activate */
    UFUNCTION(Client, Unreliable)
    void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

    void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
    
protected:

    // If set, this table is used to look up tag relationships for activate and cancel
    UPROPERTY()
    TObjectPtr<URCAbilityTagRelationshipMapping> TagRelationshipMapping;
    
    // Handles to abilities that had their input pressed this frame.
    TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

    // Handles to abilities that had their input released this frame.
    TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

    // Handles to abilities that have their input held.
    TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

    // Number of abilities running in each activation group.
    int32 ActivationGroupCounts[(uint8)ERCAbilityActivationGroup::MAX];
};

