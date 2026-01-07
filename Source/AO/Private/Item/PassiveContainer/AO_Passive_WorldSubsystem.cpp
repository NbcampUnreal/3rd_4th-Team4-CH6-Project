#include "Item/PassiveContainer/AO_Passive_WorldSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

FString UAO_Passive_WorldSubsystem::GetPlayerPersistentId(APlayerController* PC)
{
    if (!PC || !PC->PlayerState) return FString(TEXT("InvalidPlayer"));
    return PC->PlayerState->GetPlayerName();
}

void UAO_Passive_WorldSubsystem::RecordPassiveUpgrade(APlayerController* PC, FGameplayTag PassiveTag, float Amount)
{
    if (!PC) return;

    FString PlayerId = GetPlayerPersistentId(PC);
    FAO_PlayerPassiveData& Data = PlayerPassiveStats.FindOrAdd(PlayerId);
    
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
    
    for (auto& Pair : Data.CumulativePassives)
    {
        FGameplayEventData EventData;
        EventData.EventTag = Pair.Key;
        EventData.EventMagnitude = Pair.Value;
        
        ASC->HandleGameplayEvent(Pair.Key, &EventData);
    }
}

void UAO_Passive_WorldSubsystem::ClearAllPlayerData()
{
    PlayerPassiveStats.Empty();
}