// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/AO_WerewolfController.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "Character/AO_PlayerCharacter.h"
#include "AO_Log.h"

AAO_WerewolfController::AAO_WerewolfController()
{
	// Werewolf는 좀 더 예민할 수 있음
	// AISenseConfig 설정은 BP에서 하거나 여기서 추가 설정 가능
}

void AAO_WerewolfController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(InPawn))
	{
		PackComp = Wolf->GetPackCoordComp();
		if (PackComp)
		{
			// Howl 수신 이벤트 바인딩
			PackComp->OnHowlReceived.AddDynamic(this, &AAO_WerewolfController::HandleHowlReceived);
		}
	}
}

void AAO_WerewolfController::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// 부모의 StartChase를 바로 호출하지 않고, Howl 로직을 먼저 탄다.
	
	if (!bHasHowledOrJoined)
	{
		// 처음 발견!
		bHasHowledOrJoined = true;

		// 추격 대상 설정 (StateTree에서 접근 가능하도록)
		SetChaseTarget(Player);
		
		// 여기서 StateTree의 상태가 'Idle/Roam' -> 'Howl'로 바뀌도록 해야 함.
		// StateTree는 보통 Event나 변수 변경을 통해 반응함.
		// 'HasTag' 조건 등을 활용하기 위해 AttributeSet이나 Tag를 변경하거나,
		// StateTree Event를 보낼 수 있음.
		
		// 여기서는 간단히 로그를 찍고, 실제 동작은 StateTree Condition(PlayerInSight)에 의해
		// Transition이 일어날 때, Task에서 Howl을 수행하도록 유도.
		// 다만 일반 AggressiveAI는 바로 Chase로 가기 때문에, Werewolf용 StateTree는
		// Chase 전에 Howl 검사를 먼저 하는 구조여야 함.
		
		AO_LOG(LogKSJ, Log, TEXT("[WerewolfCtrl] Player Detected! Initiating Howl sequence."));
	}
	else
	{
		// 이미 전투 중이면 그냥 추격 정보 갱신
		Super::OnPlayerDetected(Player, Location);
	}
}

void AAO_WerewolfController::HandleHowlReceived(AActor* TargetActor)
{
	AAO_PlayerCharacter* TargetPlayer = Cast<AAO_PlayerCharacter>(TargetActor);
	if (!TargetPlayer) return;

	AO_LOG(LogKSJ, Log, TEXT("[WerewolfCtrl] Heard Howl! Joining the pack."));

	bHasHowledOrJoined = true;
	SetChaseTarget(TargetPlayer);

	// Howl을 들었을 때의 로직 (StateTree에서 'HowlReceived' Condition이 true가 되어 Surround로 진입)
}
