// AO_PlayerCharacter.cpp

#include "Character/AO_PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

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
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;
}

void AAO_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

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
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
	}
}

void AAO_PlayerCharacter::StopSprint()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
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
		UE_LOG(LogTemp, Warning, TEXT("Uncrouch"));
		UnCrouch();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Crouch"));
		Crouch();
	}
}
