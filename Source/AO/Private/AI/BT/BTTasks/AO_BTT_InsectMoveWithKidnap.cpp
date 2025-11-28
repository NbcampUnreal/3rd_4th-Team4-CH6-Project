// KSJ : AO_BTT_InsectMoveWithKidnap.cpp

#include "AI/BT/BTTasks/AO_BTT_InsectMoveWithKidnap.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO_Log.h"

UAO_BTT_InsectMoveWithKidnap::UAO_BTT_InsectMoveWithKidnap()
{
	NodeName = TEXT("Move With Kidnap");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UAO_BTT_InsectMoveWithKidnap::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Insect* AIC = Cast<AAO_AIC_Insect>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->bIsKidnapping) return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	FVector TargetLocation = BB->GetValueAsVector(KidnapTargetLocationKey.SelectedKeyName);
	if (TargetLocation.IsNearlyZero())
	{
		AO_LOG(LogKSJ, Verbose, TEXT("[BTT_InsectMoveWithKidnap] Target location not set yet, waiting for EQS..."));
		return EBTNodeResult::InProgress;
	}
	
	BlackboardKey.SelectedKeyName = KidnapTargetLocationKey.SelectedKeyName;
	
	LastTargetUpdateTime = GetWorld()->GetTimeSeconds();
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UAO_BTT_InsectMoveWithKidnap::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAO_AIC_Insect* AIC = Cast<AAO_AIC_Insect>(OwnerComp.GetAIOwner());
	if (!AIC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->bIsKidnapping)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	FVector TargetLocation = BB->GetValueAsVector(KidnapTargetLocationKey.SelectedKeyName);
	if (TargetLocation.IsNearlyZero())
	{
		return;
	}
	
	if (BlackboardKey.SelectedKeyName == NAME_None)
	{
		BlackboardKey.SelectedKeyName = KidnapTargetLocationKey.SelectedKeyName;
		Super::ExecuteTask(OwnerComp, NodeMemory);
		LastTargetUpdateTime = GetWorld()->GetTimeSeconds();
	}
	
	UWorld* World = GetWorld();
	if (World)
	{
		double CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastTargetUpdateTime >= TargetUpdateInterval)
		{
			LastTargetUpdateTime = CurrentTime;
			
			AActor* TargetPlayer = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
			if (TargetPlayer)
			{
				float DistanceToOtherPlayer = FVector::Dist(Insect->GetActorLocation(), TargetPlayer->GetActorLocation());
				
				if (DistanceToOtherPlayer < 800.0f)
				{
					BB->ClearValue(KidnapTargetLocationKey.SelectedKeyName);
					AO_LOG(LogKSJ, Verbose, TEXT("[BTT_InsectMoveWithKidnap] Target location cleared for EQS recalculation"));
					BlackboardKey.SelectedKeyName = NAME_None;
					return;
				}
			}
		}
	}
	
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	if (!TargetLocation.IsNearlyZero())
	{
		float DistanceToTarget = FVector::Dist(Insect->GetActorLocation(), TargetLocation);
		if (DistanceToTarget < 200.0f)
		{
			AO_LOG(LogKSJ, Log, TEXT("[BTT_InsectMoveWithKidnap] Reached target location"));
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}
