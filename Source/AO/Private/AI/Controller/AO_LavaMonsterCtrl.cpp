// AO_LavaMonsterCtrl.cpp

#include "AI/Controller/AO_LavaMonsterCtrl.h"
#include "AI/Character/AO_LavaMonster.h"
#include "AO_Log.h"

AAO_LavaMonsterCtrl::AAO_LavaMonsterCtrl()
{
}

void AAO_LavaMonsterCtrl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(InPawn);
	if (LavaMonster)
	{
		AO_LOG(LogKSJ, Log, TEXT("LavaMonsterCtrl possessed %s"), *LavaMonster->GetName());
	}
}

AAO_LavaMonster* AAO_LavaMonsterCtrl::GetLavaMonster() const
{
	return Cast<AAO_LavaMonster>(GetPawn());
}

bool AAO_LavaMonsterCtrl::CanAttack() const
{
	AAO_LavaMonster* LavaMonster = GetLavaMonster();
	if (!LavaMonster)
	{
		return false;
	}

	// 기절 중이면 공격 불가
	if (LavaMonster->IsStunned())
	{
		return false;
	}

	// 이미 공격 중이면 불가
	if (LavaMonster->IsAttacking())
	{
		return false;
	}

	// 공격 범위 내에 타겟이 있어야 함
	return LavaMonster->IsTargetInAttackRange();
}

