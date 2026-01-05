// AO_AnimNotify_SendDeathRagdollEvent.cpp

#include "Character/AnimNotify/AO_AnimNotify_SendDeathRagdollEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AO_Log.h"
#include "Character/GAS/Ability/AO_GameplayAbility_Death.h"

void UAO_AnimNotify_SendDeathRagdollEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	checkf(World, TEXT("Failed to get World"));
	
	// 에디터에서 실행 제외
	if (!World->IsGameWorld())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	AO_LOG(LogKH, Log, TEXT("Send Death Ragdoll Event"));

	if (!RagdollEventTag.IsValid())
	{
		RagdollEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Death.Ragdoll"));
	}

	FGameplayEventData Payload;
	Payload.EventTag = RagdollEventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, RagdollEventTag, Payload);
}
