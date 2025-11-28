// KSJ : AO_Enemy_Crab.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_EnemyBase.h"
#include "GameplayTagContainer.h"
#include "AO_Enemy_Crab.generated.h"

/**
 * 게(Crab) 타입 적 AI
 */
UCLASS()
class AO_API AAO_Enemy_Crab : public AAO_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Crab();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* --- Settings --- */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FName PickupSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FGameplayTag IgnoreItemTag;

	// [속도 설정]
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float NormalSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float BurdenedSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float FleeSpeed = 700.0f;

	/* --- State --- */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Pickup")
	TObjectPtr<AActor> HeldItemActor;

	/* --- Actions --- */
	
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void GrabItem(AActor* ItemActor);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void DropItem();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetMovementSpeed(float NewSpeed);

	// 맨몸 도주 모드 켜기/끄기
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetFleeMode(bool bIsFleeing);

protected:
	UFUNCTION(Server, Reliable)
	void ServerGrabItem(AActor* ItemActor);

	UFUNCTION(Server, Reliable)
	void ServerDropItem();
};