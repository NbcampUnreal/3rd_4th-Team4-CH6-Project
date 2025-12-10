// AO_NotifyState_MontageBlendOut.cpp - KH

#include "Character/Traversal/AO_NotifyState_MontageBlendOut.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAO_NotifyState_MontageBlendOut::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}
	
	Character = Cast<ACharacter>(MeshComp->GetOwner());
	checkf(Character, TEXT("Failed to cast MeshComp to ACharacter"));

	CharacterMovement = Character->GetCharacterMovement();
	checkf(CharacterMovement, TEXT("Failed to get CharacterMovementComponent"));
	
	AnimInstance = MeshComp->GetAnimInstance();
	checkf(AnimInstance, TEXT("Failed to get AnimInstance"));
	
	AnimMontage = Cast<UAnimMontage>(Animation);
	checkf(AnimMontage, TEXT("Failed to cast AnimInstance to UAnimMontage"));
}

void UAO_NotifyState_MontageBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	bool bShouldBlendOut = false;
	if (BlendOutCondition == EAO_TraversalBlendOutCondition::ForceBlendOut)
	{
		bShouldBlendOut = true;
	}
	else if (BlendOutCondition == EAO_TraversalBlendOutCondition::WithMovementInput)
	{
		if (!CharacterMovement->GetCurrentAcceleration().IsNearlyZero(0.1f))
		{
			bShouldBlendOut = true;
		}
	}
	else if (BlendOutCondition == EAO_TraversalBlendOutCondition::IfFalling)
	{
		if (CharacterMovement->IsFalling())
		{
			bShouldBlendOut = true;
		}
	}

	if (!bShouldBlendOut)
	{
		return;
	}

	// 블렌딩 시간과 커브를 정의 (현재 커브는 사용하지 않음)
	FAlphaBlendArgs AlphaBlendArgs;
	AlphaBlendArgs.BlendTime = BlendOutTime;
	AlphaBlendArgs.BlendOption = EAlphaBlendOption::HermiteCubic;

	// BlendProfile : 몽타주 종료 시 특정 관절별로 블렌딩 가중치를 다르게 적용하는 블렌드 프로파일을 저장하고 있음
	FMontageBlendSettings BlendSettings;
	BlendSettings.BlendProfile = const_cast<UBlendProfile*>(AnimInstance->GetBlendProfileByName(BlendProfile));
	BlendSettings.Blend = AlphaBlendArgs;
	BlendSettings.BlendMode = EMontageBlendMode::Standard;

	// BlendSettings를 사용하고 현재 재생 중인 Montage를 즉시 멈추고 블렌드 아웃을 진행함
	AnimInstance->Montage_StopWithBlendSettings(BlendSettings, AnimMontage);
}

FString UAO_NotifyState_MontageBlendOut::GetNotifyName_Implementation() const
{
	FString NotifyName = TEXT("Blend Out - ");

	if (const TObjectPtr<UEnum> EnumPtr = StaticEnum<EAO_TraversalBlendOutCondition>())
	{
		FString EnumString = EnumPtr->GetNameStringByValue(static_cast<int64>(BlendOutCondition));

		NotifyName.Append(EnumString);
	}
	else
	{
		NotifyName.Append(TEXT("Unknown Condition"));
	}
	
	return NotifyName;
}
