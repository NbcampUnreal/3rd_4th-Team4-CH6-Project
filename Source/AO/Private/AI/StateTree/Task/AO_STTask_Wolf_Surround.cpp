//KSJ : AO_STTask_Wolf_Surround


#include "AI/StateTree/Task/AO_STTask_Wolf_Surround.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Controller/AO_WerewolfController.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "NavigationSystem.h"

EStateTreeRunStatus FAO_STTask_Wolf_Surround::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;
	AAO_AggressiveAICtrl* Ctrl = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
		Ctrl = Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Controller);
		Wolf = Cast<AAO_Werewolf>(Controller->GetPawn());
	}

	InstanceData.Controller = Ctrl;
	
	if (Wolf)
	{
		InstanceData.PackComp = Wolf->GetPackCoordComp();
	}

	if (!InstanceData.Controller || !InstanceData.PackComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 즉시 쿼리 실행 트리거
	InstanceData.QueryTimer = InstanceData.QueryInterval; 

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Wolf_Surround::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.PackComp || !InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(InstanceData.Controller->GetPawn());
	if (!Wolf)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 1. 일제공격 시작 여부 체크 (최우선)
	if (InstanceData.PackComp->IsCoordinatedAttackStarted())
	{
		return EStateTreeRunStatus::Succeeded; // Attack Task로 전환
	}

	// 2. 포위 모드가 끝났으면 성공 처리 -> Attack Task로 전이
	if (!InstanceData.PackComp->IsSurrounding())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 3. 포위 위치 도달 여부 체크
	FVector CurrentLocation = Wolf->GetActorLocation();
	InstanceData.PackComp->CheckSurroundPositionReached(CurrentLocation, 150.f);

	InstanceData.QueryTimer += DeltaTime;
	if (InstanceData.QueryTimer >= InstanceData.QueryInterval)
	{
		InstanceData.QueryTimer = 0.f;

		FVector TargetLocation = FVector::ZeroVector;
		bool bFoundLocation = false;

		// 도주로 할당 시도 (각 Werewolf가 다른 도주로를 차단하도록)
		AAO_WerewolfController* WolfController = Cast<AAO_WerewolfController>(InstanceData.Controller);
		int32 RouteIndex = InstanceData.PackComp->AssignEscapeRouteToWolf(Target, Wolf, 1000.f);
		
		FVector FinalLocation = FVector::ZeroVector;
		
		if (RouteIndex >= 0)
		{
			// 할당된 도주로 차단 위치 사용
			FinalLocation = InstanceData.PackComp->GetAssignedEscapeRouteBlockPosition(Wolf, Target, 500.f);
		}

		// EQS 사용 또는 직접 계산
		if (InstanceData.SurroundQuery)
		{
			// EQS 실행
			FEnvQueryRequest Request(InstanceData.SurroundQuery, InstanceData.Controller->GetPawn());
			
			Request.Execute(EEnvQueryRunMode::SingleResult, 
				FQueryFinishedSignature::CreateWeakLambda(InstanceData.Controller, 
				[Controller = InstanceData.Controller, PackComp = InstanceData.PackComp, Wolf, Target, RouteIndex, FinalLocation](TSharedPtr<FEnvQueryResult> Result)
				{
					if (Result.IsValid() && Result->IsSuccessful() && Controller && PackComp && Wolf && Target)
					{
						FVector EQSLocation = Result->GetItemAsLocation(0);
						FVector UseLocation = FVector::ZeroVector;

						// 도주로 차단 위치가 있으면 우선 사용, 없으면 EQS 결과 사용
						if (!FinalLocation.IsZero())
						{
							UseLocation = FinalLocation;
						}
						else
						{
							UseLocation = EQSLocation;
						}

						// 포위 위치 예약 시도
						if (PackComp->TryReserveSurroundPosition(UseLocation, Wolf))
						{
							PackComp->SetTargetSurroundPosition(UseLocation);
							Controller->MoveToLocation(UseLocation);
						}
						else
						{
							// 예약 실패 시 다른 위치 찾기
							FVector AvailablePos = PackComp->FindAvailableSurroundPosition(Target, 500.f, Wolf);
							if (!AvailablePos.IsZero())
							{
								if (PackComp->TryReserveSurroundPosition(AvailablePos, Wolf))
								{
									PackComp->SetTargetSurroundPosition(AvailablePos);
									Controller->MoveToLocation(AvailablePos);
								}
							}
						}
					}
				}));
		}
		else
		{
			// EQS가 없으면 도주로 차단 위치 또는 일반 포위 위치 사용
			if (WolfController)
			{
				if (!FinalLocation.IsZero())
				{
					// 도주로 차단 위치 사용
					if (InstanceData.PackComp->TryReserveSurroundPosition(FinalLocation, Wolf))
					{
						InstanceData.PackComp->SetTargetSurroundPosition(FinalLocation);
						InstanceData.Controller->MoveToLocation(FinalLocation);
						bFoundLocation = true;
					}
				}

				// 차단 위치 예약 실패 시 일반 포위 위치 찾기
				if (!bFoundLocation)
				{
					FVector SurroundPos = InstanceData.PackComp->FindAvailableSurroundPosition(Target, 500.f, Wolf);
					if (!SurroundPos.IsZero())
					{
						if (InstanceData.PackComp->TryReserveSurroundPosition(SurroundPos, Wolf))
						{
							InstanceData.PackComp->SetTargetSurroundPosition(SurroundPos);
							InstanceData.Controller->MoveToLocation(SurroundPos);
						}
					}
				}
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Wolf_Surround::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
	}
}
