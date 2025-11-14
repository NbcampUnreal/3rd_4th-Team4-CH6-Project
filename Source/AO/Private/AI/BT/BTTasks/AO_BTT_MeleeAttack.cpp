// AO_BTT_MeleeAttack.cpp

#include "AI/BT/BTTasks/AO_BTT_MeleeAttack.h"
#include "AIController.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_MeleeAttack::UAO_BTT_MeleeAttack()
{
	NodeName = TEXT("Melee Attack");
	
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_MeleeAttack, TargetActorKey),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type UAO_BTT_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                     uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AAO_Enemy_HumanoidBase* Humanoid =
		Cast<AAO_Enemy_HumanoidBase>(AIController->GetPawn());
	if(!Humanoid)
	{
		return EBTNodeResult::Failed;
	}

	if(Humanoid->GetLocalRole() != ROLE_Authority)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp || !TargetActorKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor =
		Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if(!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const FVector SelfLocation = Humanoid->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();

	const float DistanceSq = FVector::DistSquared(SelfLocation, TargetLocation);
	const float AttackRange = Humanoid->GetAttackRange();
	const float AttackRangeSq = AttackRange * AttackRange;

	if(DistanceSq > AttackRangeSq)
	{
		return EBTNodeResult::Failed;
	}
	
	UE_LOG(LogTemp,
	       Log,
	       TEXT("[Humanoid] Melee Attack: %s -> %s (Dist=%.1f)"),
	       *Humanoid->GetName(),
	       *TargetActor->GetName(),
	       FMath::Sqrt(DistanceSq));

	// TODO: Damage apply

	return EBTNodeResult::Succeeded;
}
