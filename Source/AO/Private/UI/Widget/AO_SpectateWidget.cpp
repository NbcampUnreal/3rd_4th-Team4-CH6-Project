// AO_SpectateWidget.cpp

#include "UI/Widget/AO_SpectateWidget.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "UI/HUD/AO_HealthWidget.h"
#include "UI/HUD/AO_StaminaWidget.h"

void UAO_SpectateWidget::SetObservedCharacter(ACharacter* InCharacter)
{
	checkf(InCharacter, TEXT("InCharacter is nullptr"));
	ObservedCharacter = InCharacter;

	// 관전하는 플레이어 이름 설정
	FText SpectatingPlayerName = FText::GetEmpty();
	if (TObjectPtr<APlayerState> PS = ObservedCharacter->GetPlayerState())
	{
		SpectatingPlayerName = FText::FromString(PS->GetPlayerName());
	}

	SetSpectatingPlayerName(SpectatingPlayerName);

	// 체력 위젯 바인딩
	if (HealthWidget)
	{
		HealthWidget->BindToCharacter(InCharacter);
	}
	
	// 스태미나 위젯 바인딩
	if (StaminaWidget)
	{
		StaminaWidget->BindToCharacter(InCharacter);
	}
}
