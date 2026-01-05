#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_Passive_WorldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FAO_PlayerGASSnapshot
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FString, float> AttributeBaseValues;
	
	UPROPERTY()
	TArray<FGameplayEffectSpec> ActiveSpecs;
};

UCLASS()
class AO_API UAO_Passive_WorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString, FAO_PlayerGASSnapshot> PlayerSnapshots;

	FString GetPlayerPersistentId(APlayerController* PC);
	void SnapshotPlayerData(APlayerController* PC);
	void RestorePlayerGASData(APlayerController* PC);
	void ClearAllPlayerData();
};
