// AO_NotifyState_MeleeCollision.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AO_NotifyState_MeleeCollision.generated.h"

class UGameplayEffect;

UCLASS()
class AO_API UAO_NotifyState_MeleeCollision : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAO_NotifyState_MeleeCollision();
	
protected:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	float DamageAmount = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	float TraceRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	FGameplayTag HitConfirmEventTag;

private:
	FVector StartLocation = FVector::ZeroVector;

	TWeakObjectPtr<AActor> OwningActor;
};
