// AO_DamageComponent.cpp

#include "Character/Combat/AO_DamageComponent.h"

#include "AO_Log.h"
#include "Kismet/KismetSystemLibrary.h"

UAO_DamageComponent::UAO_DamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAO_DamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerASC = Cast<UAbilitySystemComponent>(GetOwner()->GetComponentByClass(UAbilitySystemComponent::StaticClass()));
}

void UAO_DamageComponent::PerformMeleeDamage(TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount,
                                             FVector TraceStart, FVector TraceEnd, float TraceRadius)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!DamageEffectClass)
	{
		AO_LOG(LogKH, Warning, TEXT("DamageEffectClass is null"));
		return;
	}

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		TraceStart,
		TraceEnd,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true);

	if (!bHit)
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> HitActors;

	for (const auto& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) continue;
		if (HitActors.Contains(HitActor)) continue;

		HitActors.Add(HitActor);
		
		if (UAbilitySystemComponent* TargetASC = HitActor->FindComponentByClass<UAbilitySystemComponent>())
		{
			ApplyDamageEffect(TargetASC, DamageEffectClass, DamageAmount);
		}
	}
}

void UAO_DamageComponent::ApplyDamageEffect(UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount)
{
	if (!TargetASC || !DamageEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddInstigator(GetOwner(), GetOwner());

	FGameplayEffectSpecHandle Handle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);

	if (Handle.IsValid())
	{
		FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
		Handle.Data.Get()->SetSetByCallerMagnitude(DamageTag, DamageAmount);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Handle.Data.Get(), TargetASC);
	}
}
