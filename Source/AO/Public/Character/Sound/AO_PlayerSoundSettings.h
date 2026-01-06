// AO_PlayerSoundSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AO_PlayerSoundSettings.generated.h"

class UAO_PlayerSoundDataAsset;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Player Sound Settings"))
class AO_API UAO_PlayerSoundSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<UAO_PlayerSoundDataAsset> CharacterSoundDataAsset;

	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> DefaultNotEnoughStaminaSound;

	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> DefaultDamageReactSound;
};
