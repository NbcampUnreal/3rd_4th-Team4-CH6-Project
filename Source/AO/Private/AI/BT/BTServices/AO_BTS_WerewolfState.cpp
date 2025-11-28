// KSJ : AO_BTS_WerewolfState.cpp

#include "AI/BT/BTServices/AO_BTS_WerewolfState.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Manager/AO_Manager_WerewolfPackManager.h"
#include "AO_Log.h"

UAO_BTS_WerewolfState::UAO_BTS_WerewolfState()
{
	NodeName = "Update Pack State";
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UAO_BTS_WerewolfState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAO_AIC_Werewolf* AIC = Cast<AAO_AIC_Werewolf>(OwnerComp.GetAIOwner());
	if (!AIC) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	EWerewolfPackState CurrentState = AIC->GetCurrentPackState();

	if (CurrentState == EWerewolfPackState::Siege)
	{
		UAO_Manager_WerewolfPackManager* PackManager = AIC->GetPackManager();
		if (PackManager)
		{
			PackManager->UpdateSiegeFormationFor(AIC);
		}
	}

	BB->SetValueAsEnum(PackStateKey.SelectedKeyName, (uint8)CurrentState);
}
