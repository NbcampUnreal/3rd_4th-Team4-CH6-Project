//KSJ : AO_WerewolfController


#include "AI/Controller/AO_WerewolfController.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NavigationSystem.h"

AAO_WerewolfController::AAO_WerewolfController()
{
	// Werewolf는 좀 더 예민할 수 있음
	// AISenseConfig 설정은 BP에서 하거나 여기서 추가 설정 가능
}

void AAO_WerewolfController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(InPawn))
	{
		PackComp = Wolf->GetPackCoordComp();
		if (PackComp)
		{
			// Howl 수신 이벤트 바인딩
			PackComp->OnHowlReceived.AddDynamic(this, &AAO_WerewolfController::HandleHowlReceived);
			
			// 일제공격 시작 이벤트 바인딩
			PackComp->OnCoordinatedAttackStarted.AddDynamic(this, &AAO_WerewolfController::HandleCoordinatedAttackStarted);
		}
	}
}

void AAO_WerewolfController::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// 플레이어 위치 추적 (이동 방향 분석용)
	LastPlayerLocation = Location;
	if (GetWorld())
	{
		LastLocationUpdateTime = GetWorld()->GetTimeSeconds();
	}

	// 아직 Howl을 하지 않았다면, 단순히 타겟만 설정
	// 실제 Howl 실행은 StateTree의 Howl Task에서 담당
	if (!bHasHowledOrJoined)
	{
		// 추격 대상 설정 (StateTree에서 접근 가능하도록)
		SetChaseTarget(Player);
		
		// bHasHowledOrJoined는 Howl Task에서 설정됨
		// StateTree가 Howl State에 진입하면 자동으로 처리
	}
	else
	{
		// 이미 Howl을 했으면 일반적인 추격 로직
		Super::OnPlayerDetected(Player, Location);
	}
}

void AAO_WerewolfController::HandleHowlReceived(AActor* TargetActor)
{
	AAO_PlayerCharacter* TargetPlayer = Cast<AAO_PlayerCharacter>(TargetActor);
	if (!TargetPlayer)
	{
		return;
	}

	bHasHowledOrJoined = true;
	
	// 단순히 타겟만 설정하는 것이 아니라, 추격 모드를 강제 시작
	StartChase(TargetPlayer);

	// Howl을 들었을 때의 로직 (StateTree에서 'HowlReceived' Condition이 true가 되어 Surround로 진입)
	// Surround 모드로 전환은 PackCoordComp에서 자동으로 처리됨
}

void AAO_WerewolfController::OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation)
{
	Super::OnPlayerLost(Player, LastKnownLocation);
	
	// 플레이어를 놓쳤을 때 포위 모드 해제
	if (PackComp && PackComp->IsSurrounding())
	{
		PackComp->SetSurroundMode(false);
	}
}

void AAO_WerewolfController::EndSearch()
{
	// 부모 클래스의 EndSearch 호출
	Super::EndSearch();
	
	// 수색 완료 후 배회로 돌아갈 때 Howl 상태 리셋
	// 다음에 플레이어를 발견하면 다시 Howl을 실행할 수 있도록
	ResetHowlState();
}

void AAO_WerewolfController::HandleCoordinatedAttackStarted()
{
	// 일제공격 시작 시 포위 모드 해제 및 추격 모드로 전환
	if (PackComp)
	{
		PackComp->SetSurroundMode(false);
	}

	AAO_AggressiveAIBase* AI = GetAggressiveAI();
	if (AI)
	{
		AI->SetChaseMode(true);
	}
}

void AAO_WerewolfController::TriggerHowl(AAO_PlayerCharacter* Target)
{
	if (!Target || !PackComp)
	{
		return;
	}

	// GAS Ability로 Howl 실행
	AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(GetPawn());
	if (!Wolf)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Wolf->GetAbilitySystemComponent();
	if (ASC)
	{
		// Howl Ability 태그로 활성화
		FGameplayTag HowlTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Howl"));
		if (HowlTag.IsValid())
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(HowlTag);
			ASC->TryActivateAbilitiesByTag(TagContainer);
		}
	}
}

FVector AAO_WerewolfController::AnalyzePlayerMovementDirection(AAO_PlayerCharacter* Target)
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	// 방법 1: CharacterMovement의 Velocity 사용 (가장 정확)
	UCharacterMovementComponent* MoveComp = Target->GetCharacterMovement();
	if (MoveComp)
	{
		FVector Velocity = MoveComp->Velocity;
		Velocity.Z = 0.f; // 수평 방향만
		if (Velocity.SizeSquared() > 100.f) // 최소 이동 속도
		{
			return Velocity.GetSafeNormal();
		}
	}

	// 방법 2: 최근 위치 변화로 계산
	if (LastPlayerLocation != FVector::ZeroVector)
	{
		FVector CurrentLocation = Target->GetActorLocation();
		FVector Direction = (CurrentLocation - LastPlayerLocation);
		Direction.Z = 0.f;
		
		if (Direction.SizeSquared() > 100.f)
		{
			return Direction.GetSafeNormal();
		}
	}

	// 방법 3: 플레이어의 Forward 벡터 사용 (최후의 수단)
	return Target->GetActorForwardVector();
}

TArray<FVector> AAO_WerewolfController::FindPotentialEscapeRoutes(AAO_PlayerCharacter* Target, float SearchRadius)
{
	TArray<FVector> EscapeRoutes;
	
	if (!Target)
	{
		return EscapeRoutes;
	}

	FVector PlayerLocation = Target->GetActorLocation();
	FVector MovementDir = AnalyzePlayerMovementDirection(Target);
	
	// 이동 방향이 없으면 Forward 벡터 사용
	if (MovementDir.IsNearlyZero())
	{
		MovementDir = Target->GetActorForwardVector();
	}

	// 플레이어 전방, 좌우, 후방 방향으로 Raycast하여 통로 찾기
	TArray<FVector> TestDirections = {
		MovementDir,                                    // 전방
		FVector::CrossProduct(MovementDir, FVector::UpVector).GetSafeNormal(), // 우측
		-FVector::CrossProduct(MovementDir, FVector::UpVector).GetSafeNormal(), // 좌측
		-MovementDir                                    // 후방
	};

	UWorld* World = GetWorld();
	if (!World)
	{
		return EscapeRoutes;
	}

	for (const FVector& TestDir : TestDirections)
	{
		FVector Start = PlayerLocation;
		FVector End = Start + TestDir * SearchRadius;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Target);
		Params.AddIgnoredActor(GetPawn());

		// Raycast로 벽/장애물 체크
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			// 장애물이 없으면 통로로 간주
			EscapeRoutes.Add(End);
		}
		else
		{
			// 장애물이 있으면 장애물 앞 위치를 통로로 간주
			EscapeRoutes.Add(Hit.Location - TestDir * 200.f);
		}
	}

	return EscapeRoutes;
}

TArray<FVector> AAO_WerewolfController::FindPotentialEscapeRoutesImproved(AAO_PlayerCharacter* Target, float SearchRadius, int32 NumSamples)
{
	TArray<FVector> EscapeRoutes;
	
	if (!Target)
	{
		return EscapeRoutes;
	}

	FVector PlayerLocation = Target->GetActorLocation();
	FVector MovementDir = AnalyzePlayerMovementDirection(Target);
	
	// 이동 방향이 없으면 Forward 벡터 사용
	if (MovementDir.IsNearlyZero())
	{
		MovementDir = Target->GetActorForwardVector();
	}

	// 8방향 또는 더 많은 샘플링
	const float AngleStep = 2.f * UE_PI / NumSamples;
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return EscapeRoutes;
	}

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Angle = AngleStep * i;
		
		// 이동 방향을 기준으로 회전
		FVector Right = FVector::CrossProduct(MovementDir, FVector::UpVector).GetSafeNormal();
		FVector Forward = MovementDir;
		
		FVector Direction = Forward * FMath::Cos(Angle) + Right * FMath::Sin(Angle);
		Direction.Z = 0.f;
		Direction.Normalize();

		// Raycast로 통로 확인
		FVector Start = PlayerLocation;
		FVector End = Start + Direction * SearchRadius;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Target);
		Params.AddIgnoredActor(GetPawn());

		// Raycast로 벽/장애물 체크
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			// 장애물이 없으면 NavMesh 프로젝션
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
			if (NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(End, NavLocation))
				{
					EscapeRoutes.Add(NavLocation.Location);
				}
			}
		}
		else
		{
			// 장애물이 있으면 장애물 앞 위치를 NavMesh에 프로젝션
			FVector BlockedLocation = Hit.Location - Direction * 200.f;
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
			if (NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(BlockedLocation, NavLocation))
				{
					EscapeRoutes.Add(NavLocation.Location);
				}
			}
		}
	}

	return EscapeRoutes;
}

void AAO_WerewolfController::ResetHowlState()
{
	bHasHowledOrJoined = false;
	
	// PackComp 상태도 리셋
	if (PackComp)
	{
		PackComp->SetSurroundMode(false);
	}
}

FVector AAO_WerewolfController::FindEscapeRouteBlockPosition(AAO_PlayerCharacter* Target, float BlockRadius)
{
	if (!Target || !GetPawn())
	{
		return FVector::ZeroVector;
	}

	// 잠재적 도주 경로 찾기
	TArray<FVector> EscapeRoutes = FindPotentialEscapeRoutes(Target, 1000.f);
	
	if (EscapeRoutes.Num() == 0)
	{
		// 도주 경로가 없으면 플레이어 주변 위치 반환
		return Target->GetActorLocation();
	}

	// 가장 가능성 높은 도주 경로 선택 (플레이어 이동 방향과 가장 일치하는 경로)
	FVector MovementDir = AnalyzePlayerMovementDirection(Target);
	FVector BestRoute = EscapeRoutes[0];
	float BestScore = -1.f;

	for (const FVector& Route : EscapeRoutes)
	{
		FVector DirToRoute = (Route - Target->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(MovementDir, DirToRoute);
		
		if (Dot > BestScore)
		{
			BestScore = Dot;
			BestRoute = Route;
		}
	}

	// 도주 경로를 차단할 위치 계산 (플레이어와 도주 경로 사이)
	FVector PlayerLocation = Target->GetActorLocation();
	FVector BlockPosition = PlayerLocation + (BestRoute - PlayerLocation) * 0.5f;

	// NavMesh 프로젝션
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(BlockPosition, NavLocation))
		{
			return NavLocation.Location;
		}
	}

	return BlockPosition;
}
