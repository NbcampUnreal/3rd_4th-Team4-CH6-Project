// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/AO_PackCoordComp.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_WerewolfController.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"
#include "Engine/OverlapResult.h"

UAO_PackCoordComp::UAO_PackCoordComp()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick 불필요
}


void UAO_PackCoordComp::BeginPlay()
{
	Super::BeginPlay();

}


void UAO_PackCoordComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAO_PackCoordComp::BroadcastHowl(AActor* Target)
{
	if (!GetOwner()) return;

	AO_LOG(LogKSJ, Log, TEXT("[PackCoord] %s is Howling! Target: %s"), *GetOwner()->GetName(), Target ? *Target->GetName() : TEXT("None"));

	// 주변의 모든 Werewolf 검색
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); // 자신 제외

	// Pawn 채널로 검색 (설정에 따라 변경 가능)
	bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(HowlRadius),
		Params
	);

	bool bFoundAlly = false;

	if (bHit)
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* OverlapActor = Result.GetActor();
			if (OverlapActor && OverlapActor != GetOwner())
			{
				// Werewolf인지 확인
				if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(OverlapActor))
				{
					if (UAO_PackCoordComp* AllyComp = Wolf->FindComponentByClass<UAO_PackCoordComp>())
					{
						AllyComp->ReceiveHowl(GetOwner(), Target);
						bFoundAlly = true;
					}
				}
			}
		}
	}

	// 동료가 있으면 포위 모드 시작
	if (bFoundAlly)
	{
		SetSurroundMode(true);
	}
	else
	{
		// 동료가 없으면 바로 일반 추격/공격 로직으로 전환해야 함
		// (이 처리는 Controller나 StateTree에서 bIsSurrounding 체크를 통해 수행)
		AO_LOG(LogKSJ, Log, TEXT("[PackCoord] No allies nearby. Switching to solo mode."));
		SetSurroundMode(false);
	}
}

void UAO_PackCoordComp::ReceiveHowl(AActor* Sender, AActor* Target)
{
	AO_LOG(LogKSJ, Log, TEXT("[PackCoord] %s received Howl from %s"), *GetOwner()->GetName(), Sender ? *Sender->GetName() : TEXT("Unknown"));

	// 이미 포위 중이거나 공격 중이면 무시할 수도 있음 (기획에 따라 다름)
	// 여기서는 Howl을 들으면 무조건 포위 모드로 동기화한다고 가정

	if (!bIsSurrounding)
	{
		SetSurroundMode(true);
		
		// 델리게이트 방송 (StateTree Condition 등에서 반응)
		if (OnHowlReceived.IsBound())
		{
			OnHowlReceived.Broadcast(Target);
		}
	}
}

TArray<AAO_Werewolf*> UAO_PackCoordComp::GetNearbyPackMembers() const
{
	TArray<AAO_Werewolf*> Members;
	if (!GetOwner()) return Members;

	// 전체 검색보다는 Overlap이나 관리되는 리스트가 좋지만, 
	// 여기서는 편의상 OverlapMulti를 다시 사용하거나, 
	// BroadcastHowl 시 캐싱된 정보를 쓸 수도 있음.
	// EQS Context에서 호출될 함수이므로, 실시간으로 주변을 찾는게 안전함.

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(HowlRadius), // Howl 반경 내 멤버를 Pack으로 간주
		Params
	);

	for (const FOverlapResult& Result : Overlaps)
	{
		if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(Result.GetActor()))
		{
			Members.Add(Wolf);
		}
	}

	return Members;
}

void UAO_PackCoordComp::SetSurroundMode(bool bSurround)
{
	bIsSurrounding = bSurround;

	if (bIsSurrounding)
	{
		// 포위 시작 시 일제 공격 타이머 가동
		float Delay = FMath::RandRange(MinAttackDelay, MaxAttackDelay);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &UAO_PackCoordComp::ExecuteAttack, Delay, false);
		
		AO_LOG(LogKSJ, Log, TEXT("[PackCoord] Surround Mode ON. Attack in %.2f sec"), Delay);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	}
}

void UAO_PackCoordComp::StartCoordinatedAttack()
{
	// 외부(StateTree 등)에서 강제 호출 시
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	}
	ExecuteAttack();
}

void UAO_PackCoordComp::ExecuteAttack()
{
	AO_LOG(LogKSJ, Warning, TEXT("[PackCoord] Coordinated Attack Started!"));

	// 여기서 Controller에게 공격 모드로 전환하라고 알려야 함.
	// 1. bIsSurrounding = false로 끄고 일반 추격 로직으로 넘기거나
	// 2. 별도의 bIsAttacking 플래그를 켜거나
	// StateTree에서 이 상태를 감지해야 함.

	bIsSurrounding = false; // 포위 끝, 공격 시작

	// 만약 주변 동료들에게도 "공격해!"라고 알려야 한다면 여기서 다시 Broadcast 가능
	// 하지만 각자 타이머가 돌고 있을 것이므로, 약간의 시간차 공격이 자연스러울 수 있음.
}
