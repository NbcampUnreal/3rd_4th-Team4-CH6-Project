//KSJ : AO_Stalker

#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "AI/Controller/AO_StalkerController.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Stalker::AAO_Stalker()
{
	CeilingMoveComp = CreateDefaultSubobject<UAO_CeilingMoveComponent>(TEXT("CeilingMoveComp"));

	// NavAgentProps 설정 (천장 모드에서 바닥 NavMesh를 찾기 위해)
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		// 기본 NavAgentProps 설정
		MoveComp->NavAgentProps.bCanCrouch = false;
		MoveComp->NavAgentProps.bCanJump = false;
		// AgentHeight와 AgentRadius는 캡슐 크기에 맞게 자동 설정됨
	}

	// 속도 설정
	// KSJ: 요구사항 반영
	// - 배회: 300
	// - 추격(지상 스토킹/기습 포함): 500
	// - 천장 이동(배회에서만): 600 (ChaseSpeed * 1.2f 로 계산)
	RoamSpeed = 300.f;
	ChaseSpeed = 500.f;
	
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	AIControllerClass = AAO_StalkerController::StaticClass();
}

void AAO_Stalker::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Stalker::SetCeilingMode(bool bEnable)
{
	if (CeilingMoveComp)
	{
		CeilingMoveComp->SetCeilingMode(bEnable);
		
		// 천장 이동 시 속도 증가
		if (bEnable)
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed * 1.2f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		}
	}
}

void AAO_Stalker::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 기절 시 천장에서 떨어짐
	SetCeilingMode(false);
}

