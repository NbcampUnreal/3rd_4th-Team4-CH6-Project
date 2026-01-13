// JSH: AO_StageInteractable.cpp 

#include "Interaction/Interactables/AO_StageInteractable.h"

#include "EngineUtils.h"
#include "LoadingScreenManager.h"
#include "Game/GameState/AO_GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Train/AO_newTrain.h"


AAO_StageInteractable::AAO_StageInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractType = EAO_StageInteractType::ExitToNextArea;
	RequiredFuel = 20.0f;
	TutorialExitLevelName = TEXT("LV_MainMenu");
}

FAO_InteractionInfo AAO_StageInteractable::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	FAO_InteractionInfo Info = Super::GetInteractionInfo(InteractionQuery);

	bool bHasFuel = false;
	bool bHasClues = false;
	GetCurrentConditions(bHasFuel, bHasClues);

	// 모두 충족
	if (bHasFuel && bHasClues)
	{
		Info.Title = FText::FromString(TEXT("Proceed to Next Area"));
		Info.TitleTextColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		Info.HighlightStencilValue = 250;
	}
	// 연료, 단서 둘 다 부족
	else if (!bHasFuel && !bHasClues)
	{
		Info.Title = FText::FromString(TEXT("Need Fuel and Clues"));
		Info.TitleTextColor = FLinearColor::Red;
		Info.HighlightStencilValue = 252;
	}
	// 연료만 부족
	else if (!bHasFuel)
	{
		Info.Title = FText::FromString(TEXT("Need Fuel"));
		Info.TitleTextColor = FLinearColor::Red;
		Info.HighlightStencilValue = 252;
	}
	// 단서만 부족
	else
	{
		Info.Title = FText::FromString(TEXT("Need Clues"));
		Info.TitleTextColor = FLinearColor::Red;
		Info.HighlightStencilValue = 252;
	}

	return Info;
}

void AAO_StageInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	if (InteractType == EAO_StageInteractType::Tutorial)
	{
		bool bHasFuel = false;
		bool bHasClues = false;
		GetCurrentConditions(bHasFuel, bHasClues);

		// 조건 충족 시에만 레벨 이동
		if (bHasFuel && bHasClues)
		{
			// JM : 로딩화면이 잘 안떠서, 이동할 레벨 기록
			ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
			if (LSM)
			{
				LSM->PendingMapName = TutorialExitLevelName.ToString();
			}
			
			UGameplayStatics::OpenLevel(this, TutorialExitLevelName);
		}
        
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	if(!Pawn)
	{
		return;
	}
	
	AAO_PlayerController_Stage* PC = Cast<AAO_PlayerController_Stage>(Pawn->GetController());
	if(!PC)
	{
		return;
	}

	switch(InteractType)
	{
	case EAO_StageInteractType::ExitToNextArea:
		PC->Server_RequestStageExit();
		break;

	case EAO_StageInteractType::ExitToLobby:
		// 추후 필요시 구현
		break;

	default:
		break;
	}
}

void AAO_StageInteractable::GetCurrentConditions(bool& bOutHasFuel, bool& bOutHasClues) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 단서 체크
	if (AAO_GameState* GS = World->GetGameState<AAO_GameState>())
	{
		bOutHasClues = GS->CheckHintCount();
	}

	// 연료 체크
	for (TActorIterator<AAO_newTrain> It(World); It; ++It)
	{
		if (AAO_newTrain* Train = *It)
		{
			if (UAbilitySystemComponent* ASC = Train->GetAbilitySystemComponent())
			{
				const float Fuel = ASC->GetNumericAttribute(UAO_Fuel_AttributeSet::GetFuelAttribute());
				bOutHasFuel = (Fuel >= RequiredFuel);
			}
			break;
		}
	}
}