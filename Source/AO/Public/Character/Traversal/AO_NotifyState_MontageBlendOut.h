// AO_NotifyState_MontageBlendOut.h - KH

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AO_NotifyState_MontageBlendOut.generated.h"

class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EAO_TraversalBlendOutCondition : uint8
{
	ForceBlendOut		UMETA(DisplayName = "ForceBlendOut"),
	WithMovementInput	UMETA(DisplayName = "WithMovementInput"),
	IfFalling			UMETA(DisplayName = "IfFalling")
};

UCLASS()
class AO_API UAO_NotifyState_MontageBlendOut : public UAnimNotifyState
{
	GENERATED_BODY()

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
	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MontageBlendOut")
	EAO_TraversalBlendOutCondition BlendOutCondition = EAO_TraversalBlendOutCondition::ForceBlendOut;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MontageBlendOut")
	float BlendOutTime = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MontageBlendOut")
	FName BlendProfile = FName(TEXT("FastFeet_InstantRoot"));

private:
	UPROPERTY()
	TObjectPtr<ACharacter> Character;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;
	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;
	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage;
};
