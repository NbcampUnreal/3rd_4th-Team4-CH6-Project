// HSJ : AO_PushableRockElement.cpp
#include "Puzzle/Grid/AO_PushableRockElement.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "AO_Log.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Puzzle/Grid/AO_GridWall.h"

AAO_PushableRockElement::AAO_PushableRockElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ElementType = EPuzzleElementType::Toggle;
    bHandleToggleInOnInteractionSuccess = false;
}

void AAO_PushableRockElement::BeginPlay()
{
    Super::BeginPlay();

	if (!GridManager)
	{
		return;
	}

	// 현재 위치를 격자 좌표로 변환
	CurrentGridCoord = GridManager->WorldToGrid(GetActorLocation());
	InitialGridCoord = CurrentGridCoord;
    
	// 격자에 정렬
	FVector SnapPosition = GridManager->GridToWorld(CurrentGridCoord);
	SetActorLocation(SnapPosition);
    
	// GridManager에 등록
	if (HasAuthority())
	{
		GridManager->RegisterRock(this, CurrentGridCoord);
	}
}

void AAO_PushableRockElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GridManager)
	{
		GridManager->UnregisterRock(this);
	}

    TObjectPtr<UWorld> World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MoveAnimationTimer);
    }

    Super::EndPlay(EndPlayReason);
}

void AAO_PushableRockElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_PushableRockElement, CurrentGridCoord);
}

bool AAO_PushableRockElement::CheckWallsAtTarget(const FVector& InTargetLocation)
{
    if (!GridManager || !GetWorld())
    {
        return false;
    }

    // 이동 방향
    MoveDirection = (InTargetLocation - GetActorLocation()).GetSafeNormal();

    FVector Forward = MoveDirection;
    FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
    FVector TargetCenter = InTargetLocation;
    
    float TraceDistance = GridManager->CellSize;
    
    TArray<FHitResult> HitsForward, HitsLeft, HitsRight;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    
    if (GetWorld())
    {
        GetWorld()->LineTraceMultiByObjectType(
            HitsForward, TargetCenter, TargetCenter + Forward * TraceDistance,
            ObjectParams, TraceParams);
        
        GetWorld()->LineTraceMultiByObjectType(
            HitsLeft, TargetCenter, TargetCenter - Right * TraceDistance,
            ObjectParams, TraceParams);
        
        GetWorld()->LineTraceMultiByObjectType(
            HitsRight, TargetCenter, TargetCenter + Right * TraceDistance,
            ObjectParams, TraceParams);
        
        auto HasObstacle = [](const TArray<FHitResult>& Hits) -> bool
        {
            for (const FHitResult& Hit : Hits)
            {
                if (AActor* HitActor = Hit.GetActor())
                {
                    if (HitActor->IsA<AAO_GridWall>() || HitActor->IsA<AAO_PushableRockElement>())
                    {
                        return true;
                    }
                }
            }
            return false;
        };
        
        bWallForward = HasObstacle(HitsForward);
        bWallLeft = HasObstacle(HitsLeft);
        bWallRight = HasObstacle(HitsRight);
        
        bool bThreeSidesBlocked = bWallForward && bWallLeft && bWallRight;
        
        if (bThreeSidesBlocked)
        {
            TArray<FOverlapResult> Overlaps;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            
            if (MeshComponent)
            {
                GetWorld()->OverlapMultiByChannel(
                    Overlaps,
                    TargetCenter,
                    FQuat::Identity,
                    ECC_Pawn,
                    FCollisionShape::MakeSphere(GridManager->CellSize * 0.4f),
                    QueryParams
                );
                
                for (const FOverlapResult& Overlap : Overlaps)
                {
                    if (AActor* OverlappedActor = Overlap.GetActor())
                    {
                        if (OverlappedActor->IsA(APawn::StaticClass()))
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

bool AAO_PushableRockElement::TryPush(EGridDirection Direction)
{
    if (!HasAuthority() || !GridManager  || bIsMoving)
    {
        return false;
    }

    if (!GridManager->CanRockMove(CurrentGridCoord, Direction))
    {
        return false;
    }

    FIntPoint NewGridCoord = CurrentGridCoord + AAO_GridManager::GetDirectionVector(Direction);
	FVector InTargetLocation = GridManager->GridToWorld(NewGridCoord);
    
	// 목표 셀이 3면(전방, 왼쪽, 오른쪽 벽)이 막혀있는지 체크
	if (!CheckWallsAtTarget(InTargetLocation))
	{
		// 캐릭터가 갇힐 수 있으니 이동 불가
		return false;  
	}

	// 매니저에서 먼저 위치 갱신을 해서 바위가 해당 위치로 움직이는 중에 다른 바위가 해당 위치로 움직이는 것을 방지
    GridManager->UpdateRockPosition(this, CurrentGridCoord, NewGridCoord);
    
    FIntPoint OldCoord = CurrentGridCoord;
    CurrentGridCoord = NewGridCoord;
    
    StartMoveAnimation(InTargetLocation);

    return true;
}

void AAO_PushableRockElement::OnInteractionSuccess(AActor* Interactor)
{
	if (!Interactor || !HasAuthority())
	{
		return;
	}

	EGridDirection PushDirection = GetPushDirectionFromInteractor(Interactor);
    
	// 실패하면 중단
	if (!TryPush(PushDirection))
	{
		return;
	}
    
	if (ActivateEffect.IsValid())
	{
		FTransform SpawnTransform = GetInteractionTransform();
		MulticastPlayInteractionEffect(ActivateEffect, SpawnTransform);
	}
}

bool AAO_PushableRockElement::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}
    
	if (bIsMoving)
	{
		return false;
	}
    
	return true;
}

void AAO_PushableRockElement::ResetToInitialState()
{
	if (!HasAuthority() || !GridManager)
	{
		return;
	}

	// 이동 애니메이션 중지
	bIsMoving = false;
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MoveAnimationTimer);
	}

	// 좌표 업데이트
	GridManager->UpdateRockPosition(this, CurrentGridCoord, InitialGridCoord);
	CurrentGridCoord = InitialGridCoord;
    
	FVector ResetLocation = GridManager->GridToWorld(InitialGridCoord);

	// 초기 위치에 캐릭터가 있으면 이동
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
    
	if (MeshComponent && World)
	{
		World->OverlapMultiByChannel(
			Overlaps,
			ResetLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(MeshComponent->Bounds.SphereRadius),
			QueryParams
		);
        
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (ACharacter* Character = Cast<ACharacter>(Overlap.GetActor()))
			{
				// 캐릭터 이동
				FVector CharacterTargetLocation = ResetLocation + ResetPushOffset;
				Character->SetActorLocation(CharacterTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}

	// 바위 이동
	SetActorLocation(ResetLocation, false, nullptr, ETeleportType::TeleportPhysics);
    
	Super::ResetToInitialState();
}

bool AAO_PushableRockElement::IsAtGoalCell() const
{
    return GoalCells.Num() > 0 && GoalCells.Contains(CurrentGridCoord);
}

void AAO_PushableRockElement::OnRep_CurrentGridCoord()
{
    if (!HasAuthority() && GridManager)
    {
        FVector InTargetLocation = GridManager->GridToWorld(CurrentGridCoord);
        StartMoveAnimation(InTargetLocation);
    }
}

void AAO_PushableRockElement::StartMoveAnimation(const FVector& InTargetLocation)
{
    TargetWorldLocation = InTargetLocation;
    bIsMoving = true;

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
    	return;
    }

    TWeakObjectPtr<AAO_PushableRockElement> WeakThis(this);
    
    World->GetTimerManager().SetTimer(
        MoveAnimationTimer,
        FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
        {
            if (TObjectPtr<AAO_PushableRockElement> StrongThis = WeakThis.Get())
            {
                StrongThis->UpdateMoveAnimation();
            }
        }),
        0.016f,
        true
    );
}

void AAO_PushableRockElement::UpdateMoveAnimation()
{
	if (!bIsMoving)
	{
		return;
	}
    
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetWorldLocation, 0.016f, MoveSpeed);
    
	SetActorLocation(NewLocation, false);
    
	// 캐릭터 감지
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (MeshComponent && GetWorld())
    {
        GetWorld()->OverlapMultiByChannel(
            Overlaps,
            NewLocation,
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeSphere(MeshComponent->Bounds.SphereRadius * 1.2f),
            QueryParams
        );
        
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OverlappedActor = Overlap.GetActor();
            
            if (OverlappedActor && OverlappedActor->IsA(APawn::StaticClass()))
            {
                FVector ToCharacter = (OverlappedActor->GetActorLocation() - NewLocation).GetSafeNormal();
                float DotProduct = FVector::DotProduct(ToCharacter, MoveDirection);
                
                // 앞쪽에 있는 캐릭터만
            	if (DotProduct > 0.5f)
                {
                    if (ACharacter* Character = Cast<ACharacter>(OverlappedActor))
                    {
                        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
                        {
                            FVector Forward = MoveDirection;
                            FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
                            
                            // 탈출 방향 계산
                            FVector EscapeDirection = ToCharacter;
                            
                            if (bWallForward)
                            {
                                // 전방에 벽이 있으면 좌우로 탈출
                                if (!bWallRight && bWallLeft)
                                {
                                    // 오른쪽으로 탈출
                                    EscapeDirection = (Forward + Right * 2.0f).GetSafeNormal();
                                }
                                else if (!bWallLeft && bWallRight)
                                {
                                    // 왼쪽으로 탈출
                                    EscapeDirection = (Forward - Right * 2.0f).GetSafeNormal();
                                }
                                else if (!bWallLeft && !bWallRight)
                                {
                                    // 양쪽 다 비었으면 캐릭터 위치에 따라
                                    float CharacterOffset = FVector::DotProduct(ToCharacter, Right);
                                    
                                    if (FMath::Abs(CharacterOffset) < 0.1f)
                                    {
                                        // 거의 중앙이면 랜덤으로
                                        float RandomOffset = FMath::RandRange(-1.5f, 1.5f);
                                        EscapeDirection = (Forward + Right * RandomOffset).GetSafeNormal();
                                    }
                                    else
                                    {
                                        // 이미 한쪽으로 치우쳐있으면 그쪽으로
                                        float SignValue = FMath::Sign(CharacterOffset);
                                        EscapeDirection = (Forward + Right * SignValue * 2.0f).GetSafeNormal();
                                    }
                                }
                                else
                                {
                                    // 양쪽 다 막혔으면 뒤로
                                    EscapeDirection = -Forward;
                                }
                            }
                            
                            EscapeDirection.Z = 0;
                            float PushStrength = MoveSpeed * 500.0f;
                            
                            MovementComp->AddImpulse(EscapeDirection * PushStrength, true);
                        }
                    }
                }
            }
        }
    }
    
	if (FVector::Dist(NewLocation, TargetWorldLocation) < 1.0f)
	{
		SetActorLocation(TargetWorldLocation);
		bIsMoving = false;
        
		TObjectPtr<UWorld> World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(MoveAnimationTimer);
		}
        
		// 정답 셀인지 아닌지 체크
		if (HasAuthority())
		{
			// Toggle이 매번 작동하도록 상태 리셋
			bIsActivated = false;  // 다음에 밀 수 있게 리셋
            
			if (GoalCells.Num() > 0)
			{
				bool bIsAtGoal = IsAtGoalCell();
				SetActivationState(bIsAtGoal);
			}
		}
	}
}

EGridDirection AAO_PushableRockElement::GetPushDirectionFromInteractor(AActor* Interactor) const
{
    if (!Interactor)
    {
    	return EGridDirection::North;
    }

    FVector ToRock = GetActorLocation() - Interactor->GetActorLocation();
    return AAO_GridManager::GetDirectionFromVector(ToRock);
}