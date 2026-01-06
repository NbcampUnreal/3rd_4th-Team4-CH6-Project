//KSJ : AO_AIControllerBase

#include "AI/Controller/AO_AIControllerBase.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"

AAO_AIControllerBase::AAO_AIControllerBase()
{
	// Perception 컴포넌트 생성
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// State Tree 컴포넌트 생성
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
}

void AAO_AIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러는 서버에서만 실행되어야 함 (멀티플레이어 동기화)
	if (!HasAuthority())
	{
		return;
	}

	SetupPerceptionSystem();

	// Perception 이벤트 바인딩
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAO_AIControllerBase::OnTargetPerceptionUpdated);
	}
}

void AAO_AIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// AI 컨트롤러는 서버에서만 실행되어야 함 (멀티플레이어 동기화)
	if (!HasAuthority())
	{
		return;
	}

	// State Tree 설정 및 시작
	if (StateTreeComponent && DefaultStateTree)
	{
		StateTreeComponent->SetStateTree(DefaultStateTree);
		StateTreeComponent->StartLogic();
	}
}

void AAO_AIControllerBase::OnUnPossess()
{
	// AI 컨트롤러는 서버에서만 실행되어야 함 (멀티플레이어 동기화)
	if (HasAuthority())
	{
		// State Tree 정지
		if (StateTreeComponent)
		{
			StateTreeComponent->StopLogic(TEXT("UnPossessed"));
		}
	}

	Super::OnUnPossess();
}

void AAO_AIControllerBase::SetupPerceptionSystem()
{
	if (!AIPerceptionComponent)
	{
		return;
	}

	// Sight Config 설정
	SightConfig = NewObject<UAISenseConfig_Sight>(this, UAISenseConfig_Sight::StaticClass());
	if (SightConfig)
	{
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
		SightConfig->SetMaxAge(5.f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
		
		// 모든 대상 감지 (팀 관계 무시하고 일단 모두 감지)
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}

	// Hearing Config 설정
	HearingConfig = NewObject<UAISenseConfig_Hearing>(this, UAISenseConfig_Hearing::StaticClass());
	if (HearingConfig)
	{
		HearingConfig->HearingRange = HearingRange;
		HearingConfig->SetMaxAge(3.f);
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}

	AIPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	AIPerceptionComponent->RequestStimuliListenerUpdate();
}

void AAO_AIControllerBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Perception 이벤트에서 Actor가 null일 수 있음 (정상적인 상황)
	if (!Actor)
	{
		return;
	}

	// 플레이어인지 확인
	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor);

	// 시야 감지
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Player)
		{
			// 생존 여부 확인
			const AAO_PlayerState* PS = Player->GetPlayerState<AAO_PlayerState>();
			const bool bIsAlive = PS && PS->GetIsAlive();
        
			// [조건 체크] 죽었고 && 시체를 타겟팅하면 안 되는 경우 -> 무시
			if (!bIsAlive && !bCanTargetDeadPlayer)
			{
				// 시야 목록에 있었다면 제거 (죽는 순간 사라진 것으로 처리)
				if (PlayersInSight.Contains(Player))
				{
					PlayersInSight.Remove(Player);
					OnPlayerLost(Player, Stimulus.StimulusLocation);
				}
				return; 
			}

			if (Stimulus.WasSuccessfullySensed())
			{
				// 플레이어 발견
				if (!PlayersInSight.Contains(Player))
				{
					PlayersInSight.Add(Player);
				}
				OnPlayerDetected(Player, Stimulus.StimulusLocation);
			}
			else
			{
				// 플레이어 시야에서 사라짐
				PlayersInSight.Remove(Player);
				OnPlayerLost(Player, Stimulus.StimulusLocation);
			}
		}
		else
		{
			// 플레이어가 아닌 다른 액터 감지 (자식 클래스에서 처리)
			if (Stimulus.WasSuccessfullySensed())
			{
				OnActorDetected(Actor, Stimulus.StimulusLocation);
			}
			else
			{
				OnActorLost(Actor);
			}
		}
	}
	// 청각 감지
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnNoiseHeard(Actor, Stimulus.StimulusLocation, Stimulus.Strength);
		}
	}
}

void AAO_AIControllerBase::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// Player가 유효해야 감지 처리 가능
	if (!ensureMsgf(Player, TEXT("OnPlayerDetected called with null Player")))
	{
		return;
	}

	// Memory에 플레이어 위치 기록 - AI 캐릭터가 위치를 기억하도록 함
	AAO_AICharacterBase* AICharacter = Cast<AAO_AICharacterBase>(GetPawn());
	if (AICharacter)
	{
		UAO_AIMemoryComponent* Memory = AICharacter->GetMemoryComponent();
		if (Memory)
		{
			Memory->UpdatePlayerLocation(Player, Location);
		}
	}
}

void AAO_AIControllerBase::OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation)
{
	// Player가 유효해야 시야 이탈 처리 가능
	if (!ensureMsgf(Player, TEXT("OnPlayerLost called with null Player")))
	{
		return;
	}

	// Memory에 마지막 목격 위치 기록 - 추적/회피에 활용
	AAO_AICharacterBase* AICharacter = Cast<AAO_AICharacterBase>(GetPawn());
	if (AICharacter)
	{
		UAO_AIMemoryComponent* Memory = AICharacter->GetMemoryComponent();
		if (Memory)
		{
			Memory->SetLastKnownLocation(Player, LastKnownLocation);
		}
	}
}

void AAO_AIControllerBase::OnNoiseHeard(AActor* NoiseInstigator, const FVector& Location, float Volume)
{
	// KSJ:
	// 기본 구현: "소리로 감지된 위치"를 메모리에 저장한다.
	// - PlayerNearby Condition이 LastHeardLocation을 통해 상태 전이를 결정할 수 있다.
	// - 멀티플레이 리슨서버 기준으로 AI 로직은 서버에서만 수행되므로, 서버에서만 메모리를 갱신한다.
	//
	// 각 AI의 특수 반응(도망, 추격, 납치 등)은 자식 Controller/StateTree에서 처리한다.

	if (!HasAuthority())
	{
		return;
	}

	AAO_AICharacterBase* AICharacter = Cast<AAO_AICharacterBase>(GetPawn());
	if (!AICharacter)
	{
		return;
	}

	UAO_AIMemoryComponent* Memory = AICharacter->GetMemoryComponent();
	if (!Memory)
	{
		return;
	}

	Memory->SetLastHeardLocation(Location);

	// 소리 발생자가 플레이어라면 해당 플레이어의 마지막 위치도 갱신한다.
	if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(NoiseInstigator))
	{
		Memory->SetLastKnownLocation(Player, Location);
	}
}

void AAO_AIControllerBase::OnActorDetected(AActor* Actor, const FVector& Location)
{
	// 기본 구현: 자식 클래스에서 필요 시 오버라이드 (예: Crab의 아이템 감지)
}

void AAO_AIControllerBase::OnActorLost(AActor* Actor)
{
	// 기본 구현: 자식 클래스에서 필요 시 오버라이드
}

TArray<AAO_PlayerCharacter*> AAO_AIControllerBase::GetPlayersInSight() const
{
	TArray<AAO_PlayerCharacter*> ValidPlayers;
	for (const TWeakObjectPtr<AAO_PlayerCharacter>& WeakPlayer : PlayersInSight)
	{
		if (WeakPlayer.IsValid())
		{
			// 죽은 플레이어 필터링
			const AAO_PlayerCharacter* Player = WeakPlayer.Get();
			const AAO_PlayerState* PS = Player->GetPlayerState<AAO_PlayerState>();
			if (PS && !PS->GetIsAlive() && !bCanTargetDeadPlayer)
			{
				continue;
			}
			
			ValidPlayers.Add(WeakPlayer.Get());
		}
	}
	return ValidPlayers;
}

AAO_PlayerCharacter* AAO_AIControllerBase::GetNearestPlayerInSight() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	AAO_PlayerCharacter* NearestPlayer = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const TWeakObjectPtr<AAO_PlayerCharacter>& WeakPlayer : PlayersInSight)
	{
		if (WeakPlayer.IsValid())
		{
			// 타겟팅 시점에도 다시 한 번 생존 확인
			const AAO_PlayerCharacter* Player = WeakPlayer.Get();
			const AAO_PlayerState* PS = Player->GetPlayerState<AAO_PlayerState>();
			if (PS && !PS->GetIsAlive() && !bCanTargetDeadPlayer)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), WeakPlayer->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestPlayer = WeakPlayer.Get();
			}
		}
	}

	return NearestPlayer;
}

bool AAO_AIControllerBase::HasPlayerInSight() const
{
	for (const TWeakObjectPtr<AAO_PlayerCharacter>& WeakPlayer : PlayersInSight)
	{
		if (WeakPlayer.IsValid())
		{
			// 죽은 플레이어는 카운트하지 않음
			const AAO_PlayerCharacter* Player = WeakPlayer.Get();
			const AAO_PlayerState* PS = Player->GetPlayerState<AAO_PlayerState>();
			if (PS && !PS->GetIsAlive() && !bCanTargetDeadPlayer)
			{
				continue;
			}
			
			return true;
		}
	}
	return false;
}

ETeamAttitude::Type AAO_AIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	// 플레이어 캐릭터인지 확인 - 플레이어는 적으로 취급
	const AAO_PlayerCharacter* PlayerChar = Cast<AAO_PlayerCharacter>(&Other);
	if (PlayerChar)
	{
		return ETeamAttitude::Hostile;
	}

	// 다른 AI인지 확인 - 같은 팀이면 아군
	const APawn* OtherPawn = Cast<APawn>(&Other);
	if (OtherPawn)
	{
		AController* OtherController = OtherPawn->GetController();
		if (OtherController)
		{
			const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherController);
			if (TeamAgent && TeamAgent->GetGenericTeamId() == GetGenericTeamId())
			{
				return ETeamAttitude::Friendly;
			}
		}
	}

	// 그 외는 중립
	return ETeamAttitude::Neutral;
}
