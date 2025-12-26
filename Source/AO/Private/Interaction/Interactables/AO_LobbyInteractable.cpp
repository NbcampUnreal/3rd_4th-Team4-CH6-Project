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
        return true;

    case EAO_LobbyInteractType::Wardrobe:
        // 모두 커스터마이징 가능
        return true;

    default:
        return false;
    }
}

void AAO_LobbyInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
    AO_LOG(LogJSH, Log,
        TEXT("[LobbyInteract] OnSuccess Enter | This=%s Type=%d Interactor=%s"),
        *GetName(),
        static_cast<int32>(InteractType),
        *GetNameSafe(Interactor));

    Super::OnInteractionSuccess_BP_Implementation(Interactor);

    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn)
    {
        AO_LOG(LogJSH, Warning,
            TEXT("[LobbyInteract] OnSuccess: Interactor is not Pawn | This=%s Interactor=%s"),
            *GetName(),
            *GetNameSafe(Interactor));
        return;
    }

    AAO_PlayerController_Lobby* PC = Cast<AAO_PlayerController_Lobby>(Pawn->GetController());
    if (!PC)
    {
        AO_LOG(LogJSH, Warning,
            TEXT("[LobbyInteract] OnSuccess: No Lobby PC | This=%s Pawn=%s"),
            *GetName(),
            *GetNameSafe(Pawn));
        return;
    }

    PC->CustomizingInteractable = this;

    const bool bIsHost = IsLobbyHostController(PC);

    switch (InteractType)
    {
    case EAO_LobbyInteractType::ReadyToggle:
        {
            if (bIsHost)
            {
                AO_LOG(LogJSH, Log,
                    TEXT("[LobbyInteract] ReadyToggle: Host tried to use Ready, ignored | PC=%s"),
                    *GetNameSafe(PC));
                return;
            }

            if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
            {
                const bool bNewReady = !PS->IsLobbyReady();

                // 로컬 선반영: 내 화면에서 즉시 Ready 반영
                if (PC->IsLocalController())
                {
                    AO_LOG(LogJSH, Log,
                        TEXT("[LobbyInteract] ReadyToggle: local predict SetLobbyReady(%d) | PC=%s"),
                        static_cast<int32>(bNewReady),
                        *GetNameSafe(PC));

                    PS->SetLobbyReady(bNewReady);
                    PS->OnRep_LobbyIsReady();
                }

                // 서버에 실제 Ready 상태 요청
                AO_LOG(LogJSH, Log,
                    TEXT("[LobbyInteract] ReadyToggle: call Server_SetReady(%d) | PC=%s"),
                    static_cast<int32>(bNewReady),
                    *GetNameSafe(PC));

                PC->Server_SetReady(bNewReady);
            }
            else
            {
                AO_LOG(LogJSH, Warning,
                    TEXT("[LobbyInteract] ReadyToggle: PlayerState is null | PC=%s"),
                    *GetNameSafe(PC));
            }
            break;
        }
    case EAO_LobbyInteractType::StartGame:
        {
            if (!bIsHost)
            {
                AO_LOG(LogJSH, Log,
                    TEXT("[LobbyInteract] StartGame: Non-host tried to start, ignored | PC=%s"),
                    *GetNameSafe(PC));
                return;
            }
            AO_LOG(LogJSH, Log,
                TEXT("[LobbyInteract] StartGame: call Server_RequestStart | PC=%s"),
                *GetNameSafe(PC));
            PC->Server_RequestStart();
            break;
        }
    case EAO_LobbyInteractType::InviteFriends:
        {
            AO_LOG(LogJSH, Log,
                TEXT("[LobbyInteract] InviteFriends: call Server_RequestInviteOverlay | This=%s Interactor=%s HasAuthority=%d"),
                *GetName(),
                *GetNameSafe(Interactor),
                HasAuthority() ? 1 : 0);
            
            AO_LOG(LogJSH, Log,
                TEXT("[LobbyInteract] InviteFriends: Server_RequestInviteOverlay from PC=%s IsLocal=%d"),
                *GetNameSafe(PC),
                PC->IsLocalController() ? 1 : 0);

            PC->Server_RequestInviteOverlay();
            break;
        }

    case EAO_LobbyInteractType::Wardrobe:
        {
            AO_LOG(LogJSH, Log,
                TEXT("[LobbyInteract] Wardrobe: call Server_RequestWardrobe | This=%s Interactor=%s HasAuthority=%d"),
                *GetName(),
                *GetNameSafe(Interactor),
                HasAuthority() ? 1 : 0);

            AO_LOG(LogJSH, Log,
                TEXT("[LobbyInteract] Wardrobe: Server_RequestWardrobe from PC=%s IsLocal=%d"),
                *GetNameSafe(PC),
                PC->IsLocalController() ? 1 : 0);

            PC->Server_RequestWardrobe();
            break;
        }
    default:
        {
            AO_LOG(LogJSH, Warning,
                TEXT("[LobbyInteract] Unknown InteractType=%d | This=%s"),
                static_cast<int32>(InteractType),
                *GetName());
            break;
        }
    }
}
