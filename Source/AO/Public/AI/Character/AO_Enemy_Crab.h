// AO_Enemy_Crab.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AO_Enemy_Crab.generated.h"

class AAO_TestPickupItem;

UCLASS()
class AO_API AAO_Enemy_Crab : public ACharacter
{
	GENERATED_BODY()

public:
	AAO_Enemy_Crab();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SneakSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FName PickupSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	AAO_TestPickupItem* HeldItem;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UseWalkSpeed();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UseSneakSpeed();

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void GrabItem(AAO_TestPickupItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void DropItem();

	UFUNCTION(Server, Reliable)
	void ServerGrabItem(AAO_TestPickupItem* Item);

	UFUNCTION(Server, Reliable)
	void ServerDropItem();
};