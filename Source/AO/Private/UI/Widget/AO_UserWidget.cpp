// AO_UserWidget.cpp (장주만)

#include "UI/Widget/AO_UserWidget.h"

#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "UI/AO_UIStackManager.h"
#include "UI/AO_UIActionKeySubsystem.h"

FReply UAO_UserWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Super 먼저 호출하면, 자식이 이미 처리해버려서 여기까지 안 올 때가 있어
	// 그래서 공통 “닫기 키”는 먼저 처리하는 쪽이 안정적임
	if (bCloseOnEscape)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UGameInstance* GI = PC->GetGameInstance())
			{
				if (UAO_UIActionKeySubsystem* Keys = GI->GetSubsystem<UAO_UIActionKeySubsystem>())
				{
					if (Keys->IsUICloseKey(InKeyEvent.GetKey()))
					{
						RequestCloseByEscape();

						if (bConsumeEscape)
						{
							return FReply::Handled();
						}
					}
				}
			}
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UAO_UserWidget::RequestCloseByEscape()
{
	// BP에서 추가 처리가 필요하면 이벤트로도 받을 수 있게 열어둠
	OnEscapeCloseRequested();
}