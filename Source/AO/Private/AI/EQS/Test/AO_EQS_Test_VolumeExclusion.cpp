// AO_EQS_Test_VolumeExclusion.cpp

#include "AI/EQS/Test/AO_EQS_Test_VolumeExclusion.h"
#include "AI/Area/AO_Area_SpawnRestriction.h"
#include "EnvironmentQuery/Items/EnvQueryItemType.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"

UAO_EQS_Test_VolumeExclusion::UAO_EQS_Test_VolumeExclusion()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType::StaticClass();
	SetWorkOnFloatValues(false);
}

void UAO_EQS_Test_VolumeExclusion::RunTest(FEnvQueryInstance& QueryInstance) const
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

	// 스폰 금지 볼륨 찾기
	TArray<AActor*> FoundVolumes;
	if (RestrictionVolumeClass)
	{
		UGameplayStatics::GetAllActorsOfClass(World, RestrictionVolumeClass, FoundVolumes);
	}
	else
	{
		// 클래스가 지정되지 않았으면 기본 클래스 사용
		UGameplayStatics::GetAllActorsOfClass(World, AAO_Area_SpawnRestriction::StaticClass(), FoundVolumes);
	}

	if (FoundVolumes.Num() == 0)
	{
		// 금지 볼륨이 없으면 모든 위치를 통과시킴
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

		bool bIsInRestrictedVolume = false;

		// 모든 금지 볼륨과 겹침 체크
		for (AActor* VolumeActor : FoundVolumes)
		{
			if (AAO_Area_SpawnRestriction* RestrictionVolume = Cast<AAO_Area_SpawnRestriction>(VolumeActor))
			{
				if (IsValid(RestrictionVolume) && RestrictionVolume->EncompassesPoint(ItemLocation))
				{
					bIsInRestrictedVolume = true;
					break;
				}
			}
		}

		// 금지 볼륨 밖에 있으면 통과
		It.SetScore(TestPurpose, FilterType, bIsInRestrictedVolume ? 0.0f : 1.0f, 0.0f);
	}
}

