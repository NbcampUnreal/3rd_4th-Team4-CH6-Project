// AO_PlayerController_Lobby.cpp


#include "Player/PlayerController/AO_PlayerController_Lobby.h"

#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "AO/AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Customizing/AO_CustomizingCharacter.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Engine/GameInstance.h"
#include "Engine/TargetPoint.h"
#include "Interaction/Interactables/AO_LobbyInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_PlayerController_Lobby::AAO_PlayerController_Lobby()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Lobby::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();
	AO_LOG(LogJM, Log, TEXT("End"));

	// 메인 메뉴에서 넘어올 때 UIOnly 상태를 확실하게 정리
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	AO_LOG(LogJSH, Log, TEXT("Lobby PC BeginPlay: InputMode reset to GameOnly (%s)"), *GetName());

	PlayerCharacter = Cast<AAO_PlayerCharacter>(GetCharacter());

	// HUD 위젯 추가
	if (IsLocalPlayerController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			HUDWidget->AddToViewport();
		}
	}
}

void AAO_PlayerController_Lobby::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Lobby::Server_SetReady_Implementation(bool bNewReady)
{
	if(AAO_PlayerState* PS = GetPlayerState<AAO_PlayerState>())
	{
		PS->SetLobbyReady(bNewReady);
	}

	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->SetPlayerReady(this, bNewReady);
		}
	}
}

void AAO_PlayerController_Lobby::Server_RequestStart_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->RequestStartFrom(this);
		}
	}
}

void AAO_PlayerController_Lobby::Server_RequestInviteOverlay_Implementation()
{
	AO_LOG(LogJSH, Log,
		TEXT("Server_RequestInviteOverlay: Called on Server | PC=%s HasAuthority=%d IsLocal=%d"),
		*GetName(),
		HasAuthority() ? 1 : 0,
		IsLocalController() ? 1 : 0);

	Client_OpenInviteOverlay();
}

void AAO_PlayerController_Lobby::Server_RequestWardrobe_Implementation()
{
	AO_LOG(LogJSH, Log,
		TEXT("Server_RequestWardrobe: Called on Server | PC=%s HasAuthority=%d IsLocal=%d"),
		*GetName(),
		HasAuthority() ? 1 : 0,
		IsLocalController() ? 1 : 0);
	
	Client_OpenWardrobe();
}

void AAO_PlayerController_Lobby::Client_OpenInviteOverlay_Implementation()
{
	AO_LOG(LogJSH, Log,
		TEXT("Client_OpenInviteOverlay: Called on Client | PC=%s HasAuthority=%d IsLocal=%d"),
		*GetName(),
		HasAuthority() ? 1 : 0,
		IsLocalController() ? 1 : 0);

	OpenInviteOverlay();
}

void AAO_PlayerController_Lobby::Client_OpenWardrobe_Implementation()
{
	AO_LOG(LogJSH, Log,
		TEXT("Client_OpenWardrobe: Called on Client | PC=%s HasAuthority=%d IsLocal=%d"),
		*GetName(),
		HasAuthority() ? 1 : 0,
		IsLocalController() ? 1 : 0);

	OpenWardrobe();
}

void AAO_PlayerController_Lobby::OpenInviteOverlay()
{
	if( !IsLocalController() )
	{
		AO_LOG(LogJSH, Warning,
			TEXT("OpenInviteOverlay: Not LocalController | PC=%s"),
			*GetName());
		return;
	}

	if( const UGameInstance* GI = GetGameInstance() )
	{
		if( UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>() )
		{
			AO_LOG(LogJSH, Log,
				TEXT("OpenInviteOverlay: ShowInviteUI | PC=%s"),
				*GetName());

			Sub->ShowInviteUI();
			return;
		}
	}

	AO_LOG(LogJSH, Warning,
		TEXT("OpenInviteOverlay: OnlineSessionSubsystem not found | PC=%s"),
		*GetName());
}

void AAO_PlayerController_Lobby::OpenWardrobe()
{
	if( !IsLocalController() )
	{
		AO_LOG(LogJSH, Warning,
			TEXT("OpenWardrobe: Not LocalController | PC=%s"),
			*GetName());
		return;
	}

	// TODO: 실제 Wardrobe UI 열기
	AO_LOG(LogJSH, Log,
		TEXT("OpenWardrobe: Open wardrobe UI (TODO) | PC=%s"),
		*GetName());
	
	FadeIn();

	TObjectPtr<ATargetPoint> SpawnPoint = Cast<ATargetPoint>(UGameplayStatics::GetActorOfClass(GetWorld(), ATargetPoint::StaticClass()));

	if (SpawnPoint)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CustomizingDummy = GetWorld()->SpawnActor<AAO_CustomizingCharacter>
		(CustomizingDummyClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), Params);
	}

	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AAO_PlayerController_Lobby::OnFadeInFinishedOpenUI,
	                                FadeTime, false);

	AO_LOG(LogKSH, Log, TEXT("OpenWardrobe End"));
}

void AAO_PlayerController_Lobby::CloseWardrobe()
{
	if (FadeTimerHandle.IsValid())
	{
		return;
	}
	
	FadeIn();

	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AAO_PlayerController_Lobby::OnFadeInFinishedCloseUI,
									FadeTime, false);
}

void AAO_PlayerController_Lobby::Server_CloseWardrobe_Implementation()
{
	if (!CustomizingInteractable)
	{
		AO_LOG(LogJSH, Error, TEXT("CustomizingInteractable is NULL"));
		return;
	}

	AAO_PlayerCharacter* Interactor = Cast<AAO_PlayerCharacter>(GetPawn());
	if (!Interactor)
	{
		AO_LOG(LogJSH, Error, TEXT("Character is NULL"));
		return;
	}

	CustomizingInteractable->RemoveDisabledPlayer(Interactor);
}

void AAO_PlayerController_Lobby::FadeIn()
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeTime,
		                                     FLinearColor::Black, false, true);
	}
}

void AAO_PlayerController_Lobby::FadeOut()
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeTime,
		                                     FLinearColor::Black, false, true);
	}
}

void AAO_PlayerController_Lobby::OnFadeInFinishedOpenUI()
{
	if (CustomizingDummy)
	{
		SetViewTarget(CustomizingDummy);

		CustomizingWidget->AddToViewport();
	}

	// HUD 위젯 정리
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
	}
	
	FadeTimerHandle.Invalidate();
	
	FadeOut();
}

void AAO_PlayerController_Lobby::OnFadeInFinishedCloseUI()
{
	if (CustomizingDummy)
	{
		SetViewTarget(PlayerCharacter);
		
		CustomizingWidget->RemoveFromParent();
		
		CustomizingDummy->Destroy();
		CustomizingDummy = nullptr;
	}
	
	// HUD 위젯 추가
	if (!HUDWidget)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
	}
	HUDWidget->AddToViewport();

	Server_CloseWardrobe();
	
	FadeOut();
}
