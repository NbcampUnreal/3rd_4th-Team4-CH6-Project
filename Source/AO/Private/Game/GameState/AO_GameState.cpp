#include "Game/GameState/AO_GameState.h"

#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_GameState::AAO_GameState()
{
	SharedReviveCount = 0;
}

void AAO_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_GameState, SharedReviveCount);
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