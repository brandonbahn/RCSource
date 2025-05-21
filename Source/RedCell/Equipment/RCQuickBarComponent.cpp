// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/RCQuickBarComponent.h"

#include "Equipment/RCEquipmentDefinition.h"
#include "Equipment/RCEquipmentInstance.h"
#include "Equipment/RCEquipmentManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCQuickBarComponent)

class FLifetimeProperty;
class URCEquipmentDefinition;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RedCell_QuickBar_Message_SlotsChanged, "RedCell.QuickBar.Message.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RedCell_QuickBar_Message_ActiveIndexChanged, "RedCell.QuickBar.Message.ActiveIndexChanged");

URCQuickBarComponent::URCQuickBarComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);
}

void URCQuickBarComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, Slots);
    DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void URCQuickBarComponent::BeginPlay()
{
    if (Slots.Num() < NumSlots)
    {
        Slots.AddDefaulted(NumSlots - Slots.Num());
    }

    Super::BeginPlay();
}

void URCQuickBarComponent::CycleActiveSlotForward()
{
    if (Slots.Num() < 2)
    {
        return;
    }

    const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
    int32 NewIndex = ActiveSlotIndex;
    do
    {
        NewIndex = (NewIndex + 1) % Slots.Num();
        if (Slots[NewIndex] != nullptr)
        {
            SetActiveSlotIndex(NewIndex);
            return;
        }
    } while (NewIndex != OldIndex);
}

void URCQuickBarComponent::CycleActiveSlotBackward()
{
    if (Slots.Num() < 2)
    {
        return;
    }

    const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
    int32 NewIndex = ActiveSlotIndex;
    do
    {
        NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
        if (Slots[NewIndex] != nullptr)
        {
            SetActiveSlotIndex(NewIndex);
            return;
        }
    } while (NewIndex != OldIndex);
}

void URCQuickBarComponent::EquipItemInSlot()
{
    check(Slots.IsValidIndex(ActiveSlotIndex));
    check(EquippedItem == nullptr);

    if (URCInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
    {
        if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
        {
            TSubclassOf<URCEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
            if (EquipDef != nullptr)
            {
                if (URCEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
                {
                    EquippedItem = EquipmentManager->EquipItem(EquipDef);
                    if (EquippedItem != nullptr)
                    {
                        EquippedItem->SetInstigator(SlotItem);
                    }
                }
            }
        }
    }
}

void URCQuickBarComponent::UnequipItemInSlot()
{
    if (URCEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
    {
        if (EquippedItem != nullptr)
        {
            EquipmentManager->UnequipItem(EquippedItem);
            EquippedItem = nullptr;
        }
    }
}

URCEquipmentManagerComponent* URCQuickBarComponent::FindEquipmentManager() const
{
    if (AController* OwnerController = Cast<AController>(GetOwner()))
    {
        if (APawn* Pawn = OwnerController->GetPawn())
        {
            return Pawn->FindComponentByClass<URCEquipmentManagerComponent>();
        }
    }
    return nullptr;
}

void URCQuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
    if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
    {
        UnequipItemInSlot();

        ActiveSlotIndex = NewIndex;

        EquipItemInSlot();

        OnRep_ActiveSlotIndex();
    }
}

URCInventoryItemInstance* URCQuickBarComponent::GetActiveSlotItem() const
{
    return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 URCQuickBarComponent::GetNextFreeItemSlot() const
{
    int32 SlotIndex = 0;
    for (const TObjectPtr<URCInventoryItemInstance>& ItemPtr : Slots)
    {
        if (ItemPtr == nullptr)
        {
            return SlotIndex;
        }
        ++SlotIndex;
    }

    return INDEX_NONE;
}

void URCQuickBarComponent::AddItemToSlot(int32 SlotIndex, URCInventoryItemInstance* Item)
{
    if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
    {
        if (Slots[SlotIndex] == nullptr)
        {
            Slots[SlotIndex] = Item;
            OnRep_Slots();
        }
    }
}

URCInventoryItemInstance* URCQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
    URCInventoryItemInstance* Result = nullptr;

    if (ActiveSlotIndex == SlotIndex)
    {
        UnequipItemInSlot();
        ActiveSlotIndex = -1;
    }

    if (Slots.IsValidIndex(SlotIndex))
    {
        Result = Slots[SlotIndex];

        if (Result != nullptr)
        {
            Slots[SlotIndex] = nullptr;
            OnRep_Slots();
        }
    }

    return Result;
}

void URCQuickBarComponent::OnRep_Slots()
{
    FRCQuickBarSlotsChangedMessage Message;
    Message.Owner = GetOwner();
    Message.Slots = Slots;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(TAG_RedCell_QuickBar_Message_SlotsChanged, Message);
}

void URCQuickBarComponent::OnRep_ActiveSlotIndex()
{
    FRCQuickBarActiveIndexChangedMessage Message;
    Message.Owner = GetOwner();
    Message.ActiveIndex = ActiveSlotIndex;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(TAG_RedCell_QuickBar_Message_ActiveIndexChanged, Message);
}

