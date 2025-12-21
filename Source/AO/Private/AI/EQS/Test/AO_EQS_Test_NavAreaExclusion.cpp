// AO_EQS_Test_NavAreaExclusion.cpp

#include "AI/EQS/Test/AO_EQS_Test_NavAreaExclusion.h"
#include "EnvironmentQuery/Items/EnvQueryItemType.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea.h"
#include "NavAreas/NavArea_Default.h"
#include "NavModifierVolume.h"
#include "AI/NavArea/AO_NavArea_SpawnRestriction.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"

UAO_EQS_Test_NavAreaExclusion::UAO_EQS_Test_NavAreaExclusion()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType::StaticClass();
	SetWorkOnFloatValues(false);
	
	// 기본값으로 스폰 금지 NavArea 설정
	RestrictedNavAreaClass = UAO_NavArea_SpawnRestriction::StaticClass();
}

void UAO_EQS_Test_NavAreaExclusion::RunTest(FEnvQueryInstance& QueryInstance) const
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

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		// NavSystem이 없으면 모든 위치를 통과시킴
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			It.SetScore(TestPurpose, FilterType, 1.0f, 0.0f);
		}
		return;
	}

	if (!RestrictedNavAreaClass)
	{
		// 제외할 NavArea가 지정되지 않았으면 모든 위치를 통과시킴
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

		// NavMesh에 프로젝션
		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(ItemLocation, NavLocation, FVector(500.0f, 500.0f, 200.0f)))
		{
			// NavMesh에 없으면 실패
			It.SetScore(TestPurpose, FilterType, 0.0f, 0.0f);
			continue;
		}

		// NavArea 확인
		// NavModifierVolume을 통해 설정된 NavArea를 확인
		// NavModifierVolume이 특정 AreaClass를 설정하면, 해당 볼륨과의 겹침을 체크
		bool bIsInRestrictedArea = false;
		
		// 모든 NavModifierVolume을 찾아서 RestrictedNavAreaClass를 가진 볼륨과 겹침 체크
		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(World, ANavModifierVolume::StaticClass(), FoundVolumes);
		
		for (AActor* VolumeActor : FoundVolumes)
		{
			if (ANavModifierVolume* NavVolume = Cast<ANavModifierVolume>(VolumeActor))
			{
				if (IsValid(NavVolume))
				{
					// NavModifierVolume의 AreaClass 확인
					TSubclassOf<UNavArea> VolumeAreaClass = NavVolume->GetAreaClass();
					
					// RestrictedNavAreaClass와 일치하는지 확인
					if (VolumeAreaClass == RestrictedNavAreaClass)
					{
						// 해당 볼륨 내부에 있는지 확인
						if (NavVolume->EncompassesPoint(ItemLocation))
						{
							bIsInRestrictedArea = true;
							break;
						}
					}
				}
			}
		}
		
		// 금지 NavArea 밖에 있으면 통과
		It.SetScore(TestPurpose, FilterType, bIsInRestrictedArea ? 0.0f : 1.0f, 0.0f);
	}
}

