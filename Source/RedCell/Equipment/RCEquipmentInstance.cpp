// Fill out your copyright notice in the Description page of Project Settings.


#include "RCEquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "RCEquipmentDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

URCEquipmentInstance::URCEquipmentInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UWorld* URCEquipmentInstance::GetWorld() const
{
    if (APawn* OwningPawn = GetPawn())
    {
        return OwningPawn->GetWorld();
    }
    else
    {
        return nullptr;
    }
}

void URCEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, Instigator);
    DOREPLIFETIME(ThisClass, SpawnedActors);
}

#if UE_WITH_IRIS
void URCEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
    using namespace UE::Net;

    // Build descriptors and allocate PropertyReplicationFragments for this object
    FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* URCEquipmentInstance::GetPawn() const
{
    return Cast<APawn>(GetOuter());
}

APawn* URCEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
    APawn* Result = nullptr;
    if (UClass* ActualPawnType = PawnType)
    {
        if (GetOuter()->IsA(ActualPawnType))
        {
            Result = Cast<APawn>(GetOuter());
        }
    }
    return Result;
}

void URCEquipmentInstance::SpawnEquipmentActors(const TArray<FRCEquipmentActorToSpawn>& ActorsToSpawn)
{
    if (APawn* OwningPawn = GetPawn())
    {
        USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
        if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
        {
            AttachTarget = Char->GetMesh();
        }

        for (const FRCEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
        {
            AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
            NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
            NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
            NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

            SpawnedActors.Add(NewActor);
        }
    }
}

void URCEquipmentInstance::DestroyEquipmentActors()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
}

void URCEquipmentInstance::OnEquipped()
{
    K2_OnEquipped();
}

void URCEquipmentInstance::OnUnequipped()
{
    K2_OnUnequipped();
}

void URCEquipmentInstance::OnRep_Instigator()
{
}


