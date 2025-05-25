// Fill out your copyright notice in the Description page of Project Settings.
// RCHealthComponent.h
#pragma once

#include "Components/GameFrameworkComponent.h"
#include "GameplayEffectExtension.h"    // for FOnAttributeChangeData
#include "RCHealthComponent.generated.h"

class URCHealthComponent;

class URCAbilitySystemComponent;
class URCHealthSet;
class UObject;
struct FOnAttributeChangeData;

/** Delegate fired when death starts or finishes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRCHealth_DeathEvent, AActor*, OwningActor);

/** Delegate fired when an attribute changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FRCHealth_AttributeChanged,
    URCHealthComponent*, HealthComponent,
    float, OldValue,
    float, NewValue,
    AActor*, Instigator
);

/** Death state for the component */
UENUM(BlueprintType)
enum class ERCDeathState : uint8
{
    NotDead       UMETA(DisplayName="Not Dead"),
    DeathStarted  UMETA(DisplayName="Death Started"),
    DeathFinished UMETA(DisplayName="Death Finished")
};

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class REDCELL_API URCHealthComponent : public UGameFrameworkComponent
{
    GENERATED_BODY()

public:
    URCHealthComponent(const FObjectInitializer& ObjInit);

    /** Find a health component on an actor */
    UFUNCTION(BlueprintPure, Category="RedCell|Health")
    static URCHealthComponent* FindHealthComponent(const AActor* Actor)
    {
        return Actor
            ? Actor->FindComponentByClass<URCHealthComponent>()
            : nullptr;
    }

    /** Hook up to an ASC (usually from PlayerState) */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    void InitializeWithAbilitySystem(URCAbilitySystemComponent* InASC);

    /** Unhook delegates and clear references */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    void UninitializeFromAbilitySystem();

    /** Current health */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    float GetHealth() const;

    /** Maximum health */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    float GetMaxHealth() const;

    /** Normalized [0–1] health */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    float GetHealthNormalized() const;

    /** Death state */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    ERCDeathState GetDeathState() const { return DeathState; }

    /** True if dead or dying */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    bool IsDeadOrDying() const { return DeathState != ERCDeathState::NotDead; }

    /** Begin death sequence */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    virtual void StartDeath();

    /** Finish death sequence */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    virtual void FinishDeath();

    /** Instantly kill */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

    /** Fired when health changes */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Health")
    FRCHealth_AttributeChanged OnHealthChanged;

    /** Fired when max health changes */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Health")
    FRCHealth_AttributeChanged OnMaxHealthChanged;

    /** Fired when death starts */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Health")
    FRCHealth_DeathEvent OnDeathStarted;

    /** Fired when death finishes */
    UPROPERTY(BlueprintAssignable, Category="RedCell|Health")
    FRCHealth_DeathEvent OnDeathFinished;

protected:
    virtual void OnUnregister() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void ClearGameplayTags();

    // **Plain C++** handlers, *not* UFUNCTION()
    void HandleHealthChanged(const FOnAttributeChangeData& Data);
    void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
    void HandleOutOfHealth(const FOnAttributeChangeData& Data);

    UFUNCTION()
    void OnRep_DeathState(ERCDeathState OldState);

private:
    UPROPERTY()
    URCAbilitySystemComponent* AbilitySystemComponent;

    // Health set used by this component.
    UPROPERTY()
    TObjectPtr<const URCHealthSet> HealthSet;

    UPROPERTY(ReplicatedUsing=OnRep_DeathState)
    ERCDeathState DeathState;
};
