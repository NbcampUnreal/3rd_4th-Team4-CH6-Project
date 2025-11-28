// KSJ : AO_BTS_CheckCrabState.cpp

#include "AI/BT/BTServices/AO_BTS_CheckCrabState.h"
#include "AI/Character/AO_Enemy_Crab.h"
#include "AI/Item/AO_PickupComponent.h"
#include "AI/Manager/AO_Manager_CrabPickupManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h" 
#include "AO_Log.h"

UAO_BTS_CheckCrabState::UAO_BTS_CheckCrabState()
{
	NodeName = TEXT("Check Crab State & Scan Items");
	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UAO_BTS_CheckCrabState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AAO_Enemy_Crab* Crab = Cast<AAO_Enemy_Crab>(AICon->GetPawn());
	if (!Crab) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AActor* HeldItem = Crab->HeldItemActor;
	bool bIsBurdened = (HeldItem != nullptr);

	Blackboard->SetValueAsBool(IsBurdenedKey.SelectedKeyName, bIsBurdened);
	Blackboard->SetValueAsObject(HeldItemKey.SelectedKeyName, HeldItem);

	double CurrentTime = World->GetTimeSeconds();
	
	if (CurrentTime - LastRerouteTime > 1.5)
	{
		AActor* TargetPlayer = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetPlayer")));
		if (TargetPlayer)
		{
			FVector CrabLoc = Crab->GetActorLocation();
			FVector PlayerLoc = TargetPlayer->GetActorLocation();
			float DistSq = FVector::DistSquared(CrabLoc, PlayerLoc);

			bool bShouldReroute = false;

			if (DistSq < 400.0f * 400.0f)
			{
				bShouldReroute = true;
			}
			else if (Blackboard->IsVectorValueSet(TEXT("TargetLocation")))
			{
				FVector TargetLoc = Blackboard->GetValueAsVector(TEXT("TargetLocation"));
				float DistPlayerToTargetSq = FVector::DistSquared(PlayerLoc, TargetLoc);

				if (DistPlayerToTargetSq < 500.0f * 500.0f)
				{
					bShouldReroute = true;
				}
				else
				{
					FVector DirToPlayer = (PlayerLoc - CrabLoc).GetSafeNormal();
					FVector MoveDir = (TargetLoc - CrabLoc).GetSafeNormal();
					float DotProd = FVector::DotProduct(DirToPlayer, MoveDir);

					if (DotProd > 0.5f && DistSq < 800.0f * 800.0f)
					{
						bShouldReroute = true;
					}
				}
			}

			if (bShouldReroute)
			{
				Blackboard->ClearValue(TEXT("TargetLocation"));
				LastRerouteTime = CurrentTime;
			}
		}
	}

	if (!bIsBurdened)
	{
		UObject* CurrentTargetObj = Blackboard->GetValueAsObject(TargetItemKey.SelectedKeyName);
		AActor* CurrentTargetActor = Cast<AActor>(CurrentTargetObj);
		
		AActor* ItemToExclude = nullptr;
		
		if (CurrentTargetActor)
		{
			UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>();
			if (Manager)
			{
				if (!Manager->CanPickupItem(CurrentTargetActor))
				{
					AAO_Enemy_Crab* OwnerCrab = Manager->GetItemOwner(CurrentTargetActor);
					AO_LOG(LogKSJ, Warning, TEXT("[Crab %s] TargetItem %s is already owned by %s, FORCING CLEAR"), 
						*Crab->GetName(), *CurrentTargetActor->GetName(), OwnerCrab ? *OwnerCrab->GetName() : TEXT("Unknown"));
					
					ItemToExclude = CurrentTargetActor;
					Blackboard->ClearValue(TargetItemKey.SelectedKeyName);
					Blackboard->ClearValue(TEXT("TargetLocation"));
					
					UObject* CheckObj = Blackboard->GetValueAsObject(TargetItemKey.SelectedKeyName);
					if (CheckObj)
					{
						AO_LOG(LogKSJ, Error, TEXT("[Crab %s] WARNING: TargetItem was NOT cleared! Still has: %s"), 
							*Crab->GetName(), *CheckObj->GetName());
					}
					else
					{
						AO_LOG(LogKSJ, Log, TEXT("[Crab %s] TargetItem successfully cleared"), *Crab->GetName());
					}
				}
				else
				{
					return;
				}
			}
			else
			{
				UAO_PickupComponent* PickupComp = CurrentTargetActor->FindComponentByClass<UAO_PickupComponent>();
				if (!PickupComp)
				{
					ItemToExclude = CurrentTargetActor;
					Blackboard->ClearValue(TargetItemKey.SelectedKeyName);
					Blackboard->ClearValue(TEXT("TargetLocation"));
				}
				else if (PickupComp->IsPickedUp() || PickupComp->HasTag(Crab->IgnoreItemTag) || PickupComp->IsInCooldown())
				{
					ItemToExclude = CurrentTargetActor;
					Blackboard->ClearValue(TargetItemKey.SelectedKeyName);
					Blackboard->ClearValue(TEXT("TargetLocation"));
				}
				else
				{
					return;
				}
			}
		}

		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Crab);
		if (ItemToExclude)
		{
			Params.AddIgnoredActor(ItemToExclude);
		}

		bool bHit = World->OverlapMultiByObjectType(
			Overlaps,
			Crab->GetActorLocation(),
			FQuat::Identity,
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
			FCollisionShape::MakeSphere(SearchRadius),
			Params
		);

		AActor* BestItem = nullptr;
		float MinDistanceSq = FLT_MAX;

		UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>();
		
		if (bHit)
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* ItemActor = Overlap.GetActor();
				if (!ItemActor) continue;
				
				if (ItemActor == ItemToExclude)
				{
					AO_LOG(LogKSJ, Verbose, TEXT("[Crab %s] Excluding recently cleared item %s from search"), 
						*Crab->GetName(), *ItemActor->GetName());
					continue;
				}

				UAO_PickupComponent* PickupComp = ItemActor->FindComponentByClass<UAO_PickupComponent>();
				if (!PickupComp) continue;

				if (Manager && !Manager->CanPickupItem(ItemActor))
				{
					continue;
				}
				
				if (!Manager)
				{
					if (PickupComp->IsPickedUp()) continue;
					if (PickupComp->IsInCooldown()) continue;
				}
				
				if (PickupComp->HasTag(Crab->IgnoreItemTag)) continue;

				float DistSq = FVector::DistSquared(Crab->GetActorLocation(), ItemActor->GetActorLocation());
				if (DistSq < MinDistanceSq)
				{
					MinDistanceSq = DistSq;
					BestItem = ItemActor;
				}
			}
		}

		if (BestItem)
		{
			Blackboard->SetValueAsObject(TargetItemKey.SelectedKeyName, BestItem);
		}
		else
		{
			Blackboard->ClearValue(TargetItemKey.SelectedKeyName);
			Blackboard->ClearValue(TEXT("TargetLocation"));
		}
	}
	else
	{
		Blackboard->ClearValue(TargetItemKey.SelectedKeyName);
		Blackboard->ClearValue(TEXT("TargetLocation"));
	}
}