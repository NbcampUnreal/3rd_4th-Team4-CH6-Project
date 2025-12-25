// AO_AggressiveAIBase.cpp

#include "AI/Base/AO_AggressiveAIBase.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_AggressiveAIBase::AAO_AggressiveAIBase()
{
	// 베이스 클래스의 기본 속도를 RoamSpeed로 설정
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;
}

void AAO_AggressiveAIBase::BeginPlay()
{
	Super::BeginPlay();

	// 초기 이동 속도 설정
	UpdateMovementSpeed();
}

void AAO_AggressiveAIBase::SetChaseMode(bool bChasing)
{
	if (bIsChasing == bChasing)
	{
		return;
	}

	bIsChasing = bChasing;
	
	if (bIsChasing)
	{
		bIsSearching = false;  // 추격 모드로 전환 시 수색 모드 해제
	}
	
	UpdateMovementSpeed();
}

void AAO_AggressiveAIBase::SetCurrentTarget(AAO_PlayerCharacter* NewTarget)
{
	CurrentTarget = NewTarget;
}

bool AAO_AggressiveAIBase::IsTargetInAttackRange() const
{
	if (!CurrentTarget.IsValid())
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
	return DistSq <= FMath::Square(AttackRange);
}

void AAO_AggressiveAIBase::SetSearchMode(bool bSearching)
{
	if (bIsSearching == bSearching)
	{
		return;
	}

	bIsSearching = bSearching;

	if (bIsSearching)
	{
		bIsChasing = false;  // 수색 모드로 전환 시 추격 모드 해제
	}

	UpdateMovementSpeed();
}

void AAO_AggressiveAIBase::UpdateMovementSpeed()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	if (bIsChasing)
	{
		MovementComp->MaxWalkSpeed = ChaseSpeed;
	}
	else
	{
		MovementComp->MaxWalkSpeed = RoamSpeed;
	}
}
