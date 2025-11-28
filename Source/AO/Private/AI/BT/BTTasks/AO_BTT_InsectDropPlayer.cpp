// KSJ : AO_BTT_InsectDropPlayer.cpp

#include "AI/BT/BTTasks/AO_BTT_InsectDropPlayer.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AO_Log.h"

UAO_BTT_InsectDropPlayer::UAO_BTT_InsectDropPlayer()
{
	NodeName = TEXT("Drop Player");
}

EBTNodeResult::Type UAO_BTT_InsectDropPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Insect* AIC = Cast<AAO_AIC_Insect>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->bIsKidnapping) return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	Insect->EndKidnap(bApplyKnockback);
	
	BB->SetValueAsBool(FName("bIsKidnapping"), false);
	BB->ClearValue(FName("KidnappedPlayer"));
	BB->ClearValue(FName("KidnapStartLocation"));
	BB->ClearValue(FName("KidnapTargetLocation"));
	
	AO_LOG(LogKSJ, Log, TEXT("[BTT_InsectDropPlayer] Player dropped"));
	
	return EBTNodeResult::Succeeded;
}
