// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "RCPawnExtensionComponent.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UGameFrameworkComponentManager;
class URCAbilitySystemComponent;
class URCPawnExtensionComponent;
class URCPawnData;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;

/**
 * Component that adds functionality to all Pawn classes so it can be used for characters/vehicles/etc.
 * This coordinates the initialization of other components.
 */
UCLASS()
class REDCELL_API URCPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()
    
public:

    URCPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);
    
    /** The name of this overall feature, this one depends on the other named component features */
    static const FName NAME_ActorFeatureName;

    //~ Begin IGameFrameworkInitStateInterface interface
    virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
    virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
    virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
    virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
    virtual void CheckDefaultInitialization() override;
    //~ End IGameFrameworkInitStateInterface interface

    /** Returns the pawn extension component if one exists on the specified actor. */
    UFUNCTION(BlueprintPure, Category = "RedCell|Pawn")
    static URCPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<URCPawnExtensionComponent>() : nullptr); }

    /** Gets the pawn data, which is used to specify pawn properties in data */
    template <class T>
    const T* GetPawnData() const { return Cast<T>(PawnData); }

    /** Sets the current pawn data */
    void SetPawnData(const URCPawnData* InPawnData);

    /** Gets the current ability system component, which may be owned by a different actor */
    UFUNCTION(BlueprintPure, Category = "RedCell|Pawn")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }

    /** Should be called by the owning pawn to become the avatar of the ability system. */
    void InitializeAbilitySystem(URCAbilitySystemComponent* InASC, AActor* InOwnerActor);

    /** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
    void UninitializeAbilitySystem();

    /** Should be called by the owning pawn when the pawn's controller changes. */
    void HandleControllerChanged();

    /** Should be called by the owning pawn when the player state has been replicated. */
    void HandlePlayerStateReplicated();
    
    /** Should be called by the owning pawn when the input component is setup. */
    void SetupPlayerInputComponent();

    /** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
    void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

    /** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
    void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:

    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnRep_PawnData();

    /** Delegate fired when our pawn becomes the ability system's avatar actor */
    FSimpleMulticastDelegate OnAbilitySystemInitialized;

    /** Delegate fired when our pawn is removed as the ability system's avatar actor */
    FSimpleMulticastDelegate OnAbilitySystemUninitialized;

    /** Pawn data used to create the pawn. Specified from a spawn function or on a placed instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_PawnData, Category = "RedCell|Pawn")
    TObjectPtr<const URCPawnData> PawnData;

    /** Pointer to the ability system component that is cached for convenience. */
    UPROPERTY(Transient)
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;
};
