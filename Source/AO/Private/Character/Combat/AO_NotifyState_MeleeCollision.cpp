// AO_NotifyState_MeleeCollision.cpp

#include "Character/Combat/AO_NotifyState_MeleeCollision.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"

#include "AO_Log.h"
#include "Character/Combat/AO_DamageComponent.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"

UAO_NotifyState_MeleeCollision::UAO_NotifyState_MeleeCollision()
{
	HitConfirmEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm"));
}

void UAO_NotifyState_MeleeCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}
	
	checkf(MeshComp, TEXT("MeshComp is invalid"));
	
	OwningActor = MeshComp->GetOwner();
	checkf(OwningActor.IsValid(), TEXT("OwningActor is invalid"));

	if (!SocketName.IsNone())
	{
		StartLocation = MeshComp->GetSocketLocation(SocketName);
	}
	else
	{
		StartLocation = MeshComp->GetComponentLocation();
	}
}

void UAO_NotifyState_MeleeCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	if (!MeshComp || !OwningActor.IsValid() || !DamageEffectClass)
	{
		return;
	}
	
	AActor* Owner = OwningActor.Get();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	FVector EndLocation = StartLocation;
	if (!SocketName.IsNone())
	{
		EndLocation = MeshComp->GetSocketLocation(SocketName);
	}
	else
	{
		EndLocation = MeshComp->GetComponentLocation();
	}
	
	UAO_MeleeHitEventPayload* Payload = NewObject<UAO_MeleeHitEventPayload>(Owner);
	checkf(Payload, TEXT("Failed to create UAO_MeleeHitEventPayload"));

	Payload->Params.TraceStart = StartLocation;
	Payload->Params.TraceEnd = EndLocation;
	Payload->Params.TraceRadius = TraceRadius;
	Payload->Params.DamageEffectClass = DamageEffectClass;
	Payload->Params.DamageAmount = DamageAmount;

	FGameplayEventData EventData;
	EventData.EventTag = HitConfirmEventTag;
	EventData.Instigator = Owner;
	EventData.OptionalObject = Payload;
	EventData.EventMagnitude = DamageAmount;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, HitConfirmEventTag, EventData);
}
