// JSH: AO_UIStackManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/EngineTypes.h"
#include "AO_UIStackManager.generated.h"

class UAO_DelegateManager;
class UAO_UserWidget;
class AAO_PlayerController;
class AAO_PlayerController_InGameBase;
class UAO_PauseMenuWidget;
class APlayerController;

UENUM(BlueprintType)
enum class EAOUIStackInputMode : uint8
{
	UIOnly,
	GameAndUI,
	GameOnly
};

USTRUCT(BlueprintType)
struct FAOUIStackPolicy
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAOUIStackInputMode InputMode = EAOUIStackInputMode::GameAndUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMouseCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMouseLockMode MouseLockMode = EMouseLockMode::DoNotLock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideCursorDuringCapture = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UWidget> InitialFocusWidget = nullptr;
};

USTRUCT()
struct FAOUIStackEntry
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	TSubclassOf<UUserWidget> WidgetClass = nullptr;

	UPROPERTY()
	FAOUIStackPolicy Policy;

	UPROPERTY()
	int32 ZOrder = 0;
};

// 입력모드 복구용 스냅샷
USTRUCT()
struct FAOUIInputSnapshot
{
	GENERATED_BODY()

public:
	EAOUIStackInputMode InputMode = EAOUIStackInputMode::GameOnly;
	bool bShowMouseCursor = false;
	EMouseLockMode MouseLockMode = EMouseLockMode::DoNotLock;
	bool bHideCursorDuringCapture = false;
};

UCLASS()
class AO_API UAO_UIStackManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// 인스턴스 Push (PauseMenu처럼 PC에서 생성/캐싱하는 위젯도 지원)
	void PushWidgetInstance(APlayerController* PC, UUserWidget* Widget, const FAOUIStackPolicy& Policy);

	// Top Pop
	void PopTop(APlayerController* PC);

	// 전체 Pop
	void PopAll(APlayerController* PC);

	// 특정 Class Pop
	void PopByClass(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass);

	// Pause 토글 (ESC 단일 경로)
	void TryTogglePauseMenu(APlayerController* PC);

public:
	// 메뉴 락
	UFUNCTION(BlueprintCallable, Category = "AO|UIStack|Pause")
	void LockPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "AO|UIStack|Pause")
	void UnlockPauseMenu();

	UFUNCTION(BlueprintPure, Category = "AO|UIStack|Pause")
	bool CanOpenPauseMenu() const;

private:
	APlayerController* GetLocalPlayerController() const;

	int32 FindIndexByClass(TSubclassOf<UUserWidget> WidgetClass) const;
	void ApplyTopPolicy(APlayerController* PC);
	void ApplyFallbackPolicy(APlayerController* PC);

	// --- 입력 정책 적용 / 스냅샷 ---
	void ApplyPolicyToPC(APlayerController* PC, const FAOUIStackPolicy& Policy);
	FAOUIInputSnapshot InferCurrentSnapshot(APlayerController* PC) const;
	void CacheBaselineIfNeeded(APlayerController* PC);
	bool RestoreBaselineInput(APlayerController* PC);

private:
	UPROPERTY()
	TObjectPtr<UAO_DelegateManager> DelegateManagerCached = nullptr;

	UPROPERTY()
	TArray<FAOUIStackEntry> Stack;

private:
	// ZOrder 정책
	static constexpr int32 StackBaseZOrder = 100;

private:
	// PC별 "UIStack 사용 이전 입력 상태"
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, FAOUIInputSnapshot> BaselineInputByPC;

	// PC별 "마지막으로 UIStack이 적용한 입력 상태"
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, FAOUIInputSnapshot> LastAppliedInputByPC;

private:
	UPROPERTY()
	int32 PauseMenuLockCount = 0;

private:
	// Delegate 핸들러
	UFUNCTION()
	void HandleSettingsOpen();

	UFUNCTION()
	void HandleSettingsClose();
};
