// AO_CeilingMoveComponent.cpp

#include "AI/Component/AO_CeilingMoveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

UAO_CeilingMoveComponent::UAO_CeilingMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAO_CeilingMoveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAO_CeilingMoveComponent, bIsCeilingMode);
}

void UAO_CeilingMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MoveComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UAO_CeilingMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCeilingMode)
	{
		UpdateCeilingPosition(DeltaTime);
	}
}

void UAO_CeilingMoveComponent::SetCeilingMode(bool bEnable)
{
	if (!OwnerCharacter) return;

	if (bEnable && !CheckCeilingAvailability())
	{
		// 천장이 없으면 활성화 불가
		return;
	}

	if (bIsCeilingMode != bEnable)
	{
		bIsCeilingMode = bEnable;

		USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
		
		if (bIsCeilingMode)
		{
			// NavMesh를 사용하기 위해 Walking 모드 유지
			// Gravity만 0으로 설정하여 중력 영향 제거
			MoveComp->GravityScale = 0.f;
			MoveComp->SetMovementMode(MOVE_Walking); // Flying이 아닌 Walking 유지
			
			// Mesh만 뒤집기 (Actor 회전은 유지하여 NavMesh와 호환)
			if (MeshComp)
			{
				// 초기 Rotation 저장 (한 번만)
				if (!bInitialRotationSaved)
				{
					InitialMeshRotation = MeshComp->GetRelativeRotation();
					bInitialRotationSaved = true;
				}
				
				// Pitch 180도로 뒤집기 (천장에 매달린 것처럼 보이도록)
				FRotator MeshRot = InitialMeshRotation;
				MeshRot.Pitch += 180.f;
				MeshComp->SetRelativeRotation(MeshRot);
			}

			// 천장 모드 진입 시 즉시 천장 위치로 이동
			UpdateCeilingPosition(0.f, true); // 즉시 적용
		}
		else
		{
			// 바닥 모드 복귀
			MoveComp->GravityScale = 1.f;
			MoveComp->SetMovementMode(MOVE_Walking);
			
			// Mesh 회전 복구
			if (MeshComp && bInitialRotationSaved)
			{
				MeshComp->SetRelativeRotation(InitialMeshRotation);
			}
		}
	}
}

bool UAO_CeilingMoveComponent::CheckCeilingAvailability() const
{
	if (!OwnerCharacter) return false;

	UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp) return false;

	// 캡슐 상단에서 시작하여 천장 검사
	FVector CapsuleLocation = CapsuleComp->GetComponentLocation();
	float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	FVector Start = CapsuleLocation;
	Start.Z += CapsuleHalfHeight; // 캡슐 상단에서 시작
	
	FVector End = Start + FVector::UpVector * CeilingTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	// WorldStatic 레이어에 대해 천장 검사
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);

	// 천장이 있고, 평평한지 확인 (Normal.Z가 -1에 가까워야 함)
	// Normal.Z < -0.7f: 천장이 수평에 가까움 (약 45도 이내)
	if (bHit && Hit.Normal.Z < -0.7f)
	{
		// 천장까지의 거리가 적절한지 확인 (너무 높으면 이동 불가)
		float DistanceToCeiling = (Hit.Location - Start).Size();
		if (DistanceToCeiling <= CeilingTraceDistance && DistanceToCeiling >= CapsuleHalfHeight * 0.5f)
		{
			return true;
		}
	}

	return false;
}

void UAO_CeilingMoveComponent::UpdateCeilingPosition(float DeltaTime, bool bImmediate)
{
	if (!OwnerCharacter || !MoveComp) return;

	UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp) return;

	// 현재 위치에서 바닥 NavMesh를 찾기 (천장 높이에서 바닥 NavMesh를 찾기 위해)
	FVector CurrentLoc = OwnerCharacter->GetActorLocation();
	FVector NavMeshLocation = CurrentLoc;
	
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		// 바닥 NavMesh로 프로젝션 (천장 높이에서 바닥까지 넓은 범위로 검색)
		FNavLocation NavLocation;
		FVector ProjectionStart = CurrentLoc;
		ProjectionStart.Z = CurrentLoc.Z - CeilingTraceDistance * 2.f; // 아래로 충분히 내려서 검색
		
		// NavMesh 프로젝션 (더 넓은 범위로 - X, Y는 500, Z는 천장 높이의 2배)
		FVector ProjectionExtent(500.f, 500.f, CeilingTraceDistance * 2.f);
		if (NavSys->ProjectPointToNavigation(ProjectionStart, NavLocation, ProjectionExtent))
		{
			// NavMesh 위치를 X, Y 기준으로 사용 (Z는 천장 높이로 조정)
			NavMeshLocation.X = NavLocation.Location.X;
			NavMeshLocation.Y = NavLocation.Location.Y;
		}
		else
		{
			// NavMesh를 찾지 못하면 현재 X, Y 유지
			AO_LOG(LogKSJ, Warning, TEXT("Stalker: Failed to project to NavMesh at ceiling mode"));
		}
	}

	// 천장 위치 확인 - NavMesh 프로젝션된 위치의 캡슐 상단에서 시작
	float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	FVector Start = NavMeshLocation;
	Start.Z += CapsuleHalfHeight; // 캡슐 상단에서 시작
	FVector End = Start + FVector::UpVector * CeilingTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);

	if (bHit)
	{
		// 천장에 붙이기 - 캡슐 상단이 천장에 붙도록 계산
		FVector TargetLoc = Hit.Location;
		TargetLoc.Z -= (CapsuleHalfHeight + CeilingOffset);

		// NavMesh 프로젝션된 X, Y를 사용하고 Z만 천장 높이로 조정
		FVector NewLocation = FVector(NavMeshLocation.X, NavMeshLocation.Y, TargetLoc.Z);
		
		float NewZ;
		if (bImmediate)
		{
			// 즉시 적용 (천장 모드 진입 시)
			NewZ = TargetLoc.Z;
		}
		else
		{
			// 부드러운 보정 (Interp 속도 증가)
			float InterpSpeed = 30.f;
			NewZ = FMath::FInterpTo(CurrentLoc.Z, TargetLoc.Z, DeltaTime, InterpSpeed);
		}
		
		NewLocation.Z = NewZ;
		
		// Sweep을 사용하여 충돌 체크 (벽에 부딪히는 것 방지)
		FHitResult SweepHit;
		bool bMoved = OwnerCharacter->SetActorLocation(NewLocation, true, &SweepHit, ETeleportType::None);
		
		// 벽에 부딪혔으면 로그만 남기고 위치는 유지 (Controller가 경로를 재계산할 것)
		if (!bMoved && SweepHit.bBlockingHit)
		{
			AO_LOG(LogKSJ, Verbose, TEXT("Stalker: Hit wall at ceiling mode, location: %s, blocking: %s"), 
				*NewLocation.ToString(), *SweepHit.GetActor()->GetName());
			// 경로를 다시 찾도록 Controller에 알림은 필요 없음 (PathFollowingComponent가 자동으로 처리)
		}
	}
	else
	{
		// 천장이 끊기면 바닥 모드로 전환 (낙하)
		SetCeilingMode(false);
	}
}

