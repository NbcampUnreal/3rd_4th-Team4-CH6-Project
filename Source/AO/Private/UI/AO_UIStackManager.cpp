// JSH: AO_UIStackManager.cpp

#include "UI/AO_UIStackManager.h"

#include "AO_DelegateManager.h"
#include "AO/AO_Log.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Player/PlayerController/AO_PlayerController.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "UI/Widget/AO_UserWidget.h"

void UAO_UIStackManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DelegateManagerCached = GetGameInstance()->GetSubsystem<UAO_DelegateManager>();
	if (DelegateManagerCached)
	{
		DelegateManagerCached->OnSettingsOpen.AddDynamic(this, &UAO_UIStackManager::HandleSettingsOpen);
		DelegateManagerCached->OnSettingsClose.AddDynamic(this, &UAO_UIStackManager::HandleSettingsClose);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("UIStackManager: DelegateManager not found"));
	}
}

void UAO_UIStackManager::Deinitialize()
{
	for (FAOUIStackEntry& E : Stack)
	{
		if (E.Widget)
		{
			E.Widget->RemoveFromParent();
		}
	}
	Stack.Empty();

	BaselineInputByPC.Empty();
	LastAppliedInputByPC.Empty();

	Super::Deinitialize();
}

void UAO_UIStackManager::PushWidgetInstance(APlayerController* PC, UUserWidget* Widget, const FAOUIStackPolicy& Policy)
{
	if (!PC || !Widget)
	{
		return;
	}

	// 오버레이를 처음 Push하는 순간의 "원래 입력 상태"를 베이스라인으로 캐시
	CacheBaselineIfNeeded(PC);

	// 이미 같은 인스턴스가 Top이면 정책만 재적용
	if (Stack.Num() > 0)
	{
		if (Stack.Last().Widget == Widget)
		{
			ApplyTopPolicy(PC);
			return;
		}
	}

	FAOUIStackEntry Entry;
	Entry.Widget = Widget;
	Entry.WidgetClass = Widget->GetClass();
	Entry.Policy = Policy;
	Entry.ZOrder = StackBaseZOrder + Stack.Num();

	Widget->RemoveFromParent();
	Widget->AddToViewport(Entry.ZOrder);
	Widget->SetVisibility(ESlateVisibility::Visible);
	Widget->SetIsFocusable(true);
	Widget->SetKeyboardFocus();

	Stack.Add(Entry);
	ApplyTopPolicy(PC);
}

void UAO_UIStackManager::PopTop(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		ApplyFallbackPolicy(PC);
		return;
	}

	FAOUIStackEntry Top = Stack.Last();
	Stack.Pop();

	if (Top.Widget)
	{
		Top.Widget->RemoveFromParent();
	}

	// Pop 후 남아있으면 Top 정책 적용
	if (Stack.Num() > 0)
	{
		ApplyTopPolicy(PC);

		FAOUIStackEntry& NewTop = Stack.Last();
		if (NewTop.Widget)
		{
			NewTop.Widget->SetIsFocusable(true);
			NewTop.Widget->SetKeyboardFocus();
		}

		return;
	}

	// 스택이 비면: 가능하면 "원래 입력상태"로 복구, 아니면 fallback
	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::PopAll(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	for (FAOUIStackEntry& E : Stack)
	{
		if (E.Widget)
		{
			E.Widget->RemoveFromParent();
		}
	}
	Stack.Empty();

	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::PopByClass(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!PC || !WidgetClass)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		ApplyFallbackPolicy(PC);
		return;
	}

	for (int32 i = Stack.Num() - 1; i >= 0; --i)
	{
		if (Stack[i].WidgetClass == WidgetClass)
		{
			if (Stack[i].Widget)
			{
				Stack[i].Widget->RemoveFromParent();
			}
			Stack.RemoveAt(i);

			break;
		}
	}

	// Top 재적용 / 스택 비면 복구
	if (Stack.Num() > 0)
	{
		ApplyTopPolicy(PC);
		return;
	}

	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::TryTogglePauseMenu(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	if (!CanOpenPauseMenu())
	{
		return;
	}

	AAO_PlayerController_InGameBase* InGamePC = Cast<AAO_PlayerController_InGameBase>(PC);
	if (!InGamePC)
	{
		return;
	}

	TSubclassOf<UAO_PauseMenuWidget> PauseClass = InGamePC->GetPauseMenuWidgetClass();
	if (!PauseClass)
	{
		return;
	}

	// InGameBase 쪽에 실제 생성/캐싱 함수 추가됨
	UAO_PauseMenuWidget* PauseWidget = InGamePC->GetOrCreatePauseMenuWidget();
	if (!PauseWidget)
	{
		return;
	}

	// Top이 Pause면 닫기
	if (Stack.Num() > 0 && Stack.Last().Widget == PauseWidget)
	{
		PopTop(PC);
		return;
	}

	FAOUIStackPolicy Policy;
	Policy.InputMode = EAOUIStackInputMode::UIOnly;
	Policy.bShowMouseCursor = true;
	Policy.MouseLockMode = EMouseLockMode::DoNotLock;
	Policy.bHideCursorDuringCapture = false;
	Policy.InitialFocusWidget = nullptr;

	PushWidgetInstance(PC, PauseWidget, Policy);
}

void UAO_UIStackManager::LockPauseMenu()
{
	++PauseMenuLockCount;
}

void UAO_UIStackManager::UnlockPauseMenu()
{
	PauseMenuLockCount = FMath::Max(0, PauseMenuLockCount - 1);
}

bool UAO_UIStackManager::CanOpenPauseMenu() const
{
	return (PauseMenuLockCount <= 0);
}

void UAO_UIStackManager::ApplyTopPolicy(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		ApplyFallbackPolicy(PC);
		return;
	}

	const FAOUIStackEntry& Top = Stack.Last();
	ApplyPolicyToPC(PC, Top.Policy);
	
	if (Top.Policy.InitialFocusWidget.IsValid())
	{
		UWidget* FocusWidget = Top.Policy.InitialFocusWidget.Get();
		if (FocusWidget)
		{
			FEventReply Reply;
			Reply = UWidgetBlueprintLibrary::SetUserFocus(Reply, FocusWidget, true);
		}
	}
}

void UAO_UIStackManager::ApplyFallbackPolicy(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	const bool bRestored = RestoreBaselineInput(PC);
	if (!bRestored)
	{
		// 최후 fallback: 인게임 안전 기본값
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// 여기로 내려서 포커스를 게임 뷰포트로 복구
	UWidgetBlueprintLibrary::SetFocusToGameViewport();

	// 간헐적으로 키 입력이 씹히는 케이스 방지
	PC->FlushPressedKeys();
	PC->SetIgnoreLookInput(false);
	PC->SetIgnoreMoveInput(false);
}

void UAO_UIStackManager::ApplyPolicyToPC(APlayerController* PC, const FAOUIStackPolicy& Policy)
{
	if (!PC)
	{
		return;
	}

	switch (Policy.InputMode)
	{
		case EAOUIStackInputMode::GameOnly:
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			break;
		}
		case EAOUIStackInputMode::UIOnly:
		{
			FInputModeUIOnly InputMode;
			InputMode.SetLockMouseToViewportBehavior(Policy.MouseLockMode);
			PC->SetInputMode(InputMode);
			break;
		}
		case EAOUIStackInputMode::GameAndUI:
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(Policy.MouseLockMode);
			InputMode.SetHideCursorDuringCapture(Policy.bHideCursorDuringCapture);
			PC->SetInputMode(InputMode);
			break;
		}
		default:
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			break;
		}
	}

	PC->bShowMouseCursor = Policy.bShowMouseCursor;

	FAOUIInputSnapshot Snapshot;
	Snapshot.InputMode = Policy.InputMode;
	Snapshot.bShowMouseCursor = Policy.bShowMouseCursor;
	Snapshot.MouseLockMode = Policy.MouseLockMode;
	Snapshot.bHideCursorDuringCapture = Policy.bHideCursorDuringCapture;

	LastAppliedInputByPC.Add(PC, Snapshot);
}

FAOUIInputSnapshot UAO_UIStackManager::InferCurrentSnapshot(APlayerController* PC) const
{
	FAOUIInputSnapshot Snapshot;
	if (!PC)
	{
		return Snapshot;
	}

	// "현재 입력모드"를 직접 얻기 어렵기 때문에, 최소한 커서 표시 여부로 추정합니다.
	// - 커서가 켜져 있으면 UIOnly로 보는 편이, 메인메뉴/설정 UX에 안전합니다.
	// - 커서가 꺼져 있으면 GameOnly로 보는 편이, 인게임 UX에 안전합니다.
	Snapshot.bShowMouseCursor = PC->bShowMouseCursor;
	Snapshot.InputMode = Snapshot.bShowMouseCursor ? EAOUIStackInputMode::UIOnly : EAOUIStackInputMode::GameOnly;
	Snapshot.MouseLockMode = EMouseLockMode::DoNotLock;
	Snapshot.bHideCursorDuringCapture = false;

	return Snapshot;
}

void UAO_UIStackManager::CacheBaselineIfNeeded(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	// 이미 베이스라인이 있으면 그대로 사용
	if (BaselineInputByPC.Contains(PC))
	{
		return;
	}

	// UIStack이 적용한 값이 있으면 그 직전 상태는 알 수 없으니, 현재 상태 추정으로 기록
	FAOUIInputSnapshot Baseline = InferCurrentSnapshot(PC);
	BaselineInputByPC.Add(PC, Baseline);
}

bool UAO_UIStackManager::RestoreBaselineInput(APlayerController* PC)
{
	if (!PC)
	{
		return false;
	}

	const FAOUIInputSnapshot* Baseline = BaselineInputByPC.Find(PC);
	if (!Baseline)
	{
		return false;
	}

	// 베이스라인을 입력모드에 반영
	switch (Baseline->InputMode)
	{
		case EAOUIStackInputMode::GameOnly:
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			break;
		}
		case EAOUIStackInputMode::UIOnly:
		{
			FInputModeUIOnly InputMode;
			InputMode.SetLockMouseToViewportBehavior(Baseline->MouseLockMode);
			PC->SetInputMode(InputMode);
			break;
		}
		case EAOUIStackInputMode::GameAndUI:
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(Baseline->MouseLockMode);
			InputMode.SetHideCursorDuringCapture(Baseline->bHideCursorDuringCapture);
			PC->SetInputMode(InputMode);
			break;
		}
		default:
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			break;
		}
	}

	PC->bShowMouseCursor = Baseline->bShowMouseCursor;

	// 스택이 완전히 비었으면 베이스라인은 그대로 유지 (다음 오버레이에도 "원래 상태" 복구)
	return true;
}

void UAO_UIStackManager::HandleSettingsOpen()
{
	AAO_PlayerController* PC = Cast<AAO_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!PC)
	{
		return;
	}

	UAO_UserWidget* Settings = PC->GetOrCreateSettingsWidgetInstance();
	if (!Settings)
	{
		return;
	}

	FAOUIStackPolicy Policy;
	Policy.InputMode = EAOUIStackInputMode::UIOnly;
	Policy.bShowMouseCursor = true;
	Policy.MouseLockMode = EMouseLockMode::DoNotLock;
	Policy.bHideCursorDuringCapture = false;
	Policy.InitialFocusWidget = nullptr;

	PushWidgetInstance(PC, Settings, Policy);
}

void UAO_UIStackManager::HandleSettingsClose()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}
	
	PopTop(PC);
}
