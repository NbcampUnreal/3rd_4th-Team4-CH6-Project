//KSJ : AO_STEval_StalkerContext

#include "AI/StateTree/Evaluator/AO_STEval_StalkerContext.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"

void FAO_STEval_StalkerContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_StalkerCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_StalkerCtx_InstanceData>(*this);
	UpdateStalkerContextData(Context, InstanceData);
}

void FAO_STEval_StalkerContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_StalkerCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_StalkerCtx_InstanceData>(*this);
	UpdateStalkerContextData(Context, InstanceData);
}

void FAO_STEval_StalkerContext::UpdateStalkerContextData(FStateTreeExecutionContext& Context, FAO_STEval_StalkerCtx_InstanceData& InstanceData) const
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
	// KSJ: Retreat 상태는 Actor(AAO_Stalker)가 단일 소스로 소유한다.
	InstanceData.bIsRetreating = Stalker->IsRetreating();

	// 2. 플레이어 시선 감지
	AActor* Target = InstanceData.CurrentTarget.Get();
	if (Target)
	{
		InstanceData.bIsPlayerLookingAtMe = StalkerCtrl->IsPlayerLookingAtMe(Target);
	}
	else
	{
		InstanceData.bIsPlayerLookingAtMe = false;
	}

	// 3. 위치 계산 (매 틱 계산은 무거울 수 있으므로 필요한 상태일 때만 계산하거나, Controller 내부 캐싱 권장)
	// 여기서는 로직 연결을 위해 직접 호출합니다.
	
	// 도주 위치는 도주 중일 때만 계산
	if (InstanceData.bIsRetreating)
	{
		InstanceData.RetreatLocation = StalkerCtrl->FindRetreatLocation();
	}
	else
	{
		InstanceData.RetreatLocation = Stalker->GetActorLocation();
	}

	// 엄폐 위치는 플레이어가 보고 있거나 전투 중일 때 계산
	if (InstanceData.bIsPlayerLookingAtMe || InstanceData.bIsChasing)
	{
		InstanceData.HideLocation = StalkerCtrl->FindHideLocation(1500.f, Target);
	}
	else
	{
		// 평소에는 현재 위치나 랜덤 배회 위치가 될 것임 (여기선 현재 위치로 초기화)
		InstanceData.HideLocation = Stalker->GetActorLocation();
	}
}


