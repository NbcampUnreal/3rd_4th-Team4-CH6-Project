#include "Game/GameState/AO_GameState.h"

#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_GameState::AAO_GameState()
{
	SharedReviveCount = 0;
}

void AAO_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_GameState, SharedReviveCount);
}

void AAO_GameState::AddPlayerState(APlayerState* PlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::AddPlayerState(PlayerState);

	// JM : 내부 데이터가 완전히 복제될 때까지의 시간이 필요 (여기서 바로 실행하면 null 가능성 있음)
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

	AO_LOG(LogJSH, Log, TEXT("AO_GameState::SetSharedReviveCount -> %d"), SharedReviveCount);
}

int32 AAO_GameState::GetSharedReviveCount() const
{
	return SharedReviveCount;
}