// KSJ : AO_BTS_InsectUpdateKidnapState.cpp

#include "AI/BT/BTServices/AO_BTS_InsectUpdateKidnapState.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO_Log.h"

UAO_BTS_InsectUpdateKidnapState::UAO_BTS_InsectUpdateKidnapState()
{
	NodeName = TEXT("Update Kidnap State");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UAO_BTS_InsectUpdateKidnapState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAO_AIC_Insect* AIC = Cast<AAO_AIC_Insect>(OwnerComp.GetAIOwner());
	if (!AIC) return;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect) return;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	bool bIsKidnapping = Insect->bIsKidnapping;
	BB->SetValueAsBool(bIsKidnappingKey.SelectedKeyName, bIsKidnapping);
	
	if (bIsKidnapping)
	{
		if (Insect->KidnappedPlayer)
		{
			BB->SetValueAsObject(KidnappedPlayerKey.SelectedKeyName, Insect->KidnappedPlayer);
		}
		
		FVector StartLocation = BB->GetValueAsVector(KidnapStartLocationKey.SelectedKeyName);
		if (StartLocation.IsNearlyZero())
		{
			StartLocation = Insect->GetActorLocation();
			BB->SetValueAsVector(KidnapStartLocationKey.SelectedKeyName, StartLocation);
		}
		
		UWorld* World = GetWorld();
		if (World)
		{
			double CurrentTime = World->GetTimeSeconds();
			if (CurrentTime - LastTargetUpdateTime >= TargetLocationUpdateInterval)
			{
				LastTargetUpdateTime = CurrentTime;
				
				TArray<AActor*> OverlappedActors;
				TArray<AActor*> ActorsToIgnore;
				ActorsToIgnore.Add(Insect);
				if (Insect->KidnappedPlayer)
				{
					ActorsToIgnore.Add(Insect->KidnappedPlayer);
				}
				
				bool bHit = UKismetSystemLibrary::SphereOverlapActors(
					World,
					Insect->GetActorLocation(),
					OtherPlayerDetectionRadius,
					TArray<TEnumAsByte<EObjectTypeQuery>>(),
					APawn::StaticClass(),
					ActorsToIgnore,
					OverlappedActors
				);
				
				if (bHit)
				{
					AO_LOG(LogKSJ, Verbose, TEXT("[Insect] Other player detected, target location should be updated"));
				}
			}
		}
	}
	else
	{
		BB->ClearValue(KidnappedPlayerKey.SelectedKeyName);
		BB->ClearValue(KidnapStartLocationKey.SelectedKeyName);
		BB->ClearValue(KidnapTargetLocationKey.SelectedKeyName);
	}
}
