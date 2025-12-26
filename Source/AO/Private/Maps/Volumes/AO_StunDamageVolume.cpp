// HSJ : AO_StunDamageVolume.cpp
#include "Maps/Volumes/AO_StunDamageVolume.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "AI/Base/AO_AICharacterBase.h"

AAO_StunDamageVolume::AAO_StunDamageVolume()
{
    StunEventTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
}

void AAO_StunDamageVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    TObjectPtr<UWorld> World = GetWorld();
    if (World)
    {
        for (auto& Pair : StunTimerHandles)
        {
            if (Pair.Value.IsValid())
            {
                World->GetTimerManager().ClearTimer(Pair.Value);
            }
        }
    }
    StunTimerHandles.Empty();

    Super::EndPlay(EndPlayReason);
}

void AAO_StunDamageVolume::OnBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    Super::OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    TObjectPtr<AAO_AICharacterBase> Enemy = Cast<AAO_AICharacterBase>(OtherActor);
    if (!Enemy)
    {
        return;
    }

    StartStunTimer(Enemy);
}

void AAO_StunDamageVolume::OnEndOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    Super::OnEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex);

    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    TObjectPtr<AAO_AICharacterBase> Enemy = Cast<AAO_AICharacterBase>(OtherActor);
    if (!Enemy)
    {
        return;
    }

    StopStunTimer(Enemy);
}

void AAO_StunDamageVolume::StartStunTimer(AActor* Enemy)
{
    if (!Enemy || !StunEventTag.IsValid())
    {
        return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
        return;
    }

    TWeakObjectPtr<AActor> WeakEnemy(Enemy);
    FTimerHandle& TimerHandle = StunTimerHandles.FindOrAdd(WeakEnemy);

    World->GetTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakEnemy]()
        {
            if (TObjectPtr<AActor> StrongEnemy = WeakEnemy.Get())
            {
                if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(StrongEnemy))
                {
                    if (TObjectPtr<UAbilitySystemComponent> ASC = ASI->GetAbilitySystemComponent())
                    {
                        FGameplayEventData EventData;
                        EventData.Instigator = GetInstigator();
                        EventData.Target = StrongEnemy;
                        
                        ASC->HandleGameplayEvent(StunEventTag, &EventData);
                    }
                }
            }
        }),
        StunInterval,
        true,
        0.0f
    );
}

void AAO_StunDamageVolume::StopStunTimer(AActor* Enemy)
{
    if (!Enemy)
    {
        return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
        return;
    }

    TWeakObjectPtr<AActor> WeakEnemy(Enemy);
    if (FTimerHandle* TimerHandle = StunTimerHandles.Find(WeakEnemy))
    {
        World->GetTimerManager().ClearTimer(*TimerHandle);
        StunTimerHandles.Remove(WeakEnemy);
    }
}