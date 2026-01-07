//KSJ : AO_CeilingMoveComponent

#include "AI/Component/AO_CeilingMoveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"

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

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// 보간 중인 경우
		if (bIsTransitioning)
		{
			float CurrentTime = GetWorld()->GetTimeSeconds();
			
			if (CurrentTime >= TransitionStartTime)
			{
				float ElapsedTime = CurrentTime - TransitionStartTime;
				float Alpha = FMath::Clamp(ElapsedTime / TransitionDuration, 0.f, 1.f);
				
				// EaseInOut 곡선 적용 (더 자연스러운 가속/감속)
				Alpha = Alpha * Alpha * (3.f - 2.f * Alpha);
				
				float CurrentOffsetZ = FMath::Lerp(StartOffsetZ, TargetOffsetZ, Alpha);
				
				USkeletalMeshComponent* MeshComp = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
				if (MeshComp && bInitialLocationSaved && bInitialRotationSaved)
				{
					// 위치 보간
					FVector NewRelLoc = InitialMeshRelativeLocation;
					NewRelLoc.Z += CurrentOffsetZ;
					MeshComp->SetRelativeLocation(NewRelLoc);
					
					// 회전 보간 (천장 모드로 전환 중 또는 바닥 모드로 전환 중)
					FRotator CurrentMeshRotation = MeshComp->GetRelativeRotation();
					FRotator InterpolatedRotation = FMath::RInterpTo(
						CurrentMeshRotation, 
						LastCeilingNormalRotation, 
						DeltaTime, 
						RotationInterpSpeed * 5.f // 보간 중에는 빠르게
					);
					MeshComp->SetRelativeRotation(InterpolatedRotation);
					
					// 천장 모드이고 보간 완료 시 회전도 업데이트
					if (bIsCeilingMode && Alpha >= 1.f)
					{
						UpdateCeilingPosition(DeltaTime, false);
					}
				}
				
				// 보간 완료
				if (Alpha >= 1.f)
				{
					bIsTransitioning = false;
					
					// 바닥 모드로 전환 완료 시 Mesh 회전/위치 복구
					if (!bIsCeilingMode && MeshComp)
					{
						MoveComp->GravityScale = 1.f;
						MoveComp->SetMovementMode(MOVE_Walking);
						
						if (bInitialRotationSaved)
						{
							MeshComp->SetRelativeRotation(InitialMeshRotation);
						}
						if (bInitialLocationSaved)
						{
							MeshComp->SetRelativeLocation(InitialMeshRelativeLocation);
						}
						
						AutoTransitionCheckTimer = 0.f;
					}
				}
			}
		}
		
		// 천장 모드이고 보간 완료된 경우 정상 업데이트
		if (bIsCeilingMode && !bIsTransitioning)
		{
			UpdateCeilingPosition(DeltaTime);
		}
	}
	else
	{
		// 클라이언트에서는 서버에서 계산된 Mesh 오프셋을 따라가기 위해
		// 간단한 보간만 수행 (실제 위치는 서버에서 리플리케이션됨)
		if (bIsCeilingMode && OwnerCharacter && MoveComp)
		{
			UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
			if (CapsuleComp)
			{
				// 클라이언트에서도 천장 감지하여 Mesh 회전 업데이트
				FVector CurrentLoc = OwnerCharacter->GetActorLocation();
				float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
				FVector Start = CurrentLoc;
				Start.Z += CapsuleHalfHeight;
				FVector End = Start + FVector::UpVector * CeilingTraceDistance;

				FHitResult Hit;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(OwnerCharacter);

				if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
				{
					// 천장 Normal에 맞춰 Mesh 회전만 업데이트
					UpdateCapsuleRotationToCeiling(Hit.Normal);
					
					// Mesh 위치 오프셋도 클라이언트에서 계산
					USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
					if (MeshComp && bInitialLocationSaved)
					{
						const float DesiredActorZ = Hit.Location.Z - (CapsuleHalfHeight + CeilingOffset);
						const float DesiredMeshOffsetZ = DesiredActorZ - CurrentLoc.Z;
						
						const float InterpSpeed = 30.f;
						const float CurrentOffsetZ = MeshComp->GetRelativeLocation().Z - InitialMeshRelativeLocation.Z;
						float NewOffsetZ = FMath::FInterpTo(CurrentOffsetZ, DesiredMeshOffsetZ, DeltaTime, InterpSpeed);
						
						FVector NewRelLoc = InitialMeshRelativeLocation;
						NewRelLoc.Z += NewOffsetZ;
						MeshComp->SetRelativeLocation(NewRelLoc);
					}
				}
			}
		}
	}
}

void UAO_CeilingMoveComponent::SetCeilingMode(bool bEnable)
{
	if (!OwnerCharacter) return;

	// 서버에서만 천장 가용성 체크
	// 주의: CheckCeilingAvailability()가 False면 강제로 리턴되므로, 
	// 디버그 라인이 Red로 뜨면 여기가 원인임.
	if (GetOwner() && GetOwner()->HasAuthority() && bEnable && !CheckCeilingAvailability())
	{
		// 천장이 없으면 활성화 불가
		return;
	}

	if (bIsCeilingMode != bEnable)
	{
		// 서버에서만 Movement 설정 변경
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
			
			if (bEnable)
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

				// 보간 시작: 현재 오프셋에서 천장 오프셋으로
				if (MeshComp && bInitialLocationSaved)
				{
					float CurrentOffsetZ = MeshComp->GetRelativeLocation().Z - InitialMeshRelativeLocation.Z;
					StartOffsetZ = CurrentOffsetZ;
					
					// 회전 보간 시작값 저장
					FRotator CurrentMeshRotation = MeshComp->GetRelativeRotation();
					
					// 천장 높이 계산 (목표 오프셋)
					UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
					if (CapsuleComp)
					{
						FVector CurrentLoc = OwnerCharacter->GetActorLocation();
						float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
						
						FVector Start = CurrentLoc;
						Start.Z += CapsuleHalfHeight;
						FVector End = Start + FVector::UpVector * CeilingTraceDistance;
						
						FHitResult Hit;
						FCollisionQueryParams Params;
						Params.AddIgnoredActor(OwnerCharacter);
						
						if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
						{
							const float DesiredActorZ = Hit.Location.Z - (CapsuleHalfHeight + CeilingOffset);
							TargetOffsetZ = DesiredActorZ - CurrentLoc.Z;
							
							// 목표 회전 계산 (축 정렬 방식)
							FVector NormalizedNormal = Hit.Normal.GetSafeNormal();
							FVector TargetUp = NormalizedNormal; // 천장 Normal (아래) = Mesh Head (아래)

							FVector ActorForward = OwnerCharacter->GetActorForwardVector();
							FVector TargetForward = FVector::VectorPlaneProject(ActorForward, TargetUp);
							if (TargetForward.IsNearlyZero())
							{
								TargetForward = OwnerCharacter->GetActorUpVector();
							}
							TargetForward.Normalize();

							FRotator TargetWorldRot = FRotationMatrix::MakeFromZX(TargetUp, TargetForward).Rotator();
							FTransform ActorTransform = OwnerCharacter->GetActorTransform();
							FRotator TargetRelativeRot = ActorTransform.InverseTransformRotation(TargetWorldRot.Quaternion()).Rotator();
							
							TargetRelativeRot.Yaw += InitialMeshRotation.Yaw + 180.f;

							// 목표 회전 저장 (보간용)
							LastCeilingNormalRotation = TargetRelativeRot;
						}
						else
						{
							TargetOffsetZ = CurrentOffsetZ; // 천장이 없으면 보간 안 함
							// 기본 180도 회전만 적용
							FRotator TargetMeshRotation = InitialMeshRotation;
							TargetMeshRotation.Pitch += 180.f;
							LastCeilingNormalRotation = TargetMeshRotation;
						}
					}
					
					// 보간 시간 계산 (몽타주 길이 기반 또는 설정값)
					if (TransitionInterpSpeed > 0.f)
					{
						TransitionDuration = FMath::Abs(TargetOffsetZ - StartOffsetZ) / TransitionInterpSpeed;
					}
					else
					{
						// 자동 계산: 40프레임 몽타주 기준, 4프레임~35프레임 = 31프레임 동안 보간
						// 30fps 가정: 31프레임 = 약 1.03초
						TransitionDuration = 1.03f;
					}
					
					bIsTransitioning = true;
					TransitionStartTime = GetWorld()->GetTimeSeconds() + TransitionStartDelay;
					
					// 보간 시작 시점에 즉시 기본 회전 적용 (180도 뒤집기)
					FRotator ImmediateRotation = InitialMeshRotation;
					ImmediateRotation.Pitch += 180.f;
					MeshComp->SetRelativeRotation(ImmediateRotation);
				}
			}
			else
			{
				// 바닥 모드 복귀: 보간으로 내려오기
				if (MeshComp && bInitialLocationSaved && bInitialRotationSaved)
				{
					float CurrentOffsetZ = MeshComp->GetRelativeLocation().Z - InitialMeshRelativeLocation.Z;
					StartOffsetZ = CurrentOffsetZ;
					TargetOffsetZ = 0.f; // 바닥 = 오프셋 0
					
					// 목표 회전은 초기 회전 (바닥 모드)
					LastCeilingNormalRotation = InitialMeshRotation;
					
					// 보간 시간 계산
					if (TransitionInterpSpeed > 0.f)
					{
						TransitionDuration = FMath::Abs(TargetOffsetZ - StartOffsetZ) / TransitionInterpSpeed;
					}
					else
					{
						// 천장→바닥도 동일하게 31프레임 동안 보간
						TransitionDuration = 1.03f;
					}
					
					bIsTransitioning = true;
					TransitionStartTime = GetWorld()->GetTimeSeconds() + TransitionStartDelay;
				}
			}
		}
		
		// 모드 플래그는 즉시 변경 (보간은 Tick에서 처리)
		bIsCeilingMode = bEnable;
		
		// 클라이언트에서는 Mesh 시각적 업데이트만 수행
		if (!GetOwner() || !GetOwner()->HasAuthority())
		{
			UpdateMeshVisualsForCeilingMode(bEnable);
		}
	}
}

bool UAO_CeilingMoveComponent::CheckCeilingAvailability() const
{
	if (!OwnerCharacter) return false;

	UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp) return false;

	// KSJ: 천장 감지 로직 개선 (SphereTrace)
	
	// 캡슐 상단 약간 아래에서 시작해서 위로 쏨 (캡슐에 가려지지 않게)
	float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	
	FVector Start = CapsuleComp->GetComponentLocation();
	Start.Z += CapsuleHalfHeight * 0.5f; 
	
	// 위쪽으로 Trace (거리 여유 있게)
	// CeilingTraceDistance가 500이라면 600까지 체크
	FVector End = Start + FVector::UpVector * (CeilingTraceDistance + 100.f);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerCharacter);

	FHitResult Hit;
	
	// LineTrace 대신 SphereTrace 사용 (반지름 30cm) -> 얇은 천장이나 틈새 감지 확률 UP
	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,
		End,
		30.f, // Radius
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic), // Trace Channel
		false, // bTraceComplex
		ActorsToIgnore,
		EDrawDebugTrace::None, // 디버그 라인 끔
		Hit,
		true, // IgnoreSelf
		FLinearColor::Red, // No Hit Color
		FLinearColor::Green, // Hit Color
		1.0f // Draw Time
	);

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
			// 천장까지의 거리 체크
			// SphereTrace는 표면 접촉점이 Hit.Location이므로 거리 계산 정확
			float DistanceToCeiling = (Hit.Location - Start).Size();
			
			// 최소 거리 체크 (너무 낮으면 안됨)
			if (DistanceToCeiling >= CapsuleHalfHeight * 0.5f)
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

void UAO_CeilingMoveComponent::OnRep_bIsCeilingMode()
{
	// 클라이언트에서 리플리케이션된 값이 변경되었을 때 Mesh 시각적 업데이트
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UpdateMeshVisualsForCeilingMode(bIsCeilingMode);
	}
}

void UAO_CeilingMoveComponent::UpdateMeshVisualsForCeilingMode(bool bEnable)
{
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp) return;

	if (bEnable)
	{
		// 초기 값 저장 (한 번만)
		if (!bInitialRotationSaved)
		{
			InitialMeshRotation = MeshComp->GetRelativeRotation();
			bInitialRotationSaved = true;
		}
		if (!bInitialLocationSaved)
		{
			InitialMeshRelativeLocation = MeshComp->GetRelativeLocation();
			bInitialLocationSaved = true;
		}

		// 클라이언트에서는 서버에서 계산된 위치를 기반으로 Mesh만 업데이트
		// 실제 천장 위치는 서버에서 계산되므로, 클라이언트는 Mesh 회전만 적용
		// 위치는 서버에서 리플리케이션된 Actor 위치를 따라감
		FRotator MeshRot = InitialMeshRotation;
		MeshRot.Pitch += 180.f;
		MeshComp->SetRelativeRotation(MeshRot);
	}
	else
	{
		// 바닥 모드 복귀: Mesh 회전/위치 복구
		if (bInitialRotationSaved)
		{
			MeshComp->SetRelativeRotation(InitialMeshRotation);
		}
		if (bInitialLocationSaved)
		{
			MeshComp->SetRelativeLocation(InitialMeshRelativeLocation);
		}
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

	FVector NormalizedNormal = CeilingNormal.GetSafeNormal();
	
	// Normal 변화가 작으면 무시 (데드존)
	if (!LastCeilingNormal.IsZero())
	{
		float NormalChange = FVector::Dist(NormalizedNormal, LastCeilingNormal);
		if (NormalChange < NormalChangeThreshold)
		{
			// 변화가 작으면 이전 회전 유지
			return;
		}
	}
	LastCeilingNormal = NormalizedNormal;

	// 기울어진 천장 지원을 위한 "축 정렬(Axis Alignment)" 방식
	// 목표: Mesh의 머리(Up)가 천장 Normal(아래)을 향하고, 
	//       Mesh의 얼굴(Forward)이 캐릭터 진행 방향을 향하도록 회전 계산.

	// 1. 월드 기준 목표 Up 벡터 (천장 Normal은 아래를 향함 -> Mesh 머리 방향)
	//    주의: Mesh의 Up 벡터(+Z)가 천장 Normal 방향(아래)이 되어야 함
	FVector TargetUp = NormalizedNormal;

	// 2. 월드 기준 목표 Forward 벡터 (캐릭터 진행 방향을 천장 평면에 투영)
	//    주의: 거꾸로 매달렸을 때 시각적 혼란을 막기 위해 벡터 방향 확인 필요
	//    증상: 진행 방향 반대 + 기울기 반전 -> Forward 벡터를 반전시켜본다.
	FVector ActorForward = OwnerCharacter->GetActorForwardVector();
	FVector TargetForward = FVector::VectorPlaneProject(ActorForward, TargetUp);
	
	// 진행 방향 반전 보정 (뒤를 보고 있다면 주석 해제/적용)
	// TargetForward = -TargetForward; 
	
	if (TargetForward.IsNearlyZero())
	{
		TargetForward = OwnerCharacter->GetActorUpVector();
	}
	TargetForward.Normalize();

	// 3. 월드 기준 목표 회전 행렬 생성 (MakeFromZX: Z=Up, X=Forward)
	FRotator TargetWorldRot = FRotationMatrix::MakeFromZX(TargetUp, TargetForward).Rotator();

	// 4. 월드 회전을 로컬(Relative) 회전으로 변환
	FTransform ActorTransform = OwnerCharacter->GetActorTransform();
	FRotator TargetRelativeRot = ActorTransform.InverseTransformRotation(TargetWorldRot.Quaternion()).Rotator();

	// 5. Mesh의 초기 회전 오프셋 보정
	//    증상: 진행 방향 반대 -> Yaw 180도 추가
	TargetRelativeRot.Yaw += InitialMeshRotation.Yaw + 180.f;
	//    InitialMeshRotation의 Z값(Height) 보정이 필요할 수도 있으나 회전만 다룸

	// 부드러운 회전 보간
	FRotator CurrentMeshRotation = MeshComp->GetRelativeRotation();
	FRotator InterpolatedRotation = FMath::RInterpTo(CurrentMeshRotation, TargetRelativeRot, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	
	MeshComp->SetRelativeRotation(InterpolatedRotation);
	LastCeilingNormalRotation = TargetRelativeRot;
}
