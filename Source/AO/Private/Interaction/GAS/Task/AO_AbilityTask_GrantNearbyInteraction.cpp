// HSJ : AO_AbilityTask_GrantNearbyInteraction.cpp
#include "Interaction/GAS/Task/AO_AbilityTask_GrantNearbyInteraction.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"
#include "Physics/AO_CollisionChannels.h"

UAO_AbilityTask_GrantNearbyInteraction* UAO_AbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(UGameplayAbility* OwningAbility, float InteractionAbilityScanRange, float InteractionAbilityScanRate)
{
	UAO_AbilityTask_GrantNearbyInteraction* Task = NewAbilityTask<UAO_AbilityTask_GrantNearbyInteraction>(OwningAbility);
	Task->InteractionAbilityScanRange = InteractionAbilityScanRange;
	Task->InteractionAbilityScanRate = InteractionAbilityScanRate;
	return Task;
}

void UAO_AbilityTask_GrantNearbyInteraction::Activate()
{
	Super::Activate();
	
	SetWaitingOnAvatar();

	// 주기적 탐색 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(QueryTimerHandle, this, &UAO_AbilityTask_GrantNearbyInteraction::QueryInteractables, InteractionAbilityScanRate, true);
	}
}

void UAO_AbilityTask_GrantNearbyInteraction::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAO_AbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActor();
	
	if (!World || !AvatarActor)
	{
		return;
	}

	// 제거 대상 어빌리티 추적 (이번 프레임에 감지되지 않으면 제거될 것)
	TSet<FObjectKey> RemoveKeys;
	GrantedInteractionAbilities.GetKeys(RemoveKeys);

	// OverlapSphere로 주변 오브젝트 탐색
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAO_AbilityTask_GrantNearbyInteraction), false);
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults, 
		AvatarActor->GetActorLocation(), 
		FQuat::Identity, 
		AO_TraceChannel_Interaction, 
		FCollisionShape::MakeSphere(InteractionAbilityScanRange), 
		Params
	);
	
	if (OverlapResults.Num() > 0)
	{
		// IAO_Interactable 인터페이스 구현체 수집
		TArray<TScriptInterface<IAO_Interface_Interactable>> Interactables;
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			// Actor 자체가 Interactable인 경우
			TScriptInterface<IAO_Interface_Interactable> InteractableActor(OverlapResult.GetActor());
			if (InteractableActor)
			{
				Interactables.AddUnique(InteractableActor);
			}
			
			// Component가 Interactable인 경우
			TScriptInterface<IAO_Interface_Interactable> InteractableComponent(OverlapResult.GetComponent());
			if (InteractableComponent)
			{
				Interactables.AddUnique(InteractableComponent);
			}
		}

		// 상호작용 정보 수집
		FAO_InteractionQuery InteractionQuery;
		InteractionQuery.RequestingAvatar = AvatarActor;
		InteractionQuery.RequestingController = Cast<AController>(AvatarActor->GetOwner());
		
		for (TScriptInterface<IAO_Interface_Interactable>& Interactable : Interactables)
		{
			// 상호작용 정보 가져오기
			FAO_InteractionInfo InteractionInfo = Interactable->GetInteractionInfo(InteractionQuery);
			InteractionInfo.Interactable = Interactable;

			// 필요한 어빌리티 부여 (중복 부여 방지)
			if (InteractionInfo.AbilityToGrant)
			{
				FObjectKey ObjectKey(InteractionInfo.AbilityToGrant);
				
				// 이미 부여된 어빌리티면 제거 목록에서 제외
				if (GrantedInteractionAbilities.Find(ObjectKey))
				{
					RemoveKeys.Remove(ObjectKey);
				}
				// 새로운 어빌리티면 부여
				else
				{
					FGameplayAbilitySpec Spec(InteractionInfo.AbilityToGrant, 1, INDEX_NONE, this);
					FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(Spec);
					GrantedInteractionAbilities.Add(ObjectKey, SpecHandle);
				}
			}
		}
	}
	
	// 범위를 벗어난 어빌리티 제거
	// (이번 프레임에 감지되지 않았으므로 RemoveKeys에 남아있음)
	for (const FObjectKey& RemoveKey : RemoveKeys)
	{
		AbilitySystemComponent->ClearAbility(GrantedInteractionAbilities[RemoveKey]);
		GrantedInteractionAbilities.Remove(RemoveKey);
	}
}