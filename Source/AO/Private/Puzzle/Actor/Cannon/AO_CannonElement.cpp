// HSJ : AO_CannonElement.cpp
#include "Puzzle/Actor/Cannon/AO_CannonElement.h"
#include "Puzzle/Actor/Cannon/AO_CannonProjectilePool.h"
#include "Puzzle/Actor/Cannon/AO_CannonProjectile.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraActor.h"
#include "AO_Log.h"

AAO_CannonElement::AAO_CannonElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComp);
    BaseMesh->SetIsReplicated(false);

    BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
    BarrelMesh->SetupAttachment(BaseMesh);
    BarrelMesh->SetIsReplicated(false);

    MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
    MuzzlePoint->SetupAttachment(BarrelMesh);

    ProjectilePool = CreateDefaultSubobject<UAO_CannonProjectilePool>(TEXT("ProjectilePool"));

    if (InspectableComponent)
    {
        InspectableComponent->CameraRelativeLocation = FVector(-200.0f, 0.0f, 50.0f);
        InspectableComponent->CameraRelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
    }
}

void AAO_CannonElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_CannonElement, ServerYaw);
    DOREPLIFETIME(AAO_CannonElement, ServerPitch);
    DOREPLIFETIME(AAO_CannonElement, CurrentOperator);
}

void AAO_CannonElement::BeginPlay()
{
    Super::BeginPlay();

    if (BaseMesh)
    {
        InitialBaseRotation = BaseMesh->GetRelativeRotation();
    }

    if (BarrelMesh)
    {
        InitialBarrelRotation = BarrelMesh->GetRelativeRotation();
    }

    // 초기값 동기화
	LocalYaw = ServerYaw;
	LocalPitch = ServerPitch;
	TargetYaw = ServerYaw;
	TargetPitch = ServerPitch;

	if (!HasAuthority())
	{
		UpdateBarrelRotation(LocalYaw, LocalPitch);
	}
}

void AAO_CannonElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopInterpolation();
	Super::EndPlay(EndPlayReason);
}

FAO_InspectionCameraSettings AAO_CannonElement::GetInspectionCameraSettings() const
{
    FAO_InspectionCameraSettings Settings;

    Settings.CameraMode = EInspectionCameraMode::RelativeToActor;
    
    if (InspectableComponent)
    {
        FTransform BarrelTransform = BarrelMesh ? BarrelMesh->GetComponentTransform() : GetActorTransform();
        FTransform CameraRelative(InspectableComponent->CameraRelativeRotation, InspectableComponent->CameraRelativeLocation);
        FTransform CameraWorld = CameraRelative * BarrelTransform;
        
        Settings.CameraLocation = CameraWorld.GetLocation();
        Settings.CameraRotation = CameraWorld.Rotator();
    }

    Settings.MovementType = EInspectionMovementType::Rotation;
    Settings.bHideCharacter = true;
    Settings.bUseActionButton = true;

    return Settings;
}

void AAO_CannonElement::OnInspectionInput(const FVector2D& InputValue, float DeltaTime)
{
	// 서버에서 WASD 입력, ServerYaw/Pitch 업데이트
    ProcessRotation(InputValue, DeltaTime, ServerYaw, ServerPitch);
    UpdateBarrelRotation(ServerYaw, ServerPitch);
    UpdateCameraTransform();

	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		TObjectPtr<APlayerController> PC = World->GetFirstPlayerController();
		if (PC)
		{
			CurrentOperator = PC->GetPawn();
		}
	}
}

void AAO_CannonElement::OnInspectionInputLocal(const FVector2D& InputValue, float DeltaTime)
{
	if (HasAuthority())
	{
		return;
	}

	// 클라에서 즉시 LocalYaw/Pitch 업데이트
	ProcessRotation(InputValue, DeltaTime, LocalYaw, LocalPitch);
	UpdateBarrelRotation(LocalYaw, LocalPitch);
	UpdateCameraTransform();

	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		TObjectPtr<APlayerController> PC = World->GetFirstPlayerController();
		if (PC)
		{
			CurrentOperator = PC->GetPawn();
		}
	}
}

void AAO_CannonElement::ProcessRotation(const FVector2D& InputValue, float DeltaTime, float& OutYaw, float& OutPitch)
{
	// 입력값으로 회전 계산
	OutYaw += InputValue.X * RotationSpeed * DeltaTime;
	OutPitch += InputValue.Y * RotationSpeed * DeltaTime;

	// 각도 클램프
	OutYaw = FMath::Clamp(OutYaw, -MaxYawAngle, MaxYawAngle);
	OutPitch = FMath::Clamp(OutPitch, MinPitchAngle, MaxPitchAngle);
}

void AAO_CannonElement::UpdateBarrelRotation(float Yaw, float Pitch)
{
	// BaseMesh는 좌우 회전
    if (BaseMesh)
    {
        FRotator NewBaseRotation = InitialBaseRotation;
        NewBaseRotation.Yaw += Yaw;
        BaseMesh->SetRelativeRotation(NewBaseRotation);
    }

	// Barrel는 상하 회전
    if (BarrelMesh)
    {
        FRotator NewBarrelRotation = InitialBarrelRotation;
        NewBarrelRotation.Pitch += Pitch;
        BarrelMesh->SetRelativeRotation(NewBarrelRotation);
    }
}

void AAO_CannonElement::UpdateCameraTransform()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

	TObjectPtr<APlayerController> PC = World->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	TObjectPtr<APawn> Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return;
	}

	TObjectPtr<UAO_InspectionComponent> InspectionComp = Pawn->FindComponentByClass<UAO_InspectionComponent>();
	if (!InspectionComp || !InspectionComp->IsInspecting())
	{
		return;
	}

	if (InspectionComp->GetInspectedActor() != this)
	{
		return;
	}

	if (!InspectionComp->InspectionCameraActor)
	{
		return;
	}

	if (BarrelMesh && InspectableComponent)
	{
		FTransform BarrelTransform = BarrelMesh->GetComponentTransform();
		FTransform CameraRelative(InspectableComponent->CameraRelativeRotation, InspectableComponent->CameraRelativeLocation);
		FTransform CameraWorld = CameraRelative * BarrelTransform;

		InspectionComp->InspectionCameraActor->SetActorLocation(CameraWorld.GetLocation());
		InspectionComp->InspectionCameraActor->SetActorRotation(CameraWorld.Rotator());
	}
}

void AAO_CannonElement::OnRep_ServerYaw()
{
	if (HasAuthority())
	{
		return;
	}

	// 타이머 없으면 시작(다른 사람 조작 시)
	TObjectPtr<UWorld> World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(InterpTimerHandle))
	{
		StartInterpolation();
	}

	// 자기가 조작 중이면 큰 차이만 스냅 보정
	if (IsLocalPlayerOperating())
	{
		float Diff = FMath::Abs(ServerYaw - LocalYaw);
		if (Diff > 5.0f)
		{
			LocalYaw = ServerYaw;
			UpdateBarrelRotation(LocalYaw, LocalPitch);
		}
		return;
	}

	// 타겟 설정
	TargetYaw = ServerYaw;
}

void AAO_CannonElement::OnRep_ServerPitch()
{
	if (HasAuthority())
	{
		return;
	}

	// 타이머 없으면 시작
	TObjectPtr<UWorld> World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(InterpTimerHandle))
	{
		StartInterpolation();
	}

	if (IsLocalPlayerOperating())
	{
		float Diff = FMath::Abs(ServerPitch - LocalPitch);
		if (Diff > 5.0f)
		{
			LocalPitch = ServerPitch;
			UpdateBarrelRotation(LocalYaw, LocalPitch);
		}
		return;
	}

	TargetPitch = ServerPitch;
}

void AAO_CannonElement::OnRep_CurrentOperator()
{
	if (HasAuthority())
	{
		return;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

	TObjectPtr<APlayerController> PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	// 조작자가 바뀌었는지 확인
	bool bIsNewOperator = CurrentOperator.IsValid() && (PC->GetPawn() != CurrentOperator.Get());
	
	if (bIsNewOperator)
	{
		// 즉시 스냅하여 부드러운 시작
		LocalYaw = ServerYaw;
		LocalPitch = ServerPitch;
		TargetYaw = ServerYaw;
		TargetPitch = ServerPitch;
		
		UpdateBarrelRotation(LocalYaw, LocalPitch);
		
		// 타이머 시작
		if (!World->GetTimerManager().IsTimerActive(InterpTimerHandle))
		{
			StartInterpolation();
		}
	}
	else if (CurrentOperator.IsValid() && !World->GetTimerManager().IsTimerActive(InterpTimerHandle))
	{
		// 조작자가 있는데 타이머 없으면 시작
		StartInterpolation();
	}
}

bool AAO_CannonElement::IsLocalPlayerOperating() const
{
	// 현재 로컬 플레이어가 조작 중인지
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return false;
	}

	TObjectPtr<APlayerController> PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	return PC->GetPawn() == CurrentOperator.Get();
}

void AAO_CannonElement::StartInterpolation()
{
	// 타이머 시작 (다른 사람 조작 시 부드러운 보간하기)
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

	// 이미 실행 중이면 스킵
	if (World->GetTimerManager().IsTimerActive(InterpTimerHandle))
	{
		return;
	}

	TWeakObjectPtr<AAO_CannonElement> WeakThis(this);

	World->GetTimerManager().SetTimer(
		InterpTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if (TObjectPtr<AAO_CannonElement> StrongThis = WeakThis.Get())
			{
				StrongThis->UpdateInterpolation();
			}
		}),
		InterpUpdateRate,
		true
	);
}

void AAO_CannonElement::StopInterpolation()
{
    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
    	return;
    }

    World->GetTimerManager().ClearTimer(InterpTimerHandle);
}

void AAO_CannonElement::UpdateInterpolation()
{
	if (HasAuthority())
	{
		return;
	}

	// 자기가 조작 중이면 보간 안함
	if (IsLocalPlayerOperating())
	{
		return;
	}
	// LocalYaw/Pitch를 TargetYaw/Pitch로 부드럽게 보간
	bool bNeedsUpdate = false;

	// Yaw 보간
	if (!FMath::IsNearlyEqual(LocalYaw, TargetYaw, 0.01f))
	{
		LocalYaw = FMath::FInterpTo(LocalYaw, TargetYaw, InterpUpdateRate, RotationInterpSpeed);
		bNeedsUpdate = true;
	}

	// Pitch 보간
	if (!FMath::IsNearlyEqual(LocalPitch, TargetPitch, 0.01f))
	{
		LocalPitch = FMath::FInterpTo(LocalPitch, TargetPitch, InterpUpdateRate, RotationInterpSpeed);
		bNeedsUpdate = true;
	}

	if (bNeedsUpdate)
	{
		UpdateBarrelRotation(LocalYaw, LocalPitch);
		UpdateCameraTransform();
	}
}

void AAO_CannonElement::OnInspectionAction()
{
    if (!HasAuthority()) 
    {
        return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    if (!World) 
    {
        return;
    }
	// 발사 쿨다운 체크
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastFireTime < FireCooldown)
    {
        return;
    }

    LastFireTime = CurrentTime;
	
	// 투사체 풀에서 가져오기
    if (!ProjectilePool || !MuzzlePoint) 
    {
        return;
    }

    TObjectPtr<AAO_CannonProjectile> Projectile = ProjectilePool->GetProjectile();
    if (!Projectile) 
    {
        return;
    }

    FVector MuzzleLocation = MuzzlePoint->GetComponentLocation();
    FRotator MuzzleRotation = MuzzlePoint->GetComponentRotation();
    FVector FireDirection = MuzzleRotation.Vector();

    Projectile->Activate(MuzzleLocation, MuzzleRotation);
    Projectile->Launch(FireDirection, ProjectileSpeed);

    MulticastPlayFireEffect();
}

void AAO_CannonElement::OnInspectionStarted()
{
	// Inspection 진입 시 타이머 시작
	if (!HasAuthority())
	{
		StartInterpolation();
	}
}

void AAO_CannonElement::OnInspectionEnded()
{
	// Inspection 끝나면 타이머 끄기
	StopInterpolation();

	if (!HasAuthority())
	{
		LocalYaw = ServerYaw;
		LocalPitch = ServerPitch;
		TargetYaw = ServerYaw;
		TargetPitch = ServerPitch;
		
		UpdateBarrelRotation(LocalYaw, LocalPitch);
	}
    
	CurrentOperator.Reset();
}

void AAO_CannonElement::MulticastPlayFireEffect_Implementation()
{
    if (!MuzzlePoint)
    {
    	return;
    }

    FVector MuzzleLocation = MuzzlePoint->GetComponentLocation();
    FRotator MuzzleRotation = MuzzlePoint->GetComponentRotation();

    if (MuzzleFlashVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, MuzzleFlashVFX, MuzzleLocation, MuzzleRotation
        );
    }
    else if (MuzzleFlashCascade)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            this, MuzzleFlashCascade, MuzzleLocation, MuzzleRotation
        );
    }

    if (FireSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, FireSFX, MuzzleLocation
        );
    }
}
