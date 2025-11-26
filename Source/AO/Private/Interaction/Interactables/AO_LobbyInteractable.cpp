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
        if (Controller == nullptr)
        {
            return false;
        }

        const AAO_PlayerState* PS = Cast<AAO_PlayerState>(Controller->PlayerState);
        if (PS == nullptr)
        {
            return false;
        }

        return PS->IsLobbyHost();
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

    case EAO_LobbyInteractType::Wardrobe:
        // 모두 커스터마이징 가능
        return true;

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
                PC->Server_SetReady(bNewReady);
            }
            break;
        }
    case EAO_LobbyInteractType::StartGame:
        {
            if (!bIsHost)
            {
                return;
            }
            PC->Server_RequestStart();
            break;
        }
    case EAO_LobbyInteractType::InviteFriends:
        {
            PC->Client_OpenInviteOverlay();
            break;
        }
    case EAO_LobbyInteractType::Wardrobe:
        {
            AO_LOG(LogJSH, Log,
                TEXT("Wardrobe Interact: %s used wardrobe interactable %s"),
                *GetNameSafe(PC),
                *GetName());
            break;
        }
    default:
        break;
    }
}