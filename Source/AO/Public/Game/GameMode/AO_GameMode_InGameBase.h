// AO_GameMode_InGameBase.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameMode.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "AO_GameMode_InGameBase.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameMode_InGameBase : public AAO_GameMode
{
	GENERATED_BODY()

public:
	AAO_GameMode_InGameBase();
	
public:
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

private:
	static void LetStartVoiceChat(AController*& C);		// HandleSeamleessTravelPlayer의 C 값을 전달해야 해서 스마트 포인터 변환 보류
	
public:
	void StopVoiceChatForAllClients() const;
	void LetUpdateVoiceMemberForAllClients(const TObjectPtr<AAO_PlayerController_InGameBase>& DeadPlayerController);
	void Test_LetUnmuteVoiceMemberForSurvivor(const TObjectPtr<AAO_PlayerController_InGameBase>& AlivePC);

	// JM : 보이스 채팅 크래시 방지를 위한 작업
	void RequestSynchronizedServerTravel(const FString& URL);	// JM : InGame Server Travel은 이 함수로 진행하기
	void NotifyPlayerCleanupCompleteForTravel(AAO_PlayerController* PC);

private:
	void StartServerTravel();

private:
	UPROPERTY()
	TSet<TObjectPtr<AAO_PlayerController>> CleanupCompletePlayers;
	
	bool bIsTravelSyncInProgress = false;
	FString PendingTravelURL;
};
