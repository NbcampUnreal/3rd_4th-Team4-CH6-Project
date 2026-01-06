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
	InstanceData.bIsRetreating = Stalker->IsRetreating();

	// 2. 플레이어 시선 감지 (모든 플레이어 대상)
	// 나를 보고 있는 플레이어 중 가장 가까운 플레이어를 찾음
	bool bAnyPlayerLooking = false;
	AActor* ClosestLookingPlayer = nullptr;
	float ClosestDistance = FLT_MAX;
	
	if (UWorld* World = Stalker->GetWorld())
	{
		TArray<AActor*> AllPlayers;
		UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), AllPlayers);

		for (AActor* PlayerActor : AllPlayers)
		{
			if (StalkerCtrl->IsPlayerLookingAtMe(PlayerActor))
			{
				bAnyPlayerLooking = true;
				
				// 가장 가까운 플레이어 찾기
				const float Distance = FVector::Dist(Stalker->GetActorLocation(), PlayerActor->GetActorLocation());
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					ClosestLookingPlayer = PlayerActor;
				}
			}
		}
	}
	
	InstanceData.bIsPlayerLookingAtMe = bAnyPlayerLooking;
	InstanceData.LookingPlayer = ClosestLookingPlayer;

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

	// 엄폐 위치는 플레이어가 보고 있거나 전투 중일 때 계산
	// 기준은 현재 추격 타겟 (타겟이 없으면 가장 가까운 플레이어나, 마지막 타겟 등 Controller 로직 따름)
	AActor* HideTarget = InstanceData.CurrentTarget.Get();
	
	if (InstanceData.bIsPlayerLookingAtMe || InstanceData.bIsChasing)
	{
		InstanceData.HideLocation = StalkerCtrl->FindHideLocation(1500.f, HideTarget);
	}
	else
	{
		InstanceData.HideLocation = Stalker->GetActorLocation();
	}
}
