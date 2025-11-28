// AO_GameMode_Stage.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameMode_InGameBase.h"
#include "AO_GameMode_Stage.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameMode_Stage : public AAO_GameMode_InGameBase
{
	GENERATED_BODY()

public:
	AAO_GameMode_Stage();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
