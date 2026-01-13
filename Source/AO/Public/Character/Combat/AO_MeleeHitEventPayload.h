// AO_MeleeHitEventPayload.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AO_MeleeHitEventPayload.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct AO_API FAO_MeleeHitTraceParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TraceEnd = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 0.f;
};

UCLASS()
class AO_API UAO_MeleeHitEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FAO_MeleeHitTraceParams Params;
};
