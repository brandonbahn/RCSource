// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "RCEquipmentManagerComponent.generated.h"

class UActorComponent;
class URCAbilitySystemComponent;
class URCEquipmentDefinition;
class URCEquipmentInstance;
class URCEquipmentManagerComponent;
class UObject;
struct FFrame;
struct FRCEquipmentList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FRCAppliedEquipmentEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FRCAppliedEquipmentEntry()
    {}

    FString GetDebugString() const;

private:
    friend FRCEquipmentList;
    friend URCEquipmentManagerComponent;

    // The equipment class that got equipped
    UPROPERTY()
    TSubclassOf<URCEquipmentDefinition> EquipmentDefinition;

    UPROPERTY()
    TObjectPtr<URCEquipmentInstance> Instance = nullptr;

    // Authority-only list of granted handles
    UPROPERTY(NotReplicated)
    FRCAbilitySet_GrantedHandles GrantedHandles;
};

/** List of applied equipment */
USTRUCT(BlueprintType)
struct FRCEquipmentList : public FFastArraySerializer
{
    GENERATED_BODY()

    FRCEquipmentList()
        : OwnerComponent(nullptr)
    {
    }

    FRCEquipmentList(UActorComponent* InOwnerComponent)
        : OwnerComponent(InOwnerComponent)
    {
    }

public:
    //~FFastArraySerializer contract
    void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FRCAppliedEquipmentEntry, FRCEquipmentList>(Entries, DeltaParms, *this);
    }

    URCEquipmentInstance* AddEntry(TSubclassOf<URCEquipmentDefinition> EquipmentDefinition);
    void RemoveEntry(URCEquipmentInstance* Instance);

private:
    URCAbilitySystemComponent* GetAbilitySystemComponent() const;

    friend URCEquipmentManagerComponent;

private:
    // Replicated list of equipment entries
    UPROPERTY()
    TArray<FRCAppliedEquipmentEntry> Entries;

    UPROPERTY(NotReplicated)
    TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FRCEquipmentList> : public TStructOpsTypeTraitsBase2<FRCEquipmentList>
{
    enum { WithNetDeltaSerializer = true };
};





/**
 * Manages equipment applied to pawn
 */
UCLASS(BlueprintType, Const)
class REDCELL_API URCEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
    URCEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    URCEquipmentInstance* EquipItem(TSubclassOf<URCEquipmentDefinition> EquipmentDefinition);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void UnequipItem(URCEquipmentInstance* ItemInstance);

    //~UObject interface
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    //~End of UObject interface

    //~UActorComponent interface
    //virtual void EndPlay() override;
    virtual void InitializeComponent() override;
    virtual void UninitializeComponent() override;
    virtual void ReadyForReplication() override;
    //~End of UActorComponent interface

    /** Returns the first equipped instance of a given type, or nullptr if none are found */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    URCEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<URCEquipmentInstance> InstanceType);

     /** Returns all equipped instances of a given type, or an empty array if none are found */
     UFUNCTION(BlueprintCallable, BlueprintPure)
    TArray<URCEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<URCEquipmentInstance> InstanceType) const;

    template <typename T>
    T* GetFirstInstanceOfType()
    {
        return (T*)GetFirstInstanceOfType(T::StaticClass());
    }

private:
    UPROPERTY(Replicated)
    FRCEquipmentList EquipmentList;
};

