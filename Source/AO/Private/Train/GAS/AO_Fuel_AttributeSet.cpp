#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "Net/UnrealNetwork.h"

UAO_Fuel_AttributeSet::UAO_Fuel_AttributeSet()
{
	// 기본값 설정
	Fuel.SetBaseValue(0.f);
	Fuel.SetCurrentValue(0.f);
}

void UAO_Fuel_AttributeSet::OnRep_Fuel(const FGameplayAttributeData& OldFuel)
{
	// 이 매크로는 ASC에 복제된 값을 알리고 예측 관련 상태를 맞춰준다
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Fuel_AttributeSet, Fuel, OldFuel);

	// 디버그용 로그 (클라이언트에서 보게 될 것)
	UE_LOG(LogTemp, Warning, TEXT("🔥 Fuel changed! Old: %.1f → New: %.1f"),
		OldFuel.GetCurrentValue(),
		Fuel.GetCurrentValue());
}

void UAO_Fuel_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Fuel_AttributeSet, Fuel, COND_None, REPNOTIFY_Always);
}

