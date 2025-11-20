// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/invenroty/AO_InventoryComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UAO_InventoryComponent::UAO_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_InventoryComponent::ServerSetSelectedSlot_Implementation(int32 NewIndex)
{
	if (NewIndex < 0 || NewIndex >= Slots.Num())
		return;

	SelectedSlotIndex = NewIndex; 
}

void UAO_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_InventoryComponent, Slots);
	DOREPLIFETIME(UAO_InventoryComponent, SelectedSlotIndex);
}

void UAO_InventoryComponent::UseSelectedItem()
{
	// 서버 권한에서만 처리하거나 서버 RPC로 라우팅해야 함
	// 예: if (GetOwnerRole() == ROLE_Authority) { ... }
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem)
{
	/*// 1) 빈 슬롯 검색
	int32 EmptyIndex = FindEmptySlot();

	if (EmptyIndex != INDEX_NONE)
	{
		Slots[EmptyIndex] = IncomingItem;
		return;
	}

	// 2) 전부 찼다면 SelectedSlotIndex 교체
	if (IsValidSlotIndex(SelectedSlotIndex))
	{
		Slots[SelectedSlotIndex] = IncomingItem;
	}*/
}

void UAO_InventoryComponent::OnRep_Slots()
{
	// 클라이언트 UI 갱신 
}
void UAO_InventoryComponent::OnRep_SelectedIndex()
{
	// 클라이언트 HUD 업데이트
}