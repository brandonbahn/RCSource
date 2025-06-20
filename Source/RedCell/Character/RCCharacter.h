// Fill out your copyright notice in the Description page of Project Settings.
// RCCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "Player/RCPlayerState.h"
#include "Character/RCHealthComponent.h"
#include "Character/RCCoreComponent.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCHeroComponent.h"
#include "Character/RCMovementModes.h"
#include "RCPreMovementTickComponent.h"
#include "Camera/RCGameplayCameraInterface.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "Player/RCPlayerController.h"
#include "Traversal/TraversableObstacleComponent.h"
#include "Traversal/RCTraversalComponent.h"
#include "RCCharacter.generated.h"

class AActor;
class AController;
class ARCPlayerController;
class ARCPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class URCAbilitySystemComponent;
class UGameplayCameraComponent;
class URCHealthComponent;
class URCCoreComponent;
class URCPawnExtensionComponent;
class URCPreMovementTickComponent;
class URCTraversalComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

USTRUCT(BlueprintType)
struct FCharacterInputState
{
    GENERATED_BODY()

    FCharacterInputState()
    {
        bWantsToSprint = false;
        bWantsToWalk = false;
        bWantsToStrafe = false;
        bWantsToAim = false;
    }

    UPROPERTY(BlueprintReadWrite)
    uint8 bWantsToSprint : 1;
    UPROPERTY(BlueprintReadWrite)
    uint8 bWantsToWalk : 1;
    UPROPERTY(BlueprintReadWrite)
    uint8 bWantsToStrafe : 1;
    UPROPERTY(BlueprintReadWrite)
    uint8 bWantsToAim : 1;
    
    // Comparison operator for if state changed
    bool operator==(const FCharacterInputState& Other) const
    {
        return bWantsToSprint == Other.bWantsToSprint &&
               bWantsToWalk == Other.bWantsToWalk &&
               bWantsToStrafe == Other.bWantsToStrafe &&
               bWantsToAim == Other.bWantsToAim;
    }
    
    bool operator!=(const FCharacterInputState& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * ARCCharacter
 *
 *    The base character pawn class used by this project.
 *    Responsible for sending events to pawn components.
 *    New behavior should be added via pawn components when possible.
 */

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class REDCELL_API ARCCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public IRCGameplayCameraInterface
{
    GENERATED_BODY()
    
    
public:
    ARCCharacter(const FObjectInitializer& ObjInit = FObjectInitializer::Get());
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
    ARCPlayerController* GetRCPlayerController() const;
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
    ARCPlayerState* GetRCPlayerState() const;
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Character")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
    virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
    virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

    void ToggleCrouch();

    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Reset() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
    //~End of AActor interface

    //~GameplayCamera interface
    virtual FCharacterPropertiesForCamera GetCharacterPropertiesForCamera_Implementation() const override;
    //~End of GameplayCameraInterface

    // Sets the movement state tags for the character
    void SetMovementStateTags(
        E_MovementSituation NewSituation,
        E_Gait              NewGait,
        E_MovementState     NewState,
        E_RotationMode      NewRotation,
        E_Stance            NewStance,
        E_MovementDirection NewDirection);

    // Execute certain functions before the CMC, using the tick pre-requisite system
    UFUNCTION(BlueprintCallable, Category = "PreCMC")
    void PreCMCTick();
    
    /** Blueprint‐callable accessor for GetHealthComponent */
    UFUNCTION(BlueprintCallable, Category="RedCell|Health")
    URCHealthComponent* GetHealthComponent() const { return HealthComponent; }
    
    /** Blueprint‐callable accessor for GetCoreComponent */
    UFUNCTION(BlueprintCallable, Category="RedCell|Core")
    URCCoreComponent* GetCoreComponent() const { return CoreComponent; }

    /** Blueprint‐callable accessor for GetTraversalComponent */
    UFUNCTION(BlueprintCallable, Category="RedCell|Traversal")
    URCTraversalComponent* GetTraversalComponent() const { return TraversalComponent; }

    //UFUNCTION(BlueprintCallable, Category="Traversal")
    //virtual void UpdateWarpTargets() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    bool bJustLanded = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    FVector LandVelocity = FVector::ZeroVector;

    // Getter for RCCharacterMovementComponent
    const FCharacterInputState& GetCharacterInputState() const { return CharacterInputState; }
    
    FORCEINLINE URCCharacterMovementComponent* GetRCCharacterMovement() const 
    { 
        return CastChecked<URCCharacterMovementComponent>(GetCharacterMovement()); 
    }
    
protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedCell|Components", meta = (DisplayName = "PreTickComponent"))
    URCPreMovementTickComponent* PreMovementTickComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedCell|Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<URCTraversalComponent> TraversalComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
    TObjectPtr<class UMotionWarpingComponent> MotionWarping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
    TObjectPtr<class USpringArmComponent> SpringArm;
    
    virtual void OnAbilitySystemInitialized();
    virtual void OnAbilitySystemUninitialized();
    
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
    
    virtual void OnRep_Controller() override;
    virtual void OnRep_PlayerState() override;
    
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    void InitializeGameplayTags();
    
    virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

    // PreCMC functions (simplified from GASP)
    UFUNCTION(BlueprintCallable, Category = "Movement")
    virtual void UpdateRotation_PreCMC();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    virtual void UpdateMovement_PreCMC();

    /** RCCharacter Current Gait */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    E_Gait EGait = E_Gait::Walk;
    
    // Begins the death sequence for the character (disables collision, disables movement, etc...)
    UFUNCTION()
    virtual void OnDeathStarted(AActor* OwningActor);
    
    // Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
    UFUNCTION()
    virtual void OnDeathFinished(AActor* OwningActor);
    
    void DisableMovementAndCollision();
    void DestroyDueToDeath();
    void UninitAndDestroy();
    
    /** Simple default team ID (everyone is on team 0) */
    UPROPERTY()
    uint8 MyTeamID = 0;
    
    // Called when the death sequence for the character has completed
    UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
    void K2_OnDeathFinished();

    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
    void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    virtual bool CanJumpInternal_Implementation() const;

    // The authoritative input state (set on server from gameplay tags)
    UPROPERTY(ReplicatedUsing=OnRep_CharacterInputState, BlueprintReadOnly)
    FCharacterInputState CharacterInputState;
    
    // Previous state for change detection
    FCharacterInputState PreviousCharacterInputState;
    
    UFUNCTION()
    void OnRep_CharacterInputState();
    
    // Update input state from gameplay tags (server only)
    void UpdateCharacterInputState();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedCell|Pawn", meta=(AllowPrivateAccess="true"))
    TObjectPtr<URCPawnExtensionComponent> PawnExtensionComponent;
    
    /** The pure C++ health component that binds to the ASC */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RedCell|Health", meta=(DisplayName="Health Component", AllowPrivateAccess="true"))
    TObjectPtr<URCHealthComponent> HealthComponent;
    
    /** The pure C++ health component that binds to the ASC */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RedCell|Core", meta=(DisplayName="Core Component", AllowPrivateAccess="true"))
    TObjectPtr<URCCoreComponent> CoreComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;

public:
    // Movement input setters (called by HeroComponent)
    void SetMovementInputMagnitude(float Magnitude) { CurrentMovementInputMagnitude = Magnitude; }
    void SetMovementInputVector(const FVector& InputVector) { CurrentMovementInputVector = InputVector; }

    // Movement input getters
    UFUNCTION(BlueprintCallable, Category = "RedCell|Input")
    float GetMovementInputMagnitude() const { return CurrentMovementInputMagnitude; }
    
    UFUNCTION(BlueprintCallable, Category = "RedCell|Input")
    FVector GetMovementInputVector() const { return CurrentMovementInputVector; }

    // Gets how fast the character is moving in its forward direction and uses the value to scale the distance of the forward trace.
    UFUNCTION(BlueprintPure, Category = "Traversal")
    FTraversalCheckInputs GetTraversalCheckInputs() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Traversal")
    FTraversalCheckResult TraversalResult = {};

protected:
    // Movement input state (replicated for networking)
    UPROPERTY(Replicated)
        float CurrentMovementInputMagnitude = 0.0f;
    UPROPERTY(Replicated)
        FVector CurrentMovementInputVector = FVector::ZeroVector;

    // Override base movement events
    virtual void OnJumped_Implementation() override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

    // Movement event handling for simulated proxies
    UFUNCTION()
    void OnCharacterMovementUpdated_Event(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
    
    void UpdatedMovementSimulated(FVector OldVelocity);

    // Blueprint events for animation system
    UFUNCTION(BlueprintImplementableEvent, Category = "Movement", meta = (DisplayName = "On Jumped"))
    void K2_OnJumpedEvent(float GroundSpeedBeforeJump);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Movement", meta = (DisplayName = "On Landed"))
    void K2_OnLandedEvent(const FVector& InLandVelocity);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Simulated")
        bool bWasMovingOnGroundLastFrameSimulated = true;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Audio")
    void PlayAudioEvent(const FGameplayTag& Value, float VolumeMultiplier, float PitchMultiplier);
    
};
