// AO_BTS_UpdateTarget.cpp

#include "AI/BT/Services/AO_BTS_UpdateTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UAO_BTS_UpdateTarget::UAO_BTS_UpdateTarget()
{
	NodeName = TEXT("Update Closest Target");

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_UpdateTarget, TargetActorKey),
		AActor::StaticClass()
	);

	HasLineOfSightKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_UpdateTarget, HasLineOfSightKey)
	);

	LastSeenLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_UpdateTarget, LastSeenLocationKey)
	);

	IsSearchingKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_UpdateTarget, IsSearchingKey)
	);

	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UAO_BTS_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp,
                                    uint8* NodeMemory,
                                    float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if(!AIController)
	{
		return;
	}

	APawn* AIPawn = AIController->GetPawn();
	if(!AIPawn || !AIPawn->HasAuthority())
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp || !TargetActorKey.SelectedKeyName.IsValid())
	{
		return;
	}

	UWorld* World = AIPawn->GetWorld();
	if(!World)
	{
		return;
	}
	
	AActor* PrevTarget =
		Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	bool bPrevHasLOS = false;
	if(HasLineOfSightKey.SelectedKeyName.IsValid())
	{
		bPrevHasLOS = BlackboardComp->GetValueAsBool(HasLineOfSightKey.SelectedKeyName);
	}

	const FVector SelfLocation = AIPawn->GetActorLocation();
	const float MaxDistSq = MaxTargetDistance * MaxTargetDistance;

	AActor* BestTarget = nullptr;
	float BestDistSq = MaxDistSq;
	
	TArray<AActor*> Pawns;
	UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), Pawns);

	for(AActor* Actor : Pawns)
	{
		APawn* OtherPawn = Cast<APawn>(Actor);
		if(!OtherPawn)
		{
			continue;
		}

		if(!OtherPawn->IsPlayerControlled())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(SelfLocation, OtherPawn->GetActorLocation());
		if(DistSq > BestDistSq)
		{
			continue;
		}
		
		if(!AIController->LineOfSightTo(OtherPawn))
		{
			continue;
		}

		BestTarget = OtherPawn;
		BestDistSq = DistSq;
	}
	
	const bool bHasNewTarget = (BestTarget != nullptr);
	
	if(bHasNewTarget)
	{
		BlackboardComp->SetValueAsObject(TargetActorKey.SelectedKeyName, BestTarget);

		if(HasLineOfSightKey.SelectedKeyName.IsValid())
		{
			BlackboardComp->SetValueAsBool(HasLineOfSightKey.SelectedKeyName, true);
		}
		
		if(IsSearchingKey.SelectedKeyName.IsValid())
		{
			BlackboardComp->SetValueAsBool(IsSearchingKey.SelectedKeyName, false);
		}
	}
	else
	{
		if(HasLineOfSightKey.SelectedKeyName.IsValid())
		{
			BlackboardComp->SetValueAsBool(HasLineOfSightKey.SelectedKeyName, false);
		}
		
	}
	
	if(bPrevHasLOS && !bHasNewTarget &&
	   LastSeenLocationKey.SelectedKeyName.IsValid() &&
	   IsSearchingKey.SelectedKeyName.IsValid() &&
	   PrevTarget != nullptr)
	{
		const FVector LastSeenLocation = PrevTarget->GetActorLocation();

		BlackboardComp->SetValueAsVector(LastSeenLocationKey.SelectedKeyName, LastSeenLocation);
		BlackboardComp->SetValueAsBool(IsSearchingKey.SelectedKeyName, true);
	}
}
