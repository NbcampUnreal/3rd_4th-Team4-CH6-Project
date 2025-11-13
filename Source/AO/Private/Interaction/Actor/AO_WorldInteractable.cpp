// HSJ :  AO_WorldInteractable.cpp
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Net/UnrealNetwork.h"

AAO_WorldInteractable::AAO_WorldInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AAO_WorldInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bWasConsumed);
}

void AAO_WorldInteractable::OnInteractActiveStarted(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	// 서버: 홀딩 중인 플레이어 추적
	if (HasAuthority())
	{
		CachedInteractors.Add(Interactor);
	}

	// 블루프린트 이벤트 호출
	K2_OnInteractActiveStarted(Interactor);
}

void AAO_WorldInteractable::OnInteractActiveEnded(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	// 서버: 홀딩 종료한 플레이어 제거
	if (HasAuthority())
	{
		CachedInteractors.RemoveSingleSwap(Interactor);
	}

	// 블루프린트 이벤트 호출
	K2_OnInteractActiveEnded(Interactor);
}

void AAO_WorldInteractable::OnInteractionSuccess(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	if (HasAuthority())
	{
		// 소모성 상호작용 처리
		if (bShouldConsume)
		{
			bWasConsumed = true;

			// 다른 홀딩 중인 플레이어들의 상호작용 취소
			// (먼저 성공한 플레이어가 독점)
			TArray<TWeakObjectPtr<AActor>> TargetInteractors = MoveTemp(CachedInteractors);

			for (TWeakObjectPtr<AActor>& TargetInteractor : TargetInteractors)
			{
				AActor* TargetActor = TargetInteractor.Get();
				if (!TargetActor || Interactor == TargetActor)
				{
					continue;
				}

				// 다른 플레이어의 상호작용 어빌리티 강제 취소
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
				{
					FGameplayTagContainer CancelAbilitiesTag;
					CancelAbilitiesTag.AddTag(AO_InteractionTags::Status_Action_AbilityInteract);
					ASC->CancelAbilities(&CancelAbilitiesTag);
				}
			}
		}
		else
		{
			// 비소모성: 홀딩 목록만 정리
			CachedInteractors.Empty();
		}
	}
	
	// 블루프린트 이벤트 호출
	K2_OnInteractionSuccess(Interactor);
}

bool AAO_WorldInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	// 소모성이면 아직 소모 안 됐을 때만 가능
	// 비소모성이면 항상 가능
	return bShouldConsume ? !bWasConsumed : true;
}

void AAO_WorldInteractable::OnRep_WasConsumed()
{
	// 소모 상태 복제 완료 (클라이언트에서 UI 업데이트 등에 사용 가능)
}