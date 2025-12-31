// HSJ : AO_DestructibleFloor.cpp
#include "Puzzle/Destructible/AO_DestructibleFloor.h"
#include "Character/AO_PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "AO_Log.h"

AAO_DestructibleFloor::AAO_DestructibleFloor()
{
    FloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorCollision"));
    FloorCollision->SetupAttachment(RootComponent);
    
    FloorCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FloorCollision->SetCollisionObjectType(ECC_WorldStatic);
    FloorCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    FloorCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    FloorCollision->SetGenerateOverlapEvents(true);
    FloorCollision->SetBoxExtent(FVector(100.f, 100.f, 10.f));

	OverlapTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapTrigger"));
	OverlapTrigger->SetupAttachment(RootComponent);
    
	OverlapTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapTrigger->SetCollisionObjectType(ECC_WorldStatic);
	OverlapTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapTrigger->SetGenerateOverlapEvents(true);
	OverlapTrigger->SetBoxExtent(FVector(110.f, 110.f, 50.f));

    DestructionDelay = 1.0f;
    CollisionDisableDelay = 0.5f;
}

void AAO_DestructibleFloor::BeginPlay()
{
    Super::BeginPlay();

	if (HasAuthority())
	{
		if (OverlapTrigger)
		{
			OverlapTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAO_DestructibleFloor::OnFloorBeginOverlap);
		}
	}
}

void AAO_DestructibleFloor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (TObjectPtr<UWorld> World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DestructionTimerHandle);
        World->GetTimerManager().ClearTimer(CollisionDisableTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void AAO_DestructibleFloor::OnFloorBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
    	return;
    }
	
    if (bDestructionStarted)
    {
    	return;
    }
    if (bIsDestroyed)
    {
    	return;
    }

    AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    bDestructionStarted = true;
    StartDestructionTimer();
}

void AAO_DestructibleFloor::StartDestructionTimer()
{
    if (!HasAuthority())
    {
    	return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    checkf(World, TEXT("World is null in StartDestructionTimer"));

    if (DestructionDelay <= 0.0f)
    {
        TriggerDestruction();
    	DisableFloorCollision();
    }
    else
    {
        TWeakObjectPtr<AAO_DestructibleFloor> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DestructionTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (TObjectPtr<AAO_DestructibleFloor> StrongThis = WeakThis.Get())
                {
                	StrongThis->TriggerDestruction();
                    // 파괴 후 DestructionDelay초 뒤 Collision 비활성화
                    StrongThis->DisableFloorCollision();
                }
            }),
            DestructionDelay,
            false
        );
    }
}

void AAO_DestructibleFloor::DisableFloorCollision()
{
    if (!HasAuthority())
    {
    	return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    checkf(World, TEXT("World is null in DisableFloorCollision"));

    if (CollisionDisableDelay <= 0.0f)
    {
        // 즉시 Collision 제거
        if (FloorCollision)
        {
            FloorCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    	
    	if (OverlapTrigger)
    	{
    		OverlapTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	}
    }
    else
    {
    	// Collision 제거
        TWeakObjectPtr<AAO_DestructibleFloor> WeakThis(this);
        World->GetTimerManager().SetTimer(
            CollisionDisableTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (TObjectPtr<AAO_DestructibleFloor> StrongThis = WeakThis.Get())
                {
                    if (StrongThis->FloorCollision)
                    {
                        StrongThis->FloorCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }

                	if (StrongThis->OverlapTrigger)
                	{
						StrongThis->OverlapTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
                	AO_LOG(LogHSJ, Warning, TEXT("DestructibleFloor: Floor collision disabled (DELAYED)"));
                }
            }),
            CollisionDisableDelay,
            false
        );
    }
}