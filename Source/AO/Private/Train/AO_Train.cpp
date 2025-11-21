#include "Train/AO_Train.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"


// Sets default values
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

	// AttributeSet을 CDO로 생성
	FuelAttributeSet = CreateDefaultSubobject<UAO_Fuel_AttributeSet>(TEXT("AttributeSet"));

	// 기본으로 부여할 Ability들
	StartupAbilities.Add(UAO_AddFuel_GameplayAbility::StaticClass());
}

UAbilitySystemComponent* AAO_Train::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAO_Train::BeginPlay()
{
	Super::BeginPlay();

	if (!ASC) return;
	// ✅ ASC 초기화
	ASC->InitAbilityActorInfo(this, this);
	if (HasAuthority())
	{
		// ✅ AttributeSet 자동 등록됨

		// ✅ StartupAbilities 로 등록된 Ability 모두 부여
		for (auto AbilityClass : StartupAbilities)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}

		UE_LOG(LogTemp, Warning, TEXT("✅ Train GAS initialized on server"));
	}

	// ✅ ASC의 현재 Ability 리스트 출력
	UE_LOG(LogTemp, Warning, TEXT("Train Abilities:"));
	for (auto& Spec : ASC->GetActivatableAbilities())
	{
		UE_LOG(LogTemp, Warning, TEXT(" - %s"), *Spec.Ability->GetName());
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

	if (Delta > 0.f)
	{
		TotalFuelGained += Delta;
		UE_LOG(LogTemp, Warning, TEXT("🔥 연료 추가 +%.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	else if (Delta < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("💨 연료 감소 %.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
}