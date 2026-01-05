#include "Game/GameState/AO_GameState.h"

#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_GameState::AAO_GameState()
{
	SharedReviveCount = 0;
}

void AAO_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_GameState, SharedReviveCount);
	DOREPLIFETIME(AAO_GameState, RunResetTrigger); // ms:패시브 초기화
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

void AAO_GameState::OnRep_SharedReviveCount()
{
	AO_LOG(LogJSH, Log, TEXT("AO_GameState::OnRep_SharedReviveCount -> %d"), SharedReviveCount);
	OnSharedReviveCountChanged.Broadcast(SharedReviveCount);	// JM : WBP_RevivalChip 업데이트하기 위함
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

int32 AAO_GameState::GetSharedReviveCount() const
{
	return SharedReviveCount;
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
	
	switch (Num)
	{
	case 1: bHint1 = true; break;
	case 2: bHint2 = true; break;
	case 3: bHint3 = true; break;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Find Hint %d"), Num);
}

bool AAO_GameState::CheckHintCount()
{
<<<<<<< feat-kms
	return bHint1 || bHint2 || bHint3;
=======
	return bHint1 && bHint2 && bHint3;
>>>>>>> develop
}
//-ms