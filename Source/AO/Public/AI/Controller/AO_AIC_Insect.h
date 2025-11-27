// KSJ : AO_AIC_Insect.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_EnemyBase.h"
#include "AO_AIC_Insect.generated.h"

/**
 * Insect 전용 AI Controller
 * - 기본 감각 설정 (시각)
 * - 납치 관련 블랙보드 관리
 */
UCLASS()
class AO_API AAO_AIC_Insect : public AAO_AIC_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Insect();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	// Base 클래스의 감각 처리 오버라이드
	virtual void HandleSight(AActor* Actor, const FAIStimulus& Stimulus) override;

private:
	// 블랙보드 키 이름 상수
	const FName BBKey_TargetPlayer = FName("TargetPlayer");
	const FName BBKey_bPlayerVisible = FName("bPlayerVisible");
	const FName BBKey_LastKnownPlayerLocation = FName("LastKnownPlayerLocation");
	const FName BBKey_bIsKidnapping = FName("bIsKidnapping");
	const FName BBKey_KidnappedPlayer = FName("KidnappedPlayer");
	const FName BBKey_KidnapStartLocation = FName("KidnapStartLocation");
	const FName BBKey_KidnapTargetLocation = FName("KidnapTargetLocation");
};
