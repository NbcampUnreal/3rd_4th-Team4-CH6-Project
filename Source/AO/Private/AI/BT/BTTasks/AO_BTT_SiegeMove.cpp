// KSJ : AO_BTT_SiegeMove.cpp

#include "AI/BT/BTTasks/AO_BTT_SiegeMove.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UAO_BTT_SiegeMove::UAO_BTT_SiegeMove()
{
	NodeName = "Siege Move";
	bNotifyTick = true;
}

EBTNodeResult::Type UAO_BTT_SiegeMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UAO_BTT_SiegeMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAO_AIC_Werewolf* AIC = Cast<AAO_AIC_Werewolf>(OwnerComp.GetAIOwner());
	if (!AIC) return;

	APawn* MyPawn = AIC->GetPawn();
	if (!MyPawn) return;

	AActor* Target = AIC->GetPackTargetPlayer();
	if (!Target)
	{
		Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor"));
	}

	if (Target)
	{
		float DistSq = FVector::DistSquared(MyPawn->GetActorLocation(), Target->GetActorLocation());
		if (DistSq <= (BreachDistance * BreachDistance))
		{
			if (AIC->GetPackManager())
			{
				AIC->GetPackManager()->StartCoordinatedAttack(Target);
			}
			
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}
