//KSJ : AO_KidnapComponent

#include "AI/Component/AO_KidnapComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "AI/Controller/AO_InsectController.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "AI/Character/AO_Insect.h"
#include "Interaction/Component/AO_InspectionComponent.h"

UAO_KidnapComponent::UAO_KidnapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 납치 중에만 Tick 활성화 가능

	// 기본 태그 설정
	KidnappedStatusTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Kidnapped"));
	KnockdownTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
}

void UAO_KidnapComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 현재 납치 중인 플레이어가 있으면 사망 감지 바인딩
	if (CurrentVictim)
	{
		BindDeathDelegate();
	}
}

void UAO_KidnapComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 컴포넌트 파괴 시 납치 해제 (안전장치)
	if (CurrentVictim)
	{
		ReleaseKidnap(false);
	}
	
	GetWorld()->GetTimerManager().ClearTimer(DotTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

void UAO_KidnapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 납치 중 매 틱마다 위치 강제 조정 (Attachment가 불안정할 경우 대비)
	// 기본적으로는 AttachToComponent로 충분하므로 비워둠. 필요시 보정 로직 추가.
}

bool UAO_KidnapComponent::TryKidnapPlayer(AAO_PlayerCharacter* TargetPlayer)
{
	if (!TargetPlayer || CurrentVictim)
	{
		return false;
	}

	// 이미 납치된 상태인지 먼저 확인 (태그 체크) - 빠른 실패
	UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
	if (TargetASC && TargetASC->HasMatchingGameplayTag(KidnappedStatusTag))
	{
		return false;
	}

	// AISubsystem을 통한 납치 예약 및 쿨다운 체크
	// 이 체크는 예약을 먼저 하고, 실패하면 바로 리턴
	if (UWorld* World = GetWorld())
	{
		if (UAO_AISubsystem* AISubsystem = World->GetSubsystem<UAO_AISubsystem>())
		{
			// 이미 다른 AI가 납치 중이거나 쿨다운 중이면 실패
			bool bReserveSuccess = AISubsystem->TryReservePlayerForKidnap(TargetPlayer, GetOwner());
			
			if (!bReserveSuccess)
			{
				return false;
			}
			
			// 예약 성공 후, 다시 한 번 태그 체크 (예약과 납치 사이에 다른 Insect가 납치했을 수 있음)
			if (TargetASC && TargetASC->HasMatchingGameplayTag(KidnappedStatusTag))
			{
				// 예약 해제
				AISubsystem->ReleasePlayerFromKidnap(TargetPlayer);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	// 납치 성공 처리
	CurrentVictim = TargetPlayer;
	
	// 1. 물리/충돌 처리
	if (UCapsuleComponent* Capsule = CurrentVictim->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 끔 (Insect와 겹치기 위해)
	}
	if (UCharacterMovementComponent* MoveComp = CurrentVictim->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_Flying); // 끌려가는 동안 Flying 모드
	}

	// 2. 소켓 부착
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		CurrentVictim->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, KidnapSocketName);
	}

	// 3. 제약 설정 및 태그 적용
	SetPlayerRestrictions(CurrentVictim, true);

	// 4. 플레이어 사망 감지 바인딩
	BindDeathDelegate();

	// 5. DoT 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DotTimerHandle, this, &UAO_KidnapComponent::ApplyDotDamage, DotInterval, true);
	}

	OnKidnapStateChanged.Broadcast(true);

	return true;
}

void UAO_KidnapComponent::ReleaseKidnap(bool bThrow)
{
	if (!CurrentVictim)
	{
		return;
	}

	AAO_PlayerCharacter* ReleasedPlayer = CurrentVictim;
	
	// 플레이어가 유효한지 확인
	if (!IsValid(ReleasedPlayer))
	{
		CurrentVictim = nullptr;
		OnKidnapStateChanged.Broadcast(false);
		return;
	}
	
	// 사망 여부 확인
	bool bIsDead = false;
	if (UAbilitySystemComponent* ASC = ReleasedPlayer->GetAbilitySystemComponent())
	{
		const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
		bIsDead = ASC->HasMatchingGameplayTag(DeathTag);
		
		// 사망 델리게이트 해제
		ASC->RegisterGameplayTagEvent(DeathTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
	
	// 1. 부착 해제 (사망한 플레이어도 안전하게 해제)
	if (IsValid(ReleasedPlayer))
	{
		ReleasedPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// 누워있던 캐릭터를 똑바로 세움 (Pitch, Roll 제거)
		FRotator CurrentRot = ReleasedPlayer->GetActorRotation();
		ReleasedPlayer->SetActorRotation(FRotator(0.f, CurrentRot.Yaw, 0.f));
	}

	// 2. 모든 Ability 취소 (Traversal 등이 실행 중일 수 있음) - 사망한 플레이어는 스킵
	if (!bIsDead)
	{
		if (UAbilitySystemComponent* ASC = ReleasedPlayer->GetAbilitySystemComponent())
		{
			FGameplayTagContainer AllAbilityTags(FGameplayTag::RequestGameplayTag(FName("Ability")));
			ASC->CancelAbilities(&AllAbilityTags);
		}
	}

	// 3. 물리/충돌 복구
	if (IsValid(ReleasedPlayer))
	{
		UCharacterMovementComponent* MoveComp = ReleasedPlayer->GetCharacterMovement();
		
		if (!bIsDead)
		{
			// 생존한 플레이어: 완전한 복구
			if (UCapsuleComponent* Capsule = ReleasedPlayer->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				Capsule->SetCollisionProfileName(TEXT("Player"));
			}
			
			if (MoveComp)
			{
				MoveComp->Velocity = FVector::ZeroVector;
				MoveComp->StopMovementImmediately();
				// 던지기 모드가 아닐 때만 Walking으로 복구 (던지기 시에는 Falling으로 유지)
				if (!bThrow)
				{
					MoveComp->SetMovementMode(MOVE_Walking);
				}
				MoveComp->GravityScale = 1.0f;
			}

			// 제약 해제 및 태그 제거
			SetPlayerRestrictions(ReleasedPlayer, false);
		}
		else
		{
			// 사망한 플레이어: 바닥에 떨어지도록 물리 복구
			if (UCapsuleComponent* Capsule = ReleasedPlayer->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				Capsule->SetCollisionProfileName(TEXT("Ragdoll"));
			}
			
			if (MoveComp)
			{
				MoveComp->Velocity = FVector::ZeroVector;
				MoveComp->StopMovementImmediately();
				MoveComp->SetMovementMode(MOVE_Walking);
				MoveComp->GravityScale = 1.0f;
			}
			
			// 사망한 플레이어도 Kidnapped 태그 제거 (다시 납치 가능하도록)
			RemoveKidnappedTag(ReleasedPlayer);
		}

		// 4. 플레이어 위치를 안전하게 설정 (NavMesh 위로) - 생존/사망 모두 적용
		if (UWorld* World = GetWorld())
		{
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
			if (NavSys && IsValid(ReleasedPlayer))
			{
				FVector CurrentLocation = ReleasedPlayer->GetActorLocation();
				FNavLocation NavLocation;
				
				// 현재 위치에서 아래로 NavMesh 찾기 (더 넓은 범위로 검색)
				if (NavSys->ProjectPointToNavigation(CurrentLocation, NavLocation, FVector(0.f, 0.f, 1000.f)))
				{
					// NavMesh 위로 위치 조정
					FVector SafeLocation = NavLocation.Location;
					if (UCapsuleComponent* Capsule = ReleasedPlayer->GetCapsuleComponent())
					{
						SafeLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
					}
					ReleasedPlayer->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
				}
				else
				{
					// NavMesh를 찾지 못한 경우, 현재 위치에서 아래로 Line Trace
					FVector TraceStart = CurrentLocation;
					FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 1000.f);
					FHitResult HitResult;
					
					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(ReleasedPlayer);
					
					if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
					{
						FVector GroundLocation = HitResult.ImpactPoint;
						if (UCapsuleComponent* Capsule = ReleasedPlayer->GetCapsuleComponent())
						{
							GroundLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
						}
						ReleasedPlayer->SetActorLocation(GroundLocation, false, nullptr, ETeleportType::TeleportPhysics);
					}
				}
			}
		}
	}

	// 4. 던지기 (Knockdown)
	if (bThrow)
	{
		// 넉다운 이벤트 전송
		FGameplayEventData EventData;
		EventData.EventTag = KnockdownTag;
		EventData.Instigator = GetOwner();
		EventData.Target = ReleasedPlayer;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ReleasedPlayer, KnockdownTag, EventData);
		
		// 살짝 던지는 물리력 (옵션)

		// 1. 확실하게 Falling 상태로 전환 (물리 적용을 위해)
		if (UCharacterMovementComponent* MC = ReleasedPlayer->GetCharacterMovement())
		{
			MC->SetMovementMode(MOVE_Falling);
		}

		// 2. 던지는 힘 상향 (300 -> 600) 및 방향 보정
		FVector ThrowDir = GetOwner()->GetActorForwardVector() + FVector::UpVector;
		ThrowDir.Normalize();
		ReleasedPlayer->LaunchCharacter(ThrowDir * 600.f, true, true);
	}

	// 5. DoT 중지
	GetWorld()->GetTimerManager().ClearTimer(DotTimerHandle);

	// 6. AISubsystem에서 납치 해제 및 쿨다운 등록
	if (UWorld* World = GetWorld())
	{
		if (UAO_AISubsystem* AISubsystem = World->GetSubsystem<UAO_AISubsystem>())
		{
			AISubsystem->ReleasePlayerFromKidnap(ReleasedPlayer);
			// 쿨다운 등록 (같은 플레이어를 바로 다시 납치하지 못하도록)
			AISubsystem->MarkPlayerAsRecentlyKidnapped(ReleasedPlayer, KidnapCooldownDuration);
		}
	}

	// 7. 추격 대상 초기화 및 추격 완전히 중단 (떨어뜨린 플레이어를 더 이상 추격하지 않도록)
	if (AAO_Insect* Insect = Cast<AAO_Insect>(GetOwner()))
	{
		if (AAO_InsectController* InsectController = Cast<AAO_InsectController>(Insect->GetController()))
		{
			if (InsectController->GetChaseTarget() == ReleasedPlayer)
			{
				// 이동 즉시 중지
				InsectController->StopMovement();
				
				// 추격 대상 초기화
				InsectController->SetChaseTarget(nullptr);
				
				// 추격 모드 종료
				Insect->SetChaseMode(false);
				
				// 수색 모드도 종료 (배회로 전환)
				InsectController->EndSearch();
			}
		}
	}

	CurrentVictim = nullptr;
	OnKidnapStateChanged.Broadcast(false);
}

void UAO_KidnapComponent::ApplyDotDamage()
{
	// CurrentVictim을 로컬 변수에 저장 (체크 후 변경 방지)
	AAO_PlayerCharacter* Victim = CurrentVictim;
	
	if (!Victim || !IsValid(Victim))
	{
		// CurrentVictim이 없거나 유효하지 않으면 타이머 정리
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DotTimerHandle);
		}
		return;
	}

	UAbilitySystemComponent* TargetASC = Victim->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		// ASC가 없으면 타이머 정리
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DotTimerHandle);
		}
		return;
	}

	// 사망 상태 확인 (사망한 플레이어에게는 데미지 주지 않음)
	const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
	if (TargetASC->HasMatchingGameplayTag(DeathTag))
	{
		// 사망한 플레이어는 DoT만 중지, 납치는 유지 (시체 끌고 다님)
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DotTimerHandle);
		}
		return;
	}

	// 무적 상태 확인
	const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		return;
	}

	// GameplayEffect를 통한 데미지 적용
	if (DotDamageEffectClass)
	{
		UAbilitySystemComponent* SourceASC = nullptr;
		if (AActor* Owner = GetOwner())
		{
			SourceASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
		}

		if (!SourceASC)
		{
			// Source ASC가 없으면 Target ASC를 사용 (Self Damage)
			SourceASC = TargetASC;
		}

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(GetOwner(), GetOwner());

		FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DotDamageEffectClass, 1.f, Context);
		if (DamageSpec.IsValid())
		{
			const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
			DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, DotDamageAmount);

			// Victim이 유효한지 확인 (ApplyGameplayEffectSpecToTarget 호출 전)
			if (!IsValid(Victim))
			{
				return;
			}

			SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		}
	}
}

void UAO_KidnapComponent::SetPlayerRestrictions(AAO_PlayerCharacter* Player, bool bRestrict)
{
	if (!Player || !IsValid(Player)) return;
	
	// 사망한 플레이어는 제약 설정/해제 스킵
	if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
	{
		const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
		if (ASC->HasMatchingGameplayTag(DeathTag))
		{
			return;
		}
	}

	// 납치 시 Inspection 모드 강제 종료 (퍼즐 상호작용 중단)
	if (bRestrict)
	{
		if (TObjectPtr<UAO_InspectionComponent> InspectionComp = Player->FindComponentByClass<UAO_InspectionComponent>())
		{
			if (InspectionComp->IsInspecting())
			{
				InspectionComp->ExitInspectionMode();
			}
		}
	}

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (PC)
	{
		if (bRestrict)
		{
			PC->SetIgnoreMoveInput(true);
			// LookInput은 false로 유지 (카메라는 움직여야 함)
			PC->SetIgnoreLookInput(false);
		}
		else
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
	}

	// 카메라 충돌 비활성화 (납치 중 시야 어지러움 방지)
	if (USpringArmComponent* SpringArm = Player->GetSpringArm())
	{
		if (bRestrict)
		{
			// 납치 전 상태 저장
			bOriginalCameraCollision = SpringArm->bDoCollisionTest;
			SpringArm->bDoCollisionTest = false;
		}
		else
		{
			// 원래 상태 복구
			SpringArm->bDoCollisionTest = bOriginalCameraCollision;
		}
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	if (ASC)
	{
		if (bRestrict)
		{
			// 납치 태그 추가 (Loose Tag) -> 이걸로 점프/스킬 차단 (Player Character Ability에서 Tag Block 필요)
			ASC->AddLooseGameplayTag(KidnappedStatusTag);
			
			// 이동/점프 등 Ability Cancel
			FGameplayTagContainer AbilityTags(FGameplayTag::RequestGameplayTag(FName("Ability")));
			ASC->CancelAbilities(&AbilityTags);
		}
		else
		{
			ASC->RemoveLooseGameplayTag(KidnappedStatusTag);
		}
	}
}

void UAO_KidnapComponent::BindDeathDelegate()
{
	if (!CurrentVictim)
	{
		return;
	}

	UAbilitySystemComponent* ASC = CurrentVictim->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 사망 태그 변경 감지
	const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
	ASC->RegisterGameplayTagEvent(DeathTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAO_KidnapComponent::OnPlayerDeathTagChanged);
}

void UAO_KidnapComponent::OnPlayerDeathTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// NewCount > 0이면 태그가 추가된 것 (사망)
	if (NewCount > 0 && CurrentVictim && IsValid(CurrentVictim))
	{
		// 즉시 DoT 타이머 정리 (사망 후 데미지 주지 않도록)
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DotTimerHandle);
		}
		
		// 사망해도 납치는 유지 - 시체도 끌고 다님
	}
}

void UAO_KidnapComponent::RemoveKidnappedTag(AAO_PlayerCharacter* Player)
{
	if (!Player || !IsValid(Player))
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(KidnappedStatusTag);
	}
}
