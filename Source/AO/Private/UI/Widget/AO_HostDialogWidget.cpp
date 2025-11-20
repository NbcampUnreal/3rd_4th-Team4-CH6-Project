// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_HostDialogWidget.h"

#include "Online/AO_OnlineSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Containers/UnrealString.h"
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
	else if(!Btn_Create)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Create is null"));
	}

	if(Btn_Cancel && !Btn_Cancel->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Cancel))
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Cancel);
	}
	else if(!Btn_Cancel)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Cancel is null"));
	}

	if(Btn_Backdrop && !Btn_Backdrop->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Backdrop))
	{
		Btn_Backdrop->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Backdrop);
	}
	else if(!Btn_Backdrop)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Backdrop is null"));
	}

	if(Txt_RoomName)
	{
		Txt_RoomName->OnTextChanged.AddDynamic(this, &ThisClass::OnRoomNameChanged);
		Txt_RoomName->OnTextCommitted.AddDynamic(this, &ThisClass::OnRoomNameCommitted);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Txt_RoomName is null"));
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

		AO_LOG(LogJSH, Log, TEXT("ApplyModalInput: UIOnly input mode set"));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("ApplyModalInput: PlayerController is null"));
	}

	if(Txt_RoomName)
	{
		Txt_RoomName->SetKeyboardFocus();
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("ApplyModalInput: Txt_RoomName is null, cannot set keyboard focus"));
	}
}

void UAO_HostDialogWidget::OnClicked_Create()
{
	const FString RoomName = GetTrimmedRoomName();
	const FString Password = GetTrimmedPassword();

	if(!IsValidRoomName(RoomName))
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Create: Create aborted, invalid room name (\"%s\")"), *RoomName);
		if(Txt_RoomName)
		{
			Txt_RoomName->SetError(FText::FromString(TEXT("방 이름을 입력하세요 (1~10자)")));
			Txt_RoomName->SetKeyboardFocus();
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Create: Txt_RoomName is null, cannot show error"));
		}
		UpdateCreateButtonState();
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("OnClicked_Create: Room='%s' PwLen=%d"), *RoomName, Password.Len());

	if(UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			AO_LOG(LogJSH, Log, TEXT("OnClicked_Create: Calling HostSessionAuto"));
			Sub->HostSessionAuto(4, RoomName, Password);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Create: OnlineSessionSubsystem is null"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Create: GameInstance is null"));
	}

	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Cancel()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Cancel: Cancel clicked"));
	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Backdrop()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Backdrop: Backdrop clicked, canceling"));
	OnClicked_Cancel();
}

void UAO_HostDialogWidget::OnRoomNameChanged(const FText&)
{
	if(Txt_RoomName)
	{
		Txt_RoomName->SetError(FText::GetEmpty());
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnRoomNameChanged: Txt_RoomName is null"));
	}
	UpdateCreateButtonState();
}

void UAO_HostDialogWidget::OnRoomNameCommitted(const FText& Text, ETextCommit::Type Method)
{
	if(Method == ETextCommit::OnEnter)
	{
		AO_LOG(LogJSH, Log, TEXT("OnRoomNameCommitted: OnEnter (Text=\"%s\")"), *Text.ToString());
		OnClicked_Create();
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("OnRoomNameCommitted: CommitMethod=%d (Text=\"%s\")"),
		static_cast<int32>(Method),
		*Text.ToString());

	UpdateCreateButtonState();
}

void UAO_HostDialogWidget::UpdateCreateButtonState()
{
	if(!Btn_Create)
	{
		AO_LOG(LogJSH, Warning, TEXT("UpdateCreateButtonState: Btn_Create is null"));
		return;
	}

	const FString Trimmed = GetTrimmedRoomName();
	const bool bEnable = IsValidRoomName(Trimmed);
	Btn_Create->SetIsEnabled(bEnable);

	AO_LOG(LogJSH, Log, TEXT("UpdateCreateButtonState: Name=\"%s\" Enable=%d"),
		*Trimmed,
		static_cast<int32>(bEnable));
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
		AO_LOG(LogJSH, Warning, TEXT("GetTrimmedRoomName: Txt_RoomName is null, returning empty string"));
		return TEXT("");
	}
	return Txt_RoomName->GetText().ToString().TrimStartAndEnd();
}

FString UAO_HostDialogWidget::GetTrimmedPassword() const
{
	if(!Txt_Password)
	{
		AO_LOG(LogJSH, Warning, TEXT("GetTrimmedPassword: Txt_Password is null, returning empty string"));
		return TEXT("");
	}

	/* 비밀번호는 길이 제한 */
	FString Trimmed = Txt_Password->GetText().ToString().TrimStartAndEnd();
	if(Trimmed.Len() > 4)
	{
		Trimmed.LeftInline(4, EAllowShrinking::No);
	}
	return Trimmed;
}
