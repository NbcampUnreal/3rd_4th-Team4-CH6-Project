// HSJ : AO_PinballBall.cpp
#include "Puzzle/Actor/Pinball/AO_PinballBall.h"
#include "Net/UnrealNetwork.h"

AAO_PinballBall::AAO_PinballBall()
{
	bReplicates = true;
	SetReplicateMovement(true);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	RootComponent = BallMesh;

	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetEnableGravity(true);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BallMesh->SetCollisionObjectType(ECC_PhysicsBody);
	BallMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AAO_PinballBall::BeginPlay()
{
	Super::BeginPlay();
    
	if (HasAuthority())
	{
		StartPosition = GetActorLocation();
	}
}

void AAO_PinballBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAO_PinballBall, StartPosition);
}

void AAO_PinballBall::ResetToStart()
{
	if (!HasAuthority()) return;

	BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetActorLocation(StartPosition);
}