// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "RCGameplayAbility.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

class AActor;
class AController;
class ARCCharacter;
class ARCPlayerController;
class APlayerController;
class FText;
class IRCAbilitySourceInterface;
class URCAbilitySystemComponent;
class URCHeroComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEffectSpec;
struct FGameplayEventData;

/**
 * ERCAbilityActivationPolicy
 *
 *    Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class ERCAbilityActivationPolicy : uint8
{
    // Try to activate the ability when the input is triggered.
    OnInputTriggered,

    // Continually try to activate the ability while the input is active.
    WhileInputActive,

    // Try to activate the ability when an avatar is assigned.
    OnSpawn
};


/**
 * ERCAbilityActivationGroup
 *
 *    Defines how an ability activates in relation to other abilities.
 */
UENUM(BlueprintType)
enum class ERCAbilityActivationGroup : uint8
{
    // Ability runs independently of all other abilities.
    Independent,

    // Ability is canceled and replaced by other exclusive abilities.
    Exclusive_Replaceable,

    // Ability blocks all other exclusive abilities from activating.
    Exclusive_Blocking,

    MAX    UMETA(Hidden)
};

/**
 * URCGameplayAbility
 *
 *    The base gameplay ability class used by this project.
 */
UCLASS()
class REDCELL_API URCGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    URCGameplayAbility(const FObjectInitializer& ObjInit = FObjectInitializer::Get());
    
    /** Rich tag‐query in place of simple Required/Blocked lists */
    UPROPERTY(EditDefaultsOnly, Category="Tags")
    FGameplayTagQuery ActivationRequiredTagQuery;

    UPROPERTY(EditDefaultsOnly, Category="Tags")
    FGameplayTagQuery ActivationBlockedTagQuery;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    URCAbilitySystemComponent* GetRCAbilitySystemComponentFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    ARCPlayerController* GetRCPlayerControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    AController* GetControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    ARCCharacter* GetRCCharacterFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|Ability")
    URCHeroComponent* GetHeroComponentFromActorInfo() const;

    ERCAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
    ERCAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

    void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

    // Returns true if the requested activation group is a valid transition.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "RedCell|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool CanChangeActivationGroup(ERCAbilityActivationGroup NewGroup) const;

    // Tries to change the activation group.  Returns true if it successfully changed.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "RedCell|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool ChangeActivationGroup(ERCAbilityActivationGroup NewGroup);
    
#if 0   // Disable failure callback until we wire up messaging
    /** Called when an ability fails to activate. */
    void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
    {
        NativeOnAbilityFailedToActivate(FailedReason);
        ScriptOnAbilityFailedToActivate(FailedReason);
    }
#endif

protected:

#if 0   //
    // Called when the ability fails to activate
    virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

    // Called when the ability fails to activate
    UFUNCTION(BlueprintImplementableEvent)
    void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;
#endif
    
    //~UGameplayAbility interface
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
    virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
    //virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
    virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    //~End of UGameplayAbility interface

    virtual void OnPawnAvatarSet();

    virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IRCAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

    /** Called when this ability is granted to the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
    void K2_OnAbilityAdded();

    /** Called when this ability is removed from the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
    void K2_OnAbilityRemoved();

    /** Called when the ability system is initialized with a pawn avatar. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
    void K2_OnPawnAvatarSet();

protected:
    
    // Defines how this ability is meant to activate.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Ability Activation")
    ERCAbilityActivationPolicy ActivationPolicy;

    // Defines the relationship between this ability activating and other abilities activating.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedCell|Ability Activation")
    ERCAbilityActivationGroup ActivationGroup;
    
    // Map of failure tags to simple error messages
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;
};
