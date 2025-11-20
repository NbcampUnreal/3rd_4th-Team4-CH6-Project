// HSJ : AO_InspectionComponent.cpp
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Interaction/GAS/Ability/AO_GameplayAbility_Inspect_Click.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "AO_Log.h"

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
    if (!TargetActor)
    {
        return;
    }

    if (!TargetActor->GetClass()->ImplementsInterface(UAO_Interface_Inspectable::StaticClass()))
    {
        return;
    }

    // 클릭된 컴포넌트 찾기 (ComponentName으로 검색)
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Comp : PrimitiveComponents)
    {
        if (Comp && Comp->GetFName() == ComponentName)
        {
            // 인터페이스를 통해 클릭 이벤트 전달
            IAO_Interface_Inspectable* Inspectable = Cast<IAO_Interface_Inspectable>(TargetActor);
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

void UAO_InspectionComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent)
    {
        return;
    }

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
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
}

void UAO_InspectionComponent::EnterInspectionMode(AActor* InspectableActor)
{
    if (!InspectableActor || bIsInspecting)
    {
        return;
    }

    AActor* Owner = GetOwner();
    
    if (!Owner || !Owner->HasAuthority())
    {
        return;
    }

    UAO_InspectableComponent* InspectableComp = InspectableActor->FindComponentByClass<UAO_InspectableComponent>();
    if (!InspectableComp)
    {
        return;
    }

    CurrentInspectedActor = InspectableActor;
	// 이 값이 복제되면 PlayerCharacter의 Move()에서 입력 차단
    bIsInspecting = true; 

    // Inspection 중에만 마우스 클릭으로 버튼을 누를 수 있도록 동적으로 클릭 어빌리티 부여
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (ASC)
    {
        TSubclassOf<UGameplayAbility> ClickAbilityClass = UAO_GameplayAbility_Inspect_Click::StaticClass();
        FGameplayAbilitySpec Spec(ClickAbilityClass, 1, INDEX_NONE, this);
        GrantedClickAbilityHandle = ASC->GiveAbility(Spec);
    }

    // ClientRPC로 클라이언트도 어빌리티를 활성화할 수 있도록 Handle도 함께 전달
    ClientNotifyInspectionStarted_WithHandle(InspectableActor, GrantedClickAbilityHandle);
    
    // 리슨서버에서 ClientRPC는 호스트 본인에게는 전달되지 않으므로 로컬에서 직접 실행
    APlayerController* LocalPC = Cast<APlayerController>(Owner->GetOwner());
    if (LocalPC && LocalPC->IsLocalController())
    {
        FTransform CameraTransform = InspectableComp->GetInspectionCameraTransform();
        ClientEnterInspection(CameraTransform.GetLocation(), CameraTransform.Rotator());
    }
}

void UAO_InspectionComponent::ExitInspectionMode()
{
    if (!bIsInspecting)
    {
        return;
    }

    AActor* Owner = GetOwner();
    
    if (!Owner || !Owner->HasAuthority())
    {
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    
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
        UAO_InspectableComponent* InspectableComp = CurrentInspectedActor->FindComponentByClass<UAO_InspectableComponent>();
        if (InspectableComp)
        {
            InspectableComp->SetInspectionLocked(false, nullptr);
        }
    }

    // 상태 초기화 (복제됨)
    CurrentInspectedActor = nullptr;
    bIsInspecting = false; // PlayerCharacter의 Move()에서 입력 허용

    // ClientRPC로 클라이언트들에게 Inspection 종료 알림
    ClientNotifyInspectionEnded();
    
    // 리슨서버에서 호스트인 경우 따로 처리
    APlayerController* LocalPC = Cast<APlayerController>(Owner->GetOwner());
    if (LocalPC && LocalPC->IsLocalController())
    {
        ClientExitInspection();
    }
}

void UAO_InspectionComponent::ClientNotifyInspectionStarted_WithHandle_Implementation(AActor* InspectableActor, FGameplayAbilitySpecHandle AbilityHandle)
{
    if (!InspectableActor)
    {
        return;
    }

    // 서버에서 전달받은 Handle 저장
    GrantedClickAbilityHandle = AbilityHandle;
	
    UAO_InspectableComponent* InspectableComp = InspectableActor->FindComponentByClass<UAO_InspectableComponent>();
    if (!InspectableComp)
    {
        return;
    }
	
	// InspectableComponent에서 카메라 Transform 정보 가져오기
    FTransform CameraTransform = InspectableComp->GetInspectionCameraTransform();
    ClientEnterInspection(CameraTransform.GetLocation(), CameraTransform.Rotator());
}

void UAO_InspectionComponent::ClientNotifyInspectionEnded_Implementation()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (Owner->HasAuthority())
    {
        return;
    }

    ClientExitInspection();
}

// Inspection 진입 처리 (카메라 전환, UI 설정)
void UAO_InspectionComponent::ClientEnterInspection(const FVector& CameraLocation, const FRotator& CameraRotation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetOwner());
    
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    // 이미 Inspection 카메라가 있으면 스킵(서버일 경우)
    if (InspectionCameraActor)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character)
    {
        // 자기 자신 캐릭터의 모든 Primitive 컴포넌트 숨기기(조사 중에는 자기 자신 메시가 안보여야 함)
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Character->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        HiddenComponents.Empty();
        
        for (UPrimitiveComponent* Comp : PrimitiveComponents)
        {
            // 이미 숨겨진 컴포넌트는 제외
            if (Comp && !Comp->bHiddenInGame)
            {
                Comp->SetHiddenInGame(true);
                HiddenComponents.Add(Comp); // 나중에 복원하기 위해 저장
            }
        }

        // 애니메이션 정지 (Inspection 중에는 움직이면 안됨)
        USkeletalMeshComponent* Mesh = Character->GetMesh();
        if (Mesh)
        {
            UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Stop(0.0f);
            }
            Mesh->SetComponentTickEnabled(false);
        }
    }
    
    // Inspection 카메라로 전환
    TransitionToInspectionCamera(CameraLocation, CameraRotation);

    // 마우스 커서 표시, GameAndUI 모드로 전환
    PC->bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
    InputMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(InputMode);
}

// Inspection 나가기 처리 (카메라 복원, UI 설정)
void UAO_InspectionComponent::ClientExitInspection()
{
    if (!InspectionCameraActor)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetOwner());
    
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    // 플레이어 카메라로 복귀
    TransitionToPlayerCamera();

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character)
    {
        // 숨겼던 컴포넌트들 다시 표시
        for (UPrimitiveComponent* Comp : HiddenComponents)
        {
            if (Comp)
            {
                Comp->SetHiddenInGame(false);
            }
        }
        HiddenComponents.Empty();

        // 애니메이션 복구
        USkeletalMeshComponent* Mesh = Character->GetMesh();
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
}

void UAO_InspectionComponent::OnExitPressed()
{
    if (!bIsInspecting)
    {
        return;
    }

    AActor* Owner = GetOwner();
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

    AActor* Owner = GetOwner();
    
    if (!Owner || !GrantedClickAbilityHandle.IsValid())
    {
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (!ASC)
    {
        return;
    }

    // GA_Inspect_Click 어빌리티 활성화
    // LocalPredicted 정책이므로 로컬에서 즉시 실행 후 서버에 전달
    ASC->TryActivateAbility(GrantedClickAbilityHandle);
}

// 조사 카메라로 전환
void UAO_InspectionComponent::TransitionToInspectionCamera(const FVector& CameraLocation, const FRotator& CameraRotation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetOwner());
    if (!PC)
    {
        return;
    }

    // 현재 ViewTarget 저장 (나중에 복귀하기 위해)
    AActor* CurrentViewTarget = PC->GetViewTarget();
    
    // InspectionCamera가 아닐 때만 저장 (이미 InspectionCamera면 잘못된 참조)
    if (!InspectionCameraActor || CurrentViewTarget != InspectionCameraActor)
    {
        OriginalViewTarget = CurrentViewTarget;
    }

    // Inspection 카메라 액터 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    InspectionCameraActor = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(),
        CameraLocation,
        CameraRotation,
        SpawnParams
    );

    if (InspectionCameraActor)
    {
        UCameraComponent* CameraComp = InspectionCameraActor->GetCameraComponent();
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
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetOwner());
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