//KSJ : AO_STCond_WolfPackState

#include "AI/StateTree/Condition/AO_STCond_WolfPackState.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_WerewolfController.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_WolfPackState::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	bool bResult = false;

	// Controller와 PackComp를 매번 직접 가져옴 (캐시에 의존하지 않고 항상 최신 상태 반영)
	AAO_WerewolfController* Controller = nullptr;
	UAO_PackCoordComp* PackComp = nullptr;
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
		Controller = Cast<AAO_WerewolfController>(Pawn->GetController());
	}
	else if (AController* Ctrl = Cast<AController>(OwnerActor))
	{
		Controller = Cast<AAO_WerewolfController>(Ctrl);
		Wolf = Cast<AAO_Werewolf>(Ctrl->GetPawn());
	}

	if (Wolf)
	{
		PackComp = Wolf->GetPackCoordComp();
	}

	switch (InstanceData.StateToCheck)
	{
	case EAO_WolfPackStateCheck::HasHowledOrJoined:
		if (Controller)
		{
			bResult = Controller->HasHowledOrJoined();
		}
		break;

	case EAO_WolfPackStateCheck::IsSurrounding:
		if (PackComp)
		{
			bResult = PackComp->IsSurrounding();
		}
		break;

	case EAO_WolfPackStateCheck::IsCoordinatedAttackStarted:
		if (PackComp)
		{
			bResult = PackComp->IsCoordinatedAttackStarted();
		}
		break;

	case EAO_WolfPackStateCheck::IsHowlInitiator:
		if (PackComp)
		{
			bResult = PackComp->IsHowlInitiator();
		}
		break;

	case EAO_WolfPackStateCheck::HasReachedSurroundPosition:
		if (PackComp)
		{
			bResult = PackComp->HasReachedSurroundPosition();
		}
		break;

	case EAO_WolfPackStateCheck::HasPackMembers:
		if (PackComp)
		{
			TArray<AAO_Werewolf*> Members = PackComp->GetNearbyPackMembers();
			bResult = (Members.Num() > 0);
		}
		break;
	}

	return InstanceData.bInvert ? !bResult : bResult;
}

void FAO_STCond_WolfPackState::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
		InstanceData.Controller = Cast<AAO_WerewolfController>(Pawn->GetController());
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Controller->GetPawn());
		InstanceData.Controller = Cast<AAO_WerewolfController>(Controller);
	}

	if (Wolf)
	{
		InstanceData.PackComp = Wolf->GetPackCoordComp();
	}
}

