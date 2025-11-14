// AO_EQC_PackTargetLocationFromBB.cpp

#include "AI/EQS/Contexts/AO_EQC_PackTargetLocationFromBB.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Pawn.h"

UAO_EQC_PackTargetLocationFromBB::UAO_EQC_PackTargetLocationFromBB()
{
	PackTargetLocationBBKeyName = TEXT("PackTargetLocation");
}

void UAO_EQC_PackTargetLocationFromBB::ProvideContext(FEnvQueryInstance& QueryInstance,
													  FEnvQueryContextData& ContextData) const
{
	const UObject* OwnerObj = QueryInstance.Owner.Get();
	const AActor* OwnerActor = Cast<AActor>(OwnerObj);

	const AAIController* AICon = nullptr;

	if(const AAIController* AsAICon = Cast<AAIController>(OwnerActor))
	{
		AICon = AsAICon;
	}
	else if(const APawn* AsPawn = Cast<APawn>(OwnerActor))
	{
		AICon = Cast<AAIController>(AsPawn->GetController());
	}

	const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
	if(BB == nullptr || PackTargetLocationBBKeyName.IsNone())
	{
		return;
	}

	const FVector Location =
		BB->GetValueAsVector(PackTargetLocationBBKeyName);

	if(Location.IsNearlyZero())
	{
		return;
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Location);
}
