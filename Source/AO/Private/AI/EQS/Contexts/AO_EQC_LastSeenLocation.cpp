// AO_EQC_LastSeenLocation.cpp

#include "AI/EQS/Contexts/AO_EQC_LastSeenLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Pawn.h"

void UAO_EQC_LastSeenLocation::ProvideContext(FEnvQueryInstance& QueryInstance,
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

	const UBlackboardComponent* BB =
		AICon ? AICon->GetBlackboardComponent() : nullptr;

	FVector LastSeenLocation = FVector::ZeroVector;
	bool bHasLocation = false;

	if(BB != nullptr)
	{
		static const FName LastSeenKeyName(TEXT("LastSeenLocation"));

		const FVector Value = BB->GetValueAsVector(LastSeenKeyName);
		if(Value.IsNearlyZero() == false)
		{
			LastSeenLocation = Value;
			bHasLocation = true;
		}
	}
	
	if(!bHasLocation && OwnerActor != nullptr)
	{
		LastSeenLocation = OwnerActor->GetActorLocation();
		bHasLocation = true;
	}

	if(bHasLocation)
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, LastSeenLocation);
	}
}
