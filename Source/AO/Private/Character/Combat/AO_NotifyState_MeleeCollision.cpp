// AO_NotifyState_MeleeCollision.cpp

#include "Character/Combat/AO_NotifyState_MeleeCollision.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"

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
	checkf(World, TEXT("Failed to get World"));

	// 에디터에서 실행 제외
	if (!World->IsGameWorld())
	{
		return;
	}
	
	checkf(MeshComp, TEXT("MeshComp is invalid"));
	
	OwningActor = MeshComp->GetOwner();
	checkf(OwningActor.IsValid(), TEXT("OwningActor is invalid"));
	
	bHasSentHitEvent = false;

	if (!SocketName.IsNone())
	{
		StartLocation = MeshComp->GetSocketLocation(SocketName);
		PreviousLocation = StartLocation;
	}
	else
	{
		StartLocation = MeshComp->GetComponentLocation();
		PreviousLocation = StartLocation;
	}
}

void UAO_NotifyState_MeleeCollision::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (bHasSentHitEvent)
	{
		return;
	}

	TObjectPtr<UWorld> World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	if (!OwningActor.IsValid())
	{
		return;
	}

	AActor* Owner = OwningActor.Get();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}
	
	FVector CurrentLocation;
	if (!SocketName.IsNone())
	{
		CurrentLocation = MeshComp->GetSocketLocation(SocketName);
	}
	else
	{
		CurrentLocation = MeshComp->GetComponentLocation();
	}
	
	SendHitConfirmEvent(MeshComp);
	
	PreviousLocation = CurrentLocation;
}

void UAO_NotifyState_MeleeCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (bHasSentHitEvent)
	{
		return;
	}

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

	// Tick에서 못 보낸 경우 End에서 전송 (폴백)
	SendHitConfirmEvent(MeshComp);
}

void UAO_NotifyState_MeleeCollision::SendHitConfirmEvent(USkeletalMeshComponent* MeshComp)
{
	if (bHasSentHitEvent)
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

	// 현재 소켓 위치 가져오기
	FVector CurrentLocation;
	if (!SocketName.IsNone())
	{
		CurrentLocation = MeshComp->GetSocketLocation(SocketName);
	}
	else
	{
		CurrentLocation = MeshComp->GetComponentLocation();
	}

	UAO_MeleeHitEventPayload* Payload = NewObject<UAO_MeleeHitEventPayload>(Owner);
	if (!Payload)
	{
		return;
	}

	// 이전 위치에서 현재 위치까지의 궤적으로 Trace 정보 설정
	Payload->Params.TraceStart = PreviousLocation;
	Payload->Params.TraceEnd = CurrentLocation;
	Payload->Params.TraceRadius = TraceRadius;
	Payload->Params.DamageEffectClass = DamageEffectClass;
	Payload->Params.DamageAmount = DamageAmount;

	FGameplayEventData EventData;
	EventData.EventTag = HitConfirmEventTag;
	EventData.Instigator = Owner;
	EventData.OptionalObject = Payload;
	EventData.EventMagnitude = DamageAmount;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, HitConfirmEventTag, EventData);

	// 이벤트 전송 완료 플래그 설정
	bHasSentHitEvent = true;
}