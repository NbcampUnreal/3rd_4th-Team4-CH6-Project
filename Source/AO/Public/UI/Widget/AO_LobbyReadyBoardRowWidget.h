#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "AO_LobbyReadyBoardRowWidget.generated.h"

class UTextBlock;

UCLASS()
class AO_API UAO_LobbyReadyBoardRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 한 줄(플레이어) 정보 세팅
	void Setup(const FAOLobbyReadyBoardEntry& Entry);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_PlayerName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Status;
};
