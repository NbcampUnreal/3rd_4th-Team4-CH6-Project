// AO_NameplateComponent.cpp

#include "Character/Components/AO_NameplateComponent.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerState/AO_PlayerState.h"
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

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TryInitNameFromOwner();
	}
	else
	{
		ApplyDisplayNameToWidget();
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

void UAO_NameplateComponent::OnRep_DisplayName()
{
	ApplyDisplayNameToWidget();
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
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseZOffset));

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

