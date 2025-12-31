// AO_NameTagWidget.cpp

#include "UI/Player/AO_NameTagWidget.h"

#include "Components/TextBlock.h"

void UAO_NameTagWidget::SetPlayerName(const FText& InName)
{
	if (Text_Name)
	{
		Text_Name->SetText(InName);
	}
}

void UAO_NameTagWidget::SetUIScale(float InScale)
{
	SetRenderScale(FVector2D(InScale, InScale));
}
