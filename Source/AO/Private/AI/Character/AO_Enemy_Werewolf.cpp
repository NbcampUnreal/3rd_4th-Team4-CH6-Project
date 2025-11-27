// KSJ : AO_Enemy_Werewolf.cpp

#include "AI/Character/AO_Enemy_Werewolf.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO_Log.h"

AAO_Enemy_Werewolf::AAO_Enemy_Werewolf()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	bUseControllerRotationYaw = false; 
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAO_Enemy_Werewolf::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Enemy_Werewolf::PerformAttack()
{
	if (!HasAuthority()) return;

	AO_LOG(LogKSJ, Log, TEXT("[Werewolf] PerformAttack Called!"));

	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}

	FVector Start = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Center = Start + (Forward * (AttackRange * 0.5f));

	TArray<AActor*> OverlappedActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		this,
		Center,
		AttackRange,
		TArray<TEnumAsByte<EObjectTypeQuery>>(), 
		APawn::StaticClass(),
		ActorsToIgnore,
		OverlappedActors
	);

	if (bHit)
	{
		AO_LOG(LogKSJ, Log, TEXT("[Werewolf] Hit Detected! Count: %d"), OverlappedActors.Num());
		for (AActor* Target : OverlappedActors)
		{
			if (APawn* TargetPawn = Cast<APawn>(Target))
			{
				if (TargetPawn->IsPlayerControlled())
				{
					AO_LOG(LogKSJ, Log, TEXT("[Werewolf] Target is Player: %s"), *Target->GetName());
					ApplyAttackEffect(Target);
				}
			}
		}
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("[Werewolf] Attack Missed! (No Overlap) - Range: %f"), AttackRange);
	}
}

void AAO_Enemy_Werewolf::ApplyAttackEffect(AActor* Target)
{
	if (!Target || !AbilitySystemComponent) return;
	
	if (!AttackEffectClass)
	{
		AO_LOG(LogKSJ, Error, TEXT("[Werewolf] AttackEffectClass is NULL! Check Blueprint!"));
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(AttackEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -AttackDamageAmount);
		
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target));
		
		AO_LOG(LogKSJ, Log, TEXT("[Werewolf] Applied GE to %s! Damage: %.1f"), *Target->GetName(), AttackDamageAmount);
	}
}
