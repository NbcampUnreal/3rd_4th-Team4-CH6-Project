#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_Passive_WorldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FAO_PlayerPassiveData
{
	GENERATED_BODY()

	// 어떤 패시브 태그에 대해 얼마만큼의 수치가 누적되었는지 저장
	// Key: Event Tag (예: Event.Interaction.AddPassive.MaxHP)
	// Value: 누적된 Magnitude 총합
	UPROPERTY()
	TMap<FGameplayTag, float> CumulativePassives;
};

UCLASS()
class AO_API UAO_Passive_WorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 플레이어의 패시브 획득 기록을 누적합니다 (아이템 사용 시 호출) */
	void RecordPassiveUpgrade(APlayerController* PC, FGameplayTag PassiveTag, float Amount);

	/** 저장된 모든 패시브를 캐릭터에게 다시 적용합니다 (부활 시 호출) */
	void ReapplyAllPassives(APlayerController* PC);

	/** 플레이어별 고유 ID를 가져옵니다 */
	FString GetPlayerPersistentId(APlayerController* PC);

	/** 게임 종료나 특정 상황에서 데이터를 초기화합니다 */
	void ClearAllPlayerData();

private:
	// 플레이어 이름(ID) 기반 누적 패시브 데이터 저장소
	UPROPERTY()
	TMap<FString, FAO_PlayerPassiveData> PlayerPassiveStats;
};