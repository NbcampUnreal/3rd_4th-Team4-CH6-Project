// AO_AIControllerBase.cpp

#include "AI/Controller/AO_AIControllerBase.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"
#include "AO_Log.h"

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

	// State Tree 설정 및 시작
	if (StateTreeComponent && DefaultStateTree)
	{
		StateTreeComponent->SetStateTree(DefaultStateTree);
		StateTreeComponent->StartLogic();
	}
}

void AAO_AIControllerBase::OnUnPossess()
{
	// State Tree 정지
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("UnPossessed"));
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
	// 기본 구현: 자식 클래스에서 오버라이드
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
