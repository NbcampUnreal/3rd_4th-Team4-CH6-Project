// AO_AICharacterBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
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
};
