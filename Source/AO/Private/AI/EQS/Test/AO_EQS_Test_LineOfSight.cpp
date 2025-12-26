// AO_EQS_Test_LineOfSight.cpp

#include "AI/EQS/Test/AO_EQS_Test_LineOfSight.h"
#include "EnvironmentQuery/Items/EnvQueryItemType.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "AI/EQS/AO_EQS_Context_AllPlayers.h"
#include "Character/AO_PlayerCharacter.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"

UAO_EQS_Test_LineOfSight::UAO_EQS_Test_LineOfSight()
{
	Cost = EEnvTestCost::Medium;
	ValidItemType = UEnvQueryItemType::StaticClass();
	SetWorkOnFloatValues(false);
}

void UAO_EQS_Test_LineOfSight::RunTest(FEnvQueryInstance& QueryInstance) const
{
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

	// 모든 플레이어 가져오기
	TArray<AAO_PlayerCharacter*> AllPlayers;
	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), FoundPlayers);
	
	for (AActor* Actor : FoundPlayers)
	{
		if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor))
		{
			AllPlayers.Add(Player);
		}
	}

	if (AllPlayers.Num() == 0)
	{
		// 플레이어가 없으면 모든 위치를 통과시킴
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			It.SetScore(TestPurpose, FilterType, 1.0f, 0.0f);
		}
		return;
	}

	// 각 아이템에 대해 테스트
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		ItemLocation.Z += SpawnLocationHeightOffset;

		bool bAllPlayersCannotSee = true;

		// 모든 플레이어에 대해 Line Trace 수행
		for (AAO_PlayerCharacter* Player : AllPlayers)
		{
			if (!IsValid(Player))
			{
				continue;
			}

			// 플레이어 눈 위치 계산
			FVector PlayerEyeLocation = Player->GetActorLocation();
			PlayerEyeLocation.Z += PlayerEyeHeightOffset;

			// Line Trace 수행
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Player);
			if (AActor* QueryOwnerActor = Cast<AActor>(QueryOwner))
			{
				QueryParams.AddIgnoredActor(QueryOwnerActor);
			}

			bool bHit = World->LineTraceSingleByChannel(
				HitResult,
				PlayerEyeLocation,
				ItemLocation,
				TraceChannel,
				QueryParams
			);

			if (!bHit || HitResult.Distance > FVector::Dist(PlayerEyeLocation, ItemLocation) * 0.95f)
			{
				// 막히지 않았거나 거의 막히지 않음 = 플레이어가 볼 수 있음
				bAllPlayersCannotSee = false;
				break;
			}
		}

		// 모든 플레이어가 못 보면 통과
		It.SetScore(TestPurpose, FilterType, bAllPlayersCannotSee ? 1.0f : 0.0f, 0.0f);
	}
}

