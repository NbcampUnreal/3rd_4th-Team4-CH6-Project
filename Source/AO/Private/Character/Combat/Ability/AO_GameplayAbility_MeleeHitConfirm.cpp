// AO_GameplayAbility_MeleeHitConfirm.cpp

#include "Character/Combat/Ability/AO_GameplayAbility_MeleeHitConfirm.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "AO_Log.h"

UAO_GameplayAbility_MeleeHitConfirm::UAO_GameplayAbility_MeleeHitConfirm()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	TraceChannel = ECC_Pawn;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	HitReactEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact"));
}

void UAO_GameplayAbility_MeleeHitConfirm::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	checkf(TriggerEventData, TEXT("TriggerEventData is null"));

	DoMeleeHitConfirm(ActorInfo, TriggerEventData);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAO_GameplayAbility_MeleeHitConfirm::DoMeleeHitConfirm(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayEventData* EventData)
{
	checkf(ActorInfo, TEXT("Failed to get ActorInfo"));
	checkf(ActorInfo->AvatarActor.IsValid(), TEXT("Failed to get AvatarActor"));

	const UAO_MeleeHitEventPayload* Payload = Cast<UAO_MeleeHitEventPayload>(EventData->OptionalObject);
	checkf(Payload, TEXT("Failed to cast EventData to UAO_MeleeHitEventPayload"));

	const FAO_MeleeHitTraceParams& Params = Payload->Params;
	
	TObjectPtr<UWorld> World = GetWorld();
	checkf(World, TEXT("Failed to get World"));

	TArray<FHitResult> HitResults;
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	if (bIgnoreInstigator)
	{
		ActorsToIgnore.Add(ActorInfo->AvatarActor.Get());
	}

	UKismetSystemLibrary::SphereTraceMulti(
		World,
		Params.TraceStart,
		Params.TraceEnd,
		Params.TraceRadius,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResults,
		true);

	TSet<TWeakObjectPtr<AActor>> UniqueHitActors;

	for (const auto& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;
		if (UniqueHitActors.Contains(HitActor)) continue;

		UniqueHitActors.Add(HitActor);
		ApplyDamageToActor(HitActor, Params, ActorInfo);
	}
}

void UAO_GameplayAbility_MeleeHitConfirm::ApplyDamageToActor(AActor* TargetActor, const FAO_MeleeHitTraceParams& Params,
	const FGameplayAbilityActorInfo* ActorInfo)
{
	checkf(TargetActor, TEXT("TargetActor is null"));
	checkf(Params.DamageEffectClass, TEXT("DamageEffectClass is null"));

	TObjectPtr<UAbilitySystemComponent> SourceASC = ActorInfo->AbilitySystemComponent.Get();
	checkf(SourceASC, TEXT("Failed to get SourceASC"));

	TObjectPtr<UAbilitySystemComponent> TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	// 무적 상태인 경우 적용하지 않음
	const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		return;
	}
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

	FGameplayEffectSpecHandle Handle = SourceASC->MakeOutgoingSpec(Params.DamageEffectClass, 1.f, Context);
	checkf(Handle.IsValid(), TEXT("Failed to make Spec Handle"));

	const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
	Handle.Data.Get()->SetSetByCallerMagnitude(DamageTag, Params.DamageAmount);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Handle.Data.Get(), TargetASC);

	SendHitReactEvent(TargetActor, ActorInfo->AvatarActor.Get(), Params.DamageAmount);
}

void UAO_GameplayAbility_MeleeHitConfirm::SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor,
	float DamageAmount)
{
	checkf(TargetActor, TEXT("TargetActor is null"));
	checkf(HitReactEventTag.IsValid(), TEXT("HitReactEventTag is null"));

	FGameplayEventData HitEvent;
	HitEvent.EventTag = HitReactEventTag;
	HitEvent.Instigator = InstigatorActor;
	HitEvent.Target = TargetActor;
	HitEvent.EventMagnitude = DamageAmount;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HitReactEventTag, HitEvent);
}
