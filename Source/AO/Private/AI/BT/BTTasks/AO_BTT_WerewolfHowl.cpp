// AO_BTT_WerewolfHowl.cpp

#include "AI/BT/BTTasks/AO_BTT_WerewolfHowl.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Character/AO_Enemy_Werewolf.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_WerewolfHowl::UAO_BTT_WerewolfHowl()
{
	NodeName = TEXT("Werewolf Howl");

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, TargetActorKey),
		AActor::StaticClass()
	);

	PackTargetLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, PackTargetLocationKey)
	);

	IsAlertedKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, IsAlertedKey)
	);

	IsHowlSourceKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, IsHowlSourceKey)
	);

	IsPackNotifiedKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_WerewolfHowl, IsPackNotifiedKey)
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

	AAO_Enemy_Werewolf* Werewolf =
		Cast<AAO_Enemy_Werewolf>(AIController->GetPawn());
	if(!Werewolf)
	{
		return EBTNodeResult::Failed;
	}

	if(Werewolf->GetLocalRole() != ROLE_Authority)
	{
		return EBTNodeResult::Failed;
	}
	
	if(Werewolf->HasHowled())
	{
		return EBTNodeResult::Succeeded;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor =
		Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	FVector HowlLocation = Werewolf->GetActorLocation();
	if(TargetActor)
	{
		HowlLocation = TargetActor->GetActorLocation();
	}
	
	Werewolf->PerformHowl();
	
	if(IsAlertedKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsAlertedKey.SelectedKeyName, true);
	}

	if(IsHowlSourceKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsHowlSourceKey.SelectedKeyName, true);
	}

	if(IsPackNotifiedKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsPackNotifiedKey.SelectedKeyName, true);
	}

	if(PackTargetLocationKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsVector(PackTargetLocationKey.SelectedKeyName, HowlLocation);
	}
	
	if(AAO_AIC_Werewolf* WolfController = Cast<AAO_AIC_Werewolf>(AIController))
	{
		WolfController->HandleHowlEvent(HowlLocation);
	}

	return EBTNodeResult::Succeeded;
}
