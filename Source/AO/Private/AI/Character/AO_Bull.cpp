//KSJ : AO_Bull

#include "AI/Character/AO_Bull.h"
#include "AI/Controller/AO_BullController.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Animation/AO_Bull_AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "NavigationSystem.h"
#include "AIController.h"

AAO_Bull::AAO_Bull()
{
	// 캡슐 콜리전 크기 설정 (4족 보행 대형 몬스터)
	GetCapsuleComponent()->SetCapsuleHalfHeight(150.f);
	GetCapsuleComponent()->SetCapsuleRadius(120.f);

	// 충돌 박스 생성
	ChargeCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ChargeCollisionBox"));
	ChargeCollisionBox->SetupAttachment(GetRootComponent());
	ChargeCollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // 적절한 프로필로 변경 필요
	ChargeCollisionBox->SetGenerateOverlapEvents(true);
	// 초기에는 비활성화 (돌진 시에만 활성화)
	ChargeCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 기본 속도 설정
	RoamSpeed = 300.f;
	ChaseSpeed = 600.f; // 추격 시 속도
	ChargeSpeed = 900.f; // 돌진 시 속도 (더 빠름)

	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	// AI Controller 설정
	AIControllerClass = AAO_BullController::StaticClass();

	// 근접 공격 기본값 설정
	MeleeAttackConfig.Damage = 20.f;
	MeleeAttackConfig.KnockbackStrength = 300.f;
	MeleeAttackConfig.AttackRadius = 100.f;
	MeleeAttackConfig.AttackDistance = 150.f;
	MeleeAttackConfig.AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Melee"));
}

FEnemyAttackConfig AAO_Bull::GetCurrentAttackConfig_Implementation() const
{
	// 돌진 공격은 별도 GA(AO_GA_Bull_Charge)에서 처리하지만,
	// 만약 추후 통합한다면 여기서 분기 처리 가능.
	// 현재는 "일반 근접 공격" 요청 시 MeleeAttackConfig 반환.
	return MeleeAttackConfig;
}

void AAO_Bull::BeginPlay()
{
	Super::BeginPlay();

	if (ChargeCollisionBox)
	{
		ChargeCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAO_Bull::OnChargeOverlap);
	}
}

void AAO_Bull::SetIsCharging(bool bCharging)
{
	if (bIsCharging != bCharging)
	{
		bIsCharging = bCharging;
		
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			MoveComp->MaxWalkSpeed = bCharging ? ChargeSpeed : (IsInChaseMode() ? ChaseSpeed : RoamSpeed);
		}

		// 돌진 중에만 충돌 판정 활성화
		if (ChargeCollisionBox)
		{
			ChargeCollisionBox->SetCollisionEnabled(bCharging ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		}
	}
}

void AAO_Bull::OnChargeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsCharging || !OtherActor || OtherActor == this) return;

	// 플레이어만 타격
	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
	if (!Player) return;

	// 1. 데미지 적용
	if (DamageEffectClass)
	{
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);

		if (SourceASC && TargetASC)
		{
			// 무적 상태 확인
			const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
			if (!TargetASC->HasMatchingGameplayTag(InvulnerableTag))
			{
				// 데미지 적용
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				Context.AddInstigator(this, this);

				FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
				if (DamageSpec.IsValid())
				{
					const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
					DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, ChargeDamage);
					SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
				}
			}
		}
	}

	// 2. 넉다운 태그 이벤트 전송 (Player가 HitReact하도록)
	FGameplayTag KnockdownTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = Player;
	EventData.EventMagnitude = ChargeDamage;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Player, KnockdownTag, EventData);

	// 3. 물리 넉백 (Launch)
	FVector KnockbackDir = GetActorForwardVector();
	KnockbackDir.Z = 0.1f; // 살짝만 위로 (넉백이 바닥에 박히지 않도록)
	KnockbackDir.Normalize();
	Player->LaunchCharacter(KnockbackDir * KnockbackStrength, true, true);

	// 충돌했으므로 돌진 멈춤 (선택사항, 계속 뚫고 갈지 멈출지)
	// 여기서는 잠깐 멈추거나 상태를 리셋할 수 있음
	SetIsCharging(false); 
	
	// TODO: Bull 자체도 충돌 애니메이션이나 잠시 멈칫하는 로직 필요
}

void AAO_Bull::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 돌진 중단
	SetIsCharging(false);

	// 서버에서 Multicast로 기절 애니메이션 재생 (모든 클라이언트에서 보이도록)
	if (HasAuthority())
	{
		if (UAO_Bull_AnimInstance* BullAnimInstance = Cast<UAO_Bull_AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UAnimMontage* StunMontage = BullAnimInstance->GetStunMontage();
			if (StunMontage)
			{
				Multicast_PlayStunMontage(StunMontage, BullAnimInstance->GetStunMontagePlayRate());
			}
		}
	}
}

void AAO_Bull::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 서버에서 Multicast로 기절 애니메이션 중지
	if (HasAuthority())
	{
		Multicast_StopStunMontage(0.25f);
	}
}

void AAO_Bull::BeginDestroy()
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PostAttackTimerHandle);
		World->GetTimerManager().ClearTimer(RetreatCheckTimerHandle);
	}

	Super::BeginDestroy();
}

void AAO_Bull::StartPostAttackRetreat()
{
	// 이미 쿨다운 중이면 무시
	if (bInPostAttackCooldown)
	{
		return;
	}

	bInPostAttackCooldown = true;
	bIsRetreating = true;

	// 플레이어 반대 방향으로 후퇴 위치 계산
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		// Controller 없으면 바로 대기 상태로
		OnRetreatComplete();
		return;
	}

	AAO_PlayerCharacter* Target = nullptr;
	if (AAO_AggressiveAICtrl* AOController = Cast<AAO_AggressiveAICtrl>(AIController))
	{
		Target = AOController->GetChaseTarget();
	}

	if (Target)
	{
		// 플레이어 반대 방향 계산
		FVector RetreatDirection = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
		RetreatDirection.Z = 0.f;
		
		FVector DesiredLocation = GetActorLocation() + (RetreatDirection * RetreatDistance);

		// NavMesh 상의 유효한 위치로 보정
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(DesiredLocation, NavLocation, FVector(200.f, 200.f, 200.f)))
			{
				RetreatTargetLocation = NavLocation.Location;
			}
			else
			{
				// NavMesh에 투영 실패하면 원래 위치 사용
				RetreatTargetLocation = DesiredLocation;
			}
		}
		else
		{
			RetreatTargetLocation = DesiredLocation;
		}

		// 후퇴 이동 명령
		AIController->MoveToLocation(RetreatTargetLocation, 50.f);

		// 후퇴 완료 체크 타이머 (0.1초마다 체크)
		GetWorld()->GetTimerManager().SetTimer(
			RetreatCheckTimerHandle,
			this,
			&AAO_Bull::OnRetreatComplete,
			0.1f,
			true
		);
	}
	else
	{
		// 타겟 없으면 바로 대기 상태로
		OnRetreatComplete();
	}
}

void AAO_Bull::OnRetreatComplete()
{
	// 후퇴 완료 체크
	if (bIsRetreating)
	{
		float DistToTarget = FVector::Dist(GetActorLocation(), RetreatTargetLocation);
		
		// 목표 지점 근처에 도착했거나, 충분히 시간이 지났으면 후퇴 완료
		if (DistToTarget < 100.f)
		{
			// 후퇴 완료
			bIsRetreating = false;
			GetWorld()->GetTimerManager().ClearTimer(RetreatCheckTimerHandle);

			// 이동 중지
			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				AIController->StopMovement();
			}

			// 대기 타이머 시작
			GetWorld()->GetTimerManager().SetTimer(
				PostAttackTimerHandle,
				this,
				&AAO_Bull::EndPostAttackCooldown,
				PostAttackWaitTime,
				false
			);
		}
	}
}

void AAO_Bull::EndPostAttackCooldown()
{
	bInPostAttackCooldown = false;
	bIsRetreating = false;
}

