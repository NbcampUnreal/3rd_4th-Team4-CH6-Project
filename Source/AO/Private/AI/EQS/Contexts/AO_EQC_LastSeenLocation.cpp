// KSJ : AO_EQC_LastSeenLocation.cpp

#include "AI/EQS/Contexts/AO_EQC_LastSeenLocation.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AO_Log.h"

UAO_EQC_LastSeenLocation::UAO_EQC_LastSeenLocation()
{
}

void UAO_EQC_LastSeenLocation::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QueryOwner) return;

	// 쿼리 주체(Querier)가 AIController인지 Pawn인지 확인
	AAIController* AICon = Cast<AAIController>(QueryOwner);
	if (!AICon)
	{
		// Pawn이라면 Controller를 가져옴
		if (APawn* Pawn = Cast<APawn>(QueryOwner))
		{
			AICon = Cast<AAIController>(Pawn->GetController());
		}
	}

	if (AICon && AICon->GetBlackboardComponent())
	{
		// 블랙보드에서 벡터 값 가져오기 (KeyName: LastKnownPlayerLocation)
		// 주의: Blackboard Key 이름이 틀리면 0,0,0이 반환됨
		FVector LastLocation = AICon->GetBlackboardComponent()->GetValueAsVector(TEXT("LastKnownPlayerLocation"));
		
		if (LastLocation != FVector::ZeroVector)
		{
			UEnvQueryItemType_Point::SetContextHelper(ContextData, LastLocation);
		}
	}
}
