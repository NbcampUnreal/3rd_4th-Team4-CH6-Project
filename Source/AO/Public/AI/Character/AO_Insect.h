// AO_Insect.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Insect.generated.h"

class UAO_KidnapComponent;

/**
 * Insect (곤충형) AI 캐릭터
 * 
 * 특징:
 * - 플레이어를 납치하여 다른 플레이어들로부터 격리시킴
 * - 납치 중에는 뒷걸음질로 이동
 * - 기절 시 납치 중인 플레이어를 떨어뜨림
 * 
 * 속도:
 * - 기본/추격: 500
 * - 납치 이동: 400
 */
UCLASS()
class AO_API AAO_Insect : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_Insect();

	// 납치 컴포넌트 접근
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Insect")
	UAO_KidnapComponent* GetKidnapComponent() const { return KidnapComponent; }

	// 현재 납치 중인지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Insect")
	bool IsKidnapping() const;

	// 납치 후 이동할 안전한 위치 계산 (EQS 대신 C++ 로직 사용 시)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Insect")
	FVector CalculateSafeDropLocation(const FVector& ExcludeLocation = FVector::ZeroVector) const;

	// 이동 속도 업데이트
	void UpdateMovementSpeed();

protected:
	virtual void BeginPlay() override;

	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

	// 납치 상태 변경 이벤트 핸들러
	UFUNCTION()
	void OnKidnapStateChanged(bool bIsKidnapping);

protected:
	// 납치 기능 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Insect")
	TObjectPtr<UAO_KidnapComponent> KidnapComponent;

	// 이동 속도 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Insect|Movement")
	float NormalSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Insect|Movement")
	float DragSpeed = 400.f;

	// 안전 위치 탐색 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Insect|Logic")
	float SafeLocationSearchRadius = 3000.f;

	// 납치 시작 시 캐시된 플레이어 위치들
	UPROPERTY()
	TArray<FVector> CachedPlayerLocations;
};

