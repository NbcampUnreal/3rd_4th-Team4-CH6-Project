// AO_STEval_CrabContext.cpp

#include "AI/StateTree/Evaluator/AO_STEval_CrabContext.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Controller/AO_CrabController.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Item/AO_MasterItem.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"

void FAO_STEval_CrabContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_CrabContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_CrabContext_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

void FAO_STEval_CrabContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_CrabContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_CrabContext_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

AAO_Crab* FAO_STEval_CrabContext::GetCrab(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_Crab* Crab = Cast<AAO_Crab>(Owner))
		{
			return Crab;
		}
		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Cast<AAO_Crab>(Controller->GetPawn());
		}
	}
	return nullptr;
}

AAO_CrabController* FAO_STEval_CrabContext::GetCrabController(FStateTreeExecutionContext& Context) const
{
	if (AAO_Crab* Crab = GetCrab(Context))
	{
		return Cast<AAO_CrabController>(Crab->GetController());
	}
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		return Cast<AAO_CrabController>(Owner);
	}
	return nullptr;
}

void FAO_STEval_CrabContext::UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_CrabContext_InstanceData& InstanceData) const
{
	// Crab과 Controller가 유효해야 컨텍스트 업데이트 가능
	AAO_Crab* Crab = GetCrab(Context);
	AAO_CrabController* Controller = GetCrabController(Context);

	if (!ensureMsgf(Crab, TEXT("CrabContext: Crab is null")) || 
		!ensureMsgf(Controller, TEXT("CrabContext: Controller is null")))
	{
		return;
	}

	// 아이템 소지 여부
	InstanceData.bIsCarryingItem = Crab->IsCarryingItem();

	// 시야 내 플레이어 확인
	InstanceData.bHasPlayerInSight = Controller->HasPlayerInSight();

	// 기절 상태 확인
	InstanceData.bIsStunned = false;
	if (UAbilitySystemComponent* ASC = Crab->GetAbilitySystemComponent())
	{
		FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned"));
		InstanceData.bIsStunned = ASC->HasMatchingGameplayTag(StunnedTag);
	}

	// 가장 가까운 아이템 거리
	InstanceData.NearestItemDistance = MAX_FLT;
	if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
	{
		if (AAO_MasterItem* NearestItem = CarryComp->FindNearbyItem(ItemSearchRadius))
		{
			InstanceData.NearestItemDistance = FVector::Dist(Crab->GetActorLocation(), NearestItem->GetActorLocation());
		}
	}

	// 위협 감지 (시야 또는 소리)
	InstanceData.bThreatDetected = InstanceData.bHasPlayerInSight;
	InstanceData.NearestThreatDistance = MAX_FLT;
	InstanceData.NearestThreatLocation = FVector::ZeroVector;

	// 시야 내 플레이어 위치
	if (InstanceData.bHasPlayerInSight)
	{
		if (AActor* NearestPlayer = Controller->GetNearestPlayerInSight())
		{
			InstanceData.NearestThreatLocation = NearestPlayer->GetActorLocation();
			InstanceData.NearestThreatDistance = FVector::Dist(Crab->GetActorLocation(), InstanceData.NearestThreatLocation);
		}
	}

	// 소리로 감지된 위치 확인
	if (UAO_AIMemoryComponent* MemComp = Crab->GetMemoryComponent())
	{
		FVector HeardLocation = MemComp->GetLastHeardLocation();
		if (!HeardLocation.IsZero())
		{
			float HeardDistance = FVector::Dist(Crab->GetActorLocation(), HeardLocation);
			if (HeardDistance < ThreatDetectionRadius)
			{
				InstanceData.bThreatDetected = true;

				// 더 가까운 위협이면 업데이트
				if (HeardDistance < InstanceData.NearestThreatDistance)
				{
					InstanceData.NearestThreatDistance = HeardDistance;
					InstanceData.NearestThreatLocation = HeardLocation;
				}
			}
		}
	}
}
