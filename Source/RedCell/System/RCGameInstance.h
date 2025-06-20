// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_CommonUserInitialize.h"
#include "CommonGameInstance.h"
#include "RCGameInstance.generated.h"

class ARCPlayerController;
class UObject;

/**
 * 
 */
UCLASS(Config = Game)
class REDCELL_API URCGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:
	URCGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ARCPlayerController* GetPrimaryPlayerController() const;

	virtual bool CanJoinRequestedSession() const override;
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;
	void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;


protected:

	virtual void Init() override;
	virtual void Shutdown() override;

	void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
