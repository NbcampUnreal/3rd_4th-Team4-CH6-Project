// KSJ : AO_Enemy_Troll.cpp

#include "AI/Character/AO_Enemy_Troll.h"
#include "AI/GAS/AO_Enemy_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h" // GAS Library
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h" // SphereOverlap
#include "AI/Manager/AO_Manager_InsectKidnapManager.h"
#include "AO_Log.h"

AAO_Enemy_Troll::AAO_Enemy_Troll()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
}

void AAO_Enemy_Troll::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAO_Enemy_Troll::BeginPlay()
{
	Super::BeginPlay();

	SetMovementMode(false);
}

void AAO_Enemy_Troll::PerformAttack()
{
	if (!HasAuthority()) return;

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
		for (AActor* Target : OverlappedActors)
		{
			if (APawn* TargetPawn = Cast<APawn>(Target))
			{
				if (TargetPawn->IsPlayerControlled())
				{
					ApplyAttackEffect(Target);
				}
			}
		}
	}
	else
	{
		AO_LOG(LogKSJ, Log, TEXT("Troll Attack Missed!"));
	}
}

void AAO_Enemy_Troll::ApplyAttackEffect(AActor* Target)
{
	if (!Target || !AbilitySystemComponent) return;

	UWorld* World = GetWorld();
	bool bIsKidnapped = false;
	if (World)
	{
		if (UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>())
		{
			bIsKidnapped = Manager->IsPlayerKidnapped(Target);
		}
	}

	if (KnockbackEffectClass)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FVector KnockbackDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		Context.AddHitResult(FHitResult(Target, nullptr, FVector::ZeroVector, KnockbackDir));

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(KnockbackEffectClass, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -AttackDamageAmount);
			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target));
		}
	}
	
	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		if (!bIsKidnapped)
		{
			FVector LaunchVel = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal() * KnockbackStrength;
			LaunchVel.Z = 500.0f;
			TargetChar->LaunchCharacter(LaunchVel, true, true);
			
			AO_LOG(LogKSJ, Log, TEXT("Troll Hit %s! (Damage: %.1f, Knockback)"), *Target->GetName(), AttackDamageAmount);
		}
		else
		{
			AO_LOG(LogKSJ, Log, TEXT("Troll Hit %s! (Damage: %.1f, No Knockback - Kidnapped)"), *Target->GetName(), AttackDamageAmount);
		}
	}
}

void AAO_Enemy_Troll::SetMovementMode(bool bIsChasing)
{
	float TargetSpeed = bIsChasing ? ChaseSpeed : PatrolSpeed;

	if (HasAuthority() && AttributeSet)
	{
		AttributeSet->SetMovementSpeed(TargetSpeed);
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	}
}