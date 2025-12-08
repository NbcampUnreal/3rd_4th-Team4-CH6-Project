// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"

/*----------- 테스트용 코드------------*/
#include "Train/AO_Train.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_RemoveFuel_GameplayAbility.h"
#include "EngineUtils.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"
/*-----------------------------------*/

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

/* ---------------------임시 키 입력 코드----------------------- */
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

void AAO_PlayerController_Stage::Server_TestRemoveFuel_Implementation()
{
	AO_LOG(LogJSH, Log, TEXT("TestRemoveFuel: Server_TestRemoveFuel from %s"), *GetName());

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("TestRemoveFuel: World is null"));
		return;
	}

	// 월드에서 Train 한 대 찾기
	AAO_Train* Train = nullptr;
	for(TActorIterator<AAO_Train> It(World); It; ++It)
	{
		Train = *It;
		break;
	}

	if(Train == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("TestRemoveFuel: Train not found in world"));
		return;
	}

	// Train의 ASC 가져오기
	UAbilitySystemComponent* ASC = Train->GetAbilitySystemComponent();
	if(ASC == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("TestRemoveFuel: Train has no AbilitySystemComponent"));
		return;
	}

	// Fuel Attribute 값 직접 감소
	const FGameplayAttribute FuelAttr = UAO_Fuel_AttributeSet::GetFuelAttribute();

	const float OldFuel = ASC->GetNumericAttribute(FuelAttr);
	const float NewFuel = OldFuel - 10.0f;	// 테스트용: 10씩 감소 (원하면 -5 등으로 조절)

	ASC->SetNumericAttributeBase(FuelAttr, NewFuel);

	AO_LOG(LogJSH, Log, TEXT("TestRemoveFuel: Fuel %.1f -> %.1f"), OldFuel, NewFuel);
}

void AAO_PlayerController_Stage::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::O, IE_Pressed, this, &AAO_PlayerController_Stage::HandleStageFailInput);
		InputComponent->BindKey(EKeys::J, IE_Pressed, this, &AAO_PlayerController_Stage::HandleTestRemoveFuelInput);
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

void AAO_PlayerController_Stage::HandleTestRemoveFuelInput()
{
	AO_LOG(LogJSH, Log, TEXT("TestRemoveFuel: J key pressed on %s"), *GetName());

	if(IsLocalController())
	{
		Server_TestRemoveFuel();
	}
}
/*------------------------------------------------------------------*/