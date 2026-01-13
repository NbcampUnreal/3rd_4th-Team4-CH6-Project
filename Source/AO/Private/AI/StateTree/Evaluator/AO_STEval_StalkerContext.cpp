//KSJ : AO_STEval_StalkerContext

#include "AI/StateTree/Evaluator/AO_STEval_StalkerContext.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void FAO_STEval_StalkerContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_StalkerCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_StalkerCtx_InstanceData>(*this);
	UpdateStalkerContextData(Context, InstanceData, 0.f);
}

void FAO_STEval_StalkerContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_StalkerCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_StalkerCtx_InstanceData>(*this);
	UpdateStalkerContextData(Context, InstanceData, DeltaTime);
}

void FAO_STEval_StalkerContext::UpdateStalkerContextData(FStateTreeExecutionContext& Context, FAO_STEval_StalkerCtx_InstanceData& InstanceData, float DeltaTime) const
{
	// 부모 클래스의 업데이트 로직 먼저 실행 (기본 Aggressive AI 데이터 채움)
	UpdateContextData(Context, InstanceData);

	// Stalker Controller 캐스팅
	AAO_StalkerController* StalkerCtrl = Cast<AAO_StalkerController>(GetAggressiveController(Context));
	if (!StalkerCtrl)
	{
		return;
	}

	AAO_Stalker* Stalker = StalkerCtrl->GetStalker();
	if (!Stalker)
	{
		return;
	}

	// 1. 도주 상태 업데이트
	InstanceData.bIsRetreating = Stalker->IsRetreating();

	// 2. KSJ: Hysteresis 적용된 플레이어 시선 감지
	// Controller의 안정적인 LookingPlayer 사용 (빈번한 타겟 변경 방지)
	StalkerCtrl->UpdateLookingPlayerWithHysteresis(DeltaTime);
	
	InstanceData.bIsPlayerLookingAtMe = StalkerCtrl->IsAnyPlayerLookingAtMe();
	InstanceData.LookingPlayer = StalkerCtrl->GetStableLookingPlayer();

	// 3. 위치 계산
	
	// 도주 위치는 도주 중일 때만 계산
	if (InstanceData.bIsRetreating)
	{
		InstanceData.RetreatLocation = StalkerCtrl->FindRetreatLocation();
	}
	else
	{
		InstanceData.RetreatLocation = Stalker->GetActorLocation();
	}

	// KSJ: 다중 플레이어 시야를 고려한 엄폐 위치 계산
	// 엄폐 위치는 플레이어가 보고 있거나 전투 중일 때 계산
	if (InstanceData.bIsPlayerLookingAtMe || InstanceData.bIsChasing)
	{
		// 나를 보고 있는 모든 플레이어로부터 숨는 위치 찾기
		TArray<AActor*> LookingPlayers = StalkerCtrl->GetAllLookingPlayers();
		
		if (LookingPlayers.Num() > 0)
		{
			InstanceData.HideLocation = StalkerCtrl->FindHideLocationFromMultiple(1500.f, LookingPlayers);
		}
		else
		{
			// 아무도 안 보고 있으면 ChaseTarget 기준
			AActor* HideTarget = InstanceData.CurrentTarget.Get();
			InstanceData.HideLocation = StalkerCtrl->FindHideLocation(1500.f, HideTarget);
		}
	}
	else
	{
		InstanceData.HideLocation = Stalker->GetActorLocation();
	}
}

