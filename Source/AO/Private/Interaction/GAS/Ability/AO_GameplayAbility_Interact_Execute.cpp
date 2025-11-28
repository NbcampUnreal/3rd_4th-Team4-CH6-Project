// AO_GameplayAbility_Interact_Execute.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact_Execute.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "Interaction/Component/AO_InteractionComponent.h"
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Interaction/GAS/Task/AO_AbilityTask_WaitForInvalidInteraction.h"
#include "Interaction/GAS/Task/AO_AbilityTask_WaitInteractionInputRelease.h"
#include "Interaction/UI/AO_InteractionWidgetController.h"

UAO_GameplayAbility_Interact_Execute::UAO_GameplayAbility_Interact_Execute()
{
	ActivationPolicy = ECYAbilityActivationPolicy::Manual;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// AbilityTags
		FGameplayTagContainer Tags;
		Tags.AddTag(AO_InteractionTags::Ability_Action_AbilityInteract_Execute);
		SetAssetTags(Tags);
        
		// ActivationOwnedTags
		ActivationOwnedTags.AddTag(AO_InteractionTags::Status_Action_AbilityInteract);
        
		// AbilityTriggers
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = AO_InteractionTags::Ability_Action_AbilityInteract_Execute;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UAO_GameplayAbility_Interact_Execute::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData || !TriggerEventData->Target.Get())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 다른 플레이어가 먼저 사용할 때 해당 액터가 일회성이라 사용 불가능해지면 return
	if (!InitializeAbility(const_cast<AActor*>(TriggerEventData->Target.Get())))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 홀딩 시작 알림
	if (AAO_WorldInteractable* WorldInteractable = Cast<AAO_WorldInteractable>(InteractableActor))
	{
		WorldInteractable->OnInteractActiveStarted(GetAvatarActorFromActorInfo());
	}

	// 즉시 실행 (상호작용 Duration = 0 설정이면)
	if (InteractionInfo.Duration <= 0.f)
	{
		if (!ExecuteInteraction())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
		return;
	}

	// 홀딩 처리 (상호작용 Duration > 0 설정이면)
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
		{
			CharacterMovement->StopMovementImmediately();
		}
	}

	RotateToTarget();

	// UI: 홀딩 프로그레스 표시
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>())
	{
		if (UAO_InteractionWidgetController* Controller = InteractionComp->GetInteractionWidgetController())
		{
			FAO_InteractionMessage Message;
			Message.MessageType = EAO_InteractionMessageType::Progress;
			Message.Instigator = AvatarActor;
			Message.bShouldRefresh = true;
			Message.bSwitchActive = true;
			Message.InteractionInfo = InteractionInfo;
			Controller->BroadcastInteractionMessage(Message);
		}
	}
	
	// 애니메이션: 홀딩 자세
	if (UAnimMontage* HoldMontage = InteractionInfo.ActiveHoldMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("HoldMontage"), HoldMontage))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	// Task: 이탈 감지 (각도/거리 계산해서 홀딩 중 이탈하면 해제)
	if (UAO_AbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = UAO_AbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	// Task: 입력 해제 감지
	if (UAO_AbilityTask_WaitInteractionInputRelease* InputReleaseTask = UAO_AbilityTask_WaitInteractionInputRelease::WaitInteractionInputRelease(this))
	{
		InputReleaseTask->OnReleased.AddDynamic(this, &ThisClass::OnInteractionInputReleased);
		InputReleaseTask->ReadyForActivation();
	}

	// 홀딩 완료 타이머
	UWorld* World = GetWorld();
	if (!World)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 타이머 WeakLambda 처리
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindWeakLambda(this, [this]()
	{
		OnDurationEnded();
	});
	
	World->GetTimerManager().SetTimer(DurationTimerHandle, TimerDelegate, InteractionInfo.Duration, false);
}

void UAO_GameplayAbility_Interact_Execute::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		if (DurationTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(DurationTimerHandle);
		}
		if (MontageTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(MontageTimerHandle);
		}
	}
	
	// 홀딩 종료 알림
	if (AAO_WorldInteractable* WorldInteractable = Cast<AAO_WorldInteractable>(InteractableActor))
	{
		WorldInteractable->OnInteractActiveEnded(GetAvatarActorFromActorInfo());
	}

	// UI: 홀딩 프로그레스 숨김
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>())
	{
		if (UAO_InteractionWidgetController* Controller = InteractionComp->GetInteractionWidgetController())
		{
			FAO_InteractionMessage Message;
			Message.MessageType = EAO_InteractionMessageType::Notice;
			Message.Instigator = AvatarActor;
			Message.bShouldRefresh = false;
			Message.bSwitchActive = true;
			Controller->BroadcastInteractionMessage(Message);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_Interact_Execute::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 홀딩 중 입력 해제 시 취소
	if (InteractionInfo.Duration > 0.f)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

bool UAO_GameplayAbility_Interact_Execute::ExecuteInteraction()
{
	if (!InteractableActor || !InteractionInfo.AbilityToGrant)
	{
		return false;
	}
	
	// 타겟 방향으로 회전
	if (InteractionInfo.Duration <= 0.f)
	{
		RotateToTarget();
	}

	bool bHasMontage = false;
	float MontageLength = 0.f;

	// 애니메이션: 상호작용 완료 액션
	if (UAnimMontage* CompletionMontage = InteractionInfo.ActiveMontage)
	{
		// Interaction 컴포넌트에서 멀티캐스트, AnimInstance에 직접 재생
		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		if (UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>())
		{
			InteractionComp->MulticastPlayInteractionMontage(CompletionMontage);
			
			MontageLength = CompletionMontage->GetPlayLength() + 0.5f;
			bHasMontage = true;
		}
	}

	// Finalize 이벤트 전송
	bool bSuccess = SendFinalizeEvent();
	
	// 몽타주가 있으면 완료 후 종료, 없으면 바로 종료
	if (bHasMontage)
	{
		// 몽타주 완료 후 어빌리티 종료
		if (UWorld* World = GetWorld())
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindWeakLambda(this, [this]()
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			});
			World->GetTimerManager().SetTimer(MontageTimerHandle, TimerDelegate, MontageLength, false);
		}
	}
	else if (bSuccess)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	
	return bSuccess;
}

bool UAO_GameplayAbility_Interact_Execute::SendFinalizeEvent()
{
	FGameplayEventData Payload;
	Payload.EventTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.Target = InteractableActor;
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant))
		{
			return ASC->TriggerAbilityFromGameplayEvent(
				AbilitySpec->Handle,
				ASC->AbilityActorInfo.Get(),
				AO_InteractionTags::Ability_Action_AbilityInteract_Finalize,
				&Payload,
				*ASC
			);
		}
	}
	return false;
}

void UAO_GameplayAbility_Interact_Execute::RotateToTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !InteractableActor)
	{
		return;
	}

	// 타겟 방향 계산
	FVector ToTarget = InteractableActor->GetActorLocation() - Character->GetActorLocation();
	ToTarget.Z = 0.f;
	
	if (!ToTarget.IsNearlyZero())
	{
		FRotator TargetRotation = ToTarget.Rotation();
		Character->SetActorRotation(TargetRotation);
	}
}

void UAO_GameplayAbility_Interact_Execute::OnInvalidInteraction()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UAO_GameplayAbility_Interact_Execute::OnInteractionInputReleased()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UAO_GameplayAbility_Interact_Execute::OnDurationEnded()
{
	if (!ExecuteInteraction())
	{
		AO_LOG(LogHSJ, Error, TEXT("ExecuteInteraction failed"));
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}