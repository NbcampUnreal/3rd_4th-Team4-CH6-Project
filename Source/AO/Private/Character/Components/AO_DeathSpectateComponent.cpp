// AO_DeathSpectateComponent.cpp

#include "Character/Components/AO_DeathSpectateComponent.h"

#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Player/PlayerState/AO_PlayerState.h"

UAO_DeathSpectateComponent::UAO_DeathSpectateComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UAO_DeathSpectateComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	checkf(OwnerActor, TEXT("OwnerActor is NULL"));
	
	OwnerCharacter = Cast<AAO_PlayerCharacter>(OwnerActor);
	checkf(OwnerCharacter, TEXT("OwnerCharacter is NULL"));

	if (OwnerActor->HasAuthority())
	{
		BindDeathDelegate();
	}

	if (OwnerCharacter->IsLocallyControlled() && bStreamEnabled)
	{
		StartCameraSyncTimer();
	}
}

void UAO_DeathSpectateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCameraSyncTimer();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UAO_DeathSpectateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_DeathSpectateComponent, RepCameraView);

	DOREPLIFETIME_CONDITION(UAO_DeathSpectateComponent, bStreamEnabled, COND_OwnerOnly);
}

void UAO_DeathSpectateComponent::BindDeathDelegate()
{
	check(OwnerCharacter);

	UAO_PlayerCharacter_AttributeSet* AttributeSet = OwnerCharacter->GetAttributeSet();
	checkf(AttributeSet, TEXT("AttributeSet is NULL"));

	AttributeSet->OnPlayerDeath.AddUObject(this, &UAO_DeathSpectateComponent::OnOwnerDied);
}

bool UAO_DeathSpectateComponent::IsAlive_Server() const
{
	check(OwnerCharacter);

	if (!OwnerCharacter->HasAuthority())
	{
		return true;
	}

	const AAO_PlayerState* PS = OwnerCharacter->GetPlayerState<AAO_PlayerState>();
	checkf(PS, TEXT("PlayerState is NULL"));

	return PS->GetIsAlive();
}

void UAO_DeathSpectateComponent::AddSpectator(APlayerController* SpectatorPC)
{
	if (!ensure(SpectatorPC))
	{
		return;
	}

	check(OwnerCharacter);
	
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	const int32 PrevNum = SpectatorSet.Num();
	SpectatorSet.Add(SpectatorPC);

	if (PrevNum == 0 && SpectatorSet.Num() > 0)
	{
		bStreamEnabled = true;
		OwnerActor->ForceNetUpdate();

		if (OwnerCharacter->IsLocallyControlled())
		{
			OnRep_StreamEnabled();
		}
	}
}

void UAO_DeathSpectateComponent::RemoveSpectator(APlayerController* SpectatorPC)
{
	if (!ensure(SpectatorPC))
	{
		return;
	}

	check(OwnerCharacter);
	
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	SpectatorSet.Remove(SpectatorPC);

	if (SpectatorSet.Num() == 0)
	{
		bStreamEnabled = false;
		OwnerActor->ForceNetUpdate();

		if (OwnerCharacter->IsLocallyControlled())
		{
			OnRep_StreamEnabled();
		}
	}
}

bool UAO_DeathSpectateComponent::GetRepCameraView(FRepCameraView& OutView) const
{
	OutView = RepCameraView;
	return true;
}

void UAO_DeathSpectateComponent::NotifySpectators_TargetInvalidated()
{
	check(OwnerCharacter);

	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	// 스냅샷 (순회 중에 Set에서 제거되어서 변경될 수 있기 때문)
	TArray<TWeakObjectPtr<APlayerController>> Spectators;
	Spectators.Reserve(SpectatorSet.Num());
	for (const TWeakObjectPtr<APlayerController>& PC : SpectatorSet)
	{
		if (PC.IsValid())
		{
			Spectators.Add(PC);
		}
	}

	for (const TWeakObjectPtr<APlayerController>& PC : Spectators)
	{
		AAO_PlayerController_Stage* SpectatorPC = Cast<AAO_PlayerController_Stage>(PC.Get());
		if (!SpectatorPC)
		{
			continue;
		}

		SpectatorPC->ForceReselectSpectateTarget(OwnerCharacter);
	}
}

void UAO_DeathSpectateComponent::OnOwnerDied()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	AAO_PlayerState* PS = OwnerCharacter->GetPlayerState<AAO_PlayerState>();
	checkf(PS, TEXT("PlayerState is NULL"));

	if (!PS->GetIsAlive())
	{
		return;
	}

	PS->SetIsAlive(false);

	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		FGameplayTagContainer DeathTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Death")));
		ASC->TryActivateAbilitiesByTag(DeathTag);
	}

	if (UAO_InteractableComponent* InteractableComponent = OwnerCharacter->GetInteractableComponent())
	{
		InteractableComponent->bInteractionEnabled = true;
	}

	NotifySpectators_TargetInvalidated();

	if (Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		ClientRPC_HandleDeathView();
	}
}

void UAO_DeathSpectateComponent::ClientRPC_HandleDeathView_Implementation()
{
	check(OwnerCharacter);

	if (USpringArmComponent* Arm = OwnerCharacter->GetSpringArm())
	{
		Arm->TargetArmLength += OwnerCharacter->DeathCameraArmOffset;
		Arm->SocketOffset = FVector::ZeroVector;
	}

	if (AAO_PlayerController_Stage* PC = Cast<AAO_PlayerController_Stage>(OwnerCharacter->GetController()))
	{
		PC->ShowDeathUI();
	}
}

void UAO_DeathSpectateComponent::ServerRPC_UpdateCameraView_Implementation(const FRepCameraView& NewView)
{
	RepCameraView = NewView;
}

void UAO_DeathSpectateComponent::StartCameraSyncTimer()
{
	check(OwnerCharacter);

	if (!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	UWorld* World = GetWorld();
	checkf(World, TEXT("Failed to get World"));

	if (World->GetTimerManager().IsTimerActive(TimerHandle_CameraSync))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		TimerHandle_CameraSync,
		this,
		&UAO_DeathSpectateComponent::SendCameraViewToServer,
		0.033f,
		true);
}

void UAO_DeathSpectateComponent::StopCameraSyncTimer()
{
	UWorld* World = GetWorld();
	checkf(World, TEXT("Failed to get World"));

	World->GetTimerManager().ClearTimer(TimerHandle_CameraSync);
}

void UAO_DeathSpectateComponent::SendCameraViewToServer()
{
	check(OwnerCharacter);

	if (!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	if (!bStreamEnabled)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			FRepCameraView V;
			V.Location = PC->PlayerCameraManager->GetCameraLocation();
			V.Rotation = PC->PlayerCameraManager->GetCameraRotation();
			V.FOV = PC->PlayerCameraManager->GetFOVAngle();

			ServerRPC_UpdateCameraView(V);
		}
	}
}

void UAO_DeathSpectateComponent::OnRep_StreamEnabled()
{
	check(OwnerCharacter);

	if (!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	if (bStreamEnabled)
	{
		StartCameraSyncTimer();
	}
	else
	{
		StopCameraSyncTimer();
	}
}
