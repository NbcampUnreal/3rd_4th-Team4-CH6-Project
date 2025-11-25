// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "AO/AO_Log.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Engine/GameInstance.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_PlayerController_Lobby::AAO_PlayerController_Lobby()
{
}

void AAO_PlayerController_Lobby::BeginPlay()
{
	Super::BeginPlay();

	// 메인 메뉴에서 넘어올 때 UIOnly 상태를 확실하게 정리
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	AO_LOG(LogJSH, Log, TEXT("Lobby PC BeginPlay: InputMode reset to GameOnly (%s)"), *GetName());
}

void AAO_PlayerController_Lobby::Client_OpenInviteOverlay_Implementation()
{
	if(const UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			Sub->ShowInviteUI();
			return;
		}
	}

	AO_LOG(LogJSH, Warning, TEXT("Client_OpenInviteOverlay: OnlineSessionSubsystem not found"));
}

void AAO_PlayerController_Lobby::ServerSetReady_Implementation(bool bNewReady)
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

void AAO_PlayerController_Lobby::ServerRequestStart_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->RequestStartFrom(this);
		}
	}
}

void AAO_PlayerController_Lobby::Client_SpawnWardrobePreview_Implementation(const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	static TSubclassOf<APawn> PreviewPawnClass = nullptr;
	if(PreviewPawnClass == nullptr)
	{
		PreviewPawnClass = LoadClass<APawn>(
			nullptr,
			TEXT("/Game/AVaOut/Character/Test_JSH/BP_WardrobePreviewPawn.BP_WardrobePreviewPawn_C")
		);
	}

	if(PreviewPawnClass == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT("Client_SpawnWardrobePreview: PreviewPawnClass not found"));
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient; // 저장/복제 X

	APawn* PreviewPawn = World->SpawnActor<APawn>(PreviewPawnClass, SpawnTransform, Params);
	if(PreviewPawn == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT("Client_SpawnWardrobePreview: Spawn failed"));
		return;
	}

	// 카메라만 프리뷰 Pawn 쪽으로 전환 (로컬 전용)
	FViewTargetTransitionParams Blend;
	Blend.BlendTime = 0.25f; // 부드럽게 전환하고 싶으면 값 조절

	SetViewTarget(PreviewPawn, Blend);
}