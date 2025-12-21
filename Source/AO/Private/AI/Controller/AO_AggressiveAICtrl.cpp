// AO_AggressiveAICtrl.cpp

#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "AO_Log.h"

AAO_AggressiveAICtrl::AAO_AggressiveAICtrl()
{
	// 필수: RVO Avoidance를 위한 CrowdFollowingComponent 사용
	// PathFollowingComponent를 CrowdFollowingComponent로 대체
	
	// 선공형 AI들을 위한 기본 Perception 설정값 강제 초기화
	// 이 값이 없으면 BP 생성 시 0으로 초기화되어 감지가 작동하지 않을 수 있음
	SightRadius = 1500.f;
	LoseSightRadius = 2000.f;
	HearingRange = 2000.f;
	PeripheralVisionAngleDegrees = 120.f; // 공격성을 위해 시야각을 넓게 설정
}

void AAO_AggressiveAICtrl::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_AggressiveAICtrl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// RVO Avoidance 설정 - 서로의 경로를 막지 않도록
	if (UCrowdFollowingComponent* CrowdComp = FindComponentByClass<UCrowdFollowingComponent>())
	{
		CrowdComp->SetCrowdSimulationState(ECrowdSimulationState::Enabled);
	}
}

AAO_AggressiveAIBase* AAO_AggressiveAICtrl::GetAggressiveAI() const
{
	return Cast<AAO_AggressiveAIBase>(GetPawn());
}

AAO_PlayerCharacter* AAO_AggressiveAICtrl::GetChaseTarget() const
{
	return ChaseTarget.Get();
}

void AAO_AggressiveAICtrl::SetChaseTarget(AAO_PlayerCharacter* NewTarget)
{
	ChaseTarget = NewTarget;

	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (AI)
	{
		AI->SetCurrentTarget(NewTarget);
	}
}

void AAO_AggressiveAICtrl::StartChase(AAO_PlayerCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	// 수색 타이머 정리
	if (SearchTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
	}

	SetChaseTarget(Target);

	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (AI)
	{
		AI->SetChaseMode(true);
	}

	AO_LOG(LogKSJ, Log, TEXT("%s: Started chasing %s"), *GetName(), *Target->GetName());
}

void AAO_AggressiveAICtrl::UpdateChaseTargetToNearest()
{
	// 대상 고정 모드인 경우 갱신하지 않음
	if (bLockOnFirstTarget && ChaseTarget.IsValid())
	{
		return;
	}

	AAO_PlayerCharacter* NearestPlayer = GetNearestPlayerInSight();
	if (NearestPlayer && NearestPlayer != ChaseTarget.Get())
	{
		SetChaseTarget(NearestPlayer);
		AO_LOG(LogKSJ, Log, TEXT("%s: Chase target updated to nearest: %s"), *GetName(), *NearestPlayer->GetName());
	}
}

void AAO_AggressiveAICtrl::StartSearch()
{
	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (!AI)
	{
		return;
	}

	AI->SetSearchMode(true);

	// 수색 시간 설정 (5~10초 랜덤)
	const float SearchDuration = FMath::RandRange(5.f, 10.f);

	GetWorldTimerManager().SetTimer(
		SearchTimerHandle,
		this,
		&AAO_AggressiveAICtrl::OnSearchTimerExpired,
		SearchDuration,
		false
	);

	AO_LOG(LogKSJ, Log, TEXT("%s: Started searching for %.1f seconds"), *GetName(), SearchDuration);
}

void AAO_AggressiveAICtrl::EndSearch()
{
	if (SearchTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
	}

	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (AI)
	{
		AI->SetSearchMode(false);
	}

	// 추격 대상 초기화
	SetChaseTarget(nullptr);
	LastKnownTargetLocation = FVector::ZeroVector;

	AO_LOG(LogKSJ, Log, TEXT("%s: Search ended, returning to roam"), *GetName());
}

void AAO_AggressiveAICtrl::OnSearchTimerExpired()
{
	// 수색 중 플레이어를 다시 발견하지 못했으면 배회로 전환
	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (AI && AI->IsInSearchMode())
	{
		EndSearch();
	}
}

void AAO_AggressiveAICtrl::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	Super::OnPlayerDetected(Player, Location);

	if (!Player)
	{
		return;
	}

	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (!AI)
	{
		return;
	}

	// 수색 중이면 수색 종료하고 추격으로 전환
	if (AI->IsInSearchMode())
	{
		if (SearchTimerHandle.IsValid())
		{
			GetWorldTimerManager().ClearTimer(SearchTimerHandle);
		}
		AI->SetSearchMode(false);
	}

	// 추격 중이 아니면 추격 시작
	if (!AI->IsInChaseMode())
	{
		StartChase(Player);
	}
	else
	{
		// 이미 추격 중이면 더 가까운 플레이어로 갱신
		UpdateChaseTargetToNearest();
	}

	// 위치 기록
	LastKnownTargetLocation = Location;
}

void AAO_AggressiveAICtrl::OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation)
{
	Super::OnPlayerLost(Player, LastKnownLocation);

	if (!Player)
	{
		return;
	}

	// 현재 추격 대상이 시야에서 사라진 경우
	if (ChaseTarget.Get() == Player)
	{
		LastKnownTargetLocation = LastKnownLocation;

		// 다른 플레이어가 시야에 있는지 확인
		if (HasPlayerInSight())
		{
			// 다른 플레이어를 추격
			UpdateChaseTargetToNearest();
		}
		else
		{
			// 시야에 플레이어가 없으면 수색 모드로 전환
			AAO_AggressiveAIBase* AI = GetAggressiveAI();
			if (AI)
			{
				AI->SetChaseMode(false);
				StartSearch();
			}
		}
	}
}
