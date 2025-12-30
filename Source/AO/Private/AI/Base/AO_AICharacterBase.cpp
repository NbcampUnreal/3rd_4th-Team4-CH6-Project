// AO_AICharacterBase.cpp

#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "AI/GAS/AO_AIAttributeSet.h"
#include "AI/GAS/Ability/AO_GA_AI_Stun.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

AAO_AICharacterBase::AAO_AICharacterBase()
{
	PrimaryActorTick.bCanEverTick = true; // 디버그용 Tick 활성화
	bReplicates = true;
	
	// 네트워크 업데이트 빈도 설정 - 생성자에서 조건문 없이 설정 (AI는 자주 움직이므로 높은 빈도 필요)
	NetUpdateFrequency = 30.0f; // 기본값은 10.0f, AI는 더 자주 업데이트 필요
	MinNetUpdateFrequency = 15.0f; // 최소 업데이트 빈도
	NetPriority = 2.0f; // 기본값은 1.0f, AI는 중요하므로 높은 우선순위

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Attribute Set 생성
	AttributeSet = CreateDefaultSubobject<UAO_AIAttributeSet>(TEXT("AttributeSet"));

	// Memory 컴포넌트 생성
	MemoryComponent = CreateDefaultSubobject<UAO_AIMemoryComponent>(TEXT("MemoryComponent"));
}

void AAO_AICharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// CharacterMovementComponent 네트워크 설정 - BeginPlay에서 확실하게 설정 (생성자에서는 nullptr일 수 있음)
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetIsReplicated(true);
		
		// 물리 상호작용 비활성화 (다른 캐릭터를 밀거나 밀리지 않도록)
		MovementComp->bEnablePhysicsInteraction = false;
		
		// 서버와 클라이언트 모두 Exponential 스무딩 사용
		// 클라이언트가 서버 위치로 부드럽게 수렴하도록 함
		MovementComp->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
		
		// 네트워크 스무딩 거리 설정
		// 이 거리 이내에서는 부드럽게 보간, 이 거리를 넘으면 즉시 텔레포트
		MovementComp->NetworkMaxSmoothUpdateDistance = 92.0f;  // 기본값 92 유지
		MovementComp->NetworkNoSmoothUpdateDistance = 140.0f;  // 기본값 140 유지
		
		if (HasAuthority())
		{
			// 서버: Controller 없이도 물리 실행 (AI Controller가 Possess하기 전에도 동작)
			MovementComp->bRunPhysicsWithNoController = true;
		}
		else
		{
			// 클라이언트: Controller 없으면 물리 실행 안함 (서버 위치를 따름)
			MovementComp->bRunPhysicsWithNoController = false;
			
			// 클라이언트에서는 바닥 체크를 항상 수행하지 않음 (서버 위치를 신뢰)
			MovementComp->bAlwaysCheckFloor = false;
			
			// NavWalking 시 Sweep을 사용하지 않음 (충돌 계산 감소)
			MovementComp->bSweepWhileNavWalking = false;
			
			// 클라이언트에서 충돌로 인한 위치 조정을 최소화
			MovementComp->MaxDepenetrationWithGeometry = 0.0f;
			MovementComp->MaxDepenetrationWithPawn = 0.0f;
		}
	}

	// 클라이언트에서 AI끼리 충돌하지 않도록 (서버에서만 충돌 처리)
	if (!HasAuthority())
	{
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
	}

	if (HasAuthority())
	{
		InitializeAbilitySystem();
		
		// 기본 이동 속도 설정은 서버에서만 실행
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
		}
	}
}

void AAO_AICharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 디버그 타이머 - 1초에 한번만 로그 출력
	static TMap<AAO_AICharacterBase*, float> DebugTimers;
	float& Timer = DebugTimers.FindOrAdd(this);
	Timer += DeltaTime;
	if (Timer < 1.0f)
	{
		return;
	}
	Timer = 0.f;
	
	// 서버/클라이언트 구분 문자열
	FString RoleString = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
	
	// Actor 위치 (실제 루트 위치)
	FVector ActorLoc = GetActorLocation();
	
	// Mesh 위치 (시각적으로 보이는 위치)
	FVector MeshLoc = GetMesh() ? GetMesh()->GetComponentLocation() : FVector::ZeroVector;
	
	// Capsule 위치 (충돌체 위치)
	FVector CapsuleLoc = GetCapsuleComponent() ? GetCapsuleComponent()->GetComponentLocation() : FVector::ZeroVector;
	
	// ReplicatedMovement 위치 (네트워크로 받은 위치)
	FVector RepMoveLoc = GetReplicatedMovement().Location;
	
	// Network Role 정보 (핵심!)
	ENetRole LocalRole = GetLocalRole();
	ENetRole ActorRemoteRole = GetRemoteRole();
	
	// bReplicateMovement 확인 (이게 false면 위치 동기화 안됨)
	bool bRepMove = IsReplicatingMovement();
	
	// Root Component 정보
	USceneComponent* RootComp = GetRootComponent();
	FString RootInfo = RootComp ? 
		FString::Printf(TEXT("Root:%s Mobility:%d"), *RootComp->GetName(), static_cast<int32>(RootComp->Mobility.GetValue())) : 
		TEXT("Root:NULL");
	
	FString RoleInfo = FString::Printf(TEXT("Local:%d Remote:%d RepMove:%s %s"), 
		static_cast<int32>(LocalRole), 
		static_cast<int32>(ActorRemoteRole),
		bRepMove ? TEXT("Y") : TEXT("N"),
		*RootInfo);
	
	// MovementComponent 상태 정보
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	FString MoveCompInfo = TEXT("NULL");
	FVector Velocity = FVector::ZeroVector;
	FString MoveModeStr = TEXT("None");
	if (MoveComp)
	{
		Velocity = MoveComp->Velocity;
		
		// Movement Mode 문자열 변환
		switch (MoveComp->MovementMode)
		{
		case MOVE_None: MoveModeStr = TEXT("None"); break;
		case MOVE_Walking: MoveModeStr = TEXT("Walk"); break;
		case MOVE_NavWalking: MoveModeStr = TEXT("NavWalk"); break;
		case MOVE_Falling: MoveModeStr = TEXT("Fall"); break;
		case MOVE_Swimming: MoveModeStr = TEXT("Swim"); break;
		case MOVE_Flying: MoveModeStr = TEXT("Fly"); break;
		case MOVE_Custom: MoveModeStr = TEXT("Custom"); break;
		default: MoveModeStr = TEXT("Unknown"); break;
		}
		
		MoveCompInfo = FString::Printf(TEXT("Tick:%s Smooth:%d RunPhysNoCtrl:%s Mode:%s"),
			MoveComp->IsComponentTickEnabled() ? TEXT("ON") : TEXT("OFF"),
			static_cast<int32>(MoveComp->NetworkSmoothingMode),
			MoveComp->bRunPhysicsWithNoController ? TEXT("Y") : TEXT("N"),
			*MoveModeStr);
	}
	
	// 로그 출력 1 - 위치 정보 (KSJ 식별자 사용)
	UE_LOG(LogTemp, Warning, TEXT("[KSJ][%s][%s] Actor:(%.1f,%.1f,%.1f) RepMove:(%.1f,%.1f,%.1f) %s"),
		*RoleString,
		*GetName(),
		ActorLoc.X, ActorLoc.Y, ActorLoc.Z,
		RepMoveLoc.X, RepMoveLoc.Y, RepMoveLoc.Z,
		*RoleInfo);
	
	// 로그 출력 2 - Velocity 및 MovementComponent 정보
	UE_LOG(LogTemp, Warning, TEXT("[KSJ][%s][%s] Vel:(%.1f,%.1f,%.1f) %s"),
		*RoleString,
		*GetName(),
		Velocity.X, Velocity.Y, Velocity.Z,
		*MoveCompInfo);
	
	// 화면에 디버그 구체 표시
	if (GetWorld())
	{
		// Actor 위치 - 서버: 빨간색, 클라이언트: 파란색
		FColor ActorColor = HasAuthority() ? FColor::Red : FColor::Blue;
		DrawDebugSphere(GetWorld(), ActorLoc, 50.f, 8, ActorColor, false, 1.1f);
		
		// Mesh 위치 - 녹색
		DrawDebugSphere(GetWorld(), MeshLoc, 35.f, 8, FColor::Green, false, 1.1f);
		
		// RepMove 위치 - 노란색 (클라이언트에서만 의미있음)
		if (!HasAuthority() && !RepMoveLoc.IsZero())
		{
			DrawDebugSphere(GetWorld(), RepMoveLoc, 25.f, 8, FColor::Yellow, false, 1.1f);
		}
	}
}

void AAO_AICharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAbilitySystemComponent* AAO_AICharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AAO_AICharacterBase::IsStunned() const
{
	// AbilitySystemComponent가 없으면 기절 상태가 아닌 것으로 처리
	if (!ensureMsgf(AbilitySystemComponent, TEXT("AbilitySystemComponent is null on %s"), *GetName()))
	{
		return false;
	}

	const FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned"));
	return AbilitySystemComponent->HasMatchingGameplayTag(StunnedTag);
}

void AAO_AICharacterBase::OnStunBegin()
{
	HandleStunBegin();
}

void AAO_AICharacterBase::OnStunEnd()
{
	HandleStunEnd();
}

void AAO_AICharacterBase::HandleStunBegin()
{
	// 서버에서만 이동 제어 (클라이언트에서 호출되면 동기화 문제 발생)
	if (!HasAuthority())
	{
		return;
	}

	// 기본 기절 처리: 이동 멈춤
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void AAO_AICharacterBase::HandleStunEnd()
{
	// 기절 해제 처리
}

void AAO_AICharacterBase::TestStun()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Event.AI.Stunned 이벤트 발생
	FGameplayTag StunEventTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = this;
	
	AbilitySystemComponent->HandleGameplayEvent(StunEventTag, &EventData);
}

void AAO_AICharacterBase::TestStunEnd()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 기절 Ability 태그로 취소 (프로젝트 내 다른 코드와 동일한 패턴)
	FGameplayTagContainer StunAbilityTags;
	StunAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Stunned")));
	
	AbilitySystemComponent->CancelAbilities(&StunAbilityTags);
}

FEnemyAttackConfig AAO_AICharacterBase::GetCurrentAttackConfig_Implementation() const
{
	// 기본 구현은 빈 설정 반환
	return FEnemyAttackConfig();
}

void AAO_AICharacterBase::SetIsAttacking(bool bAttacking)
{
	bIsAttacking = bAttacking;
}

void AAO_AICharacterBase::InitializeAbilitySystem()
{
	// GAS 초기화 - AbilitySystemComponent가 반드시 존재해야 함
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	BindDefaultAbilities();
	BindDefaultEffects();
}

void AAO_AICharacterBase::BindDefaultAbilities()
{
	// 기본 Ability 바인딩 - AbilitySystemComponent 필수
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void AAO_AICharacterBase::BindDefaultEffects()
{
	// 기본 Effect 바인딩 - AbilitySystemComponent 필수
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			Context.AddInstigator(this, this);

			FGameplayEffectSpecHandle Handle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Handle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
			}
		}
	}
}
