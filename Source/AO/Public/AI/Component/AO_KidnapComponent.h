// AO_KidnapComponent.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AO_KidnapComponent.generated.h"

class AAO_PlayerCharacter;
class UAO_GA_Insect_Kidnap;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKidnapStateChanged, bool, bIsKidnapping);

/**
 * Insect의 납치 기능을 담당하는 컴포넌트
 * - 플레이어 부착/분리
 * - DoT 데미지 적용
 * - 납치 상태 관리
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_KidnapComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_KidnapComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 납치 시도 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Kidnap")
	bool TryKidnapPlayer(AAO_PlayerCharacter* TargetPlayer);

	// 납치 해제 (bThrow: 던질지 여부)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Kidnap")
	void ReleaseKidnap(bool bThrow);

	// 현재 납치 중인지
	UFUNCTION(BlueprintPure, Category = "AO|AI|Kidnap")
	bool IsKidnapping() const { return CurrentVictim != nullptr; }

	// 현재 납치 중인 대상
	UFUNCTION(BlueprintPure, Category = "AO|AI|Kidnap")
	AAO_PlayerCharacter* GetCurrentVictim() const { return CurrentVictim; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// DoT 데미지 처리
	void ApplyDotDamage();

	// 플레이어 제약 설정 (이동 불가 등)
	void SetPlayerRestrictions(AAO_PlayerCharacter* Player, bool bRestrict);

	// 플레이어 사망 감지 델리게이트 바인딩
	void BindDeathDelegate();

	// 플레이어 사망 시 호출
	UFUNCTION()
	void OnPlayerDeathTagChanged(const FGameplayTag Tag, int32 NewCount);

public:
	// 납치 상태 변경 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|Kidnap")
	FOnKidnapStateChanged OnKidnapStateChanged;

protected:
	// 납치 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	FName KidnapSocketName = FName("KidnapSocket");

	// 초당 데미지
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	float DotDamageAmount = 10.f;

	// DoT 주기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	float DotInterval = 1.0f;

	// DoT 데미지 GameplayEffect 클래스
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	TSubclassOf<UGameplayEffect> DotDamageEffectClass = nullptr;

	// 납치 해제 후 쿨다운 시간 (초) - 같은 플레이어를 바로 다시 납치하지 못하도록
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	float KidnapCooldownDuration = 10.0f;

	// 납치 시 플레이어에게 적용할 태그 (제약용)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	FGameplayTag KidnappedStatusTag;
	
	// 넉다운 효과 태그 (던질 때 사용)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Kidnap")
	FGameplayTag KnockdownTag;

private:
	// 현재 납치 중인 플레이어
	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> CurrentVictim;

	// DoT 타이머 핸들
	FTimerHandle DotTimerHandle;

	// 카메라 충돌 원래 상태 저장
	bool bOriginalCameraCollision = true;
};

