// AO_AIMemoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_AIMemoryComponent.generated.h"

class AAO_PlayerCharacter;

/**
 * 플레이어 위치 기억 정보
 */
USTRUCT(BlueprintType)
struct FAO_PlayerMemoryData
{
	GENERATED_BODY()

	// 마지막으로 확인된 위치
	UPROPERTY(BlueprintReadOnly)
	FVector LastKnownLocation = FVector::ZeroVector;

	// 마지막 확인 시간
	UPROPERTY(BlueprintReadOnly)
	float LastSeenTime = 0.f;

	// 시야에서 사라진 후 마지막 위치 (도주/배회 시 피해야 할 위치)
	UPROPERTY(BlueprintReadOnly)
	FVector LastLostLocation = FVector::ZeroVector;

	// 시야에서 사라진 시간
	UPROPERTY(BlueprintReadOnly)
	float LostTime = 0.f;

	// 현재 시야 내에 있는지
	UPROPERTY(BlueprintReadOnly)
	bool bIsInSight = false;
};

/**
 * AI의 플레이어 위치 기억 컴포넌트
 * - 각 플레이어별 마지막 목격 위치 저장
 * - 멀티플레이어 지원 (플레이어별 구분)
 * - Crab: 피해야 할 위치로 활용
 * - 선공형 AI: 수색 위치로 활용
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AO_API UAO_AIMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_AIMemoryComponent();

	// 플레이어 위치 업데이트 (시야 내에 있을 때)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void UpdatePlayerLocation(AAO_PlayerCharacter* Player, const FVector& Location);

	// 마지막 목격 위치 설정 (시야에서 사라질 때)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void SetLastKnownLocation(AAO_PlayerCharacter* Player, const FVector& Location);

	// 특정 플레이어의 메모리 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	bool GetPlayerMemory(AAO_PlayerCharacter* Player, FAO_PlayerMemoryData& OutData) const;

	// 마지막으로 목격된 플레이어 위치 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	FVector GetLastKnownLocation(AAO_PlayerCharacter* Player) const;

	// 시야에서 사라진 후의 위치 가져오기 (피해야 할 위치)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	FVector GetLastLostLocation(AAO_PlayerCharacter* Player) const;

	// 모든 기억된 플레이어들의 마지막 위치 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	TArray<FVector> GetAllLastLostLocations() const;

	// 특정 시간 이내에 목격된 플레이어 위치들 (피해야 할 위치들)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	TArray<FVector> GetRecentLostLocations(float WithinSeconds) const;

	// 메모리 초기화
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void ClearMemory();

	// 특정 플레이어 메모리 제거
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void ForgetPlayer(AAO_PlayerCharacter* Player);

	// 기억이 유효한지 (일정 시간 이내인지)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	bool IsMemoryValid(AAO_PlayerCharacter* Player, float MaxAge) const;

	// 소리로 감지된 마지막 위치 저장
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void SetLastHeardLocation(const FVector& Location);

	// 소리로 감지된 마지막 위치 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	FVector GetLastHeardLocation() const;

	// 소리 감지 메모리 초기화
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	void ClearHeardLocation();

protected:
	virtual void BeginPlay() override;

protected:
	// 플레이어별 메모리 데이터
	UPROPERTY()
	TMap<TWeakObjectPtr<AAO_PlayerCharacter>, FAO_PlayerMemoryData> PlayerMemories;

	// 메모리 유지 시간 (이 시간이 지나면 오래된 메모리는 무시됨)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Memory")
	float MemoryDuration = 60.f;

	// 소리로 감지된 마지막 위치
	UPROPERTY()
	FVector LastHeardLocation = FVector::ZeroVector;

	// 소리 감지 시간
	UPROPERTY()
	float LastHeardTime = 0.f;

	// 소리 메모리 유효 시간
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Memory")
	float HeardMemoryDuration = 10.f;
};
