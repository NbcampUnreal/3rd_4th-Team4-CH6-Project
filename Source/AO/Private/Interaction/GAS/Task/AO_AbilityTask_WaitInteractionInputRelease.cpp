// HSJ : AO_AbilityTask_WaitInteractionInputRelease.cpp
#include "Interaction/GAS/Task/AO_AbilityTask_WaitInteractionInputRelease.h"
#include "AO_Log.h"
#include "Interaction/Component/AO_InteractionComponent.h"

UAO_AbilityTask_WaitInteractionInputRelease* UAO_AbilityTask_WaitInteractionInputRelease::WaitInteractionInputRelease(UGameplayAbility* OwningAbility)
{
	return NewAbilityTask<UAO_AbilityTask_WaitInteractionInputRelease>(OwningAbility);
}

void UAO_AbilityTask_WaitInteractionInputRelease::Activate()
{
	Super::Activate();
	
	SetWaitingOnAvatar();
	
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndTask();
		return;
	}
	
	UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>();
	if (!InteractionComp)
	{
		AO_LOG(LogHSJ, Error, TEXT("No InteractionComponent found"));
		EndTask();
		return;
	}
	
	// 델리게이트 구독
	InteractionComp->OnInteractInputReleased.AddDynamic(this, &UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased);
	
	// 폴링 시작 (안전장치 - 델리게이트 누락 대비)
	GetWorld()->GetTimerManager().SetTimer(
		CheckTimerHandle,
		[this, InteractionComp]()
		{
			if (!InteractionComp->bIsHoldingInteract)
			{
				OnInputReleased();
			}
		},
		0.1f,
		true
	);
}

void UAO_AbilityTask_WaitInteractionInputRelease::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckTimerHandle);
	}
	
	// 델리게이트 구독 해제
	AActor* AvatarActor = Ability ? Ability->GetCurrentActorInfo()->AvatarActor.Get() : nullptr;
	if (AvatarActor)
	{
		if (UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>())
		{
			InteractionComp->OnInteractInputReleased.RemoveDynamic(this, &UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased);
		}
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased()
{
	OnReleased.Broadcast();
	EndTask();
}