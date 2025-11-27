#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AO_LobbyReadyBoardActor.generated.h"

class UWidgetComponent;
class UAO_LobbyReadyBoardWidget;
class APlayerState;

USTRUCT(BlueprintType)
struct FAOLobbyReadyBoardEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	bool bIsHost;

	UPROPERTY(BlueprintReadWrite)
	FString StatusLabel;

	UPROPERTY(BlueprintReadWrite)
	bool bStatusActive;

	// 정렬용: 로비 입장 순서 (0부터 시작, -1이면 미할당)
	int32 JoinOrder = -1;
};

UCLASS()
class AO_API AAO_LobbyReadyBoardActor : public AActor
{
	GENERATED_BODY()

public:
	AAO_LobbyReadyBoardActor();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|LobbyBoard", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* BoardWidgetComponent;

public:
	// 서버에서 호출 → 모든 클라에서 보드 재빌드
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRebuildBoard();

	// GameState.PlayerArray 기준으로 엔트리 재구성
	void RebuildBoard();

protected:
	// 호스트를 제외한 모든 플레이어가 레디인지 확인
	bool IsEveryoneReadyExceptHost(const TArray<APlayerState*>& Players) const;

	// 실제 UMG 위젯에 데이터 적용
	void ApplyToWidget(const TArray<FAOLobbyReadyBoardEntry>& Entries);
};
