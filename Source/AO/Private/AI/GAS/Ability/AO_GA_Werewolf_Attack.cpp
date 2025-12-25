// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GAS/Ability/AO_GA_Werewolf_Attack.h"
#include "AI/Character/AO_Werewolf.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"

UAO_GA_Werewolf_Attack::UAO_GA_Werewolf_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	// 기본 HitReact 태그를 Heavy로 설정
	HitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Heavy"));
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
			EventData.EventMagnitude = DamageAmount;
			
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HeavyHitReactTag, EventData);
		}
	}
}
