// AO_ItemCarryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_ItemCarryComponent.generated.h"

class AAO_MasterItem;
class UAO_AISubsystem;
class AAO_AIControllerBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUp, AAO_MasterItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, AAO_MasterItem*, Item);

/**
 * Crab AI의 아이템 운반 컴포넌트
 * - 아이템 줍기/드롭 로직
 * - 운반 중 상태 관리
 * - AISubsystem과 연동하여 중복 줍기 방지
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AO_API UAO_ItemCarryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_ItemCarryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 아이템 줍기 시도
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	bool TryPickupItem(AAO_MasterItem* Item);

	// 아이템 드롭
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	void DropItem();

	// 아이템 강제 드롭 (기절 시)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	void ForceDropItem();

	// 현재 운반 중인지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	bool IsCarryingItem() const { return CarriedItem != nullptr; }

	// 현재 운반 중인 아이템
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	AAO_MasterItem* GetCarriedItem() const { return CarriedItem; }

	// 주변에서 줍을 수 있는 아이템 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	AAO_MasterItem* FindNearbyItem(float SearchRadius) const;

	// 아이템이 줍기 가능한지 확인
	UFUNCTION(BlueprintCallable, Category = "AO|AI|ItemCarry")
	bool CanPickupItem(AAO_MasterItem* Item) const;

public:
	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|ItemCarry")
	FOnItemPickedUp OnItemPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "AO|AI|ItemCarry")
	FOnItemDropped OnItemDropped;

protected:
	virtual void BeginPlay() override;

	// AISubsystem 가져오기
	UAO_AISubsystem* GetAISubsystem() const;

	// AIController 가져오기
	AAO_AIControllerBase* GetAIController() const;

protected:
	// 현재 운반 중인 아이템
	UPROPERTY(ReplicatedUsing = OnRep_CarriedItem, BlueprintReadOnly, Category = "AO|AI|ItemCarry")
	TObjectPtr<AAO_MasterItem> CarriedItem;

	// 아이템을 붙일 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|ItemCarry")
	FName PickupSocketName = FName("PickupSocket");

	// 아이템 드롭 후 쿨다운 시간 (다시 줍지 못하는 시간)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|ItemCarry")
	float DropCooldownTime = 20.f;

	UFUNCTION()
	void OnRep_CarriedItem();
};
