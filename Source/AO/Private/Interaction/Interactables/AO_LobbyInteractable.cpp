// JSH : AO_LobbyInteractable.cpp


#include "Interaction/Interactables/AO_LobbyInteractable.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "AO_Log.h"

namespace
{
    static bool IsLobbyHostController(const AController* Controller)
    {
        if(Controller == nullptr)
        {
            return false;
        }

        const UWorld* World = Controller->GetWorld();
        if(World == nullptr)
        {
            return false;
        }

        const AGameStateBase* GS = World->GetGameState();
        if(GS == nullptr || GS->PlayerArray.Num() == 0)
        {
            return false;
        }

        const APlayerState* HostPS = GS->PlayerArray[0];
        return HostPS != nullptr && HostPS == Controller->PlayerState;
    }
}


AAO_LobbyInteractable::AAO_LobbyInteractable(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InteractType = EAO_LobbyInteractType::ReadyToggle;
    InteractionDuration = 0.0f;

    InteractionTitle = FText::FromString(TEXT("Lobby Action"));
    InteractionContent = FText::FromString(TEXT("Press F"));
}

bool AAO_LobbyInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    if (!Super::CanInteraction(InteractionQuery))
    {
        return false;
    }

    AController* PC = InteractionQuery.RequestingController.Get();
    if (!PC)
    {
        return false;
    }

    APawn* Pawn = Cast<APawn>(InteractionQuery.RequestingAvatar.Get());
    if (!Pawn)
    {
        return false;
    }

    const bool bIsHost = IsLobbyHostController(PC);

    switch (InteractType)
    {
    case EAO_LobbyInteractType::ReadyToggle:
        // 게스트만 가능
        return !bIsHost;

    case EAO_LobbyInteractType::StartGame:
        // 호스트만 가능
        return bIsHost;

    case EAO_LobbyInteractType::InviteFriends:
        // 모두 초대 UI 허용
        return PC->IsLocalController();

    default:
        return false;
    }
}

void AAO_LobbyInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
    Super::OnInteractionSuccess_BP_Implementation(Interactor);

    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn)
    {
        return;
    }

    AAO_PlayerController_Lobby* PC = Cast<AAO_PlayerController_Lobby>(Pawn->GetController());
    if (!PC)
    {
        return;
    }

    const bool bIsHost = IsLobbyHostController(PC);

    switch (InteractType)
    {
    case EAO_LobbyInteractType::ReadyToggle:
        {
            if (bIsHost)
            {
                return;
            }

            if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
            {
                const bool bNewReady = !PS->IsLobbyReady();
                PC->ServerSetReady(bNewReady);
            }
            break;
        }
    case EAO_LobbyInteractType::StartGame:
        {
            if (!bIsHost)
            {
                return;
            }
            PC->ServerRequestStart();
            break;
        }
    case EAO_LobbyInteractType::InviteFriends:
        {
            PC->Client_OpenInviteOverlay();
            break;
        }
    default:
        break;
    }
}