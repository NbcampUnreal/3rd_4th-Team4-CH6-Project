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
	
	AbilityTags.AddTag(AO_InteractionTags::Ability_Action_AbilityInteract_Execute);
	ActivationOwnedTags.AddTag(AO_InteractionTags::Status_Action_AbilityInteract);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
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
		if (ExecuteInteraction())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
		else
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
	
	// 애니메이션: 홀딩 시작
	if (UAnimMontage* ActiveStartMontage = InteractionInfo.ActiveStartMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("InteractMontage"), ActiveStartMontage))
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

	// 애니메이션: 홀딩 종료
	if (UAnimMontage* ActiveEndMontage = InteractionInfo.ActiveEndMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("InteractMontage"), ActiveEndMontage))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	// inalize 이벤트 발생, 상호작용 당하는 대상이 AbilityTriggers에서 Finalize 이벤트를 듣고 있음
	// 그러면 Finalize 이벤트 이후 당하는 대상이 ActivateAbility 실행
	FGameplayEventData Payload;
	Payload.EventTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.Target = InteractableActor;

	//  Payload에 추가 정보를 넣을 수 있음, 아직은 사용 X
	// 상호작용 액터에서 오버라이드해서 사용하면 가능
	Interactable->CustomizeInteractionEventData(AO_InteractionTags::Ability_Action_AbilityInteract_Finalize, Payload);
	
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
	if (ExecuteInteraction())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		AO_LOG(LogHSJ, Error, TEXT("ExecuteInteraction failed"));
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}