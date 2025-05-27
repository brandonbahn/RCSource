// Fill out your copyright notice in the Description page of Project Settings.

// RCGameMode.cpp
#include "RCGameMode.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "RCLogChannels.h"
#include "Misc/CommandLine.h"
#include "System/RCAssetManager.h"
#include "RCGameState.h"
#include "System/RCGameSession.h"
#include "Player/RCPlayerController.h"
#include "Player/RCPlayerState.h"
#include "GameModes/RCGameState.h"
#include "Character/RCCharacter.h"
#include "UI/RCHUD.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Character/RCPawnData.h"
#include "GameModes/RCWorldSettings.h"
#include "GameModes/Experience/RCExperienceDefinition.h"
#include "GameModes/Experience/RCExperienceManagerComponent.h"
#include "GameModes/Experience/RCUserFacingExperienceDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Development/RCDeveloperSettings.h"
#include "Player/RCPlayerSpawningManagerComponent.h"
#include "CommonUserSubsystem.h"
#include "CommonSessionSubsystem.h"
#include "TimerManager.h"
#include "GameMapsSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(RCGameMode)

ARCGameMode::ARCGameMode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PlayerControllerClass = ARCPlayerController::StaticClass();
	GameSessionClass      = ARCGameSession     ::StaticClass();
    PlayerStateClass      = ARCPlayerState     ::StaticClass();
    GameStateClass        = ARCGameState       ::StaticClass();
    DefaultPawnClass      = ARCCharacter       ::StaticClass();
    HUDClass              = ARCHUD             ::StaticClass();
}

const URCPawnData* ARCGameMode::GetPawnDataForController(const AController* InController) const
{
    // See if pawn data is already set on the player state
    if (InController != nullptr)
    {
        if (const ARCPlayerState* RCPS = InController->GetPlayerState<ARCPlayerState>())
        {
            if (const URCPawnData* PawnData = RCPS->GetPawnData<URCPawnData>())
            {
                return PawnData;
            }
        }
    }
    
    // If not, fall back to the default for the current experience
    check(GameState);
    URCExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<URCExperienceManagerComponent>();
    check(ExperienceComponent);

    if (ExperienceComponent->IsExperienceLoaded())
    {
        const URCExperienceDefinition* Experience = ExperienceComponent->GetCurrentExperienceChecked();
        if (Experience->DefaultPawnData != nullptr)
        {
            return Experience->DefaultPawnData;
        }

        // Experience is loaded and there's still no pawn data, fall back to the default for now
        return URCAssetManager::Get().GetDefaultPawnData();
    }

    // Experience not loaded yet, so there is no pawn data to be had
    return nullptr;
}

void ARCGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void ARCGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// Precedence order (highest wins)
	//  - Matchmaking assignment (if present)
	//  - URL Options override
	//  - Developer Settings (PIE only)
	//  - Command Line override
	//  - World Settings
	//  - Dedicated server
	//  - Default experience

	UWorld* World = GetWorld();

	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(URCExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<URCDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	// see if the command line wants to set the experience
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(URCExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	// see if the world settings has a default experience
	if (!ExperienceId.IsValid())
	{
		if (ARCWorldSettings* TypedWorldSettings = Cast<ARCWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	URCAssetManager& AssetManager = URCAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		UE_LOG(LogRCExperience, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// Final fallback to the default experience
	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// This will start to host as a dedicated server
			return;
		}

		//@TODO: Pull this from a config setting or something
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("RCExperienceDefinition"), FName("B_RCDefaultExperience"));
		ExperienceIdSource = TEXT("Default");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

bool ARCGameMode::TryDedicatedServerLogin()
{
	// Some basic code to register as an active dedicated server, this would be heavily modified by the game
	FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && World && World->GetNetMode() == NM_DedicatedServer && World->URL.Map == DefaultMap)
	{
		// Only register if this is the default map on a dedicated server
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

		// Dedicated servers may need to do an online login
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ARCGameMode::OnUserInitializedForDedicatedServer);

		// There are no local users on dedicated server, but index 0 means the default platform user which is handled by the online login code
		if (!UserSubsystem->TryToLoginForOnlinePlay(0))
		{
			OnUserInitializedForDedicatedServer(nullptr, false, FText(), ECommonUserPrivilege::CanPlayOnline, ECommonUserOnlineContext::Default);
		}

		return true;
	}

	return false;
}

void ARCGameMode::HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode)
{
	FPrimaryAssetType UserExperienceType = URCUserFacingExperienceDefinition::StaticClass()->GetFName();
	
	// Figure out what UserFacingExperience to load
	FPrimaryAssetId UserExperienceId;
	FString UserExperienceFromCommandLine;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserExperience="), UserExperienceFromCommandLine) ||
		FParse::Value(FCommandLine::Get(), TEXT("Playlist="), UserExperienceFromCommandLine))
	{
		UserExperienceId = FPrimaryAssetId::ParseTypeAndName(UserExperienceFromCommandLine);
		if (!UserExperienceId.PrimaryAssetType.IsValid())
		{
			UserExperienceId = FPrimaryAssetId(FPrimaryAssetType(UserExperienceType), FName(*UserExperienceFromCommandLine));
		}
	}

	// Search for the matching experience, it's fine to force load them because we're in dedicated server startup
	URCAssetManager& AssetManager = URCAssetManager::Get();
	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssetsWithType(UserExperienceType);
	if (ensure(Handle.IsValid()))
	{
		Handle->WaitUntilComplete();
	}

	TArray<UObject*> UserExperiences;
	AssetManager.GetPrimaryAssetObjectList(UserExperienceType, UserExperiences);
	URCUserFacingExperienceDefinition* FoundExperience = nullptr;
	URCUserFacingExperienceDefinition* DefaultExperience = nullptr;

	for (UObject* Object : UserExperiences)
	{
		URCUserFacingExperienceDefinition* UserExperience = Cast<URCUserFacingExperienceDefinition>(Object);
		if (ensure(UserExperience))
		{
			if (UserExperience->GetPrimaryAssetId() == UserExperienceId)
			{
				FoundExperience = UserExperience;
				break;
			}
			
			if (UserExperience->bIsDefaultExperience && DefaultExperience == nullptr)
			{
				DefaultExperience = UserExperience;
			}
		}
	}

	if (FoundExperience == nullptr)
	{
		FoundExperience = DefaultExperience;
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(FoundExperience && GameInstance))
	{
		// Actually host the game
		UCommonSession_HostSessionRequest* HostRequest = FoundExperience->CreateHostingRequest(this);
		if (ensure(HostRequest))
		{
			HostRequest->OnlineMode = OnlineMode;

			// TODO override other parameters?

			UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
			SessionSubsystem->HostSession(nullptr, HostRequest);
			
			// This will handle the map travel
		}
	}

}


void ARCGameMode::OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		// Unbind
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
		UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ARCGameMode::OnUserInitializedForDedicatedServer);

		// Dedicated servers do not require user login, but some online subsystems may expect it
		if (bSuccess && ensure(UserInfo))
		{
			UE_LOG(LogRCExperience, Log, TEXT("Dedicated server user login succeeded for id %s, starting online server"), *UserInfo->GetNetId().ToString());
		}
		else
		{
			UE_LOG(LogRCExperience, Log, TEXT("Dedicated server user login unsuccessful, starting online server as login is not required"));
		}
		
		HostDedicatedServerMatch(ECommonSessionOnlineMode::Online);
	}
}


void ARCGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogRCExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

		URCExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<URCExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogRCExperience, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}


void ARCGameMode::OnExperienceLoaded(const URCExperienceDefinition* CurrentExperience)
{
    // Spawn any players that are already attached
    //@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
    // GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Cast<APlayerController>(*Iterator);
        if ((PC != nullptr) && (PC->GetPawn() == nullptr))
        {
            if (PlayerCanRestart(PC))
            {
                RestartPlayer(PC);
            }
        }
    }
}

bool ARCGameMode::IsExperienceLoaded() const
{
    check(GameState);
    URCExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<URCExperienceManagerComponent>();
    check(ExperienceComponent);

    return ExperienceComponent->IsExperienceLoaded();
}




UClass* ARCGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (const URCPawnData* PawnData = GetPawnDataForController(InController))
    {
        if (PawnData->PawnClass)
        {
            return PawnData->PawnClass;
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ARCGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient;    // Never save the default player pawns into a map.
    SpawnInfo.bDeferConstruction = true;

    if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
    {
        if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
        {
            if (URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
            {
                if (const URCPawnData* PawnData = GetPawnDataForController(NewPlayer))
                {
                    PawnExtComp->SetPawnData(PawnData);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
                }
            }

            SpawnedPawn->FinishSpawning(SpawnTransform);

            return SpawnedPawn;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
    }

    return nullptr;
}

void ARCGameMode::InitGameState()
{
	Super::InitGameState();

	// Listen for the experience load to complete	
	URCExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<URCExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnRCExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}
