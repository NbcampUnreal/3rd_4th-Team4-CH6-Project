#include "Train/AO_Train.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "Train/GAS/AO_RemoveFuel_GameplayAbility.h"

AAO_Train::AAO_Train()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetIsReplicated(true);
	
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	FuelAttributeSet = CreateDefaultSubobject<UAO_Fuel_AttributeSet>(TEXT("AttributeSet"));
	StartupAbilities.Add(UAO_AddFuel_GameplayAbility::StaticClass());
	StartupAbilities.Add(UAO_RemoveFuel_GameplayAbility::StaticClass());
}

UAbilitySystemComponent* AAO_Train::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAO_Train::BeginPlay()
{
	Super::BeginPlay();
	if (!ASC) return;
	ASC->InitAbilityActorInfo(this, this);
	if (HasAuthority())
	{
		if (!ASC) return;

		ASC->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			if (AddEnergyAbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(AddEnergyAbilityClass, 1, 0, this));
			}
			if (LeakEnergyAbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(LeakEnergyAbilityClass, 1, 0, this));
			}
		}
	}
	
	ASC->GetGameplayAttributeValueChangeDelegate(
			UAO_Fuel_AttributeSet::GetFuelAttribute()
		).AddUObject(this, &AAO_Train::OnFuelChanged);
}

void AAO_Train::OnFuelChanged(const FOnAttributeChangeData& Data)
{
	const float OldFuel = Data.OldValue;
	const float NewFuel = Data.NewValue;
	const float Delta = NewFuel - OldFuel;

	TotalFuelGained += Delta;

	if (Delta > 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔥 연료 추가 +%.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	else if (Delta < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("💨 연료 감소 %.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
}

void AAO_Train::FuelLeakSkillOn()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);

	if (Spec)
	{
		ASC->TryActivateAbility(Spec->Handle);
	}
}

void AAO_Train::FuelLeakSkillOut()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);
	
	ASC->CancelAbility(Spec->Ability);
	ASC->ClearAbility(Spec->Handle);
}
