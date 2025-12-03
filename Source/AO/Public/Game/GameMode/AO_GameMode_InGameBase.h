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

public:
	void StopVoiceChatForAllClients() const;
	void LetUpdateVoiceMemberForAllClients(const AAO_PlayerController_InGameBase* DeadPlayerController);

	void Test_LetUnmuteVoiceMemberForSurvivor(const AAO_PlayerController_InGameBase* AlivePC);

private:
	static void LetStartVoiceChat(AController*& C);
};
