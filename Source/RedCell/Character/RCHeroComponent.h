// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Input/RCInputConfig.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "RCHeroComponent.generated.h"

namespace EEndPlayReason { enum Type : int; }
struct FLoadedMappableConfigPair;
struct FMappableConfigPair;

class UGameFrameworkComponentManager;
class UInputComponent;
class URCInputConfig;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;

/**
 * Component that sets up input and camera handling for player controlled pawns (or bots that simulate players).
 * This depends on a PawnExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class REDCELL_API URCHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()
    
public:
    
    URCHeroComponent(const FObjectInitializer& ObjectInitializer);
    
    /** Returns the hero component if one exists on the specified actor. */
    UFUNCTION(BlueprintPure, Category = "RedCell|Hero")
    static URCHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<URCHeroComponent>() : nullptr); }
    
    /** Adds mode-specific input config */
    void AddAdditionalInputConfig(const URCInputConfig* InputConfig);
    
    /** Removes a mode-specific input config if it has been added */
    void RemoveAdditionalInputConfig(const URCInputConfig* InputConfig);
    
    /** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
    bool IsReadyToBindInputs() const;
    
    /** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
    static const FName NAME_BindInputsNow;
    
    /** The name of this component-implemented feature */
    static const FName NAME_ActorFeatureName;
    
    //~ Begin IGameFrameworkInitStateInterface interface
    virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
    virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
    virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
    virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
    virtual void CheckDefaultInitialization() override;
    //~ End IGameFrameworkInitStateInterface interface
    
protected:
    
    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
    
    void Input_AbilityInputTagPressed(FGameplayTag InputTag);
    void Input_AbilityInputTagReleased(FGameplayTag InputTag);

protected:

    //Hero component mapping context selection - will come back later
    //UPROPERTY(EditAnywhere)
    //TArray<FInputMappingContextAndPriority> DefaultInputMappings;

    /** True when player input bindings have been applied, will never be true for non - players */
    bool bReadyToBindInputs;
    
    /** The DataAsset that drives ability‑input tag mappings */
    UPROPERTY(EditDefaultsOnly, Category = "Abilities|Input")
    URCInputConfig* RCInputConfig;

    /** The pawn's ability system component, once initialized */
    UPROPERTY()
    URCAbilitySystemComponent* AbilitySystemComponent;

};
