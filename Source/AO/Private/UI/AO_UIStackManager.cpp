// JSH: AO_UIStackManager.cpp

#include "UI/AO_UIStackManager.h"

#include "AO_DelegateManager.h"
#include "AO/AO_Log.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Player/PlayerController/AO_PlayerController.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "UI/Widget/AO_UserWidget.h"

void UAO_UIStackManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DelegateManagerCached = GetGameInstance()->GetSubsystem<UAO_DelegateManager>();
	if (DelegateManagerCached != nullptr)
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
	for (FAOUIStackEntry& Entry : Stack)
	{
		if (Entry.Widget != nullptr)
		{
			Entry.Widget->RemoveFromParent();
		}
	}
	Stack.Empty();

	BaselineInputByPC.Empty();
	LastAppliedInputByPC.Empty();

	Super::Deinitialize();
}

void UAO_UIStackManager::PushWidgetInstance(APlayerController* PC, UUserWidget* Widget, const FAOUIStackPolicy& Policy)
{
	if (PC == nullptr)
	{
		return;
	}

	if (Widget == nullptr)
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
			Widget->SetVisibility(ESlateVisibility::Visible);
			Widget->SetIsFocusable(true);
			Widget->SetKeyboardFocus();

			ApplyTopPolicy(PC);
			return;
		}
	}

	FAOUIStackEntry Entry;
	Entry.Widget = Widget;
	Entry.WidgetClass = Widget->GetClass();
	Entry.Policy = Policy;
	Entry.ZOrder = StackBaseZOrder + Stack.Num();

	// 이미 뷰포트에 붙어있으면 다시 AddToViewport 할 필요 없음
	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(Entry.ZOrder);
	}

	Widget->SetVisibility(ESlateVisibility::Visible);
	Widget->SetIsFocusable(true);
	Widget->SetKeyboardFocus();

	Stack.Add(Entry);
	ApplyTopPolicy(PC);
}

void UAO_UIStackManager::PopTop(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		ApplyFallbackPolicy(PC);
		return;
	}

	FAOUIStackEntry TopEntry = Stack.Last();
	Stack.Pop();

	if (TopEntry.Widget != nullptr)
	{
		TopEntry.Widget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Stack.Num() > 0)
	{
		ApplyTopPolicy(PC);
		return;
	}

	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::PopAll(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	for (FAOUIStackEntry& Entry : Stack)
	{
		if (Entry.Widget != nullptr)
		{
			Entry.Widget->RemoveFromParent();
		}
	}
	Stack.Empty();

	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::PopByClass(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass)
{
	if (PC == nullptr)
	{
		return;
	}

	if (!WidgetClass)
	{
		return;
	}

	for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
	{
		if (Stack[Index].WidgetClass == WidgetClass)
		{
			if (Stack[Index].Widget != nullptr)
			{
				Stack[Index].Widget->RemoveFromParent();
			}

			Stack.RemoveAt(Index);
			break;
		}
	}

	if (Stack.Num() > 0)
	{
		ApplyTopPolicy(PC);
		return;
	}

	ApplyFallbackPolicy(PC);
}

void UAO_UIStackManager::PopIfTopIsClass(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass)
{
	if (PC == nullptr)
	{
		return;
	}

	if (!WidgetClass)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		return;
	}

	const FAOUIStackEntry& TopEntry = Stack.Last();
	if (TopEntry.WidgetClass != WidgetClass)
	{
		return;
	}

	PopTop(PC);
}

UAO_UserWidget* UAO_UIStackManager::GetTopUserWidget() const
{
	if (Stack.Num() <= 0)
	{
		return nullptr;
	}

	if (UAO_UserWidget* UserWidget = Cast<UAO_UserWidget>(Stack.Last().Widget))
	{
		return UserWidget;
	}

	return nullptr;
}

bool UAO_UIStackManager::IsTopPauseMenu() const
{
	if (Stack.Num() <= 0)
	{
		return false;
	}

	if (Stack.Last().Widget == nullptr)
	{
		return false;
	}

	if (Stack.Last().Widget->IsA<UAO_PauseMenuWidget>())
	{
		return true;
	}

	return false;
}

bool UAO_UIStackManager::CanOpenPauseMenu() const
{
	return (PauseMenuLockCount <= 0);
}

void UAO_UIStackManager::LockPauseMenu()
{
	++PauseMenuLockCount;
}

void UAO_UIStackManager::UnlockPauseMenu()
{
	PauseMenuLockCount = FMath::Max(0, PauseMenuLockCount - 1);
}

void UAO_UIStackManager::TryTogglePauseMenu(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	if (!CanOpenPauseMenu())
	{
		return;
	}

	AAO_PlayerController_InGameBase* InGamePC = Cast<AAO_PlayerController_InGameBase>(PC);
	if (InGamePC == nullptr)
	{
		return;
	}

	TSubclassOf<UAO_PauseMenuWidget> PauseClass = InGamePC->GetPauseMenuWidgetClass();
	if (!PauseClass)
	{
		return;
	}

	UAO_PauseMenuWidget* PauseWidget = InGamePC->GetOrCreatePauseMenuWidget();
	if (PauseWidget == nullptr)
	{
		return;
	}

	// 1) 다른 UI가 Top에 떠 있는 경우: Top만 닫기 (설정창 등)
	if (Stack.Num() > 0 && Stack.Last().Widget != PauseWidget)
	{
		PopTop(PC);
		return;
	}

	// 2) Top이 Pause 인 경우: Pause 닫기
	if (Stack.Num() > 0 && Stack.Last().Widget == PauseWidget)
	{
		PopTop(PC);
		return;
	}

	// 3) 아무 UI도 없으면 Pause 열기
	FAOUIStackPolicy Policy;
	Policy.InputMode = EAOUIStackInputMode::UIOnly;
	Policy.bShowMouseCursor = true;
	Policy.MouseLockMode = EMouseLockMode::DoNotLock;
	Policy.bHideCursorDuringCapture = false;
	Policy.InitialFocusWidget = nullptr;

	PushWidgetInstance(PC, PauseWidget, Policy);
}

void UAO_UIStackManager::ApplyTopPolicy(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	if (Stack.Num() <= 0)
	{
		ApplyFallbackPolicy(PC);
		return;
	}

	const FAOUIStackEntry& TopEntry = Stack.Last();
	ApplyPolicyToPC(PC, TopEntry.Policy);

	if (TopEntry.Policy.InitialFocusWidget.IsValid())
	{
		TopEntry.Policy.InitialFocusWidget->SetKeyboardFocus();
	}
}

void UAO_UIStackManager::ApplyFallbackPolicy(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	bool bIsDead = false;

	if (TObjectPtr<AAO_PlayerState> AO_PS = Cast<AAO_PlayerState>(PC->PlayerState))
	{
		if (AO_PS->bIsAlive == false)
		{
			bIsDead = true;
		}
	}

	// 사망 상태: 사실상 UIOnly처럼 보이도록 GameAndUI + 이동/룩 입력 무시
	if (bIsDead)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);

		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		// 캐릭터 조작은 막기
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
		//TODO: 앉기 점프 등 확인 요망

		UWidgetBlueprintLibrary::SetFocusToGameViewport();
		PC->FlushPressedKeys();
		return;
	}

	// 생존 상태: 원래 입력 스냅샷으로 복구 시도
	const bool bRestored = RestoreBaselineInput(PC);

	if (!bRestored)
	{
		// 최후 fallback: 인게임 안전 기본값
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	UWidgetBlueprintLibrary::SetFocusToGameViewport();

	PC->FlushPressedKeys();
	PC->SetIgnoreLookInput(false);
	PC->SetIgnoreMoveInput(false);
}

void UAO_UIStackManager::ApplyPolicyToPC(APlayerController* PC, const FAOUIStackPolicy& Policy)
{
	if (PC == nullptr)
	{
		return;
	}

	LastAppliedInputByPC.FindOrAdd(PC) = FAOUIInputSnapshot(PC);

	switch (Policy.InputMode)
	{
	case EAOUIStackInputMode::GameOnly:
	{
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(Policy.bHideCursorDuringCapture);
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
		break;
	}
	}

	PC->bShowMouseCursor = Policy.bShowMouseCursor;

	if (Policy.InitialFocusWidget.IsValid())
	{
		Policy.InitialFocusWidget->SetKeyboardFocus();
	}
}

FAOUIInputSnapshot UAO_UIStackManager::InferCurrentSnapshot(APlayerController* PC) const
{
	FAOUIInputSnapshot Snapshot;

	if (PC == nullptr)
	{
		return Snapshot;
	}

	Snapshot.bShowMouseCursor = PC->bShowMouseCursor;
	Snapshot.bEnableClickEvents = PC->bEnableClickEvents;
	Snapshot.bEnableMouseOverEvents = PC->bEnableMouseOverEvents;
	Snapshot.bIgnoreLookInput = PC->IsLookInputIgnored();
	Snapshot.bIgnoreMoveInput = PC->IsMoveInputIgnored();

	return Snapshot;
}

void UAO_UIStackManager::CacheBaselineIfNeeded(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	if (BaselineInputByPC.Contains(PC))
	{
		return;
	}

	FAOUIInputSnapshot Baseline = InferCurrentSnapshot(PC);
	BaselineInputByPC.Add(PC, Baseline);
}

bool UAO_UIStackManager::RestoreBaselineInput(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return false;
	}

	FAOUIInputSnapshot* BaselinePtr = BaselineInputByPC.Find(PC);
	if (BaselinePtr == nullptr)
	{
		return false;
	}

	const FAOUIInputSnapshot& Baseline = *BaselinePtr;

	if (Baseline.bShowMouseCursor)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
	}
	else
	{
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
	}

	PC->bShowMouseCursor = Baseline.bShowMouseCursor;
	PC->bEnableClickEvents = Baseline.bEnableClickEvents;
	PC->bEnableMouseOverEvents = Baseline.bEnableMouseOverEvents;
	PC->SetIgnoreLookInput(Baseline.bIgnoreLookInput);
	PC->SetIgnoreMoveInput(Baseline.bIgnoreMoveInput);

	return true;
}

APlayerController* UAO_UIStackManager::GetLocalPlayerController() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
		{
			return LP->GetPlayerController(GetWorld());
		}
	}

	return nullptr;
}

int32 UAO_UIStackManager::FindIndexByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (int32 Index = 0; Index < Stack.Num(); ++Index)
	{
		if (Stack[Index].WidgetClass == WidgetClass)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void UAO_UIStackManager::HandleSettingsOpen()
{
	AAO_PlayerController* PC = Cast<AAO_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC == nullptr)
	{
		return;
	}

	UAO_UserWidget* Settings = PC->GetOrCreateSettingsWidgetInstance();
	if (Settings == nullptr)
	{
		return;
	}

	FAOUIStackPolicy Policy;
	Policy.InputMode = EAOUIStackInputMode::GameAndUI;
	Policy.bShowMouseCursor = true;
	Policy.MouseLockMode = EMouseLockMode::DoNotLock;
	Policy.bHideCursorDuringCapture = false;
	Policy.InitialFocusWidget = nullptr;

	PushWidgetInstance(PC, Settings, Policy);
	
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
}

void UAO_UIStackManager::HandleSettingsClose()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC == nullptr)
	{
		return;
	}
	
	// Top(Settings)을 Pop
	PopTop(PC);

	// 스택에 아직 다른 UI가 남아 있다면 위젯에 다시 포커스를 줘서 키 입력을 받을 수 있게 함
	if (Stack.Num() > 0)
	{
		const FAOUIStackEntry& TopEntry = Stack.Last();
		if (TopEntry.Widget != nullptr)
		{
			TopEntry.Widget->SetIsFocusable(true);
			TopEntry.Widget->SetKeyboardFocus();
		}
	}
}
