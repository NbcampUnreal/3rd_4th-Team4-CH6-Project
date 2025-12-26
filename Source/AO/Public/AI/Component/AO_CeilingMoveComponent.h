// AO_CeilingMoveComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_CeilingMoveComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class USkeletalMeshComponent;

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

	// 현재 위치 위쪽에 천장이 있어 이동 가능한지 확인
	UFUNCTION(BlueprintPure, Category = "AO|AI|Stalker")
	bool CheckCeilingAvailability() const;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateCeilingPosition(float DeltaTime, bool bImmediate = false);

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MoveComp;

	// 천장 감지 거리
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float CeilingTraceDistance = 500.f;

	// 천장 이동 활성화 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ceiling Move")
	bool bIsCeilingMode = false;

	// 천장에 붙을 때의 보정 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Ceiling Move")
	float CeilingOffset = 10.f;

	// Mesh의 초기 RelativeRotation (복구용)
	UPROPERTY()
	FRotator InitialMeshRotation;
	
	// 초기 Rotation 저장 여부
	UPROPERTY()
	bool bInitialRotationSaved = false;
};

