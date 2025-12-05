// HSJ : AO_GameplayAbility_Interact_Execute.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact_Execute.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Component/AO_InteractionComponent.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Interaction/GAS/Task/AO_AbilityTask_WaitForInvalidInteraction.h"
#include "Interaction/GAS/Task/AO_AbilityTask_WaitInteractionInputRelease.h"
#include "Interaction/UI/AO_InteractionWidgetController.h"
#include "Puzzle/Element/AO_DecayHoldElement.h"
#include "Puzzle/Element/AO_PuzzleElement.h"

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

	bHoldingPhaseCompleted = false;
	
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

	// 노티파이 리스너 등록
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTag FinalizeTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
		ASC->GenericGameplayEventCallbacks.FindOrAdd(FinalizeTag).AddUObject(this, &ThisClass::OnAnimNotifyReceived);
	}

	// 홀딩 시작 알림
	if (AAO_WorldInteractable* WorldInteractable = Cast<AAO_WorldInteractable>(InteractableActor))
	{
		WorldInteractable->OnInteractActiveStarted(GetAvatarActorFromActorInfo());
	}

	// DecayHoldElement인 경우 자체 관리
	if (Cast<AAO_DecayHoldElement>(InteractableActor))
	{
		// 입력 해제 감지
		if (UAO_AbilityTask_WaitInteractionInputRelease* InputReleaseTask = 
			UAO_AbilityTask_WaitInteractionInputRelease::WaitInteractionInputRelease(this))
		{
			InputReleaseTask->OnReleased.AddDynamic(this, &ThisClass::OnInteractionInputReleased);
			InputReleaseTask->ReadyForActivation();
		}
		return;
	}

	// 즉시 실행 (Duration = 0)
	if (InteractionInfo.Duration <= 0.f)
	{
		bHoldingPhaseCompleted = true;
		if (!ExecuteInteraction())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
		return;
	}

	// 홀딩 처리
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
	if (UAO_AbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = 
		UAO_AbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	// Task: 입력 해제 감지
	if (UAO_AbilityTask_WaitInteractionInputRelease* InputReleaseTask = 
		UAO_AbilityTask_WaitInteractionInputRelease::WaitInteractionInputRelease(this))
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
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    
    // DecayHold인 경우 움직임 다시 복구
    if (AAO_DecayHoldElement* DecayElement = Cast<AAO_DecayHoldElement>(InteractableActor))
    {
        DecayElement->MulticastSetMovementForActor(AvatarActor, true);
    }
    
    // 홀딩 종료 알림
    if (AAO_WorldInteractable* WorldInteractable = Cast<AAO_WorldInteractable>(InteractableActor))
    {
        WorldInteractable->OnInteractActiveEnded(AvatarActor);
    }
    
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

    // 노티파이 리스너 해제
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTag FinalizeTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
        ASC->GenericGameplayEventCallbacks.FindOrAdd(FinalizeTag).RemoveAll(this);
    }
    
    bHoldingPhaseCompleted = false;

    // UI: 홀딩 프로그레스 숨김
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
	if (InteractionInfo.Duration > 0.f && !bHoldingPhaseCompleted)
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

	UAnimMontage* MontageToPlay = nullptr;
	
	if (AAO_PuzzleElement* PuzzleElement = Cast<AAO_PuzzleElement>(InteractableActor))
	{
		if (PuzzleElement->GetElementType() == EPuzzleElementType::Toggle)
		{
			MontageToPlay = PuzzleElement->IsActivated() 
				? InteractionInfo.DeactivateMontage 
				: InteractionInfo.ActiveMontage;
		}
		else
		{
			MontageToPlay = InteractionInfo.ActiveMontage;
		}
	}
	else if (AAO_BaseInteractable* BaseInteractable = Cast<AAO_BaseInteractable>(InteractableActor))
	{
		if (BaseInteractable->IsToggleable())
		{
			MontageToPlay = BaseInteractable->IsActivated() 
				? InteractionInfo.DeactivateMontage 
				: InteractionInfo.ActiveMontage;
		}
		else
		{
			MontageToPlay = InteractionInfo.ActiveMontage;
		}
	}
	else
	{
		MontageToPlay = InteractionInfo.ActiveMontage;
	}

	if (!MontageToPlay)
	{
		MontageToPlay = InteractionInfo.ActiveMontage 
			? InteractionInfo.ActiveMontage 
			: InteractionInfo.DeactivateMontage;
	}

	bool bHasMontage = false;
	float MontageLength = 0.f;

	if (MontageToPlay)
	{
		// Interaction 컴포넌트에서 멀티캐스트, AnimInstance에 직접 재생
		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		if (UAO_InteractionComponent* InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>())
		{
			InteractionComp->MulticastPlayInteractionMontage(
				MontageToPlay,
				InteractionInfo.InteractionTransform,
				InteractionInfo.WarpTargetName
			);
			
			MontageLength = MontageToPlay->GetPlayLength() + 0.5f;
			bHasMontage = true;
		}
	}

	bool bShouldWaitForNotify = InteractionInfo.bWaitForAnimationNotify && bHasMontage;
	bool bSuccess = false;

	if (!bShouldWaitForNotify)
	{
		bSuccess = SendFinalizeEvent();
	}
	
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
	
	return bHasMontage || bSuccess;
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

void UAO_GameplayAbility_Interact_Execute::OnInvalidInteraction()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UAO_GameplayAbility_Interact_Execute::OnInteractionInputReleased()
{
	// Decay 홀드인 경우
	if (AAO_DecayHoldElement* DecayElement = Cast<AAO_DecayHoldElement>(InteractableActor))
    {
        bool bIsServer = GetOwningActorFromActorInfo()->HasAuthority();
        
        if (!bIsServer)
        {
            // 클라이언트인 경우 서버에 알리고 타이머로 대기
            ServerNotifyDecayHoldInputReleased();
            
            // 몽타주 완료 대기
            if (UWorld* World = GetWorld())
            {
                float RemainingTime = DecayElement->GetActiveMontageRemainingTime();
                
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindWeakLambda(this, [this]()
                {
                    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
                });
                
                World->GetTimerManager().SetTimer(MontageTimerHandle, TimerDelegate, RemainingTime, false);
            }
            return;
        }
        
        // 서버인 경우 직접 처리
        int32 NotifyCount = DecayElement->GetNotifyCount();
        
        if (NotifyCount < 2)
        {
            DecayElement->StopEarly();
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
        else
        {
            DecayElement->ReleasePause();
            
            if (UWorld* World = GetWorld())
            {
                float RemainingTime = DecayElement->GetActiveMontageRemainingTime();
                
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindWeakLambda(this, [this, DecayElement]()
                {
                    DecayElement->CleanupAfterMontage();
                    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
                });
                
                World->GetTimerManager().SetTimer(MontageTimerHandle, TimerDelegate, RemainingTime, false);
            }
        }
        return;
    }
    
    // 일반 상호작용
    if (!bHoldingPhaseCompleted)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
    }
}

void UAO_GameplayAbility_Interact_Execute::OnAnimNotifyReceived(const FGameplayEventData* EventData)
{
	if (!EventData)
	{
		return;
	}
    
	// DecayHold인 경우 내부 상태로 처리
	if (AAO_DecayHoldElement* DecayElement = Cast<AAO_DecayHoldElement>(InteractableActor))
	{
		DecayElement->OnNotifyReceived();
		return;
	}
    
	// 일반 상호작용인 경우 바로 Finalize 전송
	SendFinalizeEvent();
}

void UAO_GameplayAbility_Interact_Execute::ServerNotifyDecayHoldInputReleased_Implementation()
{
	if (AAO_DecayHoldElement* DecayElement = Cast<AAO_DecayHoldElement>(InteractableActor))
	{
		int32 NotifyCount = DecayElement->GetNotifyCount();
        
		if (NotifyCount < 2)
		{
			DecayElement->StopEarly();
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
		else
		{
			DecayElement->ReleasePause();
            
			if (UWorld* World = GetWorld())
			{
				float RemainingTime = DecayElement->GetActiveMontageRemainingTime();
                
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindWeakLambda(this, [this, DecayElement]()
				{
					DecayElement->CleanupAfterMontage();
					EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
				});
                
				World->GetTimerManager().SetTimer(MontageTimerHandle, TimerDelegate, RemainingTime, false);
			}
		}
	}
}

void UAO_GameplayAbility_Interact_Execute::OnDurationEnded()
{
	bHoldingPhaseCompleted = true;
	if (!ExecuteInteraction())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}
