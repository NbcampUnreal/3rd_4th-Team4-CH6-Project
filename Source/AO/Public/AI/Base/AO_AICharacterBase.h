// AO_AICharacterBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AI/AO_AITypes.h"
#include "AO_AICharacterBase.generated.h"

class UAbilitySystemComponent;
class UAO_AIAttributeSet;
class UAO_AIMemoryComponent;
class UGameplayAbility;
class UGameplayEffect;

/**
 * 모든 AI 캐릭터의 공통 베이스 클래스
 * - GAS 통합 (AbilitySystemComponent)
 * - 기절(Stun) 처리
 * - 플레이어 위치 기억 (Memory Component)
 */
UCLASS()
class AO_API AAO_AICharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_AICharacterBase();

	// Tick 오버라이드 (디버그용)
	virtual void Tick(float DeltaTime) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 기절 관련
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	bool IsStunned() const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	void OnStunBegin();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	void OnStunEnd();

	// Memory Component 접근
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	UAO_AIMemoryComponent* GetMemoryComponent() const { return MemoryComponent; }

	// 테스트용: 기절 트리거 (에디터/블루프린트에서 호출 가능)
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AO|AI|Debug")
	void TestStun();

	// 테스트용: 기절 해제 (에디터/블루프린트에서 호출 가능)
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AO|AI|Debug")
	void TestStunEnd();

	// --- 공통 공격 인터페이스 ---
	
	// 현재 수행해야 할 공격 설정 반환 (자식 클래스에서 오버라이드 필수)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AO|AI|Combat")
	FEnemyAttackConfig GetCurrentAttackConfig() const;
	virtual FEnemyAttackConfig GetCurrentAttackConfig_Implementation() const;

	// 공격 상태 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Combat")
	virtual void SetIsAttacking(bool bAttacking);

	// 공격 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Combat")
	virtual bool IsAttacking() const { return bIsAttacking; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// GAS 초기화
	void InitializeAbilitySystem();
	void BindDefaultAbilities();
	void BindDefaultEffects();

	// 기절 시 호출되는 가상 함수 (자식 클래스에서 오버라이드)
	virtual void HandleStunBegin();
	virtual void HandleStunEnd();

protected:
	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|GAS")
	TObjectPtr<UAO_AIAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	// Memory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Memory")
	TObjectPtr<UAO_AIMemoryComponent> MemoryComponent;

	// 이동 속도 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Movement")
	float DefaultMovementSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Movement")
	float AlertMovementSpeed = 500.f;

	// 공격 중 상태 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AO|AI|Combat")
	bool bIsAttacking = false;
};
