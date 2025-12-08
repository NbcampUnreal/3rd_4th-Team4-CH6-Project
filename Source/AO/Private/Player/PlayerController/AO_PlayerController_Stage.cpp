// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"

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

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(DeathWidget->TakeWidget());
	SetInputMode(InputMode);
	bShowMouseCursor = true;
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