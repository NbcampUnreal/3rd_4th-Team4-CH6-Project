// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_PackCoordComp.generated.h"

class AAO_Werewolf;
class AAO_PlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHowlReceived, AActor*, TargetActor);

/**
 * Werewolf 무리(Pack) 행동을 관리하는 컴포넌트
 * - Howl 전파 및 수신
 * - 무리 멤버 관리 (EQS Context용)
 * - 포위/공격 상태 동기화
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_PackCoordComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_PackCoordComp();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Howl Logic ---
	
	// 하울링 시작 (주변 아군에게 전파)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void BroadcastHowl(AActor* Target);

	// 하울링 수신 (다른 아군으로부터)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ReceiveHowl(AActor* Sender, AActor* Target);

	// 하울링 수신 이벤트 (StateTree 등에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|Werewolf")
	FOnHowlReceived OnHowlReceived;

	// 하울링 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Werewolf")
	float HowlRadius = 2000.f;

	// --- Pack Management ---

	// 주변의 모든 Werewolf 찾기 (자신 제외)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	TArray<AAO_Werewolf*> GetNearbyPackMembers() const;

	// 현재 포위 모드인지?
	bool IsSurrounding() const { return bIsSurrounding; }
	
	// 포위 모드 설정
	void SetSurroundMode(bool bSurround);

	// 일제 공격 시작 (타이머 종료 or 준비 완료)
	void StartCoordinatedAttack();

protected:
	// 현재 포위 중인지 여부
	bool bIsSurrounding = false;

	// 일제 공격 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// 일제 공격까지 대기 시간 (5~8초)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float MinAttackDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float MaxAttackDelay = 8.f;

	// 공격 명령 실행
	void ExecuteAttack();
};
