// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/AO_STTask_Wolf_Surround.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "AO_Log.h"

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

	// 포위 모드가 끝났으면 (공격 시작) 성공 처리 -> Attack Task로 전이
	if (!InstanceData.PackComp->IsSurrounding())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.QueryTimer += DeltaTime;
	if (InstanceData.QueryTimer >= InstanceData.QueryInterval)
	{
		InstanceData.QueryTimer = 0.f;

		// EQS 실행
		if (InstanceData.SurroundQuery)
		{
			FEnvQueryRequest Request(InstanceData.SurroundQuery, InstanceData.Controller->GetPawn());
			
			// FQueryFinishedSignature::CreateWeakLambda를 사용하여 안전하게 델리게이트 바인딩
			Request.Execute(EEnvQueryRunMode::SingleResult, 
				FQueryFinishedSignature::CreateWeakLambda(InstanceData.Controller, 
				[Controller = InstanceData.Controller](TSharedPtr<FEnvQueryResult> Result)
				{
					if (Result.IsValid() && Result->IsSuccessful() && Controller)
					{
						FVector TargetLoc = Result->GetItemAsLocation(0);
						Controller->MoveToLocation(TargetLoc);
					}
				}));
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
