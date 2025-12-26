// AO_InsectController.cpp

#include "AI/Controller/AO_InsectController.h"
#include "AI/Character/AO_Insect.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "BehaviorTree/BlackboardComponent.h" // StateTree 사용 시 불필요할 수 있으나 부모 호환성 유지
#include "Perception/AIPerceptionTypes.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Perception/AISense_Sight.h"

AAO_InsectController::AAO_InsectController()
{
	// Perception 설정 상속
}

void AAO_InsectController::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_InsectController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AAO_InsectController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 플레이어인지 확인
	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor);
	
	// 플레이어이고 시야 감지인 경우, 쿨타운 체크
	if (Player && Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 쿨타운 중인 플레이어는 시야 목록에 추가하지 않음 (아예 무시)
			if (UWorld* World = GetWorld())
			{
				if (UAO_AISubsystem* AISubsystem = World->GetSubsystem<UAO_AISubsystem>())
				{
					if (AISubsystem->IsPlayerRecentlyKidnapped(Player))
					{
						AO_LOG(LogKSJ, Log, TEXT("AO_InsectController: Ignoring player %s (on cooldown)"), *Player->GetName());
						return; // 쿨타운 중인 플레이어는 무시
					}
					
					// 이미 다른 Insect가 납치 중인 플레이어도 무시
					if (AISubsystem->IsPlayerBeingKidnapped(Player))
					{
						AO_LOG(LogKSJ, Log, TEXT("AO_InsectController: Ignoring player %s (already being kidnapped)"), *Player->GetName());
						return; // 이미 납치 중인 플레이어는 무시
					}
				}
			}
		}
	}
	
	// 쿨타운이 아닌 플레이어만 부모 함수 호출
	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
}

void AAO_InsectController::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	Super::OnPlayerDetected(Player, Location);

	// Insect 특화 로직:
	// 납치 이동 중 새로운 플레이어를 발견하면 경로를 재탐색해야 함.
	// 이는 StateTree Task (AO_STTask_Insect_Drag)에서 
	// Controller->GetPlayersInSight()를 주기적으로 체크하여 수행하므로
	// 여기서는 별도 이벤트 처리를 하지 않아도 됨.
}

