//KSJ : AO_EQS_Context_AllPlayers

#include "AI/EQS/AO_EQS_Context_AllPlayers.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

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

	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), AllPlayers);

	// KSJ: 죽은 플레이어 필터링 (Status.Death 태그 확인)
	TArray<AActor*> AlivePlayers;
	static const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
	
	for (AActor* PlayerActor : AllPlayers)
	{
		if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(PlayerActor))
		{
			// Status.Death 태그가 없는 플레이어만 포함 (살아있는 플레이어)
			if (const UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				if (!ASC->HasMatchingGameplayTag(DeathTag))
				{
					AlivePlayers.Add(PlayerActor);
				}
			}
		}
	}

	if (AlivePlayers.Num() > 0)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, AlivePlayers);
	}
}

