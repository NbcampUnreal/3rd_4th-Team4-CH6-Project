// KSJ : AO_BTT_InsectKidnap.cpp

#include "AI/BT/BTTasks/AO_BTT_InsectKidnap.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AO_Log.h"

UAO_BTT_InsectKidnap::UAO_BTT_InsectKidnap()
{
	NodeName = TEXT("Insect Kidnap");
}

EBTNodeResult::Type UAO_BTT_InsectKidnap::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Insect* AIC = Cast<AAO_AIC_Insect>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect) return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	AActor* TargetPlayer = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
	if (!TargetPlayer) return EBTNodeResult::Failed;
	
	float Distance = FVector::Dist(Insect->GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Distance > Insect->KidnapRange)
	{
		AO_LOG(LogKSJ, Warning, TEXT("[BTT_InsectKidnap] Target too far: %.1f (Range: %.1f)"), Distance, Insect->KidnapRange);
		return EBTNodeResult::Failed;
	}
	
	Insect->StartKidnap(TargetPlayer);
	
	BB->SetValueAsBool(FName("bIsKidnapping"), true);
	BB->SetValueAsObject(FName("KidnappedPlayer"), TargetPlayer);
	BB->SetValueAsVector(FName("KidnapStartLocation"), Insect->GetActorLocation());
	
	AO_LOG(LogKSJ, Log, TEXT("[BTT_InsectKidnap] Kidnap started for %s"), *TargetPlayer->GetName());
	
	return EBTNodeResult::Succeeded;
}
