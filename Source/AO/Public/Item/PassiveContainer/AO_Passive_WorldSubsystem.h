#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_Passive_WorldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FAO_PlayerPassiveData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGameplayTag, float> CumulativePassives;
};

UCLASS()
class AO_API UAO_Passive_WorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RecordPassiveUpgrade(APlayerController* PC, FGameplayTag PassiveTag, float Amount);
	
	void ReapplyAllPassives(APlayerController* PC);
	
	FString GetPlayerPersistentId(APlayerController* PC);
	
	void ClearAllPlayerData();

private:
	UPROPERTY()
	TMap<FString, FAO_PlayerPassiveData> PlayerPassiveStats;
};