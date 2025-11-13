// AO_AnimInstance_Crab.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AO_AnimInstnace_Crab.generated.h"

UCLASS()
class AO_API UAO_AnimInstance_Crab : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAO_AnimInstance_Crab();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData");
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData");
	float YawRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData");
	bool bIsTurningLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData");
	bool bIsTurningRight;

protected:
	float PrevYaw;

	APawn* CachedPawn;
};