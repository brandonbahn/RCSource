// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"

#include "RCSystemStatics.generated.h"

template <typename T> class TSubclassOf;

class AActor;
class UActorComponent;
class UObject;
struct FFrame;

UCLASS()
class URCSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/** Returns the soft object reference associated with a Primary Asset Id, this works even if the asset is not loaded */
	UFUNCTION(BlueprintPure, Category = "AssetManager", meta=(DeterminesOutputType=ExpectedAssetType))
	static TSoftObjectPtr<UObject> GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType);

	UFUNCTION(BlueprintCallable)
	static FPrimaryAssetId GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="RedCell", meta = (WorldContext = "WorldContextObject"))
	static void PlayNextGame(const UObject* WorldContextObject);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors = true);

	// Gets all the components that inherit from the given class
	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DefaultToSelf="TargetActor", ComponentClass="/Script/Engine.ActorComponent", DeterminesOutputType="ComponentClass"))
	static TArray<UActorComponent*> FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors = true);
};
