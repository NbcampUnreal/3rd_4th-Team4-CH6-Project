//KSJ : AO_GA_Werewolf_Attack


#include "AI/GAS/Ability/AO_GA_Werewolf_Attack.h"
#include "AI/Character/AO_Werewolf.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UAO_GA_Werewolf_Attack::UAO_GA_Werewolf_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	// 기본 HitReact 태그를 Heavy로 설정
	DefaultHitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Heavy"));
}

void UAO_GA_Werewolf_Attack::OnTargetHit(AActor* TargetActor, AActor* InstigatorActor)
{
	// 부모 클래스의 기본 처리 (데미지 + 넉백)
	Super::OnTargetHit(TargetActor, InstigatorActor);
	
	// Werewolf는 Heavy Hit React를 사용하므로
	// 부모 클래스에서 이미 HitReactTag로 이벤트를 보내지만,
	// 명시적으로 Heavy 태그로 재전송 (부모가 다른 태그를 사용할 수 있으므로)
	if (TargetActor)
	{
		FGameplayTag HeavyHitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Heavy"));
		if (HeavyHitReactTag.IsValid())
		{
			FGameplayEventData EventData;
			EventData.EventTag = HeavyHitReactTag;
			EventData.Instigator = InstigatorActor;
			EventData.Target = TargetActor;
			EventData.EventMagnitude = CurrentAttackConfig.Damage;
			
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HeavyHitReactTag, EventData);
		}
	}
}

void UAO_GA_Werewolf_Attack::ApplyDamageAndKnockback(AActor* TargetActor, AActor* InstigatorActor, const FEnemyAttackConfig& Config)
{
	if (!TargetActor || !InstigatorActor || !DamageEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	// 무적 확인
	const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		return;
	}

	// 타격음 재생 (무적 체크 통과 후, 실제로 맞았을 때만)
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			TargetActor->GetActorLocation(),
			1.f,
			1.f,
			0.f,
			HitSoundAttenuation
		);
	}

	// 데미지 적용
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(InstigatorActor, InstigatorActor);

	FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (DamageSpec.IsValid())
	{
		const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
		DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, Config.Damage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
	}

	// Hit React 이벤트 전송 (먼저 실행하여 피격 상태로 만듦)
	SendHitReactEvent(TargetActor, InstigatorActor, Config.Damage);

	// 넉백 적용
	ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
	if (TargetChar && Config.KnockbackStrength > 0.f)
	{
		FVector KnockbackDir = (TargetActor->GetActorLocation() - InstigatorActor->GetActorLocation()).GetSafeNormal();
		KnockbackDir.Z = 0.2f; // 약간 위로
		KnockbackDir.Normalize();

		if (UCharacterMovementComponent* CMC = TargetChar->GetCharacterMovement())
		{
			CMC->SetMovementMode(MOVE_Falling);
		}
		TargetChar->LaunchCharacter(KnockbackDir * Config.KnockbackStrength, true, true);
	}

	// OnTargetHit 콜백 호출
	OnTargetHit(TargetActor, InstigatorActor);
}
