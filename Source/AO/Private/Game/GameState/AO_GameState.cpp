#include "Game/GameState/AO_GameState.h"

#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_GameState::AAO_GameState()
{
	SharedReviveCount = 0;
	bIsStageFailed = false;		// JM : 초기화
	bIsGameCleared = false;		// JM : 초기화
	GameStartTime = 0;
	GameEndTime = 0;
	TeamDeathCount = 0;
	CurrentFindHintNum = 0;
	bHint1 = false;
	bHint2 = false;
	bHint3 = false;
}

void AAO_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_GameState, SharedReviveCount);
	DOREPLIFETIME(AAO_GameState, bIsStageFailed);
	DOREPLIFETIME(AAO_GameState, RunResetTrigger); // ms:패시브 초기화
	//ms : 선발대 흔적 서버 전달
	DOREPLIFETIME(AAO_GameState, CurrentFindHintNum);
	DOREPLIFETIME(AAO_GameState, bHint1);
	DOREPLIFETIME(AAO_GameState, bHint2);
	DOREPLIFETIME(AAO_GameState, bHint3);
}

void AAO_GameState::AddPlayerState(APlayerState* PlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::AddPlayerState(PlayerState);

	// JM : 내부 데이터가 완전히 복제될 때까지의 시간이 필요 (여기서 바로 실행하면 null 가능성 있음) - 테스트 결과 null 임
	// TODO : Event(Delegate) 방식으로 전환 필요
	if (GetWorldTimerManager().IsTimerActive(UnmuteVoiceTimerHandle))	// 중복바인딩 방지
	{
		GetWorldTimerManager().ClearTimer(UnmuteVoiceTimerHandle);
	}
	FTimerDelegate UnmuteDelegate;
	UnmuteDelegate.BindUFunction(this, FName("UnmuteVoiceOnAddPlayerState"), PlayerState);
	GetWorldTimerManager().SetTimer(
		UnmuteVoiceTimerHandle,
		UnmuteDelegate,
		0.1f,
		false
	);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			TeamDeathCount = GI->TeamDeathCount;	// JM : 데스 수를 유지하기 위함
			GameStartTime = GI->GameStartTime;
			GameEndTime = GI->GameEndTime;
		} 
	}
}

void AAO_GameState::UnmuteVoiceOnAddPlayerState(APlayerState* PlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	if (!AO_ENSURE(PlayerState, TEXT("InValid PlayerState")))
	{
		return;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!AO_ENSURE(OSS, TEXT("Can't Get OSS")))
	{
		return;
	}

	AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
	{
		return;
	}

	OSS->UnmuteRemoteTalker(0, AO_PS, false);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameState::SetSharedReviveCount(int32 InValue)
{
	if (HasAuthority() == false)
	{
		return;
	}

	int32 NewValue = InValue;

	if (NewValue < 0)
	{
		NewValue = 0;
	}

	if (SharedReviveCount == NewValue)
	{
		return;
	}

	SharedReviveCount = NewValue;

	if (HasAuthority())		// JM : 서버에서는 OnRep 함수가 실행되지 않기 때문에 개별적으로 호출해줌 (UI 업데이트 해야함) 
	{
		OnRep_SharedReviveCount();
	}

	AO_LOG(LogJSH, Log, TEXT("AO_GameState::SetSharedReviveCount -> %d"), SharedReviveCount);
}

void AAO_GameState::SetGameClear()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));
	
	if (!HasAuthority())
	{
		return;
	}

	if (bIsGameCleared)
	{
		return;
	}

	bIsGameCleared = true;
	SetGameEndTime();			// JM : 게임 클리어 시 End Time 갱신
	OnRep_IsGameCleared();		// JM : Host는 OnRep이 자동으로 호출되지 않으므로 수동 호출
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::AddTeamDeathCount()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (HasAuthority())
	{
		TeamDeathCount++;
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			GI->TeamDeathCount = TeamDeathCount;
		}
		OnRep_TeamDeathCount();		// JM : 서버는 OnRep 자동으로 실행 안되므로 수동으로 실행
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::SetGameStartTime()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (HasAuthority())
	{
		GameStartTime = GetWorld()->GetTimeSeconds();
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			GI->GameStartTime = GameStartTime;
		}
		OnRep_GameStartTime();		// JM : 서버는 OnRep 자동으로 실행 안되므로 수동으로 실행
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::SetGameEndTime()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (HasAuthority())
	{
		GameEndTime = GetWorld()->GetTimeSeconds();
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			GI->GameEndTime = GameEndTime;
		}
		OnRep_GameEndTime();		// JM : 서버는 OnRep 자동으로 실행 안되므로 수동으로 실행
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::SetStageFailed()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));
	if (!HasAuthority())
	{
		return;
	}

	if (bIsStageFailed)
	{
		return;
	}

	bIsStageFailed = true;
	SetGameEndTime();			// JM : 게임 실패 시 End Time 갱신
	OnRep_IsStageFailed();		// JM : Host는 OnRep이 자동으로 호출되지 않으므로 수동 호출
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::OnRep_SharedReviveCount()
{
	AO_LOG(LogJSH, Log, TEXT("AO_GameState::OnRep_SharedReviveCount -> %d"), SharedReviveCount);
	OnSharedReviveCountChanged.Broadcast(SharedReviveCount);	// JM : WBP_RevivalChip 업데이트하기 위함
}

void AAO_GameState::OnRep_IsStageFailed()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));
	if (bIsStageFailed)
	{
		AO_LOG_ROLE(LogJM, Log, TEXT("Broadcast Delegate(OnStageFailed)"));
		if (OnStageFailed.IsBound())
		{
			OnStageFailed.Broadcast();
		}
	}
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::OnRep_IsGameCleared()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));
	if (bIsGameCleared)
	{
		AO_LOG_ROLE(LogJM, Log, TEXT("Broadcast Delegate(OnGameCleared)"));
		if (OnGameCleared.IsBound())
		{
			OnGameCleared.Broadcast();
		}
	}
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::OnRep_TeamDeathCount()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		// GI->TeamDeathCount = TeamDeathCount;
		if (UAO_DelegateManager* DM = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
		{
			DM->OnStatisticsUpdated.Broadcast();
		}
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::OnRep_GameStartTime()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		// GI->GameStartTime = GameStartTime;
		if (UAO_DelegateManager* DM = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
		{
			DM->OnStatisticsUpdated.Broadcast();
		}
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_GameState::OnRep_GameEndTime()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		// GI->GameEndTime = GameEndTime;
		if (UAO_DelegateManager* DM = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
		{
			DM->OnStatisticsUpdated.Broadcast();
		}
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

int32 AAO_GameState::GetSharedReviveCount() const
{
	return SharedReviveCount;
}


void AAO_GameState::AddSharedReviveCount(int32 Delta)
{
	if (HasAuthority() == false)
	{
		return;
	}

	const int32 OldValue = SharedReviveCount;

	int32 NewValue = SharedReviveCount + Delta;
	if (NewValue < 0)
	{
		NewValue = 0;
	}

	// SetSharedReviveCount 안에서 Rep / OnRep / 로그 처리
	SetSharedReviveCount(NewValue);

	// GI 값도 같이 맞춰줌 (GI는 "저장"만 담당)
	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		GI->SharedReviveCount = SharedReviveCount;
	}

	// 값이 증가한 경우에만 Stage GameMode에 알림 (기존 GI 로직 이관)
	if (SharedReviveCount > OldValue)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>();
			if (StageGM != nullptr)
			{
				StageGM->HandleSharedReviveCountIncreased();
			}
		}
	}
}

bool AAO_GameState::TryConsumeSharedReviveCount()
{
	if (HasAuthority() == false)
	{
		return false;
	}

	if (SharedReviveCount <= 0)
	{
		return false;
	}

	const int32 NewValue = SharedReviveCount - 1;

	// SetSharedReviveCount 를 통해 Rep / OnRep / 로그 처리
	SetSharedReviveCount(NewValue);

	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		GI->SharedReviveCount = SharedReviveCount;
	}

	AO_LOG(LogJSH, Log, TEXT("GS: Consume revive -> %d left"), SharedReviveCount);

	return true;
}

//ms: 패시브 초기화
void AAO_GameState::Authority_NotifyGlobalReset()
{
	if (HasAuthority())
	{
		RunResetTrigger++;
	}
}

void AAO_GameState::OnRep_RunResetTrigger()
{
	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		GI->PassiveReset(); 
	}
}

//ms : 선발대 흔적 확인
void AAO_GameState::FindHint(int32 Num)
{
	if (!HasAuthority()) return;
	
	bool bValueUpdated = false;
	switch (Num)
	{
	case 1:
		if (bHint1 == false) { 
			bHint1 = true;
			CurrentFindHintNum++;
			bValueUpdated = true;
		}
		break;
	case 2:
		if (bHint2 == false) {
			bHint2 = true;
			CurrentFindHintNum++;
			bValueUpdated = true;
		}
		break;
	case 3:
		if (bHint3 == false) {
			bHint3 = true;
			CurrentFindHintNum++;
			bValueUpdated = true;
		}
		break;
	}

	if (bValueUpdated)
	{
		OnRep_HintCount();
	}
}

bool AAO_GameState::CheckHintCount()
{
	return bHint1 && bHint2 && bHint3;
}

void AAO_GameState::OnRep_HintCount()
{
	OnHintCountChanged.Broadcast(CurrentFindHintNum);
}
//-ms
