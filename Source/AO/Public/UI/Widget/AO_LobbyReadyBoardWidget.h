#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "AO_LobbyReadyBoardWidget.generated.h"

class UVerticalBox;
class UAO_LobbyReadyBoardRowWidget;

UCLASS()
class AO_API UAO_LobbyReadyBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 전체 엔트리(로비 인원 전체)를 받아서 리스트 갱신
	void SetEntries(const TArray<FAOLobbyReadyBoardEntry>& InEntries);

protected:
	// 각 플레이어 Row 가 들어가는 VerticalBox
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_Entries;

	// 한 줄 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "AO|LobbyBoard")
	TSubclassOf<UAO_LobbyReadyBoardRowWidget> RowWidgetClass;
};
