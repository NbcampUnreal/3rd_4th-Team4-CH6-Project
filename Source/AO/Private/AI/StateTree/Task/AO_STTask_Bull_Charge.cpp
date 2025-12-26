// AO_STTask_Bull_Charge.cpp

#include "AI/StateTree/Task/AO_STTask_Bull_Charge.h"
#include "AI/Controller/AO_BullController.h"
#include "AI/Character/AO_Bull.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"

EStateTreeRunStatus FAO_STTask_Bull_Charge::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Bull_Charge_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Bull_Charge_InstanceData>(*this);
	
	AAO_BullController* Controller = GetController(Context);
	if (!Controller) 
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Bull_Charge: Controller is null"));
		return EStateTreeRunStatus::Failed;
	}

	AAO_Bull* Bull = Controller->GetBull();
	if (!Bull) 
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Bull_Charge: Bull is null"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Controller = Controller;

	// 플레이어 타겟 가져오기
	AAO_PlayerCharacter* TargetPlayer = Controller->GetNearestPlayerInSight();
	if (!TargetPlayer)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Bull_Charge: No player in sight"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.TargetPlayer = TargetPlayer;

	// 플레이어 방향으로 회전 설정
	Controller->SetFocus(TargetPlayer);
	
	// 돌진 Ability 실행
	UAbilitySystemComponent* ASC = Bull->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTag ChargeTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Charge"));
		if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ChargeTag)))
		{
			InstanceData.bIsCharging = true;
			AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Bull_Charge: Charge ability activated, target: %s"), *TargetPlayer->GetName());
			return EStateTreeRunStatus::Running;
		}
		else
		{
			AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Bull_Charge: Failed to activate charge ability"));
		}
	}
	else
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Bull_Charge: ASC is null"));
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FAO_STTask_Bull_Charge::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Bull_Charge_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Bull_Charge_InstanceData>(*this);
	
	if (!InstanceData.Controller || !InstanceData.Controller->GetBull())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Bull* Bull = InstanceData.Controller->GetBull();
	
	// Bull의 상태를 체크하여 돌진이 끝났는지 확인
	// Ability가 끝나면 SetIsCharging(false)가 호출됨
	if (!Bull->IsCharging())
	{
		// Focus 해제
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
		return EStateTreeRunStatus::Succeeded;
	}

	// 돌진 중: 플레이어 방향으로 계속 이동
	if (InstanceData.TargetPlayer && IsValid(InstanceData.TargetPlayer))
	{
		// 플레이어 방향으로 회전 유지
		InstanceData.Controller->SetFocus(InstanceData.TargetPlayer);
		
		// 플레이어 방향으로 이동 (CharacterMovement가 자동으로 처리하도록)
		FVector ToPlayer = InstanceData.TargetPlayer->GetActorLocation() - Bull->GetActorLocation();
		ToPlayer.Z = 0.f; // 수평 이동만
		ToPlayer.Normalize();
		
		// Movement Component를 통해 이동
		UCharacterMovementComponent* MoveComp = Bull->GetCharacterMovement();
		if (MoveComp && MoveComp->MovementMode == MOVE_Walking)
		{
			// 플레이어 방향으로 이동 입력
			Bull->AddMovementInput(ToPlayer, 1.0f);
		}
	}
	else
	{
		// 타겟이 사라지면 실패
		AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Bull_Charge: Target player is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Bull_Charge::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Bull_Charge_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Bull_Charge_InstanceData>(*this);
	
	// Focus 해제
	if (InstanceData.Controller)
	{
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

AAO_BullController* FAO_STTask_Bull_Charge::GetController(FStateTreeExecutionContext& Context) const
{
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (AAO_BullController* Ctrl = Cast<AAO_BullController>(Owner))
	{
		return Ctrl;
	}
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		return Cast<AAO_BullController>(Pawn->GetController());
	}
	return nullptr;
}

