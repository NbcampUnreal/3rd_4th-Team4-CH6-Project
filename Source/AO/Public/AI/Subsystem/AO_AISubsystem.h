// AO_AISubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_AISubsystem.generated.h"

class AAO_MasterItem;
class AAO_PlayerCharacter;

/**
 * 아이템 예약 정보
 */
USTRUCT()
struct FAO_ItemReservation
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> ReservedBy;

	UPROPERTY()
	float ReservationTime = 0.f;
};

/**
 * 최근 드롭된 아이템 정보
 */
USTRUCT()
struct FAO_RecentlyDroppedItem
{
	GENERATED_BODY()

	UPROPERTY()
	float DropTime = 0.f;

	UPROPERTY()
	float CooldownDuration = 20.f;
};

/**
 * AI 상태 관리 서브시스템
 * - 아이템 예약 상태 관리 (중복 줍기 방지)
 * - 최근 드롭된 아이템 쿨다운 관리
 * - 플레이어 납치 상태 관리 (Insect용, 추후 확장)
 */
UCLASS()
class AO_API UAO_AISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 아이템 예약 시스템
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool TryReserveItem(AAO_MasterItem* Item, AActor* Reserver);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	void ReleaseItem(AAO_MasterItem* Item);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool IsItemReserved(AAO_MasterItem* Item) const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	AActor* GetItemReserver(AAO_MasterItem* Item) const;

	// 최근 드롭된 아이템 관리
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	void MarkItemAsRecentlyDropped(AAO_MasterItem* Item, float CooldownDuration);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool IsItemRecentlyDropped(AAO_MasterItem* Item) const;

	// 플레이어 납치 상태 관리 (Insect용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool TryReservePlayerForKidnap(AAO_PlayerCharacter* Player, AActor* Kidnapper);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	void ReleasePlayerFromKidnap(AAO_PlayerCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool IsPlayerBeingKidnapped(AAO_PlayerCharacter* Player) const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	void MarkPlayerAsRecentlyKidnapped(AAO_PlayerCharacter* Player, float CooldownDuration);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	bool IsPlayerRecentlyKidnapped(AAO_PlayerCharacter* Player) const;

	// 모든 플레이어 위치 가져오기 (아이템 드롭 위치 계산용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	TArray<FVector> GetAllPlayerLocations() const;

	// 특정 위치에서 가장 먼 NavMesh 위치 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Subsystem")
	FVector FindLocationFarthestFromPlayers(const FVector& Origin, float SearchRadius) const;

protected:
	// 만료된 예약/쿨다운 정리
	void CleanupExpiredReservations();
	void CleanupExpiredDropCooldowns();
	void CleanupExpiredKidnapCooldowns();

protected:
	// 아이템 예약 목록
	UPROPERTY()
	TMap<TWeakObjectPtr<AAO_MasterItem>, FAO_ItemReservation> ItemReservations;

	// 최근 드롭된 아이템 목록
	UPROPERTY()
	TMap<TWeakObjectPtr<AAO_MasterItem>, FAO_RecentlyDroppedItem> RecentlyDroppedItems;

	// 납치 중인 플레이어 목록
	UPROPERTY()
	TMap<TWeakObjectPtr<AAO_PlayerCharacter>, TWeakObjectPtr<AActor>> KidnappedPlayers;

	// 최근 납치된 플레이어 쿨다운
	UPROPERTY()
	TMap<TWeakObjectPtr<AAO_PlayerCharacter>, FAO_RecentlyDroppedItem> RecentlyKidnappedPlayers;

	// 예약 만료 시간
	float ReservationTimeout = 30.f;

	// 정리 타이머
	FTimerHandle CleanupTimerHandle;
};
