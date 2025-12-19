// AO_Bull.cpp

#include "AI/Character/AO_Bull.h"
#include "AI/Controller/AO_BullController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "AO_Log.h"

AAO_Bull::AAO_Bull()
{
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

		AO_LOG(LogKSJ, Log, TEXT("Bull Charging State: %s"), bCharging ? TEXT("ON") : TEXT("OFF"));
	}
}

void AAO_Bull::OnChargeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsCharging || !OtherActor || OtherActor == this) return;

	// 플레이어만 타격
	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
	if (!Player) return;

	AO_LOG(LogKSJ, Log, TEXT("Bull Hit Player: %s"), *Player->GetName());

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
					FActiveGameplayEffectHandle ActiveGE = SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
					
					AO_LOG(LogKSJ, Log, TEXT("OnChargeOverlap: Applied GE_Damage. ActiveGE Valid: %s"), ActiveGE.WasSuccessfullyApplied() ? TEXT("True") : TEXT("False"));
				}
				else
				{
					AO_LOG(LogKSJ, Warning, TEXT("OnChargeOverlap: DamageSpec is Invalid!"));
				}
			}
			else
			{
				AO_LOG(LogKSJ, Log, TEXT("OnChargeOverlap: Target is Invulnerable"));
			}
		}
		else
		{
			AO_LOG(LogKSJ, Warning, TEXT("OnChargeOverlap: Missing ASC (Source: %s, Target: %s)"), 
				SourceASC ? *SourceASC->GetName() : TEXT("Null"), 
				TargetASC ? *TargetASC->GetName() : TEXT("Null"));
		}
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("OnChargeOverlap: DamageEffectClass is not set!"));
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
}

void AAO_Bull::HandleStunEnd()
{
	Super::HandleStunEnd();
}

