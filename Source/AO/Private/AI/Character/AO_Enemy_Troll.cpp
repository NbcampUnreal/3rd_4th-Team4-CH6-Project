// AO_Enemy_Troll.cpp

#include "AI/Character/AO_Enemy_Troll.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Enemy_Troll::AAO_Enemy_Troll()
{
	EnemyType = EEnemyType::Troll;

	WalkSpeed = 300.0f;
	ChaseSpeed = 450.0f;
}

void AAO_Enemy_Troll::BeginPlay()
{
	Super::BeginPlay();
	
}

