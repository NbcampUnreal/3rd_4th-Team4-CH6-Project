// AO_BTS_UpdateTargetPlayer.cpp

#include "AO/Public/AI/BT/Services/AO_BTS_UpdateTargetPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/PlayerController.h"

UAO_BTS_UpdateTargetPlayer::UAO_BTS_UpdateTargetPlayer()
{
	NodeName = TEXT("Update Target Player");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	Interval = 0.3f;
	RandomDeviation = 0.05f;
}

void UAO_BTS_UpdateTargetPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* AIPawn = AICon ? AICon->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !AIPawn || !BB || !AICon->HasAuthority())
	{
		return;
	}

	UAIPerceptionComponent* Perception = AICon->FindComponentByClass<UAIPerceptionComponent>();
	TArray<AActor*> SeenActors;
	if (Perception)
	{
		Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);
	}

	auto IsPlayer = [](AActor* A)
	{
		if (APawn* P = Cast<APawn>(A))
		{
			AController* C = P->GetController();
			return C && C->IsPlayerController();
		}
		return false;
	};

	AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	
	for (AActor* A : SeenActors)
	{
		if (!IsPlayer(A))
		{
			continue;
		}

		const float D2 = FVector::DistSquared(AIPawn->GetActorLocation(), A->GetActorLocation());
		if (D2 < BestDistSq)
		{
			BestDistSq = D2;
			Best = A;
		}
	}
	
	if (!Best)
	{
		for (FConstPlayerControllerIterator It = AICon->GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PC = It->Get();
			if (!PC) continue;

			APawn* P = PC->GetPawn();
			if (!P) continue;

			const float D2 = FVector::DistSquared(AIPawn->GetActorLocation(), P->GetActorLocation());
			if (D2 < BestDistSq)
			{
				BestDistSq = D2;
				Best = P;
			}
		}
	}
	
	const bool bVisible = (Best && SeenActors.Contains(Best));

	if (PlayerActorKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsObject(PlayerActorKey.SelectedKeyName, Best);
	}

	if (bPlayerVisibleKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsBool(bPlayerVisibleKey.SelectedKeyName, bVisible);
	}

	if (Best && LastKnownLocationKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsVector(LastKnownLocationKey.SelectedKeyName, Best->GetActorLocation());
	}
}
