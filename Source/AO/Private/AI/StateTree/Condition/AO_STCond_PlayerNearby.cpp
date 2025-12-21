// AO_STCond_PlayerNearby.cpp

#include "AI/StateTree/Condition/AO_STCond_PlayerNearby.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_PlayerNearby::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_PlayerNearby_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_PlayerNearby_InstanceData>(*this);

	// AIController가 유효해야 플레이어 감지 확인 가능
	AAO_AIControllerBase* AIController = GetAIController(Context);
	if (!ensureMsgf(AIController, TEXT("STCond_PlayerNearby: AIController is null")))
	{
		return InstanceData.bInvert;
	}

	bool bPlayerNearby = false;

	// AggressiveAICtrl인 경우 ChaseTarget이 설정되어 있으면 플레이어가 근처에 있는 것으로 간주
	// (OnPlayerDetected에서 StartChase가 호출되어 ChaseTarget이 설정됨)
	if (AAO_AggressiveAICtrl* AggressiveCtrl = Cast<AAO_AggressiveAICtrl>(AIController))
	{
		if (AggressiveCtrl->GetChaseTarget())
		{
			bPlayerNearby = true;
		}
	}

	// 시야 내 플레이어 확인
	if (!bPlayerNearby && AIController->HasPlayerInSight())
	{
		bPlayerNearby = true;
	}
	else if (!bPlayerNearby && !InstanceData.bOnlySightCheck)
	{
		// 소리로 감지된 위치 확인 - 시야 외에도 청각으로 플레이어 감지
		AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(AIController->GetPawn());
		if (AIChar)
		{
			UAO_AIMemoryComponent* MemComp = AIChar->GetMemoryComponent();
			if (MemComp)
			{
				FVector HeardLocation = MemComp->GetLastHeardLocation();
				if (!HeardLocation.IsZero())
				{
					float Distance = FVector::Dist(AIChar->GetActorLocation(), HeardLocation);
					if (Distance < InstanceData.DetectionRadius)
					{
						bPlayerNearby = true;
					}
				}
			}
		}
	}

	return InstanceData.bInvert ? !bPlayerNearby : bPlayerNearby;
}

AAO_AIControllerBase* FAO_STCond_PlayerNearby::GetAIController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_AIControllerBase* Controller = Cast<AAO_AIControllerBase>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_AIControllerBase>(Pawn->GetController());
		}
	}
	return nullptr;
}
