//KSJ : AO_Werewolf


#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
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
