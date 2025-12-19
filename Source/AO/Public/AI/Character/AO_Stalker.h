// AO_Stalker.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Stalker.generated.h"

class UAO_CeilingMoveComponent;

/**
 * Stalker (추적자형) AI 캐릭터
 * 
 * 특징:
 * - 사족보행, 천장 이동 가능
 * - 은신 및 기습 (엄폐물 활용)
 * - 치고 빠지기 (Hit & Run)
 */
UCLASS()
class AO_API AAO_Stalker : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_Stalker();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	UAO_CeilingMoveComponent* GetCeilingMoveComponent() const { return CeilingMoveComp; }

	// 천장 이동 모드 전환
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void SetCeilingMode(bool bEnable);

	// 공격 후 후퇴 모드 설정
	void SetRetreatMode(bool bRetreat) { bIsRetreating = bRetreat; }
	bool IsRetreating() const { return bIsRetreating; }

protected:
	virtual void BeginPlay() override;
	virtual void HandleStunBegin() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Stalker")
	TObjectPtr<UAO_CeilingMoveComponent> CeilingMoveComp;

	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Stalker")
	bool bIsRetreating = false;
};

