// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_HostDialogWidget.h"

#include "Online/AO_OnlineSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AO/AO_Log.h"

namespace
{
	static constexpr int32 kRoomNameMaxLen = 10;
}

void UAO_HostDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(Btn_Create && !Btn_Create->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Create))
	{
		Btn_Create->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Create);
	}
	if(Btn_Cancel && !Btn_Cancel->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Cancel))
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Cancel);
	}
	if(Btn_Backdrop && !Btn_Backdrop->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Backdrop))
	{
		Btn_Backdrop->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Backdrop);
	}

	if(Txt_RoomName)
	{
		Txt_RoomName->OnTextChanged.AddDynamic(this, &ThisClass::OnRoomNameChanged);
		Txt_RoomName->OnTextCommitted.AddDynamic(this, &ThisClass::OnRoomNameCommitted);
	}

	ApplyModalInput();
	UpdateCreateButtonState();

	AO_LOG(LogJSH, Log, TEXT("HostDialog shown (mouse-only modal)"));
}

void UAO_HostDialogWidget::ApplyModalInput()
{
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if(Txt_RoomName)
		{
			Mode.SetWidgetToFocus(Txt_RoomName->TakeWidget());
		}
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}

	if(Txt_RoomName)
	{
		Txt_RoomName->SetKeyboardFocus();
	}
}

void UAO_HostDialogWidget::OnClicked_Create()
{
	const FString RoomName = GetTrimmedRoomName();
	const FString Password = GetTrimmedPassword();

	if(!IsValidRoomName(RoomName))
	{
		AO_LOG(LogJSH, Warning, TEXT("Create aborted: invalid room name"));
		if(Txt_RoomName)
		{
			Txt_RoomName->SetError(FText::FromString(TEXT("방 이름을 입력하세요 (1~10자)")));
			Txt_RoomName->SetKeyboardFocus();
		}
		UpdateCreateButtonState();
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("Create clicked: Room='%s' PwLen=%d"), *RoomName, Password.Len());

	if(UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			Sub->HostSessionAuto(4, RoomName, Password);
		}
	}

	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Cancel()
{
	AO_LOG(LogJSH, Log, TEXT("Cancel clicked"));
	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Backdrop()
{
	OnClicked_Cancel();
}

void UAO_HostDialogWidget::OnRoomNameChanged(const FText&)
{
	if(Txt_RoomName)
	{
		Txt_RoomName->SetError(FText::GetEmpty());
	}
	UpdateCreateButtonState();
}

void UAO_HostDialogWidget::OnRoomNameCommitted(const FText& , ETextCommit::Type Method)
{
	if(Method == ETextCommit::OnEnter)
	{
		OnClicked_Create();
		return;
	}
	UpdateCreateButtonState();
}

void UAO_HostDialogWidget::UpdateCreateButtonState()
{
	if(!Btn_Create)
	{
		return;
	}

	const FString Trimmed = GetTrimmedRoomName();
	const bool bEnable = IsValidRoomName(Trimmed);
	Btn_Create->SetIsEnabled(bEnable);
}

bool UAO_HostDialogWidget::IsValidRoomName(const FString& Name) const
{
	if(Name.IsEmpty())
	{
		return false;
	}

	if(Name.Len() > kRoomNameMaxLen)
	{
		return false;
	}

	for(const TCHAR C : Name)
	{
		if(C < 0x20)
		{
			return false;
		}
	}

	/* 필요시 금지문자 추가 (예: 줄바꿈, 탭 외 특수문자 등)
	   if(Name.Contains(TEXT("|")) || Name.Contains(TEXT("\\"))) { return false; } */

	return true;
}

FString UAO_HostDialogWidget::GetTrimmedRoomName() const
{
	if(!Txt_RoomName)
	{
		return TEXT("");
	}
	return Txt_RoomName->GetText().ToString().TrimStartAndEnd();
}

FString UAO_HostDialogWidget::GetTrimmedPassword() const
{
	if(!Txt_Password)
	{
		return TEXT("");
	}
	/* 비밀번호는 길이 제한 */
	FString Trimmed = Txt_Password->GetText().ToString().TrimStartAndEnd();
	if(Trimmed.Len() > 4)
	{
		Trimmed.LeftInline(4, false);
	}
	return Trimmed;
}
