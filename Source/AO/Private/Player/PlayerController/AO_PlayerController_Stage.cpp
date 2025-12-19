// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"
#include "OnlineSubsystemTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Widget/AO_SpectateWidget.h"

/*----------- 테스트용 코드------------*/
#include "Train/AO_Train.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_RemoveFuel_GameplayAbility.h"
#include "EngineUtils.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Components/AO_DeathSpectateComponent.h"
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

		if (HUDWidget)
		{
			HUDWidget->RemoveFromParent();
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

void AAO_PlayerController_Stage::ForceReselectSpectateTarget(APawn* InvalidTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentSpectateTarget != InvalidTarget)
	{
		return;
	}

	int32 NewIndex = INDEX_NONE;
	TObjectPtr<APawn> NewTarget = FindNextSpectateTarget(true, NewIndex);

	if (!NewTarget)
	{
		ServerRPC_SetSpectateTarget(nullptr);

		CurrentSpectateTarget = nullptr;
		CurrentSpectatePlayerIndex = INDEX_NONE;
		return;
	}

	ServerRPC_SetSpectateTarget(NewTarget);
	
	CurrentSpectateTarget = NewTarget;
	CurrentSpectatePlayerIndex = NewIndex;
	
	ClientRPC_SetSpectateTarget(NewTarget, NewIndex);
}

void AAO_PlayerController_Stage::ServerRPC_SetSpectateTarget_Implementation(APawn* NewTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	if (PrevSpectateTarget)
	{
		if (AAO_PlayerCharacter* OldChar = Cast<AAO_PlayerCharacter>(PrevSpectateTarget))
		{
			if (UAO_DeathSpectateComponent* Comp = OldChar->FindComponentByClass<UAO_DeathSpectateComponent>())
			{
				Comp->RemoveSpectator(this);
			}
		}
	}

	PrevSpectateTarget = NewTarget;

	if (PrevSpectateTarget)
	{
		if (AAO_PlayerCharacter* NewChar = Cast<AAO_PlayerCharacter>(PrevSpectateTarget))
		{
			if (UAO_DeathSpectateComponent* Comp = NewChar->FindComponentByClass<UAO_DeathSpectateComponent>())
			{
				Comp->AddSpectator(this);
			}
		}
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
		if (!PS || PS == MyPS || !PS->GetIsAlive())
		{
			continue;
		}

		TObjectPtr<APawn> OtherPawn = PS->GetPawn();
		if (!OtherPawn)
		{
			continue;
		}

		NewIndex = i;
		NewTarget = OtherPawn;
		break;
	}

	if (NewTarget)
	{
		ServerRPC_SetSpectateTarget(NewTarget);
		
		CurrentSpectateTarget = NewTarget;
		CurrentSpectatePlayerIndex = NewIndex;
		
		ClientRPC_SetSpectateTarget(NewTarget, NewIndex);
	}
}

void AAO_PlayerController_Stage::ServerRPC_RequestSpectateNext_Implementation(bool bForward)
{
	int32 NewIndex = INDEX_NONE;
	TObjectPtr<APawn> NewTarget = FindNextSpectateTarget(bForward, NewIndex);

	if (!NewTarget)
	{
		ServerRPC_SetSpectateTarget(nullptr);

		CurrentSpectateTarget = nullptr;
		CurrentSpectatePlayerIndex = INDEX_NONE;
		return;
	}

	ServerRPC_SetSpectateTarget(NewTarget);
	
	CurrentSpectateTarget = NewTarget;
	CurrentSpectatePlayerIndex = NewIndex;
	
	ClientRPC_SetSpectateTarget(NewTarget, NewIndex);
}

void AAO_PlayerController_Stage::ClientRPC_SetSpectateTarget_Implementation(APawn* NewTarget, int32 NewPlayerIndex)
{
	checkf(NewTarget, TEXT("NewTarget is null"));
	
	if (!IsLocalController())
	{
		return;
	}

	CurrentSpectateTarget = NewTarget;
	CurrentSpectatePlayerIndex = NewPlayerIndex;

	const float BlendTime = 0.4f;
	SetViewTargetWithBlend(NewTarget, BlendTime);

	TObjectPtr<ACharacter> SpectatedCharacter = Cast<ACharacter>(NewTarget);
	
	if (SpectateWidget)
	{
		if (TObjectPtr<UAO_SpectateWidget> SpectateUI = Cast<UAO_SpectateWidget>(SpectateWidget))
		{
			SpectateUI->SetSpectatingCharacter(SpectatedCharacter);
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
			|| !PS->GetPawn())
		{
			continue;
		}
			
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
		if (!PS || PS == MyPS || !PS->GetIsAlive())
		{
			continue;
		}

		TObjectPtr<APawn> OtherPawn = PS->GetPawn();
		if (!OtherPawn)
		{
			continue;
		}
		
		OutNewIndex = RawIndex;
		return OtherPawn;
	}

	return nullptr;
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

void AAO_PlayerController_Stage::Server_RequestRevive_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>();
	if (!StageGM)
	{
		return;
	}

	const bool bSuccess = StageGM->TryRevivePlayer(this);

	if (bSuccess)
	{
		AO_LOG(LogJSH, Log, TEXT("ReviveTest: Revive success for %s"), *GetName());
		Client_OnRevived();
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("ReviveTest: Revive failed for %s"), *GetName());
	}
}

void AAO_PlayerController_Stage::Client_OnRevived_Implementation()
{
	// 1) Death UI 닫기
	if (DeathWidget)
	{
		DeathWidget->RemoveFromParent();
		DeathWidget = nullptr;
	}

	// 2) HUD 위젯 완전히 갈아끼우기
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}

	// 3) 입력 모드 복구
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	// 4) 관전 UI 떠 있었으면 닫기
	if (SpectateWidget)
	{
		SpectateWidget->RemoveFromParent();
		SpectateWidget = nullptr;
	}

	// 5) 관전 중이었다면 관전 끝 알려주기
	if (IsLocalController())
	{
		ServerRPC_SetSpectateTarget(nullptr);
	}

	AO_LOG(LogJSH, Log, TEXT("ReviveTest: UI restored for %s"), *GetName());
}

void AAO_PlayerController_Stage::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::O, IE_Pressed, this, &AAO_PlayerController_Stage::HandleStageFailInput);
		InputComponent->BindKey(EKeys::J, IE_Pressed, this, &AAO_PlayerController_Stage::HandleTestRemoveFuelInput);
		InputComponent->BindKey(EKeys::K, IE_Pressed, this, &AAO_PlayerController_Stage::HandleReviveInput);
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

void AAO_PlayerController_Stage::HandleReviveInput()
{
	AO_LOG(LogJSH, Log, TEXT("ReviveTest: K key pressed on %s"), *GetName());

	if (IsLocalController())
	{
		Server_RequestRevive();
	}
}
/*------------------------------------------------------------------*/