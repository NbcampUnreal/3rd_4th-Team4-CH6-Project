// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/AO_EQS_Context_PackMembers.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UAO_EQS_Context_PackMembers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	AAO_Werewolf* Wolf = nullptr;

	if (APawn* Pawn = Cast<APawn>(QueryOwner))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
	}
	else if (AController* Ctrl = Cast<AController>(QueryOwner))
	{
		Wolf = Cast<AAO_Werewolf>(Ctrl->GetPawn());
	}

	if (Wolf)
	{
		if (UAO_PackCoordComp* PackComp = Wolf->GetPackCoordComp())
		{
			TArray<AAO_Werewolf*> Members = PackComp->GetNearbyPackMembers();
			TArray<AActor*> MemberActors;
			
			for (AAO_Werewolf* Member : Members)
			{
				if (Member)
				{
					MemberActors.Add(Member);
				}
			}

			if (MemberActors.Num() > 0)
			{
				UEnvQueryItemType_Actor::SetContextHelper(ContextData, MemberActors);
			}
		}
	}
}
