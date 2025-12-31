//KSJ : AO_STEval_TrollContext

#include "AI/StateTree/Evaluator/AO_STEval_TrollContext.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "StateTreeExecutionContext.h"

void FAO_STEval_TrollContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_TrollContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_TrollContext_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

void FAO_STEval_TrollContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_TrollContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_TrollContext_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

AAO_Troll* FAO_STEval_TrollContext::GetTroll(FStateTreeExecutionContext& Context) const
{
	AAO_TrollController* Controller = GetTrollController(Context);
	if (Controller)
	{
		return Controller->GetTroll();
	}
	return nullptr;
}

AAO_TrollController* FAO_STEval_TrollContext::GetTrollController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_TrollController>(Pawn->GetController());
		}
	}
	return nullptr;
}

void FAO_STEval_TrollContext::UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_TrollContext_InstanceData& InstanceData) const
{
	AAO_Troll* Troll = GetTroll(Context);
	AAO_TrollController* Controller = GetTrollController(Context);

	if (!Troll || !Controller)
	{
		return;
	}

	// 기본 상태
	InstanceData.bIsStunned = Troll->IsStunned();
	InstanceData.bIsChasing = Troll->IsInChaseMode();
	InstanceData.bIsSearching = Troll->IsInSearchMode();
	InstanceData.bHasPlayerInSight = Controller->HasPlayerInSight();
	InstanceData.bTargetInAttackRange = Troll->IsTargetInAttackRange();

	// Troll 전용 상태
	InstanceData.bHasWeapon = Troll->HasWeapon();
	InstanceData.bIsAttacking = Troll->IsAttacking();
	InstanceData.bIsPickingUpWeapon = Troll->IsPickingUpWeapon();
	InstanceData.bHasWeaponInSight = Controller->HasWeaponInSight();
	InstanceData.bShouldPrioritizeWeapon = Controller->ShouldPrioritizeWeaponPickup();
	InstanceData.NearestWeaponLocation = Controller->GetNearestWeaponLocation();

}

