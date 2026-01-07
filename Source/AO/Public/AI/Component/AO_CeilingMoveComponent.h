//KSJ : AO_CeilingMoveComponent

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_CeilingMoveComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 * 천장 이동 기능을 담당하는 컴포넌트 (Stalker용)
 * 
 * 구현 방식 제안:
 * 1. NavMesh는 바닥에만 존재하므로, 길찾기는 바닥 NavMesh를 이용한다.
 * 2. 천장 모드 활성화 시:
 *    - 캐릭터의 캡슐을 뒤집는다 (Upside down).
 *    - 매 틱마다 캐릭터 위치 위쪽으로 LineTrace하여 천장 높이를 찾는다.
 *    - 캐릭터의 Z 위치를 (천장 높이 - 캡슐 높이)로 보정하여 시각적으로 천장에 붙어 있는 것처럼 만든다.
 *    - 이동 입력은 그대로 바닥 NavMesh 경로를 따라가지만, 시각적 위치만 천장으로 오프셋된다.
 * 3. 천장이 없는 구간(Open Space)에서는 천장 이동이 불가능하므로, 바닥으로 떨어지거나 진입을 막아야 한다.
 *    - Trace 실패 시 바닥 모드로 강제 전환.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_CeilingMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_CeilingMoveComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 천장 모드 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void SetCeilingMode(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "AO|AI|Stalker")
	bool IsInCeilingMode() const { return bIsCeilingMode; }

	// 전환 몽타주 (BP 세팅 확인용/폴백용)
	UFUNCTION(BlueprintPure, Category = "AO|AI|Stalker")
	UAnimMontage* GetJumpUpMontage() const { return JumpUpMontage; }

	UFUNCTION(BlueprintPure, Category = "AO|AI|Stalker")
	UAnimMontage* GetJumpDownMontage() const { return JumpDownMontage; }

	// 현재 위치 위쪽에 천장이 있어 이동 가능한지 확인
	UFUNCTION(BlueprintPure, Category = "AO|AI|Stalker")
	bool CheckCeilingAvailability() const;

	// 이동 중 천장 감지 및 자동 전환 체크 (바닥 모드일 때 호출)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void CheckForCeilingAutoTransition();

protected:
	virtual void BeginPlay() override;

	// 리플리케이션 콜백: 클라이언트에서 Mesh 회전/위치 업데이트
	UFUNCTION()
	void OnRep_bIsCeilingMode();

private:
	void UpdateCeilingPosition(float DeltaTime, bool bImmediate = false);
	
	// 천장 Normal에 맞춰 캡슐 회전 조정
	void UpdateCapsuleRotationToCeiling(const FVector& CeilingNormal);

	// 클라이언트에서 Mesh 회전/위치를 업데이트하는 헬퍼 함수
	void UpdateMeshVisualsForCeilingMode(bool bEnable);

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MoveComp;

	// 천장 감지 거리
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float CeilingTraceDistance = 500.f;

	// 천장 이동 활성화 여부
	UPROPERTY(ReplicatedUsing = OnRep_bIsCeilingMode, BlueprintReadOnly, Category = "Ceiling Move")
	bool bIsCeilingMode = false;

	// 천장에 붙을 때의 보정 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float CeilingOffset = 10.f;

	// Mesh의 초기 RelativeRotation (복구용)
	UPROPERTY()
	FRotator InitialMeshRotation;

	// Mesh의 초기 RelativeLocation (천장 오프셋 복구용)
	UPROPERTY()
	FVector InitialMeshRelativeLocation = FVector::ZeroVector;
	
	// 초기 Rotation 저장 여부
	UPROPERTY()
	bool bInitialRotationSaved = false;

	// 초기 Location 저장 여부
	UPROPERTY()
	bool bInitialLocationSaved = false;

	// 자동 전환 체크 타이머
	UPROPERTY()
	float AutoTransitionCheckTimer = 0.f;

	// 자동 전환 체크 간격
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float AutoTransitionCheckInterval = 0.2f;

	// 기울어진 천장 지원 최대 각도 (도 단위, 0~90)
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float MaxCeilingAngle = 60.f;

	// 바닥 -> 천장 점프 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ceiling Move|Montage")
	TObjectPtr<UAnimMontage> JumpUpMontage;

	// 천장 -> 바닥 점프 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ceiling Move|Montage")
	TObjectPtr<UAnimMontage> JumpDownMontage;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move|Montage")
	float MontagePlayRate = 1.0f;

	// 전환 보간 관련
	UPROPERTY()
	bool bIsTransitioning = false;

	UPROPERTY()
	float TransitionStartTime = 0.f;

	UPROPERTY()
	float TransitionDuration = 0.f;

	UPROPERTY()
	float StartOffsetZ = 0.f;

	UPROPERTY()
	float TargetOffsetZ = 0.f;

	// 보간 속도 (초 단위, 0이면 즉시 전환)
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move|Transition")
	float TransitionInterpSpeed = 0.f; // 0이면 자동 계산 (몽타주 길이 기반)

	// 보간 시작 지연 (몽타주 시작 후 몇 초 후부터 보간 시작, 바닥→천장 점프 시 발 떨어지는 시점)
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move|Transition")
	float TransitionStartDelay = 0.1f; // 40프레임 기준 4프레임 = 약 0.1초 (30fps 가정)

	// 회전 보간 관련
	UPROPERTY()
	FRotator LastCeilingNormalRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector LastCeilingNormal = FVector::ZeroVector;

	// 회전 보간 속도 (낮을수록 부드럽지만 느림)
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move|Rotation")
	float RotationInterpSpeed = 2.f; // 기본 5.f에서 2.f로 낮춤

	// Normal 변화 데드존 (이 값보다 작은 변화는 무시)
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move|Rotation")
	float NormalChangeThreshold = 0.01f;
};

