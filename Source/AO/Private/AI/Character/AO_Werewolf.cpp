//KSJ : AO_Werewolf


#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "AI/Animation/AO_Werewolf_AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Werewolf::AAO_Werewolf()
{
	// 이동 속도 설정
	RoamSpeed = 400.f;
	ChaseSpeed = 700.f;

	// 컴포넌트 생성
	PackCoordComp = CreateDefaultSubobject<UAO_PackCoordComp>(TEXT("PackCoordComp"));

	// 공격 설정 기본값
	AttackConfig.Damage = 30.f;
	AttackConfig.KnockbackStrength = 500.f;
	AttackConfig.AttackRadius = 150.f;
	AttackConfig.AttackDistance = 200.f;
	AttackConfig.AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack"));
}

void AAO_Werewolf::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 초기 속도 적용
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = RoamSpeed;
	}
}

FEnemyAttackConfig AAO_Werewolf::GetCurrentAttackConfig_Implementation() const
{
	return AttackConfig;
}

void AAO_Werewolf::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 서버에서 Multicast로 기절 애니메이션 재생 (모든 클라이언트에서 보이도록)
	if (HasAuthority())
	{
		if (UAO_Werewolf_AnimInstance* WerewolfAnimInstance = Cast<UAO_Werewolf_AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UAnimMontage* StunMontage = WerewolfAnimInstance->GetStunMontage();
			if (StunMontage)
			{
				Multicast_PlayStunMontage(StunMontage, WerewolfAnimInstance->GetStunMontagePlayRate());
			}
		}
	}
}

void AAO_Werewolf::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 서버에서 Multicast로 기절 애니메이션 중지
	if (HasAuthority())
	{
		Multicast_StopStunMontage(0.25f);
	}
}
