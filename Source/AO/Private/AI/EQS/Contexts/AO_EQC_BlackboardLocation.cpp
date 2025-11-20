// AO_EQC_BlackboardLocation.cpp

#include "AI/EQS/Contexts/AO_EQC_BlackboardLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UAO_EQC_BlackboardLocation::ProvideContext(FEnvQueryInstance& QueryInstance,
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

	FVector ContextLocation = FVector::ZeroVector;
	bool bHasLocation = false;

	if(BB != nullptr && LocationBBKeyName.IsNone() == false)
	{
		const FVector FromBB = BB->GetValueAsVector(LocationBBKeyName);
		if(FromBB.IsNearlyZero() == false)
		{
			ContextLocation = FromBB;
			bHasLocation = true;
		}
	}
	
	if(!bHasLocation)
	{
		const AActor* QuerierActor = OwnerActor;
		if(QuerierActor != nullptr)
		{
			ContextLocation = QuerierActor->GetActorLocation();
			bHasLocation = true;
		}
	}

	if(bHasLocation)
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, ContextLocation);
	}
}
