// AO_StalkerController.cpp

#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQuery.h"

AAO_StalkerController::AAO_StalkerController()
{
}

void AAO_StalkerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

AAO_Stalker* AAO_StalkerController::GetStalker() const
{
	return Cast<AAO_Stalker>(GetPawn());
}

FVector AAO_StalkerController::FindHideLocation(float Radius, AActor* TargetToHideFrom)
{
	// EQS를 사용하여 엄폐물 찾기 (C++에서 EQS 직접 실행)
	// 실제로는 StateTree Task에서 EQS Query Asset을 실행하는 것이 더 일반적이나,
	// 여기서는 요청사항에 따라 로직적으로 구현 가능한 부분 설명.
	// EQS Query Asset (EQS_FindCover)가 있다고 가정하고, FEnvQueryRequest를 사용.
	
	// 여기서는 간단한 대체 로직: EnvironmentQuery를 사용하는 것을 권장.
	// StateTreeTask_Stalk_Hide에서 EQS를 실행하도록 설계하는 것이 좋음.
	
	return FVector::ZeroVector; 
}

FVector AAO_StalkerController::FindRetreatLocation()
{
	// 플레이어로부터 멀어지고 시야가 가려지는 곳
	return FVector::ZeroVector;
}

