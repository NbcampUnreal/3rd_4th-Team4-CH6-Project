// HSJ : AO_InteractionComponent.cpp
#include "Interaction/Component/AO_InteractionComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AO_Log.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Interaction/UI/AO_InteractionWidget.h"
#include "Interaction/UI/AO_InteractionWidgetController.h"
#include "Net/UnrealNetwork.h"

UAO_InteractionComponent::UAO_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_InteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAO_InteractionComponent, bIsHoldingInteract);
}

void UAO_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		AO_LOG(LogHSJ, Error, TEXT("Owner is null"));
		return;
	}

	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (!ASC)
	{
		AO_LOG(LogHSJ, Error, TEXT("ASC not found on owner"));
		return;
	}

	// 서버에서만 어빌리티 부여
	if (Owner->HasAuthority())
	{
		GiveDefaultAbilities();
	}

	// 로컬 플레이어만 UI 초기화
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			if (PC->IsLocalController())
			{
				AO_LOG(LogHSJ, Log, TEXT("Initializing UI for local player"));
				InitializeInteractionUI(PC);
			}
			else
			{
				AO_LOG(LogHSJ, Log, TEXT("Not local controller, skipping UI init"));
			}
		}
		else
		{
			AO_LOG(LogHSJ, Warning, TEXT("PlayerController not found in BeginPlay, UI init may be delayed"));
		}
	}
}

void UAO_InteractionComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent || !InteractAction)
	{
		return;
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &UAO_InteractionComponent::OnInteractPressed);
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Completed, this, &UAO_InteractionComponent::OnInteractReleased);
	}
}

void UAO_InteractionComponent::ServerTriggerInteract_Implementation(AActor* TargetActor)
{
	if (!TargetActor || !TargetActor->IsValidLowLevel())
	{
		return;
	}
	
	// 클라이언트 치팅 방지를 위한 추가 거리 검증
	const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
	constexpr float MaxInteractionDistanceSq = 250.0f * 250.0f;
	if (DistSq > MaxInteractionDistanceSq)
	{
		AO_LOG_NET(LogHSJ, Error, TEXT("Target too far: %.1fm"), FMath::Sqrt(DistSq) / 100.0f);
		return;
	}
	
	bIsHoldingInteract = true;
	
	// GameplayEvent로 Execute 어빌리티 트리거
	if (UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.EventTag = AO_InteractionTags::Ability_Action_AbilityInteract_Execute;
		Payload.Instigator = GetOwner();
		Payload.Target = TargetActor;
		
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

void UAO_InteractionComponent::ServerNotifyInteractReleased_Implementation()
{
	bIsHoldingInteract = false;
	OnInteractInputReleased.Broadcast();
}

void UAO_InteractionComponent::InitializeInteractionUI(APlayerController* PC)
{
	if (!InteractionWidgetClass)
	{
		AO_LOG(LogHSJ, Error, TEXT("InteractionWidgetClass is not set in Blueprint!"));
		return;
	}

	InteractionWidget = CreateWidget<UAO_InteractionWidget>(PC, InteractionWidgetClass);
	if (!InteractionWidget)
	{
		AO_LOG(LogHSJ, Error, TEXT("Failed to create InteractionWidget"));
		return;
	}

	InteractionWidgetController = NewObject<UAO_InteractionWidgetController>(this);
	if (!InteractionWidgetController)
	{
		AO_LOG(LogHSJ, Error, TEXT("Failed to create InteractionWidgetController"));
		return;
	}

	InteractionWidget->SetWidgetController(InteractionWidgetController);
	InteractionWidget->AddToViewport();
	
	AO_LOG(LogHSJ, Log, TEXT("InteractionUI initialized successfully"));
}

void UAO_InteractionComponent::GiveDefaultAbilities()
{
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (!ASC || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
		ASC->GiveAbility(Spec);

		AO_LOG(LogHSJ, Log, TEXT("Granted ability: %s"), *AbilityClass->GetName());
	}
}

void UAO_InteractionComponent::OnInteractPressed()
{
	bIsHoldingInteract = true;
	
	if (CurrentInteractTarget)
	{
		ServerTriggerInteract(CurrentInteractTarget);
	}
}

void UAO_InteractionComponent::OnInteractReleased()
{
	bIsHoldingInteract = false;
	OnInteractInputReleased.Broadcast();
	ServerNotifyInteractReleased();
}

UAbilitySystemComponent* UAO_InteractionComponent::GetOwnerAbilitySystemComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	}
	return nullptr;
}