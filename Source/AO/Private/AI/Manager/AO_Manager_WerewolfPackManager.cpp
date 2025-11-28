// KSJ : AO_Manager_WerewolfPackManager.cpp

#include "AI/Manager/AO_Manager_WerewolfPackManager.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "NavMesh/RecastNavMesh.h"
#include "AO_Log.h" // LogKSJ

void UAO_Manager_WerewolfPackManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AO_LOG(LogKSJ, Log, TEXT("[WerewolfManager] Initialized"));
}

void UAO_Manager_WerewolfPackManager::Deinitialize()
{
	for (auto& Elem : ActiveSieges)
	{
		UWorld* World = GetWorld();
		if (World && Elem.Value.RushTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(Elem.Value.RushTimerHandle);
		}
	}
	ActiveSieges.Empty();
	RegisteredWerewolves.Empty();

	Super::Deinitialize();
}

void UAO_Manager_WerewolfPackManager::RegisterWerewolf(AAO_AIC_Werewolf* WerewolfAIC)
{
	if (WerewolfAIC && !RegisteredWerewolves.Contains(WerewolfAIC))
	{
		RegisteredWerewolves.Add(WerewolfAIC);
	}
}

void UAO_Manager_WerewolfPackManager::UnregisterWerewolf(AAO_AIC_Werewolf* WerewolfAIC)
{
		if (RegisteredWerewolves.Contains(WerewolfAIC))
		{
			RegisteredWerewolves.Remove(WerewolfAIC);
			
			for (auto It = ActiveSieges.CreateIterator(); It; ++It)
		{
			FPackSiegeInfo& Info = It.Value();
			if (Info.AssignedWerewolves.Contains(WerewolfAIC))
			{
				Info.AssignedWerewolves.Remove(WerewolfAIC);
				
				if (Info.AssignedWerewolves.Num() == 0)
				{
					UWorld* World = GetWorld();
					if (World)
					{
						World->GetTimerManager().ClearTimer(Info.RushTimerHandle);
					}
					It.RemoveCurrent();
				}
			}
		}
	}
}

bool UAO_Manager_WerewolfPackManager::ReportPlayerDetection(AAO_AIC_Werewolf* Howler, AActor* TargetActor)
{
	if (!Howler || !TargetActor) return false;

	if (IsTargetAlreadyMarked(TargetActor))
	{
		FPackSiegeInfo* Info = ActiveSieges.Find(TargetActor);
		if (Info && !Info->AssignedWerewolves.Contains(Howler))
		{
			Info->AssignedWerewolves.Add(Howler);
		}
		return false;
	}
	AO_LOG(LogKSJ, Warning, TEXT("[WerewolfManager] NEW Siege Started on Target: %s by %s"), *TargetActor->GetName(), *Howler->GetName());

	FPackSiegeInfo NewSiege;
	NewSiege.TargetPlayer = TargetActor;
	NewSiege.AssignedWerewolves.Add(Howler);
		NewSiege.SiegeStartTime = FPlatformTime::Seconds();

	const FVector HowlLocation = Howler->GetPawn() ? Howler->GetPawn()->GetActorLocation() : FVector::ZeroVector;

	for (AAO_AIC_Werewolf* Wolf : RegisteredWerewolves)
	{
		if (Wolf == Howler) continue; // 본인 제외
		if (!Wolf->GetPawn()) continue;

		float DistSq = FVector::DistSquared(Wolf->GetPawn()->GetActorLocation(), HowlLocation);
		if (DistSq <= (HowlPropagationRadius * HowlPropagationRadius))
		{
			NewSiege.AssignedWerewolves.Add(Wolf);
			Wolf->ReceiveHowlSignal(HowlLocation, TargetActor);
		}
	}

	ActiveSieges.Add(TargetActor, NewSiege);

	if (NewSiege.AssignedWerewolves.Num() > 1)
	{
		LastEscapeRouteCalcTime.Add(TargetActor, FPlatformTime::Seconds());
		AssignSiegeSlots(TargetActor);
	}

	if (NewSiege.AssignedWerewolves.Num() <= 1)
	{
		StartCoordinatedAttack(TargetActor);
	}
	else
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerDelegate TimerDel;
			TimerDel.BindUObject(this, &UAO_Manager_WerewolfPackManager::StartCoordinatedAttack, TargetActor);
			
			if (ActiveSieges.Contains(TargetActor))
			{
				World->GetTimerManager().SetTimer(ActiveSieges[TargetActor].RushTimerHandle, TimerDel, SiegeDuration, false);
			}
		}
	}

	return true;
}

bool UAO_Manager_WerewolfPackManager::IsTargetAlreadyMarked(AActor* TargetActor) const
{
	return ActiveSieges.Contains(TargetActor);
}

EWerewolfPackState UAO_Manager_WerewolfPackManager::GetPackStateFor(AAO_AIC_Werewolf* RequestingWolf) const
{
	if (!RequestingWolf) return EWerewolfPackState::Idle;

	for (const auto& Elem : ActiveSieges)
	{
		const FPackSiegeInfo& Info = Elem.Value;
		if (Info.AssignedWerewolves.Contains(RequestingWolf))
		{
			UWorld* World = GetWorld();
			if (World && World->GetTimerManager().IsTimerActive(Info.RushTimerHandle))
			{
				return EWerewolfPackState::Siege;
			}
			else
			{
				return EWerewolfPackState::Rush;
			}
		}
	}

	return EWerewolfPackState::Idle;
}

AActor* UAO_Manager_WerewolfPackManager::GetSiegeTargetFor(AAO_AIC_Werewolf* RequestingWolf) const
{
	for (const auto& Elem : ActiveSieges)
	{
		if (Elem.Value.AssignedWerewolves.Contains(RequestingWolf))
		{
			return Elem.Value.TargetPlayer;
		}
	}
	return nullptr;
}

void UAO_Manager_WerewolfPackManager::OnTargetSwitched(AAO_AIC_Werewolf* Wolf, AActor* OldTarget, AActor* NewTarget)
{
	if (!Wolf) return;

	if (OldTarget && ActiveSieges.Contains(OldTarget))
	{
		FPackSiegeInfo& OldInfo = ActiveSieges[OldTarget];
		OldInfo.AssignedWerewolves.Remove(Wolf);
		
		if (OldInfo.AssignedWerewolves.Num() == 0)
		{
			GetWorld()->GetTimerManager().ClearTimer(OldInfo.RushTimerHandle);
			ActiveSieges.Remove(OldTarget);
		}
	}

	if (NewTarget)
	{
		if (ActiveSieges.Contains(NewTarget))
		{
			FPackSiegeInfo& NewInfo = ActiveSieges[NewTarget];
			if (!NewInfo.AssignedWerewolves.Contains(Wolf))
			{
				NewInfo.AssignedWerewolves.Add(Wolf);
			}
		}
		else
		{
			ReportPlayerDetection(Wolf, NewTarget);
		}
	}
}

void UAO_Manager_WerewolfPackManager::StartCoordinatedAttack(AActor* TargetActor)
{
	if (!TargetActor || !ActiveSieges.Contains(TargetActor)) return;

	FPackSiegeInfo& Info = ActiveSieges[TargetActor];
	
	AO_LOG(LogKSJ, Warning, TEXT("[WerewolfManager] RUSH TRIGGERED! Target: %s, Wolves Count: %d"), *TargetActor->GetName(), Info.AssignedWerewolves.Num());

	GetWorld()->GetTimerManager().ClearTimer(Info.RushTimerHandle);
}

TArray<FVector> UAO_Manager_WerewolfPackManager::CalculateEscapeRoutes(AActor* TargetActor) const
{
	TArray<FVector> EscapeRoutes;
	if (!TargetActor) return EscapeRoutes;

	FVector TargetLocation = TargetActor->GetActorLocation();
	FRotator TargetRotation = TargetActor->GetActorRotation();

	UWorld* World = GetWorld();
	if (!World) return EscapeRoutes;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys) return EscapeRoutes;

	struct FEscapeRouteCandidate
	{
		FVector Direction;
		float PriorityScore; // 정면일수록 높음
		float DistanceScore; // 멀리 갈 수 있을수록 높음
	};

	TArray<FEscapeRouteCandidate> Candidates;

	for (float Angle = 0.0f; Angle < 360.0f; Angle += EscapeRouteAngleStep)
	{
		FRotator RotatedRot = TargetRotation + FRotator(0.0f, Angle, 0.0f);
		FVector Direction = RotatedRot.Vector();
		FVector EndLocation = TargetLocation + (Direction * EscapeRouteCheckDistance);

		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::LineTraceSingle(
			World,
			TargetLocation,
			EndLocation,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::None,
			HitResult,
			true
		);

		if (bHit && HitResult.Distance < EscapeRouteCheckDistance * 0.5f)
		{
			continue;
		}

		FNavLocation NavLocation;
		bool bNavValid = NavSys->ProjectPointToNavigation(EndLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
		if (!bNavValid)
		{
			continue;
		}

		FEscapeRouteCandidate Candidate;
		Candidate.Direction = Direction.GetSafeNormal();
		
		float AngleFromForward = FMath::Abs(Angle);
		if (AngleFromForward > 180.0f) AngleFromForward = 360.0f - AngleFromForward;
		Candidate.PriorityScore = 1.0f - (AngleFromForward / 180.0f);
		
		float ActualDistance = bHit ? HitResult.Distance : EscapeRouteCheckDistance;
		Candidate.DistanceScore = ActualDistance / EscapeRouteCheckDistance;

		Candidates.Add(Candidate);
	}

	Candidates.Sort([](const FEscapeRouteCandidate& A, const FEscapeRouteCandidate& B)
	{
		float ScoreA = A.PriorityScore * 2.0f + A.DistanceScore;
		float ScoreB = B.PriorityScore * 2.0f + B.DistanceScore;
		return ScoreA > ScoreB;
	});

	int32 MaxRoutes = FMath::Min(8, Candidates.Num());
	for (int32 i = 0; i < MaxRoutes; i++)
	{
		EscapeRoutes.Add(Candidates[i].Direction);
	}

	AO_LOG(LogKSJ, Log, TEXT("[PackManager] Calculated %d escape routes for %s"), EscapeRoutes.Num(), *TargetActor->GetName());

	return EscapeRoutes;
}

void UAO_Manager_WerewolfPackManager::AssignSiegeSlots(AActor* TargetActor)
{
	FPackSiegeInfo* Info = ActiveSieges.Find(TargetActor);
	if (!Info) return;

	Info->EscapeRouteDirections = CalculateEscapeRoutes(TargetActor);
	
	TArray<int32> UsedSlots;
	for (const auto& Pair : Info->SlotAssignment)
	{
		if (Pair.Value >= 0 && Info->AssignedWerewolves.Contains(Pair.Key))
		{
			UsedSlots.Add(Pair.Value);
		}
	}

	TArray<AAO_AIC_Werewolf*> NewWolves;
	for (AAO_AIC_Werewolf* Wolf : Info->AssignedWerewolves)
	{
		if (!Info->SlotAssignment.Contains(Wolf) || Info->SlotAssignment[Wolf] < 0)
		{
			NewWolves.Add(Wolf);
		}
	}

	int32 SlotIndex = 0;
	for (AAO_AIC_Werewolf* NewWolf : NewWolves)
	{
		while (SlotIndex < Info->EscapeRouteDirections.Num() && UsedSlots.Contains(SlotIndex))
		{
			SlotIndex++;
		}

		if (SlotIndex < Info->EscapeRouteDirections.Num())
		{
			Info->SlotAssignment.Add(NewWolf, SlotIndex);
			UsedSlots.Add(SlotIndex);
			SlotIndex++;
		}
		else
		{
			Info->SlotAssignment.Add(NewWolf, -1);
		}
	}

	for (AAO_AIC_Werewolf* Wolf : Info->AssignedWerewolves)
	{
		int32* CurrentSlot = Info->SlotAssignment.Find(Wolf);
		if (CurrentSlot && (*CurrentSlot >= Info->EscapeRouteDirections.Num()))
		{
			*CurrentSlot = -1;
		}
	}

	AO_LOG(LogKSJ, Log, TEXT("[PackManager] Assigned Slots for %s: Routes=%d, Wolves=%d, New=%d"), 
		*TargetActor->GetName(), Info->EscapeRouteDirections.Num(), Info->AssignedWerewolves.Num(), NewWolves.Num());
}

bool UAO_Manager_WerewolfPackManager::GetAssignedSlotDirection(AAO_AIC_Werewolf* RequestingWolf, FVector& OutDirection) const
{
	if (!RequestingWolf) return false;

	for (const auto& Elem : ActiveSieges)
	{
		const FPackSiegeInfo& Info = Elem.Value;
		if (Info.AssignedWerewolves.Contains(RequestingWolf))
		{
			const int32* SlotIndexPtr = Info.SlotAssignment.Find(RequestingWolf);
			if (SlotIndexPtr && *SlotIndexPtr >= 0 && *SlotIndexPtr < Info.EscapeRouteDirections.Num())
			{
				OutDirection = Info.EscapeRouteDirections[*SlotIndexPtr];
				return true;
			}
		}
	}

	return false;
}

void UAO_Manager_WerewolfPackManager::UpdateSiegeFormation(AActor* TargetActor)
{
	FPackSiegeInfo* Info = ActiveSieges.Find(TargetActor);
	if (!Info || Info->AssignedWerewolves.Num() <= 1) return;

	double* LastCalcTime = LastEscapeRouteCalcTime.Find(TargetActor);
	double CurrentTime = FPlatformTime::Seconds();
	
	if (LastCalcTime && (CurrentTime - *LastCalcTime) < EscapeRouteRecalcInterval)
	{
		return;
	}

	LastEscapeRouteCalcTime.Add(TargetActor, CurrentTime);
	AssignSiegeSlots(TargetActor);
}

void UAO_Manager_WerewolfPackManager::UpdateSiegeFormationFor(AAO_AIC_Werewolf* RequestingWolf)
{
	AActor* Target = GetSiegeTargetFor(RequestingWolf);
	if (Target)
	{
		UpdateSiegeFormation(Target);
	}
}
