// AO_CeilingMoveComponent.cpp

#include "AI/Component/AO_CeilingMoveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"

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

	// 서버에서만 실행되어야 함 (위치 조정은 서버에서만)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsCeilingMode)
	{
		UpdateCeilingPosition(DeltaTime);
	}
	else
	{
		// 바닥 모드일 때 주기적으로 천장 감지 및 자동 전환 체크
		AutoTransitionCheckTimer += DeltaTime;
		if (AutoTransitionCheckTimer >= AutoTransitionCheckInterval)
		{
			AutoTransitionCheckTimer = 0.f;
			CheckForCeilingAutoTransition();
		}
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
			// 실제 회전은 UpdateCeilingPosition에서 천장 Normal에 맞춰 조정됨
			if (MeshComp)
			{
				// 초기 Rotation 저장 (한 번만)
				if (!bInitialRotationSaved)
				{
					InitialMeshRotation = MeshComp->GetRelativeRotation();
					bInitialRotationSaved = true;
				}
				// 초기 Location 저장 (한 번만)
				if (!bInitialLocationSaved)
				{
					InitialMeshRelativeLocation = MeshComp->GetRelativeLocation();
					bInitialLocationSaved = true;
				}
			}

			// 천장 모드 진입 시 즉시 천장 위치로 "보이도록" 보정 (회전/오프셋)
			UpdateCeilingPosition(0.f, true); // 즉시 적용
		}
		else
		{
			// 바닥 모드 복귀
			MoveComp->GravityScale = 1.f;
			MoveComp->SetMovementMode(MOVE_Walking);
			
			// Mesh 회전/위치 복구
			if (MeshComp && bInitialRotationSaved)
			{
				MeshComp->SetRelativeRotation(InitialMeshRotation);
			}
			if (MeshComp && bInitialLocationSaved)
			{
				MeshComp->SetRelativeLocation(InitialMeshRelativeLocation);
			}

			// 자동 전환 체크 타이머 리셋
			AutoTransitionCheckTimer = 0.f;
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

	if (bHit)
	{
		// 천장 Normal의 Z 성분을 각도로 변환
		// Normal.Z = -1.0 (수평) ~ 0.0 (수직)
		// MaxCeilingAngle에 맞춰 허용 각도 계산
		float MinNormalZ = FMath::Cos(FMath::DegreesToRadians(90.f - MaxCeilingAngle));
		MinNormalZ = -MinNormalZ; // 천장이므로 음수

		// 천장이 허용 각도 내에 있는지 확인
		if (Hit.Normal.Z <= MinNormalZ)
		{
			// 천장까지의 거리가 적절한지 확인 (너무 높으면 이동 불가)
			float DistanceToCeiling = (Hit.Location - Start).Size();
			if (DistanceToCeiling <= CeilingTraceDistance && DistanceToCeiling >= CapsuleHalfHeight * 0.5f)
			{
				return true;
			}
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
		// 핵심: NavMesh는 바닥에만 있으므로, Actor(캡슐)는 바닥 NavMesh를 따라 움직여야 한다.
		// 천장 모드에서는 Actor를 천장으로 "텔레포트"하지 않고, Mesh만 위로 오프셋하여
		// 시각적으로 천장에 붙어 보이게 만든다. (MoveTo/PathFollowing 안정화 목적)

		USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
		if (!MeshComp || !bInitialLocationSaved)
		{
			return;
		}

		// 천장에 붙어 보일 목표 Z (Actor를 옮길 경우의 목표 Z)
		const float DesiredActorZ = Hit.Location.Z - (CapsuleHalfHeight + CeilingOffset);
		const float DesiredMeshOffsetZ = DesiredActorZ - CurrentLoc.Z; // Actor는 그대로이므로 Mesh만 올림

		float NewOffsetZ = DesiredMeshOffsetZ;
		if (!bImmediate)
		{
			// 부드러운 보정
			const float InterpSpeed = 30.f;
			const float CurrentOffsetZ = MeshComp->GetRelativeLocation().Z - InitialMeshRelativeLocation.Z;
			NewOffsetZ = FMath::FInterpTo(CurrentOffsetZ, DesiredMeshOffsetZ, DeltaTime, InterpSpeed);
		}

		// 기울어진 천장에 맞춰 Mesh 회전 조정
		UpdateCapsuleRotationToCeiling(Hit.Normal);

		// Mesh 위치 오프셋 적용 (X/Y는 유지)
		FVector NewRelLoc = InitialMeshRelativeLocation;
		NewRelLoc.Z += NewOffsetZ;
		MeshComp->SetRelativeLocation(NewRelLoc);
		
	}
	else
	{
		// 천장이 끊기면 바닥 모드로 전환 (낙하)
		SetCeilingMode(false);
	}
}

void UAO_CeilingMoveComponent::CheckForCeilingAutoTransition()
{
	// 바닥 모드일 때만 자동 전환 체크
	if (bIsCeilingMode || !OwnerCharacter)
	{
		return;
	}

	// 천장이 있는지 확인
	if (CheckCeilingAvailability())
	{
		// 천장이 있으면 자동으로 천장 모드로 전환
		SetCeilingMode(true);
	}
}

void UAO_CeilingMoveComponent::UpdateCapsuleRotationToCeiling(const FVector& CeilingNormal)
{
	if (!OwnerCharacter || !bIsCeilingMode)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp || !bInitialRotationSaved)
	{
		return;
	}

	// 천장 Normal이 거의 수평이면 기본 180도 회전만 적용
	if (CeilingNormal.Z < -0.99f)
	{
		// 수평 천장: 초기 회전 + 180도만 적용
		FRotator MeshRot = InitialMeshRotation;
		MeshRot.Pitch += 180.f;
		MeshComp->SetRelativeRotation(MeshRot);
		return;
	}

	// 기울어진 천장: 천장 Normal에 맞춰 추가 회전 계산
	// 기본 Up 벡터에서 천장 Up 벡터로의 회전 계산
	FVector DefaultUp = FVector::UpVector;
	FVector CeilingUp = -CeilingNormal.GetSafeNormal();
	
	// 두 벡터 사이의 회전 쿼터니언 계산
	FQuat RotationQuat = FQuat::FindBetweenNormals(DefaultUp, CeilingUp);
	FRotator AdditionalRotation = RotationQuat.Rotator();
	
	// 초기 회전 + 180도 (거꾸로 매달림) + 천장 기울기 보정
	FRotator FinalMeshRotation = InitialMeshRotation;
	FinalMeshRotation.Pitch += 180.f;
	
	// 추가 회전을 부드럽게 보간
	FRotator CurrentMeshRotation = MeshComp->GetRelativeRotation();
	FRotator TargetRotation = FinalMeshRotation + AdditionalRotation;
	
	// 부드러운 회전 보간 (회전 속도 조절 가능)
	float RotationSpeed = 5.f;
	FRotator InterpolatedRotation = FMath::RInterpTo(CurrentMeshRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed);
	
	MeshComp->SetRelativeRotation(InterpolatedRotation);
}

