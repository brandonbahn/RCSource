// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/RCTeamAgentInterface.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "RCPlayerState.generated.h"

struct FRCVerbMessage;

class AController;
class ARCPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class URCAbilitySystemComponent;
class URCExperienceDefinition;
class URCPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/** Defines the types of client connected */
UENUM()
enum class ERCPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};


/**
 * ARCPlayerState
 *
 *    Base player state class used by this project.
 */
UCLASS(Config = Game)
class REDCELL_API ARCPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public IRCTeamAgentInterface
{
  GENERATED_BODY()

public:
    ARCPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    ARCPlayerController* GetRCPlayerController() const;

    UFUNCTION(BlueprintCallable, Category = "RedCell|PlayerState")
    URCAbilitySystemComponent* GetRCAbilitySystemComponent() const { return AbilitySystemComponent; }
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    template <class T>
    const T* GetPawnData() const { return Cast<T>(PawnData); }
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetPawnData(const URCPawnData* InPawnData);

    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void PostInitializeComponents() override;
    //~End of AActor interface

    //~APlayerState interface
    virtual void Reset() override;
    virtual void ClientInitialize(AController* C) override;
    virtual void CopyProperties(APlayerState* PlayerState) override;
    virtual void OnDeactivated() override;
    virtual void OnReactivated() override;
    //~End of APlayerState interface

	//~IRCTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnRCTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IRCTeamAgentInterface interface
    
    static const FName NAME_RCAbilityReady;

	void SetPlayerConnectionType(ERCPlayerConnectionType NewType);
	ERCPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	/** Returns the Squad ID of the squad the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	/** Returns the Team ID of the team the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}

	void SetSquadID(int32 NewSquadID);

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "RedCell|PlayerState")
	void ClientBroadcastMessage(const FRCVerbMessage Message);

    // Default AbilitySet to grant on possession (server only)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilitySystem")
    URCAbilitySet* DefaultAbilitySet;

private:
	void OnExperienceLoaded(const URCExperienceDefinition* CurrentExperience);
	
    
protected:
    UFUNCTION()
    void OnRep_PawnData();

protected:

    UPROPERTY(ReplicatedUsing = OnRep_PawnData)
    TObjectPtr<const URCPawnData> PawnData;
    
private:

    // The ability system component sub-object used by player characters.
    UPROPERTY(VisibleAnywhere, Category = "RedCell|PlayerState")
    TObjectPtr<URCAbilitySystemComponent> AbilitySystemComponent;

    // Health attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<URCHealthSet> HealthSet;
    
    // Combat attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<const class URCCombatSet> CombatSet;
    
    // Core attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<URCCoreSet> CoreSet;

	UPROPERTY(Replicated)
	ERCPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY()
	FOnRCTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

private:
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	UFUNCTION()
	void OnRep_MySquadID();
};
