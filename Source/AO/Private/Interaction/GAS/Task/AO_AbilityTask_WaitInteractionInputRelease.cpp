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
	
	checkf(Ability, TEXT("Ability is null in Activate"));
	
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	checkf(ActorInfo, TEXT("ActorInfo is null in Activate"));
	
	TObjectPtr<AActor> AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndTask();
		return;
	}
	
	TWeakObjectPtr<UAO_InteractionComponent> InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>();
	if (!InteractionComp.IsValid())
	{
		AO_LOG(LogHSJ, Error, TEXT("No InteractionComponent found"));
		EndTask();
		return;
	}
	
	// 델리게이트 구독
	InteractionComp->OnInteractInputReleased.AddDynamic(this, &UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased);
	
	// 폴링 시작
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		TWeakObjectPtr<UAO_AbilityTask_WaitInteractionInputRelease> WeakThis(this);
		
		World->GetTimerManager().SetTimer(
			CheckTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakThis, InteractionComp]()
			{
				if (!InteractionComp.IsValid())
				{
					if (TObjectPtr<UAO_AbilityTask_WaitInteractionInputRelease> StrongThis = WeakThis.Get())
					{
						StrongThis->OnInputReleased();
					}
					return;
				}
                
				if (!InteractionComp->bIsHoldingInteract)
				{
					if (TObjectPtr<UAO_AbilityTask_WaitInteractionInputRelease> StrongThis = WeakThis.Get())
					{
						StrongThis->OnInputReleased();
					}
				}
			}),
			0.1f,
			true
		);
	}
}

void UAO_AbilityTask_WaitInteractionInputRelease::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(CheckTimerHandle);
	}
	
	// 델리게이트 구독 해제
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		if (ActorInfo)
		{
			TObjectPtr<AActor> AvatarActor = ActorInfo->AvatarActor.Get();
			if (AvatarActor)
			{
				TObjectPtr<UAO_InteractionComponent> InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>();
				if (InteractionComp)
				{
					InteractionComp->OnInteractInputReleased.RemoveDynamic(this, &UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased);
				}
			}
		}
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAO_AbilityTask_WaitInteractionInputRelease::OnInputReleased()
{
	OnReleased.Broadcast();
	EndTask();
}