// AO_WeaponHolderComp.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_WeaponHolderComp.generated.h"

class AAO_TrollWeapon;

// 무기 상태 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged, bool, bHasWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponPickedUp, AAO_TrollWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponDropped, AAO_TrollWeapon*, Weapon);

/**
 * Troll의 무기 소지/드롭을 관리하는 컴포넌트
 * 
 * 역할:
 * - 무기 줍기/드롭 로직
 * - 무기 소지 상태 관리
 * - 시야 내 무기 탐지
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_WeaponHolderComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_WeaponHolderComp();

	// 무기 소지 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	bool HasWeapon() const { return CurrentWeapon.IsValid(); }

	// 현재 무기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	AAO_TrollWeapon* GetCurrentWeapon() const { return CurrentWeapon.Get(); }

	// 무기 줍기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	bool PickupWeapon(AAO_TrollWeapon* Weapon);

	// 무기 드롭
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	void DropWeapon();

	// 시야 내 무기 찾기 (가장 가까운 무기)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	AAO_TrollWeapon* FindNearestWeaponInSight() const;

	// 주변 무기 찾기 (시야 무관, 반경 기반)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	AAO_TrollWeapon* FindNearestWeaponInRadius(float Radius) const;

	// 감지된 무기 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|WeaponHolder")
	const TArray<AAO_TrollWeapon*>& GetWeaponsInSight() const;

	// 무기 감지 이벤트 (Controller에서 호출)
	void OnWeaponDetected(AAO_TrollWeapon* Weapon);
	void OnWeaponLost(AAO_TrollWeapon* Weapon);

public:
	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|WeaponHolder")
	FOnWeaponStateChanged OnWeaponStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "AO|AI|WeaponHolder")
	FOnWeaponPickedUp OnWeaponPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "AO|AI|WeaponHolder")
	FOnWeaponDropped OnWeaponDropped;

protected:
	virtual void BeginPlay() override;

protected:
	// 현재 소지 중인 무기
	UPROPERTY()
	TWeakObjectPtr<AAO_TrollWeapon> CurrentWeapon;

	// 시야 내 감지된 무기들
	UPROPERTY()
	TArray<TWeakObjectPtr<AAO_TrollWeapon>> WeaponsInSight;

	// 캐싱용 배열
	mutable TArray<AAO_TrollWeapon*> CachedWeaponsInSight;
};
