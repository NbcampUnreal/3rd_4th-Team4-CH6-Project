// AO_EQS_Context_AllPlayers.cpp

#include "AI/EQS/AO_EQS_Context_AllPlayers.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AO_PlayerCharacter.h"
#include "AO_Log.h"

void UAO_EQS_Context_AllPlayers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), Players);

	if (Players.Num() > 0)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Players);
	}
	else
	{
		// 플레이어가 없으면 Owner(AI 자신)라도 넣어서 쿼리가 실패하지 않도록 처리할 수도 있으나,
		// 여기서는 문맥상 플레이어가 없으면 결과가 없는 것이 맞음.
		// AO_LOG(LogKSJ, Warning, TEXT("AO_EQS_Context_AllPlayers: No players found."));
	}
}

