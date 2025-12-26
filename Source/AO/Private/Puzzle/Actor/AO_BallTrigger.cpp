// HSJ : AO_BallTrigger.cpp
#include "Puzzle/Actor/AO_BallTrigger.h"
#include "Puzzle/Actor/AO_RollingBall.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "AO_Log.h"

AAO_BallTrigger::AAO_BallTrigger()
{
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);
    TriggerBox->SetBoxExtent(FVector(200, 200, 200));

    DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
}

void AAO_BallTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAO_BallTrigger::OnTriggerBeginOverlap);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAO_BallTrigger::OnTriggerEndOverlap);
        
        // PlayerDetection 타입, 살아있는 캐릭터가 트리거 존에 있을 때만 동작하도록
        if (TriggerType == EBallTriggerType::PlayerDetection)
        {
            TArray<AActor*> OverlappingActors;
            TriggerBox->GetOverlappingActors(OverlappingActors, AAO_PlayerCharacter::StaticClass());
            
            for (AActor* Actor : OverlappingActors)
            {
                if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor))
                {
                    OverlappingPlayers.AddUnique(TWeakObjectPtr<AActor>(Player));
                }
            }

            const int32 AliveCount = GetAlivePlayerCount();
        	for (AAO_RollingBall* Ball : TargetBalls)
        	{
        		if (Ball)
        		{
        			Ball->SetShouldMove(AliveCount > 0);
        		}
        	}
        }
    }
}

void AAO_BallTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor)
    {
    	return;
    }

    if (TriggerType == EBallTriggerType::Reset)
    {
    	AAO_RollingBall* Ball = Cast<AAO_RollingBall>(OtherActor);
    	if (Ball && TargetBalls.Contains(Ball))
    	{
    		Ball->ResetToStart();
    	}
    }
    else // PlayerDetection
    {
        AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
        if (!Player)
        {
        	return;
        }

        OverlappingPlayers.AddUnique(TWeakObjectPtr<AActor>(Player));
        
        // 살아있는 플레이어 있으면 Ball이 움직이게
    	const int32 AliveCount = GetAlivePlayerCount();
    	if (AliveCount > 0)
    	{
    		for (AAO_RollingBall* Ball : TargetBalls)
    		{
    			if (Ball)
    			{
    				Ball->SetShouldMove(true);
    			}
    		}
    	}

        // 죽음 감지를 위해 타이머 시작
    	TObjectPtr<UWorld> World = GetWorld();
    	if (World && !World->GetTimerManager().IsTimerActive(AliveCheckTimerHandle))
    	{
    		TWeakObjectPtr<AAO_BallTrigger> WeakThis(this);
    		World->GetTimerManager().SetTimer(
				AliveCheckTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
				{
					if (TObjectPtr<AAO_BallTrigger> StrongThis = WeakThis.Get())
					{
						const int32 Count = StrongThis->GetAlivePlayerCount();
                        
						// 모든 Ball 업데이트
						for (AAO_RollingBall* Ball : StrongThis->TargetBalls)
						{
							if (Ball)
							{
								Ball->SetShouldMove(Count > 0);
							}
						}
					}
				}),
				AliveCheckInterval,
				true
			);
    	}
    }
}

void AAO_BallTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!HasAuthority() || !OtherActor)
    {
    	return;
    }
    if (TriggerType != EBallTriggerType::PlayerDetection)
    {
    	return;
    }

    OverlappingPlayers.Remove(TWeakObjectPtr<AActor>(OtherActor));

	const int32 AliveCount = GetAlivePlayerCount();
    
	for (AAO_RollingBall* Ball : TargetBalls)
	{
		if (Ball)
		{
			Ball->SetShouldMove(AliveCount > 0);
		}
	}

    // 모든 플레이어 나가면 타이머 정리
    if (OverlappingPlayers.Num() == 0)
    {
        TObjectPtr<UWorld> World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(AliveCheckTimerHandle);
        }
    }
}

int32 AAO_BallTrigger::GetAlivePlayerCount() const
{
    int32 Count = 0;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : OverlappingPlayers)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
        {
        	continue;
        }

        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                if (!ASC->HasMatchingGameplayTag(DeathTag))
                {
                    Count++;
                }
            }
        }
    }
    
    return Count;
}