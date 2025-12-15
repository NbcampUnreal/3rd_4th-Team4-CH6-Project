// HSJ : AO_InspectionComponent.cpp
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Interaction/GAS/Ability/AO_GameplayAbility_Inspect_Click.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AO_Log.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Interaction/Interface/AO_Interface_InspectionCameraTypes.h"
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "Puzzle/Element/AO_OverwatchInspectionPuzzle.h"

UAO_InspectionComponent::UAO_InspectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UAO_InspectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UAO_InspectionComponent, CurrentInspectedActor);
    DOREPLIFETIME(UAO_InspectionComponent, bIsInspecting);
}

// 클라이언트가 버튼을 클릭하면 서버로 전달하는 ServerRPC
// AAO_InspectionPuzzle은 Owner가 없어서 직접 ServerRPC를 호출할 수 없으므로
// PlayerCharacter가 소유한 이 컴포넌트를 통해 서버로 전달
void UAO_InspectionComponent::ServerProcessInspectionClick_Implementation(AActor* TargetActor, FName ComponentName)
{
    if (!TargetActor || !CurrentInspectedActor)
    {
        return;
    }

    // 현재 검사 중인 액터가 Inspectable 인터페이스를 구현하는지 확인
    if (!CurrentInspectedActor->GetClass()->ImplementsInterface(UAO_Interface_Inspectable::StaticClass()))
    {
        return;
    }

    // 클릭된 컴포넌트 찾기 (ComponentName으로 검색)
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (TObjectPtr<UPrimitiveComponent> Comp : PrimitiveComponents)
    {
        if (Comp && Comp->GetFName() == ComponentName)
        {
            // 인터페이스를 통해 클릭 이벤트 전달
            IAO_Interface_Inspectable* Inspectable = Cast<IAO_Interface_Inspectable>(CurrentInspectedActor);
            if (Inspectable)
            {
                Inspectable->OnInspectionMeshClicked(Comp);
            }
            break;
        }
    }
}

void UAO_InspectionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAO_InspectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 모든 타이머 클리어
	if (TObjectPtr<UWorld> World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoverTraceTimerHandle);
	}
    
	// 하이라이트 해제
	UpdateHoverHighlight(nullptr);
}

void UAO_InspectionComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent)
    {
        return;
    }

    TObjectPtr<UEnhancedInputComponent> EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        return;
    }

    if (ExitInspectionAction)
    {
        EnhancedInput->BindAction(ExitInspectionAction, ETriggerEvent::Started, this, &UAO_InspectionComponent::OnExitPressed);
    }

    if (InspectionClickInputAction)
    {
        EnhancedInput->BindAction(InspectionClickInputAction, ETriggerEvent::Started, this, &UAO_InspectionComponent::OnInspectionClick);
    }

    if (CameraMoveAction)
    {
        EnhancedInput->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &UAO_InspectionComponent::OnCameraMoveInput);
    }

	if (SpacebarAction)
	{
		EnhancedInput->BindAction(SpacebarAction, ETriggerEvent::Started, this, &UAO_InspectionComponent::OnSpacebarPressed);
	}
}

void UAO_InspectionComponent::EnterInspectionMode(AActor* InspectableActor)
{
	checkf(InspectableActor, TEXT("InspectableActor is null"));

	if (bIsInspecting)
	{
		return;
	}

    TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in EnterInspectionMode"));
    
	if (!Owner->HasAuthority())
	{
		return;
	}

    TObjectPtr<UAO_InspectableComponent> InspectableComp = InspectableActor->FindComponentByClass<UAO_InspectableComponent>();
	if (!InspectableComp)
	{
		return;
	}

    CurrentInspectedActor = InspectableActor;
	// 이 값이 복제되면 PlayerCharacter의 Move()에서 입력 차단
    bIsInspecting = true;

	RegisterCancelTags();

    // Inspection 중에만 마우스 클릭으로 버튼을 누를 수 있도록 동적으로 클릭 어빌리티 부여
    TObjectPtr<UAbilitySystemComponent> ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (ASC)
    {
        TSubclassOf<UGameplayAbility> ClickAbilityClass = UAO_GameplayAbility_Inspect_Click::StaticClass();
        FGameplayAbilitySpec Spec(ClickAbilityClass, 1, INDEX_NONE, this);
        GrantedClickAbilityHandle = ASC->GiveAbility(Spec);
    }

    // 카메라 설정
    FAO_InspectionCameraSettings CameraSettings;

    // 인터페이스를 통한 설정 가져오기
    if (IAO_Interface_InspectionCameraProvider* CameraProvider = Cast<IAO_Interface_InspectionCameraProvider>(InspectableActor))
    {
        CameraSettings = CameraProvider->GetInspectionCameraSettings();
    }
    else
    {
        // 기본 설정 (InspectableComponent에 있는 설정 사용)
        FTransform CameraTransform = InspectableComp->GetInspectionCameraTransform();
        CameraSettings.CameraMode = EInspectionCameraMode::RelativeToActor;
        CameraSettings.CameraLocation = CameraTransform.GetLocation();
        CameraSettings.CameraRotation = CameraTransform.Rotator();
        CameraSettings.MovementType = EInspectionMovementType::None;
    	CameraSettings.bHideCharacter = true;
    }

    // ClientRPC로 클라이언트도 어빌리티를 활성화할 수 있도록 Handle도 함께 전달
    ClientNotifyInspectionStarted(InspectableActor, GrantedClickAbilityHandle, CameraSettings);
    
    // 리슨서버에서 ClientRPC는 호스트 본인에게는 전달되지 않으므로 로컬에서 직접 실행
	TObjectPtr<APlayerController> LocalPC = Cast<APlayerController>(Owner->GetOwner());
	if (LocalPC && LocalPC->IsLocalController())
	{
		CurrentCameraSettings = CameraSettings;
		ClientEnterInspection(CameraSettings.CameraLocation, CameraSettings.CameraRotation);
	}
}

void UAO_InspectionComponent::ExitInspectionMode()
{
    if (!bIsInspecting)
    {
        return;
    }

	TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in ExitInspectionMode"));
    
	if (!Owner->HasAuthority())
	{
		return;
	}

    TObjectPtr<UAbilitySystemComponent> ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    
    // 클릭 어빌리티 제거
    if (ASC && GrantedClickAbilityHandle.IsValid())
    {
        ASC->ClearAbility(GrantedClickAbilityHandle);
        GrantedClickAbilityHandle = FGameplayAbilitySpecHandle(); // 핸들 무효화
    }

    // Inspect Enter 어빌리티 Cancel
    // GA_Inspect_Enter가 Status.Action.Inspecting 태그를 부여하므로 정리
    if (ASC && InspectEnterAbilityHandle.IsValid())
    {
        ASC->CancelAbilityHandle(InspectEnterAbilityHandle);
        InspectEnterAbilityHandle = FGameplayAbilitySpecHandle();
    }

    // Lock 해제 (다른 플레이어가 검사할 수 있도록)
    if (CurrentInspectedActor)
    {
        TObjectPtr<UAO_InspectableComponent> InspectableComp = CurrentInspectedActor->FindComponentByClass<UAO_InspectableComponent>();
        if (InspectableComp)
        {
            InspectableComp->SetInspectionLocked(false, nullptr);
        }
    }

	UnregisterCancelTags();

	TObjectPtr<AActor> InspectableActorBackup = CurrentInspectedActor;

    // 상태 초기화 (복제됨)
    CurrentInspectedActor = nullptr;
    bIsInspecting = false; // PlayerCharacter의 Move()에서 입력 허용

    // ClientRPC로 클라이언트들에게 Inspection 종료 알림
    ClientNotifyInspectionEnded(InspectableActorBackup);
    
    // 리슨서버에서 호스트인 경우 따로 처리
    TObjectPtr<APlayerController> LocalPC = Cast<APlayerController>(Owner->GetOwner());
    if (LocalPC && LocalPC->IsLocalController())
    {
    	TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(InspectableActorBackup);
    	if (OverwatchPuzzle)
    	{
    		OverwatchPuzzle->ClearAllExternalHighlights();
    	}
    	
        ClientExitInspection();
    }
}

void UAO_InspectionComponent::ClientNotifyInspectionStarted_Implementation(AActor* InspectableActor, FGameplayAbilitySpecHandle AbilityHandle, FAO_InspectionCameraSettings CameraSettings)
{
    if (!InspectableActor)
    {
        return;
    }

	CurrentInspectedActor = InspectableActor;

    // 서버에서 전달받은 Handle 저장
    GrantedClickAbilityHandle = AbilityHandle;
    CurrentCameraSettings = CameraSettings;

    ClientEnterInspection(CameraSettings.CameraLocation, CameraSettings.CameraRotation);
}

void UAO_InspectionComponent::ClientNotifyInspectionEnded_Implementation(AActor* InspectableActor)
{
    TObjectPtr<AActor> Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (Owner->HasAuthority())
    {
        return;
    }

	if (TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(InspectableActor))
	{
		OverwatchPuzzle->ClearAllExternalHighlights();
	}

    ClientExitInspection();
}

// Inspection 진입 처리 (카메라 전환, UI 설정)
void UAO_InspectionComponent::ClientEnterInspection(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in ClientEnterInspection"));

	TObjectPtr<APlayerController> PC = Cast<APlayerController>(Owner->GetOwner());
    
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

	if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		// Inspection Context를 높은 우선순위로 추가
		if (InspectionInputContext)
		{
			InputSubsystem->AddMappingContext(InspectionInputContext, InspectionInputPriority);
		}
	}

    // 이미 Inspection 카메라가 있으면 스킵(서버일 경우)
    if (InspectionCameraActor)
    {
        return;
    }

	TObjectPtr<ACharacter> Character = Cast<ACharacter>(Owner);
    if (Character && CurrentCameraSettings.bHideCharacter)
    {
        // 자기 자신 캐릭터의 모든 Primitive 컴포넌트 숨기기(조사 중에는 자기 자신 메시가 안보여야 함)
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Character->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        HiddenComponents.Empty();
        
        for (TObjectPtr<UPrimitiveComponent> Comp : PrimitiveComponents)
        {
            // 이미 숨겨진 컴포넌트는 제외
            if (Comp && !Comp->bHiddenInGame)
            {
                Comp->SetHiddenInGame(true);
                HiddenComponents.Add(Comp); // 나중에 복원하기 위해 저장
            }
        }

        // 애니메이션 정지 (Inspection 중에는 움직이면 안됨)
    	TObjectPtr<USkeletalMeshComponent> Mesh = Character->GetMesh();
        if (Mesh)
        {
        	TObjectPtr<UAnimInstance> AnimInstance = Mesh->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Stop(0.0f);
            }
            Mesh->SetComponentTickEnabled(false);
        }
    }

    // 초기 카메라 위치 저장 (클램프 기준)
    InitialCameraLocation = CameraLocation;
    
    // Inspection 카메라로 전환
    TransitionToInspectionCamera(CameraLocation, CameraRotation);

	if (TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(CurrentInspectedActor))
	{
		OverwatchPuzzle->HighlightAllExternalMeshes();
	}

    // 마우스 커서 표시, GameAndUI 모드로 전환
    PC->bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
    InputMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(InputMode);

	// Hover trace 시작
	if (!IsSpacebarMode())
	{
		StartHoverTrace();
	}
}

// Inspection 나가기 처리 (카메라 복원, UI 설정)
void UAO_InspectionComponent::ClientExitInspection()
{
    if (!InspectionCameraActor)
    {
        return;
    }

	// Hover trace 중지 및 하이라이트 해제
	StopHoverTrace();

	TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in ClientExitInspection"));

    TObjectPtr<APlayerController> PC = Cast<APlayerController>(Owner->GetOwner());
    
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

	if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (InspectionInputContext)
		{
			InputSubsystem->RemoveMappingContext(InspectionInputContext);
		}
	}

    // 플레이어 카메라로 복귀
    TransitionToPlayerCamera();

    TObjectPtr<ACharacter> Character = Cast<ACharacter>(Owner);
    if (Character && CurrentCameraSettings.bHideCharacter)
    {
        // 숨겼던 컴포넌트들 다시 표시
        for (TObjectPtr<UPrimitiveComponent> Comp : HiddenComponents)
        {
            if (Comp)
            {
                Comp->SetHiddenInGame(false);
            }
        }
        HiddenComponents.Empty();

        // 애니메이션 복구
        TObjectPtr<USkeletalMeshComponent> Mesh = Character->GetMesh();
        if (Mesh)
        {
            Mesh->SetComponentTickEnabled(true);
        }
    }

    // 마우스 커서 숨김, GameOnly 모드로 전환
    PC->bShowMouseCursor = false;
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);

    // Inspection 카메라 파괴 및 정리
    if (InspectionCameraActor)
    {
        InspectionCameraActor->Destroy();
        InspectionCameraActor = nullptr;
    }
    
    OriginalViewTarget = nullptr;

    // 설정 초기화
    CurrentCameraSettings = FAO_InspectionCameraSettings();
    InitialCameraLocation = FVector::ZeroVector;
}

void UAO_InspectionComponent::OnExitPressed()
{
    if (!bIsInspecting)
    {
        return;
    }

    TObjectPtr<AActor> Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 클라이언트면 서버로 요청, 서버면 직접 실행
    if (!Owner->HasAuthority())
    {
        ServerNotifyInspectionEnded();
    }
    else
    {
        ExitInspectionMode();
    }
}

void UAO_InspectionComponent::OnInspectionClick()
{
    if (!bIsInspecting)
    {
        return;
    }

    TObjectPtr<AActor> Owner = GetOwner();
    
    if (!Owner || !GrantedClickAbilityHandle.IsValid())
    {
        return;
    }

    TObjectPtr<UAbilitySystemComponent> ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (!ASC)
    {
        return;
    }

    // GA_Inspect_Click 어빌리티 활성화
    // LocalPredicted 정책이므로 로컬에서 즉시 실행 후 서버에 전달
    ASC->TryActivateAbility(GrantedClickAbilityHandle);
}

void UAO_InspectionComponent::OnCameraMoveInput(const FInputActionInstance& Instance)
{
    if (CurrentCameraSettings.MovementType == EInspectionMovementType::None)
    {
        return;
    }

    if (!bIsInspecting || !InspectionCameraActor)
    {
        return;
    }

    FVector2D InputValue = Instance.GetValue().Get<FVector2D>();
    if (InputValue.IsNearlyZero())
    {
        return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
        return;
    }

    float DeltaTime = World->GetDeltaSeconds();
    FVector CurrentLocation = InspectionCameraActor->GetActorLocation();
    FVector NewLocation = CurrentLocation;

    // 이동 타입에 따른 처리
    if (CurrentCameraSettings.MovementType == EInspectionMovementType::Planar)
    {
    	// 카메라 Yaw 기준으로 이동 방향 계산 (Pitch는 무시하고 XY 평면에서만 이동)
    	FRotator CameraRotation = InspectionCameraActor->GetActorRotation();
    	FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);
        
    	FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    	FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
    	// W/S = Forward 방향, A/D = Right 방향
    	FVector MoveDelta = (ForwardDirection * InputValue.Y + RightDirection * InputValue.X) 
						  * CurrentCameraSettings.MovementSpeed * DeltaTime;
        
    	NewLocation = ClampCameraPosition(CurrentLocation + MoveDelta);
    }

    InspectionCameraActor->SetActorLocation(NewLocation);
}

void UAO_InspectionComponent::OnSpacebarPressed()
{
	if (!bIsInspecting)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] Not inspecting, ignoring"));
		return;
	}

	if (!CurrentInspectedActor)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] No inspected actor, ignoring"));
		return;
	}

	TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(CurrentInspectedActor);
	if (!OverwatchPuzzle)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] Current actor is not OverwatchPuzzle: %s"), *CurrentInspectedActor->GetName());
		return;
	}

	if (!OverwatchPuzzle->bUseSpacebar)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] OverwatchPuzzle.bUseSpacebar is false"));
		return;
	}

	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] Owner is null"));
		return;
	}

	if (!Owner->HasAuthority())
	{
		ServerNotifySpacebarPressed();
	}
	else
	{
		OverwatchPuzzle->ActiveAllLinkedElements();
	}
}

void UAO_InspectionComponent::ServerNotifySpacebarPressed_Implementation()
{
	if (!bIsInspecting || !CurrentInspectedActor)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] ServerRPC: Invalid state"));
		return;
	}

	TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(CurrentInspectedActor);
	if (!OverwatchPuzzle)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] ServerRPC: Not OverwatchPuzzle"));
		return;
	}

	if (!OverwatchPuzzle->bUseSpacebar)
	{
		AO_LOG(LogHSJ, Warning, TEXT("[Spacebar] ServerRPC: bUseSpacebar is false"));
		return;
	}

	OverwatchPuzzle->ActiveAllLinkedElements();
}

// 조사 카메라로 전환
void UAO_InspectionComponent::TransitionToInspectionCamera(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in TransitionToInspectionCamera"));

    TObjectPtr<APlayerController> PC = Cast<APlayerController>(Owner->GetOwner());
    if (!PC)
    {
        return;
    }

    // 현재 ViewTarget 저장 (나중에 복귀하기 위해)
    TObjectPtr<AActor> CurrentViewTarget = PC->GetViewTarget();
    
    // InspectionCamera가 아닐 때만 저장 (이미 InspectionCamera면 잘못된 참조)
    if (!InspectionCameraActor || CurrentViewTarget != InspectionCameraActor)
    {
        OriginalViewTarget = CurrentViewTarget;
    }

	TObjectPtr<UWorld> World = GetWorld();
	checkf(World, TEXT("World is null in TransitionToInspectionCamera"));

    // Inspection 카메라 액터 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	InspectionCameraActor = World->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(),
		CameraLocation,
		CameraRotation,
		SpawnParams
	);

    if (InspectionCameraActor)
    {
        TObjectPtr<UCameraComponent> CameraComp = InspectionCameraActor->GetCameraComponent();
        if (CameraComp)
        {
            CameraComp->SetFieldOfView(90.0f);
        }

        // 카메라 전환 (블렌드 적용)
        PC->SetViewTargetWithBlend(InspectionCameraActor, CameraBlendTime);
    }
}

// 플레이어 카메라로 복귀
void UAO_InspectionComponent::TransitionToPlayerCamera()
{
	TObjectPtr<AActor> Owner = GetOwner();
	checkf(Owner, TEXT("Owner is null in TransitionToPlayerCamera"));

    TObjectPtr<APlayerController> PC = Cast<APlayerController>(Owner->GetOwner());
    if (!PC)
    {
        return;
    }

    // OriginalViewTarget 유효성 체크
    if (!OriginalViewTarget || !IsValid(OriginalViewTarget))
    {
        // 저장된 ViewTarget이 없거나 유효하지 않으면 Owner(PlayerCharacter)로 복귀
        OriginalViewTarget = Owner;
    }

    // 원래 카메라로 복귀 (블렌드 적용)
    PC->SetViewTargetWithBlend(OriginalViewTarget, CameraBlendTime);
}

void UAO_InspectionComponent::ServerNotifyInspectionEnded_Implementation()
{
    ExitInspectionMode();
}

FVector UAO_InspectionComponent::ClampCameraPosition(const FVector& NewPosition) const
{
    FVector Extent = CurrentCameraSettings.MovementBoundsExtent;
    
    return FVector(
        FMath::Clamp(NewPosition.X, InitialCameraLocation.X - Extent.X, InitialCameraLocation.X + Extent.X),
        FMath::Clamp(NewPosition.Y, InitialCameraLocation.Y - Extent.Y, InitialCameraLocation.Y + Extent.Y),
        FMath::Clamp(NewPosition.Z, InitialCameraLocation.Z - Extent.Z, InitialCameraLocation.Z + Extent.Z)
    );
}

void UAO_InspectionComponent::StartHoverTrace()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

    TWeakObjectPtr<UAO_InspectionComponent> WeakThis(this);
    
    World->GetTimerManager().SetTimer(
        HoverTraceTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
        {
        	if (TObjectPtr<UAO_InspectionComponent> StrongThis = WeakThis.Get())
        	{
				StrongThis->PerformHoverTrace();
			}
        }),
        HoverTraceRate,
        true
    );
}

void UAO_InspectionComponent::StopHoverTrace()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(HoverTraceTimerHandle);

    // 마지막 하이라이트 해제
    UpdateHoverHighlight(nullptr);
}

void UAO_InspectionComponent::PerformHoverTrace()
{
	if (!bIsInspecting || !CurrentInspectedActor) return;

	TObjectPtr<AActor> Owner = GetOwner();
    if (!Owner) return;

	TObjectPtr<APlayerController> PC = Cast<APlayerController>(Owner->GetOwner());
    if (!PC || !PC->IsLocalController()) return;

    // 마우스 커서 위치에서 Trace
    FVector2D MousePosition;
    if (!PC->GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        return;
    }

    FVector WorldLocation, WorldDirection;
    if (!PC->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection))
    {
        return;
    }

    FVector TraceStart = WorldLocation;
    FVector TraceEnd = WorldLocation + (WorldDirection * HoverTraceRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return;
	}

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetComponent() && HitResult.GetActor())
    {
    	TObjectPtr<AActor> HitActor = HitResult.GetActor();
    	TObjectPtr<UPrimitiveComponent> HitComponent = HitResult.GetComponent();
        
    	// 자기 자신(InspectionPuzzle)이면 내부 메시인지 확인
    	if (HitActor == CurrentInspectedActor)
    	{
    		// 내부 메시인지 ElementMappings로 확인
    		if (IsInternalClickableComponent(HitComponent))
    		{
    			UpdateHoverHighlight(HitComponent);
    			CachedHoverComponent = HitComponent;
    			CachedHoverActor = HitActor;
    			return;
    		}
            
    		// 내부 메시가 아니면 패널 배경이므로 제외
    		UpdateHoverHighlight(nullptr);
    		CachedHoverComponent = nullptr;
    		CachedHoverActor = nullptr;
    		return;
    	}
    	
    	// 외부 메시 체크
    	if (IsValidExternalClickTarget(HitActor, HitComponent))
    	{
    		UpdateHoverHighlight(HitComponent);
    		CachedHoverComponent = HitComponent;
    		CachedHoverActor = HitActor;
    		return;
    	}
    }

    // Hit 없거나 유효하지 않으면 하이라이트 해제
    UpdateHoverHighlight(nullptr);
	CachedHoverComponent = nullptr;
	CachedHoverActor = nullptr;
}

void UAO_InspectionComponent::UpdateHoverHighlight(UPrimitiveComponent* NewHoveredComponent)
{
    // 이전 하이라이트 해제
    if (CurrentHoveredComponent.IsValid() && CurrentHoveredComponent.Get() != NewHoveredComponent)
    {
    	CurrentHoveredComponent->SetRenderCustomDepth(false);
    }

    // 새 하이라이트 적용
    CurrentHoveredComponent = NewHoveredComponent;
    
    if (CurrentHoveredComponent.IsValid())
    {
        if (TObjectPtr<UMeshComponent> MeshComp = Cast<UMeshComponent>(CurrentHoveredComponent.Get()))
        {
            MeshComp->SetRenderCustomDepth(true);
            MeshComp->SetCustomDepthStencilValue(250);
        }
    }
}

bool UAO_InspectionComponent::IsInternalClickableComponent(UPrimitiveComponent* Component) const
{
	if (!Component || !CurrentInspectedActor) 
	{
		return false;
	}

	if (TObjectPtr<AAO_InspectionPuzzle> InspectionPuzzle = Cast<AAO_InspectionPuzzle>(CurrentInspectedActor))
	{
		return InspectionPuzzle->IsInternalComponentClickable(Component->GetFName());
	}

	return false;
}

bool UAO_InspectionComponent::IsValidExternalClickTarget(AActor* HitActor, UPrimitiveComponent* Component) const
{
    if (!CurrentInspectedActor || !HitActor)
    {
        return false;
    }

    // 검사 중인 액터 자체면 true
    if (HitActor == CurrentInspectedActor)
    {
        return true;
    }

    // 인터페이스를 통해 외부 클릭 대상 확인
    if (IAO_Interface_InspectionCameraProvider* CameraProvider = Cast<IAO_Interface_InspectionCameraProvider>(CurrentInspectedActor))
    {
        return CameraProvider->IsValidClickTarget(HitActor, Component);
    }

    return false;
}

void UAO_InspectionComponent::RegisterCancelTags()
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	TObjectPtr<UAbilitySystemComponent> ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC || CancelInspectionTags.IsEmpty())
	{
		return;
	}

	// 각 취소 태그에 대해 변경 감지 등록
	for (const FGameplayTag& CancelTag : CancelInspectionTags)
	{
		if (!CancelTag.IsValid())
		{
			continue;
		}

		FDelegateHandle Handle = ASC->RegisterGameplayTagEvent(CancelTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UAO_InspectionComponent::OnCancelTagChanged);
        
		CancelTagDelegateHandles.Add(Handle);
	}
}

void UAO_InspectionComponent::UnregisterCancelTags()
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TObjectPtr<UAbilitySystemComponent> ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
	{
		return;
	}

	// 모든 델리게이트 해제
	for (const FDelegateHandle& Handle : CancelTagDelegateHandles)
	{
		if (Handle.IsValid())
		{
			// 태그별로 등록된 델리게이트 제거
			for (const FGameplayTag& CancelTag : CancelInspectionTags)
			{
				if (CancelTag.IsValid())
				{
					ASC->RegisterGameplayTagEvent(CancelTag, EGameplayTagEventType::NewOrRemoved).Remove(Handle);
				}
			}
		}
	}

	CancelTagDelegateHandles.Empty();
}

void UAO_InspectionComponent::OnCancelTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그가 추가되었을 때만 (NewCount > 0) 취소
	if (NewCount > 0 && bIsInspecting)
	{
		ExitInspectionMode();
	}
}

bool UAO_InspectionComponent::IsSpacebarMode() const
{
	if (!CurrentInspectedActor)
	{
		return false;
	}

	if (TObjectPtr<AAO_OverwatchInspectionPuzzle> OverwatchPuzzle = Cast<AAO_OverwatchInspectionPuzzle>(CurrentInspectedActor))
	{
		return OverwatchPuzzle->bUseSpacebar;
	}

	return false;
}
