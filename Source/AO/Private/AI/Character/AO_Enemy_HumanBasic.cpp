// AO_Enemy_HumanBasic.cpp

#include "AO/Public/AI/Character/AO_Enemy_HumanBasic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AAO_Enemy_HumanBasic::AAO_Enemy_HumanBasic()
{
	PrimaryActorTick.bCanEverTick = true;

	WalkSpeed = 200.0f;
	ChaseSpeed = 300.0f;

	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	StimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
	StimuliSource->RegisterWithPerceptionSystem();

	bIsPlayerVisible = false;
}

void AAO_Enemy_HumanBasic::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AAO_Enemy_HumanBasic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAO_Enemy_HumanBasic::OnPlayerSpotted(AActor* PlayerActor)
{
	bIsPlayerVisible = true;
	CurrentTarget = PlayerActor;
	LastKnownPlayerLocation = PlayerActor->GetActorLocation();

	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
}

void AAO_Enemy_HumanBasic::OnPlayerLost()
{
	bIsPlayerVisible = false;
	CurrentTarget = NULL;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AAO_Enemy_HumanBasic::PerformAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("[AI] Performing attack!"));
}




