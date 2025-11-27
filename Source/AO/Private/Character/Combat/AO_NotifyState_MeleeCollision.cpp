// AO_NotifyState_MeleeCollision.cpp

#include "Character/Combat/AO_NotifyState_MeleeCollision.h"

#include "AO_Log.h"
#include "GameplayEffect.h"
#include "Character/Combat/AO_DamageComponent.h"

void UAO_NotifyState_MeleeCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		AO_LOG(LogKH, Error, TEXT("MeshComp or MeshComp->GetOwner() is null"));
		return;
	}
	
	OwningActor = MeshComp->GetOwner();

	if (!MeshComp->DoesSocketExist(SocketName))
	{
		AO_LOG(LogKH, Error, TEXT("SocketName does not exist on MeshComp"));
		return;
	}
	
	StartLocation = MeshComp->GetSocketLocation(SocketName);
}

void UAO_NotifyState_MeleeCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (OwningActor.IsValid() && DamageEffectClass)
	{
		if (UAO_DamageComponent* DamageComponent = OwningActor->FindComponentByClass<UAO_DamageComponent>())
		{
			const FVector EndLocation = MeshComp->GetSocketLocation(SocketName);

			DamageComponent->PerformMeleeDamage(DamageEffectClass, DamageAmount, StartLocation, EndLocation, TraceRadius);
		}
	}
}
