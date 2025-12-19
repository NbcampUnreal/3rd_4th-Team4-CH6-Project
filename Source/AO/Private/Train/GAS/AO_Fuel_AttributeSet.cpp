#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Net/UnrealNetwork.h"

UAO_Fuel_AttributeSet::UAO_Fuel_AttributeSet()
{
}

void UAO_Fuel_AttributeSet::InitFromGameInstance()
{
	//UE_LOG(LogTemp, Error, TEXT("[FuelSet] InitFromGameInstance"));

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	//UE_LOG(LogTemp, Error, TEXT("[FuelSet] ASC: %s"), ASC ? TEXT("VALID") : TEXT("NULL"));
	if (!ASC) return;

	AActor* OwnerActor = ASC->GetOwnerActor();

	/*UE_LOG(LogTemp, Error, TEXT("[FuelSet] OwnerActor: %s (%s)"),
		*GetNameSafe(OwnerActor),
		OwnerActor ? *OwnerActor->GetClass()->GetName() : TEXT("[FuelSet] NULL"));*/

	if (!OwnerActor) return;

	UWorld* World = OwnerActor->GetWorld();
	if (!World) return;

	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(World->GetGameInstance()))
	{
		const float LastFuel = FMath::Max(0.0f, GI->SharedTrainFuel);
		Fuel.SetBaseValue(LastFuel);
		Fuel.SetCurrentValue(LastFuel);
	}
}


void UAO_Fuel_AttributeSet::OnRep_Fuel(const FGameplayAttributeData& OldFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Fuel_AttributeSet, Fuel, OldFuel);
}

void UAO_Fuel_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Fuel_AttributeSet, Fuel, COND_None, REPNOTIFY_Always);
}
