//KSJ : AO_EQS_Context_AllPlayers

#include "AI/EQS/AO_EQS_Context_AllPlayers.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AO_PlayerCharacter.h"

void UAO_EQS_Context_AllPlayers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// QueryInstance의 Owner에서 World 가져오기 (올바른 방법)
	// GetWorld()를 직접 호출하면 CDO에서 nullptr을 반환할 수 있음
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
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
}

