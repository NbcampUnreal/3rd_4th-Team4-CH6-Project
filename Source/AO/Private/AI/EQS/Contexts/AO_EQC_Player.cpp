// KSJ : AO_EQC_Player.cpp

#include "AI/EQS/Contexts/AO_EQC_Player.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UAO_EQC_Player::UAO_EQC_Player()
{
}

void UAO_EQC_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> ResultActors;

	// 모든 플레이어 컨트롤러 순회 (멀티플레이어 대응)
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			APawn* MyPawn = PC->GetPawn();
			if (MyPawn)
			{
				// 살아있는지 확인 (필요하다면 Health 체크 추가 가능)
				ResultActors.AddUnique(MyPawn);
			}
		}
	}

	// EQS에 결과 전달
	if (ResultActors.Num() > 0)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultActors);
	}
}
