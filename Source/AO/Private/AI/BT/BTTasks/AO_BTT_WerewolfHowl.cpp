// AO_BTT_WerewolfHowl.cpp

#include "AI/BT/BTTasks/AO_BTT_WerewolfHowl.h"

#include "AI/Character/AO_Enemy_Werewolf.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UAO_BTT_WerewolfHowl::UAO_BTT_WerewolfHowl()
{
	NodeName = TEXT("Werewolf Howl & Notify Pack");

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, TargetActorKey),
		AActor::StaticClass()
	);

	IsHowlSourceKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, IsHowlSourceKey)
	);

	IsPackNotifiedKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, IsPackNotifiedKey)
	);

	PackTargetLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, PackTargetLocationKey)
	);
}

EBTNodeResult::Type UAO_BTT_WerewolfHowl::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                      uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AIController->GetPawn();
	if(!AIPawn || !AIPawn->HasAuthority())
	{
		return EBTNodeResult::Failed;
	}

	AAO_Enemy_Werewolf* SelfWerewolf = Cast<AAO_Enemy_Werewolf>(AIPawn);
	if(!SelfWerewolf)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* SelfBB = OwnerComp.GetBlackboardComponent();
	if(!SelfBB)
	{
		return EBTNodeResult::Failed;
	}

	if(!TargetActorKey.SelectedKeyName.IsValid() ||
	   !IsPackNotifiedKey.SelectedKeyName.IsValid() ||
	   !PackTargetLocationKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor =
		Cast<AActor>(SelfBB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if(!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector SelfLocation = SelfWerewolf->GetActorLocation();
	
	if(IsHowlSourceKey.SelectedKeyName.IsValid())
	{
		SelfBB->SetValueAsBool(IsHowlSourceKey.SelectedKeyName, true);
	}

	SelfBB->SetValueAsBool(IsPackNotifiedKey.SelectedKeyName, true);
	SelfBB->SetValueAsVector(PackTargetLocationKey.SelectedKeyName, TargetLocation);
	
	UWorld* World = AIPawn->GetWorld();
	if(World)
	{
		TArray<AActor*> WerewolfActors;
		UGameplayStatics::GetAllActorsOfClass(
			World,
			AAO_Enemy_Werewolf::StaticClass(),
			WerewolfActors
		);

		const float RadiusSq = HowlNotifyRadius * HowlNotifyRadius;

		for(AActor* Actor : WerewolfActors)
		{
			AAO_Enemy_Werewolf* OtherWolf = Cast<AAO_Enemy_Werewolf>(Actor);
			if(!OtherWolf || OtherWolf == SelfWerewolf)
			{
				continue;
			}

			const float DistSq =
				FVector::DistSquared(SelfLocation, OtherWolf->GetActorLocation());
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
			
			const bool bAlreadyNotified =
				OtherBB->GetValueAsBool(IsPackNotifiedKey.SelectedKeyName);

			if(!bAlreadyNotified)
			{
				OtherBB->SetValueAsBool(IsPackNotifiedKey.SelectedKeyName, true);
			}
			
			if(IsHowlSourceKey.SelectedKeyName.IsValid())
			{
				OtherBB->SetValueAsBool(IsHowlSourceKey.SelectedKeyName, false);
			}
			
			OtherBB->SetValueAsVector(
				PackTargetLocationKey.SelectedKeyName,
				TargetLocation
			);

			// 디버그용 로그
			 UE_LOG(LogTemp, Log, TEXT("Werewolf notified: %s"), *OtherWolf->GetName());
		}
	}

	return EBTNodeResult::Succeeded;
}
