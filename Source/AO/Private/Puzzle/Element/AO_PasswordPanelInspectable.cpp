// HSJ : AO_PasswordPanelInspectable.cpp
#include "Puzzle/Element/AO_PasswordPanelInspectable.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"

AAO_PasswordPanelInspectable::AAO_PasswordPanelInspectable()
{
	InspectionTitle = FText::FromString("Password Panel");
	InspectionContent = FText::FromString(": Press F to Inspect");
}

void AAO_PasswordPanelInspectable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(AAO_PasswordPanelInspectable, CurrentPassword);
}

void AAO_PasswordPanelInspectable::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
	if (!ClickedComponent || !HasAuthority())
	{
		return;
	}
	
	MulticastNotifyButtonClicked(ClickedComponent);
}

void AAO_PasswordPanelInspectable::MulticastNotifyButtonClicked_Implementation(UPrimitiveComponent* ButtonComponent)
{
	OnButtonClickedEvent(ButtonComponent);
}

void AAO_PasswordPanelInspectable::NotifyPasswordCorrect()
{
	if (!HasAuthority())
	{
		return;
	}
    
	OnPasswordCorrectEvent.Broadcast();
}

void AAO_PasswordPanelInspectable::GenerateRandomPassword()
{
	if (!HasAuthority())
	{
		return;
	}

	// 0000 ~ 9999 랜덤 생성
	CurrentPassword = FMath::RandRange(0, 9999);
}

void AAO_PasswordPanelInspectable::GetPasswordDigits(int32& Digit1, int32& Digit2, int32& Digit3, int32& Digit4) const
{
	// 4자리 숫자를 각 자릿수로 분해
	Digit1 = (CurrentPassword / 1000) % 10;
	Digit2 = (CurrentPassword / 100) % 10;
	Digit3 = (CurrentPassword / 10) % 10;
	Digit4 = CurrentPassword % 10;
}