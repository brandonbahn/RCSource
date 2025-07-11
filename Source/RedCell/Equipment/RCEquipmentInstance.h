// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"

#include "RCEquipmentInstance.generated.h"

class AActor;
class APawn;
struct FFrame;
struct FRCEquipmentActorToSpawn;


/**
 * A piece of equipment spawned and applied to a pawn
 */
UCLASS(BlueprintType, Blueprintable)
class URCEquipmentInstance : public UObject
{
	GENERATED_BODY()
	
public:
    URCEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UObject interface
    virtual bool IsSupportedForNetworking() const override { return true; }
    virtual UWorld* GetWorld() const override final;
    //~End of UObject interface

    UFUNCTION(BlueprintPure, Category=Equipment)
    UObject* GetInstigator() const { return Instigator; }

    void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

    UFUNCTION(BlueprintPure, Category=Equipment)
    APawn* GetPawn() const;

    UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
    APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

    UFUNCTION(BlueprintPure, Category=Equipment)
    TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

    virtual void SpawnEquipmentActors(const TArray<FRCEquipmentActorToSpawn>& ActorsToSpawn);
    virtual void DestroyEquipmentActors();

    virtual void OnEquipped();
    virtual void OnUnequipped();

protected:
#if UE_WITH_IRIS
    /** Register all replication fragments */
    virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

    UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnEquipped"))
    void K2_OnEquipped();

    UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnUnequipped"))
    void K2_OnUnequipped();

private:
    UFUNCTION()
    void OnRep_Instigator();

private:
    UPROPERTY(ReplicatedUsing=OnRep_Instigator)
    TObjectPtr<UObject> Instigator;

    UPROPERTY(Replicated)
    TArray<TObjectPtr<AActor>> SpawnedActors;
};

