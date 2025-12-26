// AO_Spawner_AIManager.cpp

#include "AI/Spawner/AO_Spawner_AIManager.h"
#include "AI/Area/AO_Area_SpawnRestriction.h"
#include "AI/Area/AO_Area_SpawnIntensive.h"
#include "AI/NavArea/AO_NavArea_SpawnIntensive.h"
#include "Character/AO_PlayerCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavModifierVolume.h"
#include "AIController.h"
#include "AO_Log.h"

AAO_Spawner_AIManager::AAO_Spawner_AIManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AAO_Spawner_AIManager::BeginPlay()
{
	Super::BeginPlay();

	// PIE 모드에서는 항상 실행, 멀티플레이어에서는 서버에서만
	bool bShouldSpawn = HasAuthority() || GetWorld()->IsPlayInEditor();
	
	if (!bShouldSpawn)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Not running on server (HasAuthority: %d, IsPIE: %d)"), HasAuthority(), GetWorld()->IsPlayInEditor());
		return;
	}

	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: BeginPlay - Starting initialization"));

	// 플레이어 목록 초기화
	UpdatePlayerList();
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Found %d players"), CachedPlayers.Num());

	// 첫 스폰 시도까지 대기 시간 계산
	float InitialDelay = FMath::Max(0.1f, SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation));
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, InitialDelay, false);
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: First spawn scheduled in %.2f seconds"), InitialDelay);

	// 집중 스폰 영역 초기화 (볼륨 기반)
	for (TObjectPtr<AAO_Area_SpawnIntensive> IntensiveArea : IntensiveVolumes)
	{
		if (IsValid(IntensiveArea) && IntensiveArea->bEnabled)
		{
			float IntensiveDelay = FMath::Max(0.1f, IntensiveArea->IntensiveSpawnInterval + FMath::RandRange(-IntensiveArea->IntensiveSpawnIntervalRandomDeviation, IntensiveArea->IntensiveSpawnIntervalRandomDeviation));
			FTimerHandle IntensiveTimerHandle;
			GetWorldTimerManager().SetTimer(IntensiveTimerHandle, [this, IntensiveArea]()
			{
				TrySpawnInIntensiveArea(IntensiveArea);
			}, IntensiveDelay, false); // false로 설정하고, TrySpawnInIntensiveArea에서 다음 타이머 재설정
			IntensiveSpawnTimers.Add(IntensiveArea, IntensiveTimerHandle);
		}
	}

	// NavArea 기반 집중 스폰은 별도 타이머로 처리하지 않고,
	// TrySpawnAI에서 NavArea를 확인하여 처리

	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Initialized with %d monster classes, max %d monsters, EQS Query: %s"), 
		MonsterClasses.Num(), MaxMonstersInLevel, IsValid(SpawnLocationQuery) ? *SpawnLocationQuery->GetName() : TEXT("NULL"));
}

void AAO_Spawner_AIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);

		for (auto& Pair : IntensiveSpawnTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
		IntensiveSpawnTimers.Empty();
	}

	// 스폰된 AI들의 OnDestroyed 바인딩 해제
	for (TObjectPtr<AAO_AICharacterBase> SpawnedMonster : SpawnedMonsters)
	{
		if (IsValid(SpawnedMonster))
		{
			SpawnedMonster->OnDestroyed.RemoveDynamic(this, &AAO_Spawner_AIManager::OnAIDestroyed);
		}
	}
	SpawnedMonsters.Empty();

	Super::EndPlay(EndPlayReason);
}

void AAO_Spawner_AIManager::TrySpawnAI()
{
	bool bShouldSpawn = HasAuthority() || GetWorld()->IsPlayInEditor();
	if (!bShouldSpawn)
	{
		return;
	}

	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: TrySpawnAI called"));

	// 최대 수 체크
	int32 CurrentCount = GetCurrentSpawnedAICount();
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Current monster count: %d/%d"), CurrentCount, MaxMonstersInLevel);
	if (CurrentCount >= MaxMonstersInLevel)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Max monsters reached (%d/%d)"), CurrentCount, MaxMonstersInLevel);
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	// 몬스터 종류 체크
	if (MonsterClasses.Num() == 0)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: No monster classes configured"));
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	// EQS 쿼리 실행
	if (!IsValid(SpawnLocationQuery))
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: SpawnLocationQuery is not set"));
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	// 플레이어 목록 업데이트
	UpdatePlayerList();
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Found %d players"), CachedPlayers.Num());
	if (CachedPlayers.Num() == 0)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: No players found, skipping spawn"));
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	// EQS 쿼리 실행
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Executing EQS query: %s"), *SpawnLocationQuery->GetName());
	FEnvQueryRequest QueryRequest(SpawnLocationQuery, this);
	QueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &AAO_Spawner_AIManager::OnEQSQueryFinished);
}

void AAO_Spawner_AIManager::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: EQS query finished - Valid: %d, Items: %d"), 
		Result.IsValid() ? 1 : 0, Result.IsValid() ? Result->Items.Num() : 0);
	
	if (!Result.IsValid() || Result->Items.Num() == 0)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: EQS query failed or no valid locations found"));
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	// 최고 점수 위치 선택 (첫 번째 아이템 사용, 또는 점수가 있는 경우 최고 점수 아이템 찾기)
	int32 BestItemIndex = INDEX_NONE;
	float BestScore = -FLT_MAX;
	
	for (int32 i = 0; i < Result->Items.Num(); ++i)
	{
		float ItemScore = Result->GetItemScore(i);
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Item[%d] Score: %.2f"), i, ItemScore);
		if (ItemScore > BestScore)
		{
			BestScore = ItemScore;
			BestItemIndex = i;
		}
	}
	
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Best item index: %d, Score: %.2f"), BestItemIndex, BestScore);
	
	if (BestItemIndex == INDEX_NONE)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: No valid item found in EQS result"));
		
		// 다음 스폰 시도 예약
		float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
		return;
	}

	FVector SpawnLocation = Result->GetItemAsLocation(BestItemIndex);

	// NavMesh에 프로젝션
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(SpawnLocation, ProjectedLocation, FVector(500.0f, 500.0f, 200.0f)))
		{
			SpawnLocation = ProjectedLocation.Location;
		}
	}

	// 몬스터 종류 랜덤 선택
	TSubclassOf<AAO_AICharacterBase> SelectedClass = MonsterClasses[FMath::RandRange(0, MonsterClasses.Num() - 1)];

	// 스폰 실행
	ExecuteSpawn(SpawnLocation, SelectedClass);

	// 다음 스폰 시도 예약
	float NextDelay = SpawnInterval + FMath::RandRange(-SpawnIntervalRandomDeviation, SpawnIntervalRandomDeviation);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::TrySpawnAI, NextDelay, false);
}

void AAO_Spawner_AIManager::ExecuteSpawn(const FVector& SpawnLocation, TSubclassOf<AAO_AICharacterBase> MonsterClass)
{
	if (!IsValid(MonsterClass))
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Invalid monster class"));
		return;
	}

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;

	// 스폰 실행
	FRotator SpawnRotation = FRotator::ZeroRotator;
	AAO_AICharacterBase* SpawnedMonster = GetWorld()->SpawnActor<AAO_AICharacterBase>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (IsValid(SpawnedMonster))
	{
		SpawnedMonsters.Add(SpawnedMonster);
		SpawnedMonster->OnDestroyed.AddDynamic(this, &AAO_Spawner_AIManager::OnAIDestroyed);
		
		// Controller가 없으면 생성하고 Possess
		if (!SpawnedMonster->GetController())
		{
			// Character의 AIControllerClass를 사용하여 Controller 생성
			TSubclassOf<AController> ControllerClass = SpawnedMonster->AIControllerClass;
			if (!ControllerClass)
			{
				// AIControllerClass가 설정되지 않았으면 기본 AIController 사용
				ControllerClass = AAIController::StaticClass();
				AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: %s has no AIControllerClass, using default AAIController"), *SpawnedMonster->GetName());
			}
			
			// Controller 생성
			FActorSpawnParameters ControllerSpawnParams;
			ControllerSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			AController* NewController = GetWorld()->SpawnActor<AController>(ControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, ControllerSpawnParams);
			if (IsValid(NewController))
			{
				// Controller가 Pawn을 Possess
				NewController->Possess(SpawnedMonster);
				AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Created and possessed controller %s for %s"), 
					*NewController->GetClass()->GetName(), *SpawnedMonster->GetName());
			}
			else
			{
				AO_LOG(LogKSJ, Error, TEXT("AO_Spawner_AIManager: Failed to spawn controller for %s (AIControllerClass: %s)"), 
					*SpawnedMonster->GetName(), 
					ControllerClass ? *ControllerClass->GetName() : TEXT("NULL"));
			}
		}
		else
		{
			AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: %s already has controller: %s"), 
				*SpawnedMonster->GetName(), *SpawnedMonster->GetController()->GetClass()->GetName());
		}
		
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Spawned %s at %s"), *SpawnedMonster->GetName(), *SpawnLocation.ToString());
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: Failed to spawn monster"));
	}
}

void AAO_Spawner_AIManager::TrySpawnInIntensiveArea(AAO_Area_SpawnIntensive* IntensiveArea)
{
	if (!IsValid(IntensiveArea) || !IntensiveArea->bEnabled)
	{
		return;
	}

	// 영역 내 최대 수 체크
	if (IntensiveArea->MaxMonstersInArea > 0)
	{
		int32 CurrentInArea = IntensiveArea->GetCurrentMonsterCountInArea();
		if (CurrentInArea >= IntensiveArea->MaxMonstersInArea)
		{
			return; // 영역 내 최대 수 도달
		}
	}

	// 사용 가능한 몬스터 종류 결정
	TArray<TSubclassOf<AAO_AICharacterBase>> AvailableClasses;
	if (IntensiveArea->AllowedMonsterClasses.Num() > 0)
	{
		AvailableClasses = IntensiveArea->AllowedMonsterClasses;
	}
	else
	{
		AvailableClasses = MonsterClasses;
	}

	if (AvailableClasses.Num() == 0)
	{
		return;
	}

	// 영역 내 랜덤 위치 선택
	FBoxSphereBounds AreaBounds = IntensiveArea->GetBounds();
	FVector RandomLocation = FMath::RandPointInBox(AreaBounds.GetBox());
	
	// NavMesh에 프로젝션
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(RandomLocation, ProjectedLocation, FVector(500.0f, 500.0f, 200.0f)))
		{
			RandomLocation = ProjectedLocation.Location;
		}
		else
		{
			return; // NavMesh에 프로젝션 실패
		}
	}

	// 영역 내부인지 확인
	if (!IntensiveArea->EncompassesPoint(RandomLocation))
	{
		return; // 영역 밖
	}

	// 몬스터 종류 랜덤 선택
	TSubclassOf<AAO_AICharacterBase> SelectedClass = AvailableClasses[FMath::RandRange(0, AvailableClasses.Num() - 1)];

	// 스폰 실행
	ExecuteSpawn(RandomLocation, SelectedClass);

	// 다음 집중 스폰 예약 (랜덤 간격으로 재설정)
	float NextDelay = FMath::Max(0.1f, IntensiveArea->IntensiveSpawnInterval + FMath::RandRange(-IntensiveArea->IntensiveSpawnIntervalRandomDeviation, IntensiveArea->IntensiveSpawnIntervalRandomDeviation));
	if (FTimerHandle* TimerHandle = IntensiveSpawnTimers.Find(IntensiveArea))
	{
		GetWorldTimerManager().SetTimer(*TimerHandle, [this, IntensiveArea]()
		{
			TrySpawnInIntensiveArea(IntensiveArea);
		}, NextDelay, false);
	}
}

int32 AAO_Spawner_AIManager::GetCurrentSpawnedAICount() const
{
	// 유효하지 않은 AI 제거
	int32 ValidCount = 0;
	for (const TObjectPtr<AAO_AICharacterBase> Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			ValidCount++;
		}
	}

	return ValidCount;
}

void AAO_Spawner_AIManager::UpdatePlayerList()
{
	CachedPlayers.Empty();
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundPlayers;
		UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), FoundPlayers);
		
		for (AActor* Actor : FoundPlayers)
		{
			if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor))
			{
				CachedPlayers.Add(Player);
			}
		}
	}
}

void AAO_Spawner_AIManager::CheckIntensiveNavAreas()
{
	if (!bUseNavAreaIntensiveSpawn)
	{
		return;
	}

	// 최대 수 체크 (NavArea 기반 집중 스폰은 최대 수 무시)
	// 하지만 전체 레벨 최대 수는 체크
	int32 CurrentCount = GetCurrentSpawnedAICount();
	if (CurrentCount >= MaxMonstersInLevel)
	{
		// 다음 체크 예약
		float NextDelay = FMath::Max(0.1f, NavAreaIntensiveSpawnInterval + FMath::RandRange(-NavAreaIntensiveSpawnIntervalRandomDeviation, NavAreaIntensiveSpawnIntervalRandomDeviation));
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::CheckIntensiveNavAreas, NextDelay, false);
		return;
	}

	// NavModifierVolume에서 AO_NavArea_SpawnIntensive를 가진 볼륨 찾기
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(World, ANavModifierVolume::StaticClass(), FoundVolumes);

	for (AActor* VolumeActor : FoundVolumes)
	{
		if (ANavModifierVolume* NavVolume = Cast<ANavModifierVolume>(VolumeActor))
		{
			if (IsValid(NavVolume))
			{
				// NavModifierVolume의 AreaClass 확인
				// GetAreaClass()가 없을 수 있으므로, 프로퍼티 직접 접근 시도
				// 실제로는 NavModifierVolume의 AreaClass 프로퍼티를 확인해야 함
				// 일단 모든 NavModifierVolume을 체크하고, AreaClass가 AO_NavArea_SpawnIntensive인 것만 처리
				
				// NavModifierVolume의 AreaClass는 프로퍼티로 접근해야 할 수 있음
				// GetAreaClass() 함수가 없다면 다른 방법 사용 필요
				
				// 임시: 모든 NavModifierVolume에서 AO_NavArea_SpawnIntensive를 찾는 방법
				// 실제로는 NavMesh에서 해당 위치의 NavArea를 확인해야 함
			}
		}
	}

	// 다음 체크 예약
	float NextDelay = FMath::Max(0.1f, NavAreaIntensiveSpawnInterval + FMath::RandRange(-NavAreaIntensiveSpawnIntervalRandomDeviation, NavAreaIntensiveSpawnIntervalRandomDeviation));
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAO_Spawner_AIManager::CheckIntensiveNavAreas, NextDelay, false);
}

void AAO_Spawner_AIManager::OnAIDestroyed(AActor* DestroyedActor)
{
	if (AAO_AICharacterBase* DestroyedMonster = Cast<AAO_AICharacterBase>(DestroyedActor))
	{
		SpawnedMonsters.Remove(DestroyedMonster);
		DestroyedMonster->OnDestroyed.RemoveDynamic(this, &AAO_Spawner_AIManager::OnAIDestroyed);
	}
}

void AAO_Spawner_AIManager::DebugSpawn()
{
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: DebugSpawn called - HasAuthority: %d, IsPIE: %d"), 
		HasAuthority(), GetWorld()->IsPlayInEditor());
	
	bool bShouldSpawn = HasAuthority() || GetWorld()->IsPlayInEditor();
	if (!bShouldSpawn)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_Spawner_AIManager: DebugSpawn can only be called on server or in PIE"));
		return;
	}

	// 최대 수 체크 (디버그 모드에서는 경고만)
	int32 CurrentCount = GetCurrentSpawnedAICount();
	if (CurrentCount >= MaxMonstersInLevel)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: DebugSpawn: Max monsters reached (%d/%d), spawning anyway"), CurrentCount, MaxMonstersInLevel);
	}

	// 몬스터 종류 체크
	if (MonsterClasses.Num() == 0)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_Spawner_AIManager: DebugSpawn: No monster classes configured"));
		return;
	}

	// EQS 쿼리 실행 (동기적으로)
	if (!IsValid(SpawnLocationQuery))
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_Spawner_AIManager: DebugSpawn: SpawnLocationQuery is not set"));
		return;
	}

	UpdatePlayerList();
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: DebugSpawn: Found %d players"), CachedPlayers.Num());
	if (CachedPlayers.Num() == 0)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_Spawner_AIManager: DebugSpawn: No players found - cannot spawn"));
		return;
	}

	// EQS 쿼리 실행
	AO_LOG(LogKSJ, Warning, TEXT("AO_Spawner_AIManager: DebugSpawn: Executing EQS query: %s"), *SpawnLocationQuery->GetName());
	FEnvQueryRequest QueryRequest(SpawnLocationQuery, this);
	QueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &AAO_Spawner_AIManager::OnEQSQueryFinished);
}

