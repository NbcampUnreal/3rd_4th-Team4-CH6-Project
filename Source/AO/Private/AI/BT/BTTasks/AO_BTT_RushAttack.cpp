// KSJ : AO_BTT_RushAttack.cpp

#include "AI/BT/BTTasks/AO_BTT_RushAttack.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Character/AO_Enemy_Werewolf.h"
#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_RushAttack::UAO_BTT_RushAttack()
{
	NodeName = "Rush Attack";
}

EBTNodeResult::Type UAO_BTT_RushAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Werewolf* AIC = Cast<AAO_AIC_Werewolf>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;

	AAO_Enemy_Werewolf* Wolf = Cast<AAO_Enemy_Werewolf>(AIC->GetPawn());
	if (!Wolf) return EBTNodeResult::Failed;

	if (Wolf->AttackMontage)
	{
		Wolf->PlayAnimMontage(Wolf->AttackMontage);
	}

	return EBTNodeResult::Succeeded;
}
