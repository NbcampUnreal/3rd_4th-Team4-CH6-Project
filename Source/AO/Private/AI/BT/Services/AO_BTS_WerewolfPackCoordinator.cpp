// AO_BTS_WerewolfPackCoordinator.cpp

#include "AI/BT/Services/AO_BTS_WerewolfPackCoordinator.h"

#include "AI/Character/AO_Enemy_Werewolf.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UAO_BTS_WerewolfPackCoordinator::UAO_BTS_WerewolfPackCoordinator()
{
	NodeName = TEXT("Werewolf Pack Coordinator");

	IsHowlSourceKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, IsHowlSourceKey)
	);

	IsPackNotifiedKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, IsPackNotifiedKey)
	);

	IsInPackPositionKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, IsInPackPositionKey)
	);

	ShouldStartPackAttackKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, ShouldStartPackAttackKey)
	);

	PackTargetLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, PackTargetLocationKey)
	);

	HasPackMatesKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTS_WerewolfPackCoordinator, HasPackMatesKey)
	);

	Interval = 0.3f;
	RandomDeviation = 0.05f;
}

void UAO_BTS_WerewolfPackCoordinator::TickNode(UBehaviorTreeComponent& OwnerComp,
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

	AAO_Enemy_Werewolf* SelfWerewolf = Cast<AAO_Enemy_Werewolf>(AIPawn);
	if(!SelfWerewolf)
	{
		return;
	}

	UBlackboardComponent* SelfBB = OwnerComp.GetBlackboardComponent();
	if(!SelfBB)
	{
		return;
	}
	
	if(!IsPackNotifiedKey.SelectedKeyName.IsValid() ||
	   !IsInPackPositionKey.SelectedKeyName.IsValid() ||
	   !ShouldStartPackAttackKey.SelectedKeyName.IsValid())
	{
		return;
	}

	const bool bSelfNotified =
		SelfBB->GetValueAsBool(IsPackNotifiedKey.SelectedKeyName);

	if(!bSelfNotified)
	{
		if(HasPackMatesKey.SelectedKeyName.IsValid())
		{
			SelfBB->SetValueAsBool(HasPackMatesKey.SelectedKeyName, false);
		}
		return;
	}

	const bool bAlreadyStartAttack =
		SelfBB->GetValueAsBool(ShouldStartPackAttackKey.SelectedKeyName);
	
	if(bAlreadyStartAttack)
	{
		return;
	}
	
	FVector PackCenter = SelfWerewolf->GetActorLocation();
	if(PackTargetLocationKey.SelectedKeyName.IsValid())
	{
		const FVector FromBB =
			SelfBB->GetValueAsVector(PackTargetLocationKey.SelectedKeyName);

		if(!FromBB.IsNearlyZero())
		{
			PackCenter = FromBB;
		}
	}

	UWorld* World = AIPawn->GetWorld();
	if(!World)
	{
		return;
	}

	TArray<AActor*> WerewolfActors;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_Enemy_Werewolf::StaticClass(), WerewolfActors);

	const float RadiusSq = PackRadius * PackRadius;

	int32 NotifiedCount = 0;
	int32 ReadyCount = 0;
	
	for(AActor* Actor : WerewolfActors)
	{
		AAO_Enemy_Werewolf* OtherWolf = Cast<AAO_Enemy_Werewolf>(Actor);
		if(!OtherWolf)
		{
			continue;
		}

		const float DistSq =
			FVector::DistSquared(PackCenter, OtherWolf->GetActorLocation());
		if(DistSq > RadiusSq)
		{
			continue;
		}

		AAIController* OtherController =
			Cast<AAIController>(OtherWolf->GetController());
		if(!OtherController)
		{
			continue;
		}

		UBlackboardComponent* OtherBB = OtherController->GetBlackboardComponent();
		if(!OtherBB)
		{
			continue;
		}

		const bool bOtherNotified =
			OtherBB->GetValueAsBool(IsPackNotifiedKey.SelectedKeyName);

		if(!bOtherNotified)
		{
			continue;
		}

		++NotifiedCount;

		const bool bOtherInPosition =
			OtherBB->GetValueAsBool(IsInPackPositionKey.SelectedKeyName);

		if(bOtherInPosition)
		{
			++ReadyCount;
		}
	}
	
	const bool bHasPackMates = (NotifiedCount >= 2);

	if(HasPackMatesKey.SelectedKeyName.IsValid())
	{
		SelfBB->SetValueAsBool(HasPackMatesKey.SelectedKeyName, bHasPackMates);
	}
	
	if(bHasPackMates && ReadyCount >= MinPackSize)
	{
		for(AActor* Actor : WerewolfActors)
		{
			AAO_Enemy_Werewolf* OtherWolf = Cast<AAO_Enemy_Werewolf>(Actor);
			if(!OtherWolf)
			{
				continue;
			}

			const float DistSq =
				FVector::DistSquared(PackCenter, OtherWolf->GetActorLocation());
			if(DistSq > RadiusSq)
			{
				continue;
			}

			AAIController* OtherController =
				Cast<AAIController>(OtherWolf->GetController());
			if(!OtherController)
			{
				continue;
			}

			UBlackboardComponent* OtherBB = OtherController->GetBlackboardComponent();
			if(!OtherBB)
			{
				continue;
			}

			const bool bOtherNotified =
				OtherBB->GetValueAsBool(IsPackNotifiedKey.SelectedKeyName);

			const bool bOtherInPosition =
				OtherBB->GetValueAsBool(IsInPackPositionKey.SelectedKeyName);

			if(bOtherNotified && bOtherInPosition)
			{
				OtherBB->SetValueAsBool(
					ShouldStartPackAttackKey.SelectedKeyName,
					true
				);
			}
		}
	}
}
