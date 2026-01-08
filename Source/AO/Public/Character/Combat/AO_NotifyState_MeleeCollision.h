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

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	// 히트 이벤트 전송 함수 (중복 방지를 위해 분리)
	void SendHitConfirmEvent(USkeletalMeshComponent* MeshComp);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	float DamageAmount = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	float TraceRadius = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Damage")
	FGameplayTag HitConfirmEventTag;

private:
	FVector StartLocation = FVector::ZeroVector;
	FVector PreviousLocation = FVector::ZeroVector;

	TWeakObjectPtr<AActor> OwningActor;

	// 이번 NotifyState 구간에서 이미 히트 이벤트를 보냈는지 여부
	bool bHasSentHitEvent = false;
};
