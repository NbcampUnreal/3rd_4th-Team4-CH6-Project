// AO_Enemy_Werewolf.cpp

#include "AI/Character/AO_Enemy_Werewolf.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AAO_Enemy_Werewolf::AAO_Enemy_Werewolf()
{
	EnemyType = EEnemyType::Werewolf;

	WalkSpeed = 400.0f;
	ChaseSpeed = 600.0f;

	bReplicates = true;
}

void AAO_Enemy_Werewolf::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Enemy_Werewolf::PerformHowl()
{
	if(GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if(bHasHowled)
	{
		return;
	}

	bHasHowled = true;

	UE_LOG(LogTemp, Warning, TEXT("[Werewolf] Howl triggered! 주변 늑대인간에게 위치 정보 공유 예정."));
}

void AAO_Enemy_Werewolf::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_Enemy_Werewolf, bHasHowled);
}
