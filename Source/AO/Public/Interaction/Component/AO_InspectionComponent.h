// HSJ : AO_InspectionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "Interaction/Interface/AO_Interface_InspectionCameraTypes.h"
#include "AO_InspectionComponent.generated.h"

class UAO_InspectableComponent;
class ACameraActor;
class UInputAction;
class UGameplayAbility;
struct FInputActionInstance;
class UInputMappingContext;

/**
 * Inspection 시스템의 핵심 컴포넌트
 * 
 * 1. Inspection 상태 관리 (bIsInspecting, CurrentInspectedActor)
 * 2. 클릭 어빌리티 동적 부여/제거
 * 3. 네트워크 동기화 (ClientRPC를 통한 클라이언트 알림)
 * 4. 로컬 UI/카메라 제어 (카메라 전환, 마우스 표시)
 * 
 * 네트워크 구조
 * - Enter: 서버 -> ClientRPC (클라이언트들) + 로컬 직접 실행 (호스트)
 * - Click: 로컬 -> ServerRPC -> 서버에서 처리
 * - Exit:  서버 -> ClientRPC (클라이언트들) + 로컬 직접 실행 (호스트)
 * 
 * - ClientRPC는 호스트에게 전달되지 않으므로
 * - if (LocalController) 체크로 호스트는 로컬에서 직접 실행
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InspectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAO_InspectionComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetupInputBinding(UInputComponent* PlayerInputComponent);

    UFUNCTION(BlueprintCallable, Category = "Inspection")
    void EnterInspectionMode(AActor* InspectableActor);

    UFUNCTION(BlueprintCallable, Category = "Inspection")
    void ExitInspectionMode();

    UFUNCTION(BlueprintPure, Category = "Inspection")
    bool IsInspecting() const { return bIsInspecting; }

    UFUNCTION(BlueprintPure, Category = "Inspection")
    AActor* GetInspectedActor() const { return CurrentInspectedActor; }

	// GA_Inspect_Enter의 Handle 저장 (나중에 Cancel 하기 위해)
    void SetInspectEnterHandle(const FGameplayAbilitySpecHandle& Handle) 
    { 
        InspectEnterAbilityHandle = Handle; 
    }

    UFUNCTION(Server, Reliable)
    void ServerProcessInspectionClick(AActor* TargetActor, FName ComponentName);

    // 외부 클릭 대상 유효성 검사 (GA_Inspect_Click에서 사용)
    bool IsValidExternalClickTarget(AActor* HitActor, UPrimitiveComponent* Component) const;
	// 내부 컴포넌트 검사
	bool IsInternalClickableComponent(UPrimitiveComponent* Component) const;

protected:
    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void OnExitPressed();
    void OnInspectionClick();
    void OnCameraMoveInput(const FInputActionInstance& Instance);
	void OnSpacebarPressed();

	UFUNCTION(Server, Reliable)
	void ServerNotifySpacebarPressed();
    
    void TransitionToInspectionCamera(const FVector& CameraLocation, const FRotator& CameraRotation);
    void TransitionToPlayerCamera();

    void ClientEnterInspection(const FVector& CameraLocation, const FRotator& CameraRotation);
    void ClientExitInspection();

    FVector ClampCameraPosition(const FVector& NewPosition) const;

    UFUNCTION(Client, Reliable)
    void ClientNotifyInspectionStarted(AActor* InspectableActor, FGameplayAbilitySpecHandle AbilityHandle, FAO_InspectionCameraSettings CameraSettings);

    UFUNCTION(Client, Reliable)
    void ClientNotifyInspectionEnded(AActor* InspectableActor);

    UFUNCTION(Server, Reliable)
    void ServerNotifyInspectionEnded();

public:
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> ExitInspectionAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InspectionClickInputAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> CameraMoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SpacebarAction;

    UPROPERTY(EditAnywhere, Category = "Inspection")
    float CameraBlendTime = 0.5f;

	TWeakObjectPtr<UPrimitiveComponent> CachedHoverComponent;
	TWeakObjectPtr<AActor> CachedHoverActor;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> InspectionInputContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	int32 InspectionInputPriority = 10;

	UPROPERTY(EditAnywhere, Category = "Inspection")
	FGameplayTagContainer CancelInspectionTags;

private:
	void StartHoverTrace();
	void StopHoverTrace();
	void PerformHoverTrace();
	void UpdateHoverHighlight(UPrimitiveComponent* NewHoveredComponent);
	void RegisterCancelTags();
	void UnregisterCancelTags();
	void OnCancelTagChanged(const FGameplayTag Tag, int32 NewCount);
	bool IsSpacebarMode() const;

	FTimerHandle HoverTraceTimerHandle;
	TWeakObjectPtr<UPrimitiveComponent> CurrentHoveredComponent;
    
	UPROPERTY(EditAnywhere, Category = "Inspection|Hover")
	float HoverTraceRate = 0.1f;
    
	UPROPERTY(EditAnywhere, Category = "Inspection|Hover")
	float HoverTraceRange = 10000.0f;
	
    UPROPERTY(Replicated)
    TObjectPtr<AActor> CurrentInspectedActor;

    UPROPERTY(Replicated)
    bool bIsInspecting = false;

    UPROPERTY()
    TObjectPtr<ACameraActor> InspectionCameraActor;

    UPROPERTY()
    TObjectPtr<AActor> OriginalViewTarget;

    FGameplayAbilitySpecHandle GrantedClickAbilityHandle;
    FGameplayAbilitySpecHandle InspectEnterAbilityHandle;

    UPROPERTY()
    TArray<TObjectPtr<UPrimitiveComponent>> HiddenComponents;

    // 현재 적용된 카메라 설정 (클라이언트에서 사용)
    FAO_InspectionCameraSettings CurrentCameraSettings;
    
    // 초기 카메라 위치 (클램프 기준점)
    FVector InitialCameraLocation;

	// 태그 변경 감지 핸들
	TArray<FDelegateHandle> CancelTagDelegateHandles;
};