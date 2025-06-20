// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RCPreMovementTickComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RCCharacterMovementComponent.h"
#include "RCCharacter.h"
#include "RCLogChannels.h"

URCPreMovementTickComponent::URCPreMovementTickComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), RCCharacterMovementComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void URCPreMovementTickComponent::BeginPlay()
{
	Super::BeginPlay();

	// Add more detailed logging
	UE_LOG(LogRC, Warning, TEXT("PreMovementTickComponent BeginPlay - Role: %s, Actor: %s"), 
		GetOwner()->HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*GetOwner()->GetName());

	if (AActor* Owner = GetOwner())
	{
		RCCharacterMovementComponent = Owner->FindComponentByClass<URCCharacterMovementComponent>();
		if (RCCharacterMovementComponent)
		{
			AddTickPrerequisiteComponent(RCCharacterMovementComponent);
			UE_LOG(LogRC, Warning, TEXT("PreCMC: Added tick prerequisite to CMC"));
		}
	}	
}

// Called every frame
void URCPreMovementTickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (ARCCharacter* OwnerCharacter = Cast<ARCCharacter>(GetOwner()))
	{
		OwnerCharacter->PreCMCTick();
	}
}

