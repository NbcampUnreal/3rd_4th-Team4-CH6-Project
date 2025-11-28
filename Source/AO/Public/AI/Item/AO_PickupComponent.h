// KSJ : AO_PickupComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AO_PickupComponent.generated.h"

/**
 * 아이템액터(MasterItem)에 부착해 AI(Crab)이 주울 수 있게 하는 Component
 * - 물리 시뮬레이션 제어
 * - 소유권 및 상태 태그 관리
 * - 멀티플레이 동기화 될 것
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AO_API UAO_PickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_PickupComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/* --- AI Actions --- */

	// 아이템 줍기 시도 (Server)
	UFUNCTION(BlueprintCallable, Category = "AI Pickup")
	bool TryPickup(USceneComponent* AttachTo, FName SocketName);

	// 아이템 떨어뜨리기 시도 (Server)
	UFUNCTION(BlueprintCallable, Category = "AI Pickup")
	void TryDrop();

	/* --- State Queries --- */
	UFUNCTION(BlueprintCallable, Category = "AI Pickup")
	bool IsPickedUp() const { return bIsPickedUp; }

	UFUNCTION(BlueprintCallable, Category = "AI Pickup")
	bool HasTag(FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintCallable, Category = "AI Pickup")
	void AddTag(FGameplayTag TagToAdd);

	const FGameplayTagContainer& GetItemTags() const { return ItemTags; }

	void SetIgnoreCooldown(float Duration);
	bool IsInCooldown() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp, VisibleAnywhere, BlueprintReadOnly)
	bool bIsPickedUp;

	UPROPERTY(Replicated)
	FGameplayTagContainer ItemTags;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> TargetPhysicsComp;
	
	double CooldownExpireTime = 0.0;

	UFUNCTION()
	void OnRep_IsPickedUp();

	void ConfigurePhysics(bool bEnablePhysics);
};