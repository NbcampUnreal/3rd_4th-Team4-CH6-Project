// AO_NameplateComponent.cpp

#include "Character/Components/AO_NameplateComponent.h"

#include "AO_Log.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Character/Customizing/AO_CustomizingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "UI/Player/AO_NameTagWidget.h"

UAO_NameplateComponent::UAO_NameplateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
}

void UAO_NameplateComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureWidgetComponent();
	ApplyDistanceVisuals();

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	if (Owner && Owner->HasAuthority())
	{
		TryInitNameFromOwner();
	}
	else
	{
		ApplyDisplayNameToWidget();
	}

	UAO_CustomizingComponent* CustomizingComp = Owner->FindComponentByClass<UAO_CustomizingComponent>();
	if (CustomizingComp)
	{
		CachedCustomizingComponent = CustomizingComp;
		CustomizingComp->OnCapsuleChangedDelegate.AddUObject(this, &UAO_NameplateComponent::HandleCapsuleSizeChanged);
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (OwnerCharacter && OwnerCharacter->GetCapsuleComponent())
	{
		HandleCapsuleSizeChanged(OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}
}

void UAO_NameplateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!WidgetComponent)
	{
		return;
	}
	
	ApplyDistanceVisuals();
}

void UAO_NameplateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedCustomizingComponent.IsValid())
	{
		CachedCustomizingComponent->OnCapsuleChangedDelegate.RemoveAll(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UAO_NameplateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_NameplateComponent, DisplayName);
}

void UAO_NameplateComponent::HandlePlayerStateChanged(APlayerState* NewPlayerState)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!NewPlayerState)
	{
		return;
	}

	const FString PlayerName = NewPlayerState->GetPlayerName();
	if (!PlayerName.IsEmpty())
	{
		SetDisplayName_Server(FText::FromString(PlayerName));
	}
	else if (AAO_PlayerState* PS = Cast<AAO_PlayerState>(NewPlayerState))
	{
		PS->OnPlayerNameReady.AddUObject(this, &UAO_NameplateComponent::SetDisplayName_Server);
	}
}

void UAO_NameplateComponent::SetVOIPTalker(UVOIPTalker* InTalker)
{
	AO_LOG(LogJM, Log, TEXT("SetVOIPTalker: %s"), *InTalker->GetName());

	CachedVOIPTalker = InTalker;
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_NameplateComponent::OnRep_DisplayName()
{
	ApplyDisplayNameToWidget();
}

void UAO_NameplateComponent::HandleCapsuleSizeChanged(float NewHalfHeight)
{
	if (!WidgetComponent)
	{
		return;
	}

	CapsuleHeight = NewHalfHeight;
	const float ZOffset = CapsuleHeight + BaseZOffset;
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, ZOffset));
}

void UAO_NameplateComponent::SetDisplayName_Server(const FText& InName)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!DisplayName.EqualTo(InName))
	{
		DisplayName = InName;

		ApplyDisplayNameToWidget();
	}
}

void UAO_NameplateComponent::ApplyDisplayNameToWidget()
{
	EnsureWidgetComponent();

	if (WidgetInstance)
	{
		WidgetInstance->SetPlayerName(DisplayName);
	}
}

void UAO_NameplateComponent::TryInitNameFromOwner()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner)
	{
		return;
	}

	APlayerState* PS = PawnOwner->GetPlayerState();
	if (PS)
	{
		HandlePlayerStateChanged(PS);
	}
}

void UAO_NameplateComponent::EnsureWidgetComponent()
{
	if (WidgetComponent)
	{
		return;
	}

	AActor* Owner = GetOwner();
	checkf(Owner, TEXT("Owner is invalid"));

	WidgetComponent = NewObject<UWidgetComponent>(Owner, TEXT("NameplateWidgetComponent"));
	WidgetComponent->RegisterComponent();
	WidgetComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	if (NameplateWidgetClass)
	{
		WidgetComponent->SetWidgetClass(NameplateWidgetClass);
		WidgetComponent->InitWidget();
	}

	if (UUserWidget* Widget = WidgetComponent->GetUserWidgetObject())
	{
		WidgetInstance = Cast<UAO_NameTagWidget>(Widget);
	}
}

void UAO_NameplateComponent::ApplyDistanceVisuals()
{
	if (!WidgetComponent)
	{
		return;
	}

	// 자기 자신 숨김
	if (ShouldHideForSelf())
	{
		WidgetComponent->SetVisibility(false, true);
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerCameraManager)
	{
		WidgetComponent->SetVisibility(true, true);
		return;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const float Distance = FVector::Distance(CameraLocation, OwnerLocation);

	// 너무 멀면 숨김
	if (Distance > HideDistance)
	{
		WidgetComponent->SetVisibility(false, true);
		return;
	}
	WidgetComponent->SetVisibility(true, true);

	// 멀수록 작아지게
	const float Alpha = FMath::Clamp((Distance - MinScaleDistance) / FMath::Max(1.f, (MaxScaleDistance - MinScaleDistance)), 0.f, 1.f);
	const float UIScale = FMath::Lerp(MaxScale, MinScale, Alpha);

	if (WidgetInstance)
	{
		WidgetInstance->SetUIScale(UIScale);
	}

	// 멀수록 살짝 위로
	const float ZOffset = CapsuleHeight + BaseZOffset + FMath::Lerp(0.f, ExtraZOffset, Alpha);
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, ZOffset));

	// JM :
	UpdateIsTalking();
}

bool UAO_NameplateComponent::ShouldHideForSelf() const
{
	if (!bHideForSelf)
	{
		return false;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());

	return (Pawn && Pawn->IsLocallyControlled());
}

void UAO_NameplateComponent::UpdateIsTalking()
{
	if (!CachedVOIPTalker)
	{
		AO_LOG(LogJM, Warning, TEXT("CachedVOIPTalker is NULL"));
		return;
	}
	
	if (WidgetInstance)
	{
		if (bIsTalking && CachedVOIPTalker->GetVoiceLevel() <= 0.01f)	// JM : 0.01보다 작아져야 스피커 안보이게 함(같은 수치로하면 너무 깜빡깜빡거려서)
		{
			bIsTalking = false;
		}
		else if (!bIsTalking && CachedVOIPTalker->GetVoiceLevel() > 0.02f)
		{
			bIsTalking = true;
		}
		// AO_LOG(LogJM, Log, TEXT("Voice Level: %f"), CachedVOIPTalker->GetVoiceLevel());
		// const bool bIsTalking = (CachedVOIPTalker->GetVoiceLevel() > 0.02f);
		WidgetInstance->SetPlayerTalkingVisibility(bIsTalking);
	}
}

