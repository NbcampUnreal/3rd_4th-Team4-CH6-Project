// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_WerewolfController.generated.h"

class UAO_PackCoordComp;

/**
 * Werewolf Controller
 * - 최초 발견 시 Howl 수행
 * - 포위/도주로 차단 EQS 활용
 */
UCLASS()
class AO_API AAO_WerewolfController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_WerewolfController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 플레이어 감지 오버라이드 (Howl 로직)
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;

	// Howl 수신 시 처리
	UFUNCTION()
	void HandleHowlReceived(AActor* TargetActor);

protected:
	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp;

	// 이미 Howl을 했거나 참여했는지 여부 (상태 관리용)
	bool bHasHowledOrJoined = false;
};
