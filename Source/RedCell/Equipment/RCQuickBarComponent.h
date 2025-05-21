// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Inventory/RCInventoryItemInstance.h"

#include "RCQuickBarComponent.generated.h"

class AActor;
class ULyraEquipmentInstance;
class ULyraEquipmentManagerComponent;
class UObject;
struct FFrame;

/**
 * Component used for QuickBarSlots UI
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class URCQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()
	
public:
    URCQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category="RedCell")
    void CycleActiveSlotForward();

    UFUNCTION(BlueprintCallable, Category="RedCell")
    void CycleActiveSlotBackward();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category="RedCell")
    void SetActiveSlotIndex(int32 NewIndex);

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    TArray<URCInventoryItemInstance*> GetSlots() const
    {
        return Slots;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

    UFUNCTION(BlueprintCallable, BlueprintPure = false)
    URCInventoryItemInstance* GetActiveSlotItem() const;

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    int32 GetNextFreeItemSlot() const;

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void AddItemToSlot(int32 SlotIndex, URCInventoryItemInstance* Item);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    URCInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

    virtual void BeginPlay() override;

private:
    void UnequipItemInSlot();
    void EquipItemInSlot();

    URCEquipmentManagerComponent* FindEquipmentManager() const;

protected:
    UPROPERTY()
    int32 NumSlots = 3;

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

private:
    UPROPERTY(ReplicatedUsing=OnRep_Slots)
    TArray<TObjectPtr<URCInventoryItemInstance>> Slots;

    UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
    int32 ActiveSlotIndex = -1;

    UPROPERTY()
    TObjectPtr<URCEquipmentInstance> EquippedItem;
};


USTRUCT(BlueprintType)
struct FRCQuickBarSlotsChangedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = Inventory)
    TArray<TObjectPtr<URCInventoryItemInstance>> Slots;
};


USTRUCT(BlueprintType)
struct FRCQuickBarActiveIndexChangedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 ActiveIndex = 0;
};

