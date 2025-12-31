// AO_NameplateComponent.cpp

#include "Character/Components/AO_NameplateComponent.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Player/AO_NameTagWidget.h"

UAO_NameplateComponent::UAO_NameplateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAO_NameplateComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureWidgetComponent();

	ApplyDistanceVisuals();
}

void UAO_NameplateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CachedPlayerState.Reset();
	Super::EndPlay(EndPlayReason);
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

void UAO_NameplateComponent::SetDisplayName(const FText& InName)
{
	EnsureWidgetComponent();
	if (WidgetInstance)
	{
		WidgetInstance->SetPlayerName(InName);
	}
}

void UAO_NameplateComponent::HandlePlayerStateChanged(APlayerState* NewPlayerState)
{
	CachedPlayerState = NewPlayerState;

	if (AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(NewPlayerState))
	{
		RefreshNameFromPlayerState();

		AO_PS->OnPlayerNameReady.AddUObject(this, &UAO_NameplateComponent::SetDisplayName);
	}
	else
	{
		RefreshNameFromPlayerState();
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
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseZOffset));

	if (NameplateWidgetClass)
	{
		WidgetComponent->SetWidgetClass(NameplateWidgetClass);
	}

	if (UUserWidget* Widget = WidgetComponent->GetUserWidgetObject())
	{
		WidgetInstance = Cast<UAO_NameTagWidget>(Widget);
	}
}

void UAO_NameplateComponent::RefreshNameFromPlayerState()
{
	APlayerState* PS = CachedPlayerState.Get();
	if (ensure(!PS))
	{
		return;
	}

	const FString PlayerName = PS->GetPlayerName();
	if (!PlayerName.IsEmpty())
	{
		SetDisplayName(FText::FromString(PlayerName));
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
	const float ZOffset = BaseZOffset + FMath::Lerp(0.f, ExtraZOffset, Alpha);
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, ZOffset));
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

