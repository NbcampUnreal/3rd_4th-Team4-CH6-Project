// KSJ : AO_BTS_InsectCheckKidnapCooldown.cpp

#include "AI/BT/BTServices/AO_BTS_InsectCheckKidnapCooldown.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Manager/AO_Manager_InsectKidnapManager.h"
#include "AO_Log.h"

UAO_BTS_InsectCheckKidnapCooldown::UAO_BTS_InsectCheckKidnapCooldown()
{
	NodeName = TEXT("Check Kidnap Cooldown");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UAO_BTS_InsectCheckKidnapCooldown::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>();
	if (!Manager) return;
	
	AActor* TargetPlayer = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
	if (!TargetPlayer)
	{
		BB->SetValueAsBool(bCanKidnapKey.SelectedKeyName, false);
		return;
	}
	
	bool bCanKidnap = Manager->CanKidnapPlayer(TargetPlayer);
	BB->SetValueAsBool(bCanKidnapKey.SelectedKeyName, bCanKidnap);
	
	if (!bCanKidnap)
	{
		AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
		if (CurrentTarget == TargetPlayer)
		{
			BB->ClearValue(TargetPlayerKey.SelectedKeyName);
			BB->SetValueAsBool(FName("bPlayerVisible"), false);
			AO_LOG(LogKSJ, Log, TEXT("[BTS_CheckKidnapCooldown] Target %s is on cooldown, clearing target"), *TargetPlayer->GetName());
		}
	}
}
