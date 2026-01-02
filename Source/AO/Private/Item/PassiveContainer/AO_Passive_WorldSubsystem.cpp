#include "Item/PassiveContainer/AO_Passive_WorldSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"
#include "GameFramework/PlayerState.h"

FString UAO_Passive_WorldSubsystem::GetPlayerPersistentId(APlayerController* PC)
{
    if (!PC || !PC->PlayerState) return FString(TEXT("InvalidPlayer"));
    return PC->PlayerState->GetPlayerName();
}

void UAO_Passive_WorldSubsystem::SnapshotPlayerData(APlayerController* PC)
{
    if (!PC || !PC->HasAuthority()) return;

    FString PlayerId = GetPlayerPersistentId(PC);
    APawn* Pawn = PC->GetPawn();
    
    if (!Pawn)
    {
        if (PlayerSnapshots.Contains(PlayerId))
        {
            return;
        }
        return;
    }

    UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return;
    
    FAO_PlayerGASSnapshot& Snapshot = PlayerSnapshots.FindOrAdd(PlayerId);
    const UAO_PlayerCharacter_AttributeSet* AS = Cast<UAO_PlayerCharacter_AttributeSet>(ASC->GetAttributeSet(UAO_PlayerCharacter_AttributeSet::StaticClass()));
    if (AS)
    {
        Snapshot.AttributeBaseValues.Add(TEXT("MaxHealth"), AS->GetMaxHealth());
        Snapshot.AttributeBaseValues.Add(TEXT("Health"), AS->GetHealth());
        Snapshot.AttributeBaseValues.Add(TEXT("MaxStamina"), AS->GetMaxStamina());
        Snapshot.AttributeBaseValues.Add(TEXT("Stamina"), AS->GetStamina());
    }
    FGameplayEffectQuery UniversalQuery;
    TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(UniversalQuery);
    
    Snapshot.ActiveSpecs.Empty();
    for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
    {
        const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
        if (ActiveGE && ActiveGE->Spec.Def->DurationPolicy != EGameplayEffectDurationType::Instant)
        {
            Snapshot.ActiveSpecs.Add(ActiveGE->Spec);
        }
    }
}

void UAO_Passive_WorldSubsystem::RestorePlayerGASData(APlayerController* PC)
{
    if (!PC || !PC->HasAuthority() || !PC->GetPawn()) return;

    FString PlayerId = GetPlayerPersistentId(PC);
    if (!PlayerSnapshots.Contains(PlayerId)) return;

    UAbilitySystemComponent* ASC = PC->GetPawn()->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return;

    FAO_PlayerGASSnapshot& Snapshot = PlayerSnapshots[PlayerId];
    
    if (UAO_PlayerCharacter_AttributeSet* AS = const_cast<UAO_PlayerCharacter_AttributeSet*>(
        Cast<UAO_PlayerCharacter_AttributeSet>(ASC->GetAttributeSet(UAO_PlayerCharacter_AttributeSet::StaticClass()))))
    {
        if (Snapshot.AttributeBaseValues.Contains(TEXT("MaxHealth")))
        {
            AS->SetMaxHealth(Snapshot.AttributeBaseValues[TEXT("MaxHealth")]);
        }
        
        if (Snapshot.AttributeBaseValues.Contains(TEXT("MaxStamina")))
        {
            AS->SetMaxStamina(Snapshot.AttributeBaseValues[TEXT("MaxStamina")]);
        }
        
        if (Snapshot.AttributeBaseValues.Contains(TEXT("Health")))
        {
            AS->SetHealth(Snapshot.AttributeBaseValues[TEXT("Health")]);
        }
        
        if (Snapshot.AttributeBaseValues.Contains(TEXT("Stamina")))
        {
            AS->SetStamina(Snapshot.AttributeBaseValues[TEXT("Stamina")]);
        }
    }
    
    PlayerSnapshots.Remove(PlayerId);
}

void UAO_Passive_WorldSubsystem::ClearAllPlayerData()
{
    PlayerSnapshots.Empty();
}
