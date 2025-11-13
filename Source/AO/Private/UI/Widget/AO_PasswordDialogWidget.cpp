// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_PasswordDialogWidget.h"

#include "UI/Widget/AO_LobbyListWidget.h"
#include "Online/AO_OnlineSessionSubsystem.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "AO/AO_Log.h"

void UAO_PasswordDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(Btn_Ok && !Btn_Ok->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Ok))
	{
		Btn_Ok->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Ok);
	}
	else if(!Btn_Ok)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Ok is null"));
	}

	if(Btn_Back && !Btn_Back->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Back))
	{
		Btn_Back->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Back);
	}
	else if(!Btn_Back)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Back is null"));
	}

	// 팝업 포커스/입력 준비
	SetIsFocusable(true);
	if(Txt_Password)
	{
		Txt_Password->SetIsReadOnly(false);
		Txt_Password->SetKeyboardFocus();
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Txt_Password is null"));
	}
}

void UAO_PasswordDialogWidget::OnClicked_Ok()
{
	if(TargetIndex < 0)
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: TargetIndex invalid (%d)"), TargetIndex);
		return;
	}

	const FString Pw = Txt_Password ? Txt_Password->GetText().ToString() : TEXT("");
	if(!Txt_Password)
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: Txt_Password is null, using empty password"));
	}

	if(UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			// 로컬 해시 기반 사전 검증
			if(Sub->VerifyPasswordAgainstIndex(TargetIndex, Pw))
			{
				AO_LOG(LogJSH, Log, TEXT("OnClicked_Ok: Password verified, joining session (Index=%d)"), TargetIndex);
				Sub->JoinSessionByIndex(TargetIndex);
				RemoveFromParent(); // 맵 이동/로딩으로 전환
				return;
			}

			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: Password verification failed (Index=%d)"), TargetIndex);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: OnlineSessionSubsystem is null"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: GameInstance is null"));
	}

	// 실패 안내(필요 시 TextBlock 바인딩으로 UI 피드백 확장 가능)
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AO_LOG(LogJSH, Log, TEXT("OnClicked_Ok: Password invalid, showing client message"));
		PC->ClientMessage(TEXT("비밀번호가 올바르지 않습니다."));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Ok: PlayerController is null, cannot show client message"));
	}
}

void UAO_PasswordDialogWidget::OnClicked_Back()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Back: Closing password dialog"));
	RemoveFromParent();

	// 리스트 화면 다시 보이기 + 포커스 복구(리스트는 팝업 아래에 존재)
	if(ParentList.IsValid())
	{
		ParentList->SetVisibility(ESlateVisibility::Visible);

		if(APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(ParentList->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;

			AO_LOG(LogJSH, Log, TEXT("OnClicked_Back: Restored focus to ParentList"));
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Back: PlayerController is null, cannot restore focus"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Back: ParentList invalid, cannot restore list visibility/focus"));
	}
}
