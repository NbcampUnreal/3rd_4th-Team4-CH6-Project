// AO_InsectController.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_InsectController.generated.h"

/**
 * Insect 전용 AI Controller
 */
UCLASS()
class AO_API AAO_InsectController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_InsectController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// Perception 업데이트 오버라이드: 쿨타운 중인 플레이어 무시
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;

	// 납치 관련 시야 업데이트 처리 (필요시 오버라이드)
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;
};

