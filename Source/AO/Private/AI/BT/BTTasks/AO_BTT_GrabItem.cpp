// KSJ : AO_BTT_GrabItem.cpp

#include "AI/BT/BTTasks/AO_BTT_GrabItem.h"
#include "AI/Character/AO_Enemy_Crab.h"
#include "AI/Manager/AO_Manager_CrabPickupManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AO_Log.h"
#include "AI/Item/AO_PickupComponent.h"

struct FBTGrabItemMemory
{
	AActor* OriginalTargetItem = nullptr;
};

UAO_BTT_GrabItem::UAO_BTT_GrabItem()
{
	NodeName = TEXT("Grab Item");
	bCreateNodeInstance = true;
}

uint16 UAO_BTT_GrabItem::GetInstanceMemorySize() const
{
	return sizeof(FBTGrabItemMemory);
}

EBTNodeResult::Type UAO_BTT_GrabItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAO_Enemy_Crab* Crab = Cast<AAO_Enemy_Crab>(AICon->GetPawn());
	if (!Crab) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	UObject* TargetObject = Blackboard->GetValueAsObject(GetSelectedBlackboardKey());
	AActor* TargetItem = Cast<AActor>(TargetObject);

	if (!TargetItem)
	{
		AO_LOG(LogKSJ, Warning, TEXT("BTT_GrabItem: TargetItem is null"));
		return EBTNodeResult::Failed;
	}
	
	UWorld* World = GetWorld();
	if (!World) return EBTNodeResult::Failed;
	
	UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>();
	if (!Manager)
	{
		AO_LOG(LogKSJ, Error, TEXT("BTT_GrabItem: CrabPickupManager not found!"));
		return EBTNodeResult::Failed;
	}
	
	FBTGrabItemMemory* MyMemory = reinterpret_cast<FBTGrabItemMemory*>(NodeMemory);
	if (MyMemory)
	{
		MyMemory->OriginalTargetItem = TargetItem;
	}
	
	float DistanceSq = FVector::DistSquared(Crab->GetActorLocation(), TargetItem->GetActorLocation());
	float MaxGrabDistanceSq = 250.0f * 250.0f;
	if (DistanceSq > MaxGrabDistanceSq)
	{
		if (!Manager->CanPickupItem(TargetItem))
		{
			AAO_Enemy_Crab* OwnerCrab = Manager->GetItemOwner(TargetItem);
			AO_LOG(LogKSJ, Warning, TEXT("[BTT_GrabItem] %s: TargetItem %s was picked up by %s while moving, FORCING CLEAR"), 
				*Crab->GetName(), *TargetItem->GetName(), OwnerCrab ? *OwnerCrab->GetName() : TEXT("Unknown"));
			Blackboard->ClearValue(GetSelectedBlackboardKey());
			Blackboard->ClearValue(TEXT("TargetLocation"));
			if (MyMemory)
			{
				MyMemory->OriginalTargetItem = nullptr;
			}
			return EBTNodeResult::Failed;
		}
		
		AO_LOG(LogKSJ, Verbose, TEXT("[BTT_GrabItem] %s: TargetItem %s is too far away (Distance: %.1f), will retry"), 
			*Crab->GetName(), *TargetItem->GetName(), FMath::Sqrt(DistanceSq));
		return EBTNodeResult::InProgress;
	}

	if (!Manager->CanPickupItem(TargetItem))
	{
		AO_LOG(LogKSJ, Log, TEXT("[BTT_GrabItem] %s: TargetItem %s cannot be picked up (already owned or in cooldown), clearing from blackboard"), 
			*Crab->GetName(), *TargetItem->GetName());
		Blackboard->ClearValue(GetSelectedBlackboardKey());
		Blackboard->ClearValue(TEXT("TargetLocation"));
		if (MyMemory)
		{
			MyMemory->OriginalTargetItem = nullptr;
		}
		return EBTNodeResult::Failed;
	}

	Crab->GrabItem(TargetItem);

	if (Crab->HeldItemActor == TargetItem)
	{
		Blackboard->ClearValue(GetSelectedBlackboardKey());
		if (MyMemory)
		{
			MyMemory->OriginalTargetItem = nullptr;
		}
		return EBTNodeResult::Succeeded;
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("[BTT_GrabItem] %s: Failed to grab %s (may be picked up by another Crab)"), 
			*Crab->GetName(), *TargetItem->GetName());
		Blackboard->ClearValue(GetSelectedBlackboardKey());
		Blackboard->ClearValue(TEXT("TargetLocation"));
		if (MyMemory)
		{
			MyMemory->OriginalTargetItem = nullptr;
		}
		return EBTNodeResult::Failed;
	}
}

void UAO_BTT_GrabItem::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;
	
	AAO_Enemy_Crab* Crab = Cast<AAO_Enemy_Crab>(AICon->GetPawn());
	if (!Crab) return;
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;
	
	UObject* TargetObject = Blackboard->GetValueAsObject(GetSelectedBlackboardKey());
	AActor* TargetItem = Cast<AActor>(TargetObject);
	
	FBTGrabItemMemory* MyMemory = reinterpret_cast<FBTGrabItemMemory*>(NodeMemory);
	if (!MyMemory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (!TargetItem)
	{
		AO_LOG(LogKSJ, Log, TEXT("[BTT_GrabItem] %s: TargetItem was cleared/changed, aborting"), *Crab->GetName());
		Blackboard->ClearValue(TEXT("TargetLocation"));
		MyMemory->OriginalTargetItem = nullptr;
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (MyMemory->OriginalTargetItem && TargetItem != MyMemory->OriginalTargetItem)
	{
		AO_LOG(LogKSJ, Warning, TEXT("[BTT_GrabItem] %s: TargetItem changed from %s to %s, aborting old task"), 
			*Crab->GetName(), 
			MyMemory->OriginalTargetItem ? *MyMemory->OriginalTargetItem->GetName() : TEXT("None"),
			*TargetItem->GetName());
		Blackboard->ClearValue(TEXT("TargetLocation"));
		MyMemory->OriginalTargetItem = nullptr;
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>();
	if (!Manager) return;
	
	if (!Manager->CanPickupItem(TargetItem))
	{
		AAO_Enemy_Crab* OwnerCrab = Manager->GetItemOwner(TargetItem);
		AO_LOG(LogKSJ, Warning, TEXT("[BTT_GrabItem] %s: TargetItem %s was picked up by %s during movement, aborting"), 
			*Crab->GetName(), *TargetItem->GetName(), OwnerCrab ? *OwnerCrab->GetName() : TEXT("Unknown"));
		Blackboard->ClearValue(GetSelectedBlackboardKey());
		Blackboard->ClearValue(TEXT("TargetLocation"));
		MyMemory->OriginalTargetItem = nullptr;
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	float DistanceSq = FVector::DistSquared(Crab->GetActorLocation(), TargetItem->GetActorLocation());
	float MaxGrabDistanceSq = 250.0f * 250.0f;
	
	if (DistanceSq <= MaxGrabDistanceSq)
	{
		Crab->GrabItem(TargetItem);
		
		if (Crab->HeldItemActor == TargetItem)
		{
			Blackboard->ClearValue(GetSelectedBlackboardKey());
			MyMemory->OriginalTargetItem = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			AO_LOG(LogKSJ, Warning, TEXT("[BTT_GrabItem] %s: Failed to grab %s (may be picked up by another Crab)"), 
				*Crab->GetName(), *TargetItem->GetName());
			Blackboard->ClearValue(GetSelectedBlackboardKey());
			Blackboard->ClearValue(TEXT("TargetLocation"));
			MyMemory->OriginalTargetItem = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}
