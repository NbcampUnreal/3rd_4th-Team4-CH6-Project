// Fill out your copyright notice in the Description page of Project Settings.


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
