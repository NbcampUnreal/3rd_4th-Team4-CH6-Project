// KSJ : AO_EQC_KidnapEscapeLocation.cpp

#include "AI/EQS/Contexts/AO_EQC_KidnapEscapeLocation.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "AO_Log.h"

UAO_EQC_KidnapEscapeLocation::UAO_EQC_KidnapEscapeLocation()
{
}

void UAO_EQC_KidnapEscapeLocation::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QueryOwner) return;
	
	AAO_AIC_Insect* AIC = nullptr;
	
	if (AAIController* Controller = Cast<AAIController>(QueryOwner))
	{
		AIC = Cast<AAO_AIC_Insect>(Controller);
	}
	else if (APawn* Pawn = Cast<APawn>(QueryOwner))
	{
		AIC = Cast<AAO_AIC_Insect>(Pawn->GetController());
	}
	
	if (!AIC) return;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->bIsKidnapping) return;
	
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return;
	
	FVector KidnapStartLocation = BB->GetValueAsVector(TEXT("KidnapStartLocation"));
	if (KidnapStartLocation.IsNearlyZero())
	{
		KidnapStartLocation = Insect->GetActorLocation();
	}
	
	TArray<FVector> OtherPlayerLocations;
	UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC)
			{
				APawn* PlayerPawn = PC->GetPawn();
				if (PlayerPawn && PlayerPawn != Insect->KidnappedPlayer)
				{
					OtherPlayerLocations.Add(PlayerPawn->GetActorLocation());
				}
			}
		}
	}
	
	FVector CurrentLocation = Insect->GetActorLocation();
	
	FVector Direction = (CurrentLocation - KidnapStartLocation).GetSafeNormal();
	FVector EscapeLocation = CurrentLocation + (Direction * 1000.0f);
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, EscapeLocation);
}
