//KSJ : AO_BullController

#include "AI/Controller/AO_BullController.h"
#include "AI/Character/AO_Bull.h"
#include "Character/AO_PlayerCharacter.h"

AAO_BullController::AAO_BullController()
{
}

void AAO_BullController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

AAO_Bull* AAO_BullController::GetBull() const
{
	return Cast<AAO_Bull>(GetPawn());
}

bool AAO_BullController::CanChargeAttack() const
{
	AAO_Bull* Bull = GetBull();
	if (!Bull || Bull->IsStunned() || Bull->IsCharging())
	{
		return false;
	}

	// 공격 후 쿨다운 중이면 불가
	if (Bull->IsInPostAttackCooldown())
	{
		return false;
	}

	AAO_PlayerCharacter* Target = GetChaseTarget();
	if (!Target)
	{
		return false;
	}

	// 거리 체크
	float DistSq = FVector::DistSquared(Bull->GetActorLocation(), Target->GetActorLocation());
	if (DistSq < MinChargeDistance * MinChargeDistance || DistSq > MaxChargeDistance * MaxChargeDistance)
	{
		return false;
	}

	// 시야 체크 (직선 경로에 장애물이 없는지 확인은 Task나 Ability에서 수행)
	if (!HasPlayerInSight())
	{
		return false;
	}

	return true;
}

bool AAO_BullController::CanMeleeAttack() const
{
	AAO_Bull* Bull = GetBull();
	if (!Bull || Bull->IsStunned() || Bull->IsCharging())
	{
		return false;
	}

	// 공격 후 쿨다운 중이면 불가
	if (Bull->IsInPostAttackCooldown())
	{
		return false;
	}

	AAO_PlayerCharacter* Target = GetChaseTarget();
	if (!Target)
	{
		return false;
	}

	// 거리 체크: 돌진 최소 거리보다 가까우면 근접 공격 시도
	float DistSq = FVector::DistSquared(Bull->GetActorLocation(), Target->GetActorLocation());
	if (DistSq < MinChargeDistance * MinChargeDistance)
	{
		return true;
	}

	return false;
}
