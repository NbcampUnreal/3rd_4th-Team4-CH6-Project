// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_InventoryComponent();
	
	// 슬롯들 (예: 4개 단축키용)
	UPROPERTY(ReplicatedUsing=OnRep_Slots, EditAnywhere, BlueprintReadWrite)
	TArray<FInventorySlot> Slots;

	UFUNCTION(Server, Reliable)
	void ServerSetSelectedSlot(int32 NewIndex);
	
	// 현재 선택된 슬롯 인덱스 (1~4)
	UPROPERTY(ReplicatedUsing=OnRep_SelectedIndex, BlueprintReadWrite)
	int32 SelectedSlotIndex;

	// 아이템 사용
	UFUNCTION(BlueprintCallable)
	void UseSelectedItem();

protected:
	void PickupItem(const FInventorySlot& IncomingItem);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRep_Slots();
	UFUNCTION()
	void OnRep_SelectedIndex();
};