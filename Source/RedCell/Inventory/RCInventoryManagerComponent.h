// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "RCInventoryManagerComponent.generated.h"

class URCInventoryItemDefinition;
class URCInventoryItemInstance;
class URCInventoryManagerComponent;
class UObject;
struct FFrame;
struct FRCInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FRCInventoryChangeMessage
{
    GENERATED_BODY()

    //@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<UActorComponent> InventoryOwner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = Inventory)
    TObjectPtr<URCInventoryItemInstance> Instance = nullptr;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 NewCount = 0;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FRCInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FRCInventoryEntry()
    {}

    FString GetDebugString() const;

private:
    friend FRCInventoryList;
    friend URCInventoryManagerComponent;

    UPROPERTY()
    TObjectPtr<URCInventoryItemInstance> Instance = nullptr;

    UPROPERTY()
    int32 StackCount = 0;

    UPROPERTY(NotReplicated)
    int32 LastObservedCount = INDEX_NONE;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FRCInventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    FRCInventoryList()
        : OwnerComponent(nullptr)
    {
    }

    FRCInventoryList(UActorComponent* InOwnerComponent)
        : OwnerComponent(InOwnerComponent)
    {
    }

    TArray<URCInventoryItemInstance*> GetAllItems() const;

public:
    //~FFastArraySerializer contract
    void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FRCInventoryEntry, FRCInventoryList>(Entries, DeltaParms, *this);
    }

    URCInventoryItemInstance* AddEntry(TSubclassOf<URCInventoryItemDefinition> ItemClass, int32 StackCount);
    void AddEntry(URCInventoryItemInstance* Instance);

    void RemoveEntry(URCInventoryItemInstance* Instance);

private:
    void BroadcastChangeMessage(FRCInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
    friend URCInventoryManagerComponent;

private:
    // Replicated list of items
    UPROPERTY()
    TArray<FRCInventoryEntry> Entries;

    UPROPERTY(NotReplicated)
    TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FRCInventoryList> : public TStructOpsTypeTraitsBase2<FRCInventoryList>
{
    enum { WithNetDeltaSerializer = true };
};







/**
 * Manages an inventory
 */
UCLASS(BlueprintType)
class REDCELL_API URCInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    URCInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    bool CanAddItemDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 StackCount = 1);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    URCInventoryItemInstance* AddItemDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 StackCount = 1);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    void AddItemInstance(URCInventoryItemInstance* ItemInstance);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    void RemoveItemInstance(URCInventoryItemInstance* ItemInstance);

    UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
    TArray<URCInventoryItemInstance*> GetAllItems() const;

    UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
    URCInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef) const;

    int32 GetTotalItemCountByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef) const;
    bool ConsumeItemsByDefinition(TSubclassOf<URCInventoryItemDefinition> ItemDef, int32 NumToConsume);

    //~UObject interface
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    virtual void ReadyForReplication() override;
    //~End of UObject interface

private:
    UPROPERTY(Replicated)
    FRCInventoryList InventoryList;
};
