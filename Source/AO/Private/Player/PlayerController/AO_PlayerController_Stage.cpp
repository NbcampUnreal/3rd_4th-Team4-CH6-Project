// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"
#include "OnlineSubsystemTypes.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Widget/AO_SpectateWidget.h"

AAO_PlayerController_Stage::AAO_PlayerController_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Stage::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			HUDWidget->AddToViewport();
		}
	}

	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Stage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));	
}

void AAO_PlayerController_Stage::Server_RequestStageExit_Implementation()
{
	AO_LOG(LogJSH, Log, TEXT("Server_RequestStageExit"));

	if(UWorld* World = GetWorld())
	{
		if(AAO_GameMode_Stage* GM = World->GetAuthGameMode<AAO_GameMode_Stage>())
		{
			GM->HandleStageExitRequest(this);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("Server_RequestStageExit: GameMode_Stage not found"));
		}
	}
}

void AAO_PlayerController_Stage::Server_RequestStageFail_Implementation()
{
	AO_LOG(LogJSH, Log, TEXT("StageFailTest: Server_RequestStageFail from %s"), *GetName());

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageFailTest: World is null"));
		return;
	}

	AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>();
	if(StageGM == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageFailTest: GameMode is not AAO_GameMode_Stage"));
		return;
	}

	// 여기서 실제 실패 처리 로직 호출
	StageGM->HandleStageFail(this);
}

void AAO_PlayerController_Stage::ShowDeathUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!DeathWidget && DeathWidgetClass)
	{
		DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass);
	}
	
	if (DeathWidget)
	{
		DeathWidget->AddToViewport();
	}

	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AAO_PlayerController_Stage::RequestSpectate()
{
	if (IsLocalController())
	{
		if (!SpectateWidget && SpectateWidgetClass)
		{
			SpectateWidget = CreateWidget<UUserWidget>(this, SpectateWidgetClass);
		}
	
		if (SpectateWidget)
		{
			SpectateWidget->AddToViewport();
		}

		if (DeathWidget)
		{
			DeathWidget->RemoveFromParent();
		}
		
		ServerRPC_RequestSpectate();
	}
}

void AAO_PlayerController_Stage::RequestSpectateNext(bool bForward)
{
	if (IsLocalController())
	{
		ServerRPC_RequestSpectateNext(bForward);
	}
}

void AAO_PlayerController_Stage::ServerRPC_RequestSpectate_Implementation()
{
	TObjectPtr<APawn> NewTarget = nullptr;
	int32 NewIndex = INDEX_NONE;

	TObjectPtr<UWorld> World = GetWorld();
	checkf(World, TEXT("World is null"));
	
	TObjectPtr<AGameStateBase> GS = GetWorld()->GetGameState<AGameStateBase>();
	checkf(GS, TEXT("GameState is null"));

	TObjectPtr<AAO_PlayerState> MyPS = GetPlayerState<AAO_PlayerState>();
	checkf(MyPS, TEXT("PlayerState is null"));

	const TArray<TObjectPtr<APlayerState>>& Players = GS->PlayerArray;
	const int32 NumPlayers = Players.Num();
	for (int32 i = 0; i < NumPlayers; ++i)
	{
		TObjectPtr<AAO_PlayerState> PS = Cast<AAO_PlayerState>(Players[i]);
		if (!PS || PS == MyPS || !PS->GetIsAlive()) continue;

		TObjectPtr<APawn> OtherPawn = PS->GetPawn();
		if (!OtherPawn) continue;

		NewIndex = i;
		NewTarget = OtherPawn;
		break;
	}

	if (NewTarget)
	{
		CurrentSpectateTarget = NewTarget;
		CurrentSpectatePlayerIndex = NewIndex;
		
		ClientRPC_SetSpectateTarget(NewTarget, NewIndex);
	}
}

void AAO_PlayerController_Stage::ServerRPC_RequestSpectateNext_Implementation(bool bForward)
{
	int32 NewIndex = INDEX_NONE;
	TObjectPtr<APawn> NewTarget = FindNextSpectateTarget(bForward, NewIndex);

	if (NewTarget && NewIndex != INDEX_NONE)
	{
		CurrentSpectateTarget = NewTarget;
		CurrentSpectatePlayerIndex = NewIndex;
		
		ClientRPC_SetSpectateTarget(NewTarget, NewIndex);
	}
}

void AAO_PlayerController_Stage::ClientRPC_SetSpectateTarget_Implementation(APawn* NewTarget, int32 NewPlayerIndex)
{
	if (!IsLocalController() || !NewTarget)
	{
		return;
	}

	CurrentSpectateTarget = NewTarget;
	CurrentSpectatePlayerIndex = NewPlayerIndex;

	const float BlendTime = 0.4f;
	SetViewTargetWithBlend(NewTarget, BlendTime);

	FText SpectatingPlayerName = FText::GetEmpty();
	if (TObjectPtr<APlayerState> PS = NewTarget->GetPlayerState())
	{
		SpectatingPlayerName = FText::FromString(PS->GetPlayerName());
	}
	
	if (SpectateWidget)
	{
		if (TObjectPtr<UAO_SpectateWidget> SpectateUI = Cast<UAO_SpectateWidget>(SpectateWidget))
		{
			SpectateUI->SetSpectatingPlayerName(SpectatingPlayerName);
		}
	}
}

TObjectPtr<APawn> AAO_PlayerController_Stage::FindNextSpectateTarget(bool bForward, int32& OutNewIndex)
{
	OutNewIndex = INDEX_NONE;
	
	TObjectPtr<UWorld> World = GetWorld();
	checkf(World, TEXT("World is null"));
	
	TObjectPtr<AGameStateBase> GS = GetWorld()->GetGameState<AGameStateBase>();
	checkf(GS, TEXT("GameState is null"));

	TObjectPtr<AAO_PlayerState> MyPS = GetPlayerState<AAO_PlayerState>();
	checkf(MyPS, TEXT("PlayerState is null"));

	const TArray<TObjectPtr<APlayerState>>& Players = GS->PlayerArray;
	const int32 NumPlayers = Players.Num();
	if (NumPlayers == 0) return nullptr;

	// 살아있는 후보 만들기
	TArray<int32> AliveIndices;
	AliveIndices.Reserve(NumPlayers);
	
	for (int32 i = 0; i < NumPlayers; ++i)
	{
		TObjectPtr<AAO_PlayerState> PS = Cast<AAO_PlayerState>(Players[i]);
		if (!PS || PS == MyPS
			|| !PS->GetIsAlive()
			|| !PS->GetPawn()) continue;

		AliveIndices.Add(i);
	}

	if (AliveIndices.Num() == 0)
	{
		return nullptr;
	}

	// 현재 관전 인덱스 기준으로 다음/이전 찾기
	int32 StartIndexInPlayerArray = INDEX_NONE;

	if (CurrentSpectatePlayerIndex != INDEX_NONE && Players.IsValidIndex(CurrentSpectatePlayerIndex))
	{
		StartIndexInPlayerArray = CurrentSpectatePlayerIndex;
	}
	else
	{
		StartIndexInPlayerArray = AliveIndices[0];
	}

	for (int32 Offset = 1; Offset < NumPlayers; ++Offset)
	{
		int32 RawIndex;
		if (bForward)
		{
			RawIndex = (StartIndexInPlayerArray + Offset) % NumPlayers;
		}
		else
		{
			RawIndex = (StartIndexInPlayerArray - Offset + NumPlayers) % NumPlayers;
		}

		TObjectPtr<AAO_PlayerState> PS = Cast<AAO_PlayerState>(Players[RawIndex]);
		if (!PS || PS == MyPS || !PS->GetIsAlive()) continue;

		TObjectPtr<APawn> OtherPawn = PS->GetPawn();
		if (!OtherPawn) continue;

		OutNewIndex = RawIndex;
		return OtherPawn;
	}

	return nullptr;
}

// 임시 키 입력 코드
void AAO_PlayerController_Stage::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::O, IE_Pressed, this, &AAO_PlayerController_Stage::HandleStageFailInput);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("StagePC: InputComponent is null in SetupInputComponent"));
	}
}

void AAO_PlayerController_Stage::HandleStageFailInput()
{
	AO_LOG(LogJSH, Log, TEXT("StageFailTest: O key pressed on %s"), *GetName());

	if(IsLocalController())
	{
		Server_RequestStageFail();
	}
}