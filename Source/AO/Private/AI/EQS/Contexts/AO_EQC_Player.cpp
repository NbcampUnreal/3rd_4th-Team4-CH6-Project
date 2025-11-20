#include "AO/Public/AI/EQS/Contexts/AO_EQC_Player.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UAO_EQC_Player::ProvideContext(FEnvQueryInstance& QueryInstance,
                                    FEnvQueryContextData& ContextData) const
{
	const UObject* OwnerObj = QueryInstance.Owner.Get();
	const AActor* OwnerActor = Cast<AActor>(OwnerObj);

	const AAIController* AICon = nullptr;

	if (const AAIController* AsAICon = Cast<AAIController>(OwnerActor))
	{
		AICon = AsAICon;
	}
	else if (const APawn* AsPawn = Cast<APawn>(OwnerActor))
	{
		AICon = Cast<AAIController>(AsPawn->GetController());
	}

	const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
	const AActor* PlayerActor = nullptr;
	
	if (BB != nullptr && PlayerActorBBKeyName.IsNone() == false)
	{
		if (const UObject* Obj = BB->GetValueAsObject(PlayerActorBBKeyName))
		{
			PlayerActor = Cast<AActor>(Obj);
		}
	}
	
	if (PlayerActor == nullptr)
	{
		UWorld* World = GEngine
			? GEngine->GetWorldFromContextObject(QueryInstance.Owner.Get(), EGetWorldErrorMode::ReturnNull)
			: nullptr;

		if (World != nullptr)
		{
			const AActor* QuerierActor = OwnerActor;
			const FVector Origin = QuerierActor ? QuerierActor->GetActorLocation() : FVector::ZeroVector;

			float BestDistSq = TNumericLimits<float>::Max();
			APawn* BestPawn = nullptr;

			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				const APlayerController* PC = It->Get();
				if (PC == nullptr)
				{
					continue;
				}

				APawn* Pawn = PC->GetPawn();
				if (Pawn == nullptr)
				{
					continue;
				}

				if (!Origin.IsNearlyZero())
				{
					const float DistSq = FVector::DistSquared(Origin, Pawn->GetActorLocation());
					if (DistSq >= BestDistSq)
					{
						continue;
					}

					BestDistSq = DistSq;
				}

				BestPawn = Pawn;
			}

			PlayerActor = BestPawn;
		}
	}
	
	if (PlayerActor != nullptr)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerActor);
	}
}
