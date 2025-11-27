// KSJ : AO_BTT_Howl.cpp

#include "AI/BT/BTTasks/AO_BTT_Howl.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Character/AO_Enemy_Werewolf.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

UAO_BTT_Howl::UAO_BTT_Howl()
{
	NodeName = "Howl";
	bNotifyTick = true;
}

EBTNodeResult::Type UAO_BTT_Howl::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Werewolf* AIC = Cast<AAO_AIC_Werewolf>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;

	AAO_Enemy_Werewolf* Wolf = Cast<AAO_Enemy_Werewolf>(AIC->GetPawn());
	if (!Wolf) return EBTNodeResult::Failed;

	AIC->StopMovement();

	UAnimMontage* MontageToPlay = Wolf->HowlMontage;
	if (MontageToPlay)
	{
		UAnimInstance* AnimInstance = Wolf->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float Duration = Wolf->PlayAnimMontage(MontageToPlay);
			if (Duration > 0.0f)
			{
				FOnMontageEnded EndedDelegate;
				EndedDelegate.BindUObject(this, &UAO_BTT_Howl::OnMontageEnded, &OwnerComp);
				AnimInstance->Montage_SetEndDelegate(EndedDelegate, MontageToPlay);

				return EBTNodeResult::InProgress;
			}
		}
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("[BTT_Howl] No Howl Montage assigned!"));
	}

	return EBTNodeResult::Succeeded;
}

void UAO_BTT_Howl::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}

void UAO_BTT_Howl::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	if (OwnerComp)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
}
