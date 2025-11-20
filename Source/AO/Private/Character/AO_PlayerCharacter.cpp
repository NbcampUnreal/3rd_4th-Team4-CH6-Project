// AO_PlayerCharacter.cpp

#include "Character/AO_PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interaction/Component/AO_InteractionComponent.h"
#include "Item/invenroty/AO_InventoryComponent.h"

AAO_PlayerCharacter::AAO_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = 400.f;
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = false;
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	// For Crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;

	// 승조 : AbilitySystemComponent 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 승조 : InteractionComponent 생성
	InteractionComponent = CreateDefaultSubobject<UAO_InteractionComponent>(TEXT("InteractionComponent"));

	//ms: inventory component
	InventoryComp = CreateDefaultSubobject<UAO_InventoryComponent>(TEXT("InventoryComponent"));
}

UAbilitySystemComponent* AAO_PlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAO_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 승조 : ASC 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(IMC_Player, 0);
			}
		}
	}
}

void AAO_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::Look);
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AAO_PlayerCharacter::StartSprint);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AAO_PlayerCharacter::StopSprint);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AAO_PlayerCharacter::HandleCrouch);
		EIC->BindAction(IA_Walk, ETriggerEvent::Started, this, &AAO_PlayerCharacter::HandleWalk);
		
		//ms_inventory key binding
		EIC->BindAction(Select_inventory_Slot, ETriggerEvent::Started, this, &AAO_PlayerCharacter::SelectInventorySlot);
		
	}
	
	// 승조 : InteractionComponent에서 Interaction 따로 바인딩
	if (InteractionComponent)
	{
		InteractionComponent->SetupInputBinding(PlayerInputComponent);
	}
}

void AAO_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAO_PlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	Super::EndPlay(EndPlayReason);
}

void AAO_PlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_PlayerCharacter, Gait);
	DOREPLIFETIME(AAO_PlayerCharacter, LandVelocity);
	DOREPLIFETIME(AAO_PlayerCharacter, bJustLanded);
}

void AAO_PlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (HasAuthority())
	{
		LandVelocity = GetCharacterMovement()->Velocity;
		bJustLanded = true;

		GetWorldTimerManager().ClearTimer(TimerHandle_JustLanded);
		GetWorldTimerManager().SetTimer(TimerHandle_JustLanded, [this]()
		{
			bJustLanded = false;
		}, 0.3f, false);
	}
}

void AAO_PlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();

	if (GetController())
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, InputValue.Y);
		AddMovementInput(RightDirection, InputValue.X);
	}
}

void AAO_PlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(InputValue.X);
		AddControllerPitchInput(InputValue.Y);
	}
}

void AAO_PlayerCharacter::StartSprint()
{
	CharacterInputState.bWantsToSprint = true;
	CharacterInputState.bWantsToWalk = false;
	SetCurrentGait();

	if (!HasAuthority())
	{
		ServerRPC_SetInputState(CharacterInputState.bWantsToSprint, CharacterInputState.bWantsToWalk);
	}
}

void AAO_PlayerCharacter::StopSprint()
{
	CharacterInputState.bWantsToSprint = false;
	CharacterInputState.bWantsToWalk = false;
	SetCurrentGait();
	
	if (!HasAuthority())
	{
		ServerRPC_SetInputState(CharacterInputState.bWantsToSprint, CharacterInputState.bWantsToWalk);
	}
}

void AAO_PlayerCharacter::HandleWalk()
{
	if (CharacterInputState.bWantsToSprint)
	{
		return;
	}

	CharacterInputState.bWantsToWalk = !CharacterInputState.bWantsToWalk;
	SetCurrentGait();
	
	if (!HasAuthority())
	{
		ServerRPC_SetInputState(CharacterInputState.bWantsToSprint, CharacterInputState.bWantsToWalk);
	}
}

void AAO_PlayerCharacter::HandleCrouch()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (!CharacterMovementComponent || CharacterMovementComponent->IsFalling())
	{
		return;
	}

	if (IsCrouched())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AAO_PlayerCharacter::SetCurrentGait()
{
	if (CharacterInputState.bWantsToSprint)
	{
		Gait = EGait::Sprint;
	}
	else if (CharacterInputState.bWantsToWalk)
	{
		Gait = EGait::Walk;
	}
	else
	{
		Gait = EGait::Run;
	}

	OnRep_Gait();
}

void AAO_PlayerCharacter::ServerRPC_SetInputState_Implementation(bool bWantsToSprint, bool bWantsToWalk)
{
	CharacterInputState.bWantsToSprint = bWantsToSprint;
	CharacterInputState.bWantsToWalk = bWantsToWalk;

	SetCurrentGait();
}



void AAO_PlayerCharacter::OnRep_Gait()
{
	switch (Gait)
	{
	case EGait::Walk:
		GetCharacterMovement()->MaxWalkSpeed = 200.f;
		break;
	case EGait::Run:
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		break;
	case EGait::Sprint:
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
		break;
	}
}

//ms_inventory key binding
void AAO_PlayerCharacter::SelectInventorySlot(const FInputActionValue& Value)
{
	/*int32 SlotIndex = Value.Get<int32>();

	if (InventoryComp)
	{
		InventoryComp->ServerSetSelectedSlot(SlotIndex);
	}*/
}

