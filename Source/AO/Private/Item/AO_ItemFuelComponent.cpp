#include "Item/AO_ItemFuelComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAO_ItemFuelComponent::UAO_ItemFuelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_ItemFuelComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// ASC 가져오기
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!OwnerASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ Owner has no AbilitySystemComponent!"));
		return;
	}

	// AttributeSet 가져오기 (const_cast로 안전하게)
	const UAO_Fuel_AttributeSet* FuelSet = OwnerASC->GetSet<UAO_Fuel_AttributeSet>();
	if (!FuelSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ Owner ASC has no FuelAttributeSet!"));
		return;
	}

	OwnerFuelAttributeSet = const_cast<UAO_Fuel_AttributeSet*>(FuelSet);
}

void UAO_ItemFuelComponent::ApplyFuel()
{
	if (!OwnerFuelAttributeSet || !OwnerASC || !GetOwner()->HasAuthority())
		return;

	// 기존 연산: Fuel += AddFuelAmount
	float CurrentFuel = OwnerFuelAttributeSet->Fuel.GetCurrentValue();
	float NewFuel = CurrentFuel + AddFuelAmount;

	OwnerFuelAttributeSet->Fuel.SetCurrentValue(NewFuel);

	// ASC에 알리기
	OwnerASC->ForceReplication();
	UE_LOG(LogTemp, Warning, TEXT("🔥 Applied Fuel: %.1f → %.1f"), CurrentFuel, NewFuel);
}
