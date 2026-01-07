#include "Item/PassiveContainer/AO_Passive_WorldSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

FString UAO_Passive_WorldSubsystem::GetPlayerPersistentId(APlayerController* PC)
{
    if (!PC || !PC->PlayerState) return FString(TEXT("InvalidPlayer"));
    // 이름 외에 UniqueId를 쓰는 것이 멀티플레이에서 더 정확합니다.
    return PC->PlayerState->GetPlayerName();
}

void UAO_Passive_WorldSubsystem::RecordPassiveUpgrade(APlayerController* PC, FGameplayTag PassiveTag, float Amount)
{
    if (!PC) return;

    FString PlayerId = GetPlayerPersistentId(PC);
    FAO_PlayerPassiveData& Data = PlayerPassiveStats.FindOrAdd(PlayerId);

    // 해당 태그의 누적 수치를 더해줌
    float& CurrentTotal = Data.CumulativePassives.FindOrAdd(PassiveTag);
    CurrentTotal += Amount;
}

void UAO_Passive_WorldSubsystem::ReapplyAllPassives(APlayerController* PC)
{
    if (!PC || !PC->HasAuthority() || !PC->GetPawn()) return;

    FString PlayerId = GetPlayerPersistentId(PC);
    if (!PlayerPassiveStats.Contains(PlayerId)) return;

    UAbilitySystemComponent* ASC = PC->GetPawn()->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return;

    FAO_PlayerPassiveData& Data = PlayerPassiveStats[PlayerId];

    // 저장된 모든 패시브 레코드에 대해 다시 이벤트를 발생시켜 적용함
    for (auto& Pair : Data.CumulativePassives)
    {
        FGameplayEventData EventData;
        EventData.EventTag = Pair.Key;
        EventData.EventMagnitude = Pair.Value; // 누적된 전체 값을 한 번에 전달
        
        // 캐릭터의 PassiveComponent가 이 이벤트를 받아 GE를 다시 적용하게 됨
        ASC->HandleGameplayEvent(Pair.Key, &EventData);
    }
}

void UAO_Passive_WorldSubsystem::ClearAllPlayerData()
{
    PlayerPassiveStats.Empty();
}