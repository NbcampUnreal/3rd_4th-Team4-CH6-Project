// AO_EQC_PlayerPredictedLocation.cpp

#include "AI/EQS/Contexts/AO_EQC_PlayerPredictedLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UAO_EQC_PlayerPredictedLocation::UAO_EQC_PlayerPredictedLocation()
{
	PlayerActorBBKeyName = TEXT("PlayerActor");
	PredictDistance = 800.0f;
	FallbackDistance = 600.0f;
}

void UAO_EQC_PlayerPredictedLocation::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                     FEnvQueryContextData& ContextData) const
{
	const UObject* OwnerObj = QueryInstance.Owner.Get();
	const AActor* OwnerActor = Cast<AActor>(OwnerObj);

	const AAIController* AICon = nullptr;

	if(const AAIController* AsAICon = Cast<AAIController>(OwnerActor))
	{
		AICon = AsAICon;
	}
	else if(const APawn* AsPawn = Cast<APawn>(OwnerActor))
	{
		AICon = Cast<AAIController>(AsPawn->GetController());
	}

	const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
	const AActor* PlayerActor = nullptr;
	
	if(BB != nullptr && PlayerActorBBKeyName.IsNone() == false)
	{
		if(const UObject* Obj = BB->GetValueAsObject(PlayerActorBBKeyName))
		{
			PlayerActor = Cast<AActor>(Obj);
		}
	}
	
	if(PlayerActor == nullptr)
	{
		UWorld* World = OwnerActor
			? OwnerActor->GetWorld()
			: nullptr;

		if(World != nullptr)
		{
			const FVector Origin =
				OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;

			float BestDistSq = TNumericLimits<float>::Max();
			APawn* BestPawn = nullptr;

			for(FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				const APlayerController* PC = It->Get();
				if(PC == nullptr)
				{
					continue;
				}

				APawn* Pawn = PC->GetPawn();
				if(Pawn == nullptr)
				{
					continue;
				}

				const float DistSq =
					FVector::DistSquared(Origin, Pawn->GetActorLocation());
				if(DistSq >= BestDistSq)
				{
					continue;
				}

				BestDistSq = DistSq;
				BestPawn = Pawn;
			}

			PlayerActor = BestPawn;
		}
	}

	if(PlayerActor == nullptr)
	{
		return;
	}

	const FVector PlayerLocation = PlayerActor->GetActorLocation();
	
	FVector MoveDir = FVector::ZeroVector;

	if(const APawn* PlayerPawn = Cast<APawn>(PlayerActor))
	{
		const FVector Velocity = PlayerPawn->GetVelocity();
		if(!Velocity.IsNearlyZero())
		{
			MoveDir = Velocity.GetSafeNormal();
		}
	}

	if(MoveDir.IsNearlyZero())
	{
		MoveDir = PlayerActor->GetActorForwardVector().GetSafeNormal();
	}

	const float Distance =
		MoveDir.IsNearlyZero()
			? FallbackDistance
			: PredictDistance;

	const FVector PredictedLocation =
		PlayerLocation + (MoveDir * Distance);

	UEnvQueryItemType_Point::SetContextHelper(ContextData, PredictedLocation);
}
