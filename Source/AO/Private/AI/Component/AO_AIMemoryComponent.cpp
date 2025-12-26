// AO_AIMemoryComponent.cpp

#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AO_Log.h"

UAO_AIMemoryComponent::UAO_AIMemoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_AIMemoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAO_AIMemoryComponent::UpdatePlayerLocation(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// Player가 유효해야 메모리 업데이트 가능
	if (!ensureMsgf(Player, TEXT("UpdatePlayerLocation called with null Player")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FAO_PlayerMemoryData& Memory = PlayerMemories.FindOrAdd(Player);
	Memory.LastKnownLocation = Location;
	Memory.LastSeenTime = World->GetTimeSeconds();
	Memory.bIsInSight = true;
}

void UAO_AIMemoryComponent::SetLastKnownLocation(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// Player가 유효해야 마지막 위치 저장 가능
	if (!ensureMsgf(Player, TEXT("SetLastKnownLocation called with null Player")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FAO_PlayerMemoryData& Memory = PlayerMemories.FindOrAdd(Player);
	Memory.LastLostLocation = Location;
	Memory.LostTime = World->GetTimeSeconds();
	Memory.bIsInSight = false;

	AO_LOG(LogKSJ, Log, TEXT("Memory Updated - Player: %s, LastLostLocation: %s"), 
		*Player->GetName(), *Location.ToString());
}

bool UAO_AIMemoryComponent::GetPlayerMemory(AAO_PlayerCharacter* Player, FAO_PlayerMemoryData& OutData) const
{
	// Player가 null이면 메모리 조회 불가
	if (!ensureMsgf(Player, TEXT("GetPlayerMemory called with null Player")))
	{
		return false;
	}

	const FAO_PlayerMemoryData* Found = PlayerMemories.Find(Player);
	if (Found)
	{
		OutData = *Found;
		return true;
	}

	return false;
}

FVector UAO_AIMemoryComponent::GetLastKnownLocation(AAO_PlayerCharacter* Player) const
{
	// Player가 null이면 ZeroVector 반환
	if (!ensureMsgf(Player, TEXT("GetLastKnownLocation called with null Player")))
	{
		return FVector::ZeroVector;
	}

	const FAO_PlayerMemoryData* Found = PlayerMemories.Find(Player);
	if (Found)
	{
		return Found->LastKnownLocation;
	}

	return FVector::ZeroVector;
}

FVector UAO_AIMemoryComponent::GetLastLostLocation(AAO_PlayerCharacter* Player) const
{
	// Player가 null이면 ZeroVector 반환
	if (!ensureMsgf(Player, TEXT("GetLastLostLocation called with null Player")))
	{
		return FVector::ZeroVector;
	}

	const FAO_PlayerMemoryData* Found = PlayerMemories.Find(Player);
	if (Found)
	{
		return Found->LastLostLocation;
	}

	return FVector::ZeroVector;
}

TArray<FVector> UAO_AIMemoryComponent::GetAllLastLostLocations() const
{
	TArray<FVector> Locations;

	for (const auto& Pair : PlayerMemories)
	{
		if (Pair.Key.IsValid() && !Pair.Value.LastLostLocation.IsZero())
		{
			Locations.Add(Pair.Value.LastLostLocation);
		}
	}

	return Locations;
}

TArray<FVector> UAO_AIMemoryComponent::GetRecentLostLocations(float WithinSeconds) const
{
	TArray<FVector> Locations;
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const auto& Pair : PlayerMemories)
	{
		if (Pair.Key.IsValid() && !Pair.Value.LastLostLocation.IsZero())
		{
			if ((CurrentTime - Pair.Value.LostTime) <= WithinSeconds)
			{
				Locations.Add(Pair.Value.LastLostLocation);
			}
		}
	}

	return Locations;
}

void UAO_AIMemoryComponent::ClearMemory()
{
	PlayerMemories.Empty();
}

void UAO_AIMemoryComponent::ForgetPlayer(AAO_PlayerCharacter* Player)
{
	if (Player)
	{
		PlayerMemories.Remove(Player);
	}
}

bool UAO_AIMemoryComponent::IsMemoryValid(AAO_PlayerCharacter* Player, float MaxAge) const
{
	// Player가 null이면 유효하지 않음
	if (!ensureMsgf(Player, TEXT("IsMemoryValid called with null Player")))
	{
		return false;
	}

	const FAO_PlayerMemoryData* Found = PlayerMemories.Find(Player);
	if (!Found)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const float Age = CurrentTime - Found->LostTime;

	return Age <= MaxAge;
}

void UAO_AIMemoryComponent::SetLastHeardLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	LastHeardLocation = Location;
	LastHeardTime = World->GetTimeSeconds();

	AO_LOG(LogKSJ, Log, TEXT("Heard noise at: %s"), *Location.ToString());
}

void UAO_AIMemoryComponent::ClearHeardLocation()
{
	LastHeardLocation = FVector::ZeroVector;
	LastHeardTime = 0.f;
}

FVector UAO_AIMemoryComponent::GetLastHeardLocation() const
{
	// 시간이 지났으면 무효
	if (LastHeardLocation.IsZero())
	{
		return FVector::ZeroVector;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastHeardTime) > HeardMemoryDuration)
	{
		return FVector::ZeroVector;
	}

	return LastHeardLocation;
}
