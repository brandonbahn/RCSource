// RCPlayerState.cpp

#include "RCPlayerState.h"

#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCCombatSet.h"
#include "AbilitySystem/Attributes/RCCoreSet.h"
#include "AbilitySystem/RCAbilitySet.h"
#include "AbilitySystem/RCAbilitySystemComponent.h"
#include "Character/RCPawnData.h"
#include "Character/RCPawnExtensionComponent.h"
#include "Engine/World.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/Experience/RCExperienceManagerComponent.h"
#include "GameplayEffectTypes.h"
#include "RCPlayerController.h"
#include "GameModes/RCGameMode.h"
#include "Messages/RCVerbMessage.h"
#include "RCLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

// Need this for EGameplayEffectReplicationMode

const FName ARCPlayerState::NAME_RCAbilityReady("RedCellAbilitiesReady");

ARCPlayerState::ARCPlayerState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , MyPlayerConnectionType(ERCPlayerConnectionType::Player)
{
    AbilitySystemComponent = CreateDefaultSubobject<URCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);        // Make sure it's replicated
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); 
    // Mixed mode so that GameplayEffects and attribute changes
    // Goes to *only* the owning client.
    
    // These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
    HealthSet = CreateDefaultSubobject<URCHealthSet>(TEXT("HealthSet"));
    CombatSet = CreateDefaultSubobject<URCCombatSet>(TEXT("CombatSet"));
    CoreSet   = CreateDefaultSubobject<URCCoreSet>(TEXT("CoreSet"));
    
    // 3) Tell the ASC
    //AbilitySystemComponent->AddAttributeSetSubobject(HealthSet.Get());
    
    // AbilitySystemComponent needs to be updated at a high frequency.
    SetNetUpdateFrequency(100.0f);

    MyTeamID = FGenericTeamId::NoTeam;
    MySquadID = INDEX_NONE;
}

void ARCPlayerState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void ARCPlayerState::Reset()
{
    Super::Reset();
}

void ARCPlayerState::ClientInitialize(AController* C)
{
    Super::ClientInitialize(C);

    if (URCPawnExtensionComponent* PawnExtComp = URCPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
    {
        PawnExtComp->CheckDefaultInitialization();
    }
}

void ARCPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
}

void ARCPlayerState::OnDeactivated()
{
    bool bDestroyDeactivatedPlayerState = false;

    switch (GetPlayerConnectionType())
    {
    case ERCPlayerConnectionType::Player:
    case ERCPlayerConnectionType::InactivePlayer:
        //@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
        // (e.g., for long running servers where they might build up if lots of players cycle through)
        bDestroyDeactivatedPlayerState = true;
        break;
    default:
        bDestroyDeactivatedPlayerState = true;
        break;
    }
	
    SetPlayerConnectionType(ERCPlayerConnectionType::InactivePlayer);

    if (bDestroyDeactivatedPlayerState)
    {
        Destroy();
    }
}

void ARCPlayerState::OnReactivated()
{
    if (GetPlayerConnectionType() == ERCPlayerConnectionType::InactivePlayer)
    {
        SetPlayerConnectionType(ERCPlayerConnectionType::Player);
    }
}

void ARCPlayerState::OnExperienceLoaded(const URCExperienceDefinition* /*CurrentExperience*/)
{
    if (ARCGameMode* RCGameMode = GetWorld()->GetAuthGameMode<ARCGameMode>())
    {
        if (const URCPawnData* NewPawnData = RCGameMode->GetPawnDataForController(GetOwningController()))
        {
            SetPawnData(NewPawnData);
        }
        else
        {
            UE_LOG(LogRC, Error, TEXT("ARCPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
        }
    }
}

void ARCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    FDoRepLifetimeParams SharedParams;
    SharedParams.bIsPushBased = true;
    
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

    SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;

    DOREPLIFETIME(ThisClass, StatTags);	
}


ARCPlayerController* ARCPlayerState::GetRCPlayerController() const
{
    return Cast<ARCPlayerController>(GetOwner());
}

UAbilitySystemComponent* ARCPlayerState::GetAbilitySystemComponent() const
{
    return GetRCAbilitySystemComponent();
}

void ARCPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    UE_LOG(LogRC, Warning, TEXT("PlayerState PostInit: GetPawn() = %s"), *GetNameSafe(GetPawn()));
    AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

    UWorld* World = GetWorld();
    if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
    {
        AGameStateBase* GameState = GetWorld()->GetGameState();
        check(GameState);
        URCExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<URCExperienceManagerComponent>();
        check(ExperienceComponent);
        ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnRCExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
    }
}


void ARCPlayerState::SetPawnData(const URCPawnData* InPawnData)
{
    check(InPawnData);

    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (PawnData)
    {
        UE_LOG(LogRC, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
        return;
    }

    MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
    PawnData = InPawnData;

    UE_LOG(LogTemp, Warning, TEXT("PlayerState: About to grant AbilitySets"));
    for (const URCAbilitySet* AbilitySet : PawnData->AbilitySets)
    {
        if (AbilitySet)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerState: Granting AbilitySet: %s"), *GetNameSafe(AbilitySet));
            AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("PlayerState: Finished granting AbilitySets"));

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_RCAbilityReady);
    
    ForceNetUpdate();
}

void ARCPlayerState::OnRep_PawnData()
{
}

void ARCPlayerState::SetPlayerConnectionType(ERCPlayerConnectionType NewType)
{
    MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
    MyPlayerConnectionType = NewType;
}

void ARCPlayerState::SetSquadID(int32 NewSquadId)
{
    if (HasAuthority())
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

        MySquadID = NewSquadId;
    }
}

void ARCPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (HasAuthority())
    {
        const FGenericTeamId OldTeamID = MyTeamID;

        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
        MyTeamID = NewTeamID;
        ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
    }
    else
    {
        UE_LOG(LogRCTeams, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
    }
}

FGenericTeamId ARCPlayerState::GetGenericTeamId() const
{
    return MyTeamID;
}

FOnRCTeamIndexChangedDelegate* ARCPlayerState::GetOnTeamIndexChangedDelegate()
{
    return &OnTeamChangedDelegate;
}

void ARCPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ARCPlayerState::OnRep_MySquadID()
{
    //@TODO: Let the squad subsystem know (once that exists)
}

void ARCPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.AddStack(Tag, StackCount);
}

void ARCPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.RemoveStack(Tag, StackCount);
}

int32 ARCPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
    return StatTags.GetStackCount(Tag);
}

bool ARCPlayerState::HasStatTag(FGameplayTag Tag) const
{
    return StatTags.ContainsTag(Tag);
}

void ARCPlayerState::ClientBroadcastMessage_Implementation(const FRCVerbMessage Message)
{
    if (GetNetMode() == NM_Client)
    {
        UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
    } 
}
