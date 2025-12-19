// AO_PlayerCharacter.cpp

#include "Character/AO_PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "MotionWarpingComponent.h"
#include "Character/Components/AO_DeathSpectateComponent.h"
#include "Character/Customizing/AO_CustomizingComponent.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeDefaults.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Interaction/Component/AO_InteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Item/invenroty/AO_InventoryComponent.h"
#include "Item/invenroty/AO_InputModifier.h"
#include "MuCO/CustomizableSkeletalComponent.h"

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

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = 400.f;
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = false;
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// JM : VOIP Talker
	VOIPTalker = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));
	VOIPTalker->Settings.ComponentToAttachTo =  GetMesh();
	
	// For Crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UAO_PlayerCharacter_AttributeSet>(TEXT("AttributeSet"));

	InteractionComponent = CreateDefaultSubobject<UAO_InteractionComponent>(TEXT("InteractionComponent"));
	InspectionComponent = CreateDefaultSubobject<UAO_InspectionComponent>(TEXT("InspectionComponent"));
	InteractableComponent = CreateDefaultSubobject<UAO_InteractableComponent>(TEXT("InteractableComponent"));
	InteractableComponent->bInteractionEnabled = false;
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	DeathSpectateComponent = CreateDefaultSubobject<UAO_DeathSpectateComponent>(TEXT("DeathSpectateComponent"));
	InventoryComp = CreateDefaultSubobject<UAO_InventoryComponent>(TEXT("InventoryComponent"));
	PassiveComp = CreateDefaultSubobject<UAO_PassiveComponent>(TEXT("PassiveComponent"));

	//세훈: Customizable Object Instance
	BaseSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BaseSkeletalMesh"));
	BaseSkeletalMesh->SetupAttachment(GetMesh());
	
	BodySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodySkeletalMesh"));
	BodySkeletalMesh->SetupAttachment(BaseSkeletalMesh);
	BodyComponent = CreateDefaultSubobject<UCustomizableSkeletalComponent>(TEXT("BodyComponent"));
	BodyComponent->SetupAttachment(BodySkeletalMesh);
	BodyComponent->SetComponentName(FName("Body"));
	BodyComponent->SetIsReplicated(true);
	
	HeadSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadSkeletalMesh"));
	HeadSkeletalMesh->SetupAttachment(BaseSkeletalMesh);
	HeadComponent = CreateDefaultSubobject<UCustomizableSkeletalComponent>(TEXT("HeadComponent"));
	HeadComponent->SetupAttachment(HeadSkeletalMesh);
	HeadComponent->SetComponentName(FName("Head"));
	HeadComponent->SetIsReplicated(true);

	CustomizingComponent = CreateDefaultSubobject<UAO_CustomizingComponent>(TEXT("CustomizingComponent"));
	CustomizingComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AAO_PlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAO_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	checkf(AbilitySystemComponent, TEXT("AbilitySystemComponent is null"));

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	if (HasAuthority())
	{
		InitializeAttributes();
		
		BindGameplayAbilities();
		
		BindGameplayEffects();
		
		BindAttributeDelegates();
	}
}

UAO_FoleyAudioBank* AAO_PlayerCharacter::GetFoleyAudioBank_Implementation() const
{
	return DefaultFoleyAudioBank;
}

bool AAO_PlayerCharacter::CanPlayFootstepSounds_Implementation() const
{
	if (GetCharacterMovement()->IsMovingOnGround()
		|| AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Traversal"))))
	{
		return true;
	}
	return false;
}

void AAO_PlayerCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	if (IsLocallyControlled())
	{
		if (Camera)
		{
			Camera->GetCameraView(DeltaTime, OutResult);
		}
		else
		{
			Super::CalcCamera(DeltaTime, OutResult);
		}
		return;
	}

	if (DeathSpectateComponent)
	{
		FRepCameraView V;
		DeathSpectateComponent->GetRepCameraView(V);
		OutResult.Location = V.Location;
		OutResult.Rotation = V.Rotation;
		OutResult.FOV	   = V.FOV;
		return;
	}
}

bool AAO_PlayerCharacter::IsInspecting() const
{
	return InspectionComponent && InspectionComponent->IsInspecting();
}

void AAO_PlayerCharacter::StartSprint_GAS(bool bShouldSprint)
{
	if (bShouldSprint)
	{
		CharacterInputState.bWantsToSprint = true;
		CharacterInputState.bWantsToWalk = false;
	}
	else
	{
		CharacterInputState.bWantsToSprint = false;
	}

	SetCurrentGait();

	if (!HasAuthority())
	{
		ServerRPC_SetInputState(CharacterInputState.bWantsToSprint, CharacterInputState.bWantsToWalk);
	}
}

void AAO_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	BindSpeedAttributeDelegates();

	// HSJ : InteractableComponent 델리게이트 바인딩
	if (HasAuthority() && InteractableComponent)
	{
		InteractableComponent->OnInteractionSuccess.AddDynamic(this, &AAO_PlayerCharacter::HandleInteractableComponentSuccess);
	}

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(IMC_Player, 0);
			}

			// HSJ : Inspection 모드 상태일 때 레벨 전환 시 카메라 움직이지 않는 문제 해결
			PC->bShowMouseCursor = false;
			// GameOnly 모드로 설정
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}
	}

	// JM : VOIPTalker PS 에 연결될 때까지 연결 시도
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			VOIPRegisterToPSTimerHandle,
			this,
			&ThisClass::TryRegisterVoiceTalker,
			0.2f,
			true
		);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No World"));
	}
}

void AAO_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::Look);
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AAO_PlayerCharacter::StartJump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::TriggerJump);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AAO_PlayerCharacter::HandleCrouch);
		EIC->BindAction(IA_Walk, ETriggerEvent::Started, this, &AAO_PlayerCharacter::HandleWalk);

		if (IsValid(AbilitySystemComponent))
		{
			EIC->BindAction(IA_Sprint, ETriggerEvent::Triggered, this, &AAO_PlayerCharacter::HandleGameplayAbilityInputPressed, 1);
			EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AAO_PlayerCharacter::HandleGameplayAbilityInputReleased, 1);
		}			
	}
	
	// 승조 : InteractionComponent에서 Interaction 따로 바인딩
	if (InteractionComponent)
	{
		InteractionComponent->SetupInputBinding(PlayerInputComponent);
	}
	if (InspectionComponent)
	{
		InspectionComponent->SetupInputBinding(PlayerInputComponent);
	}
	// ms : inventoryComp 바인딩
	if (InventoryComp)
	{
		InventoryComp->SetupInputBinding(PlayerInputComponent);
	}
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

	PlayAudioEvent(
		FGameplayTag::RequestGameplayTag(FName("Foley.Event.Land")),
		UKismetMathLibrary::MapRangeClamped(GetCharacterMovement()->Velocity.Z, -500.f, -900.f, 0.5f, 1.5f),
		1.f);
		
	if (HasAuthority())
	{
		LandVelocity = GetCharacterMovement()->Velocity;
		bJustLanded = true;

		FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			bJustLanded = false;
		});

		GetWorldTimerManager().ClearTimer(TimerHandle_JustLanded);
		GetWorldTimerManager().SetTimer(TimerHandle_JustLanded, TimerDelegate, 0.3f, false);
	}
}

void AAO_PlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	PlayAudioEvent(
		FGameplayTag::RequestGameplayTag(FName("Foley.Event.Jump")),
		UKismetMathLibrary::MapRangeClamped(GetCharacterMovement()->Velocity.Size2D(), 0.f, 500.f, 0.5f, 1.0f),
		1.f);
}

void AAO_PlayerCharacter::Move(const FInputActionValue& Value)
{
	// 승조 : Inspection 중이면 입력 차단
	if (IsInspecting())
	{
		return;
	}
	
	const FVector2D InputValue = Value.Get<FVector2D>();

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
	// 승조 : Inspection 중이면 카메라 회전 차단
	if (IsInspecting())
	{
		return;
	}
	
	const FVector2D InputValue = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(InputValue.X);
		AddControllerPitchInput(InputValue.Y);
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

void AAO_PlayerCharacter::StartJump()
{
	if (AbilitySystemComponent->TryActivateAbilitiesByTag(
		FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Traversal")))))
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(
		FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Jump"))));
}

void AAO_PlayerCharacter::TriggerJump()
{
	AbilitySystemComponent->TryActivateAbilitiesByTag(
		FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Traversal"))));
}

void AAO_PlayerCharacter::HandleGameplayAbilityInputPressed(int32 InInputID)
{
	if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InInputID))
	{
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			AbilitySystemComponent->TryActivateAbility(Spec->Handle);
		}
	}
}

void AAO_PlayerCharacter::HandleGameplayAbilityInputReleased(int32 InInputID)
{
	if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InInputID))
	{
		Spec->InputPressed = false;
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
		}
	}
}

void AAO_PlayerCharacter::HandleCrouch()
{
	checkf(GetCharacterMovement(), TEXT("CharacterMovement is null"));
	
	if (GetCharacterMovement()->IsFalling())
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

void AAO_PlayerCharacter::PlayAudioEvent(FGameplayTag Value, float VolumeMultiplier, float PitchMultiplier)
{
	TObjectPtr<UAO_FoleyAudioBank> FoleyAudioBank = Execute_GetFoleyAudioBank(this);
	checkf(FoleyAudioBank, TEXT("FoleyAudioBank is null"));

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FoleyAudioBank->GetSoundFromFoleyEvent(Value),
		GetActorLocation(),
		FRotator::ZeroRotator,
		VolumeMultiplier,
		PitchMultiplier);
}

void AAO_PlayerCharacter::InitializeAttributes()
{
	checkf(AttributeDefaults, TEXT("AttributeDefaults is null"));
	checkf(AttributeSet, TEXT("AttributeSet is null"));
	
	if (!HasAuthority())
	{
		return;
	}

	AttributeSet->InitHealth(AttributeDefaults->Health);
	AttributeSet->InitMaxHealth(AttributeDefaults->MaxHealth);

	AttributeSet->InitStamina(AttributeDefaults->Stamina);
	AttributeSet->InitMaxStamina(AttributeDefaults->MaxStamina);

	AttributeSet->InitWalkSpeed(AttributeDefaults->WalkSpeed);
	AttributeSet->InitRunSpeed(AttributeDefaults->RunSpeed);
	AttributeSet->InitSprintSpeed(AttributeDefaults->SprintSpeed);
}

void AAO_PlayerCharacter::BindGameplayAbilities()
{
	for (const auto& DefaultAbility : DefaultAbilities)
    {
    	FGameplayAbilitySpec AbilitySpec(DefaultAbility);
    	AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
 
    for (const auto& InputAbility : InputAbilities)
    {
    	FGameplayAbilitySpec AbilitySpec(InputAbility.Value);
    	AbilitySpec.InputID = InputAbility.Key;
    	AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
}

void AAO_PlayerCharacter::BindGameplayEffects()
{
	for (const auto& DefaultEffect : DefaultEffects)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddInstigator(this, this);

		FGameplayEffectSpecHandle Handle = AbilitySystemComponent->MakeOutgoingSpec(DefaultEffect, 1.f, Context);

		if (Handle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
		}
	}
}

void AAO_PlayerCharacter::BindAttributeDelegates()
{
	checkf(AttributeSet, TEXT("AttributeSet is null"));
}

void AAO_PlayerCharacter::BindSpeedAttributeDelegates()
{
	checkf(AttributeSet, TEXT("AttributeSet is null"));
	
	// 이동속도 변경 시 발생할 델리게이트 연결
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetWalkSpeedAttribute())
		.AddUObject(this, &AAO_PlayerCharacter::OnSpeedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetRunSpeedAttribute())
		.AddUObject(this, &AAO_PlayerCharacter::OnSpeedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetSprintSpeedAttribute())
		.AddUObject(this, &AAO_PlayerCharacter::OnSpeedChanged);
}

void AAO_PlayerCharacter::OnSpeedChanged(const FOnAttributeChangeData& Data)
{
	SetCurrentGait();
	
	if (!HasAuthority())
	{
		ServerRPC_SetInputState(CharacterInputState.bWantsToSprint, CharacterInputState.bWantsToWalk);
	}
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
		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetWalkSpeed();
		break;
	case EGait::Run:
		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetRunSpeed();
		break;
	case EGait::Sprint:
		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetSprintSpeed();
		break;
	}
}

void AAO_PlayerCharacter::HandleInteractableComponentSuccess(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	// HSJ : 상호작용 비활성화
	if (InteractableComponent)
	{
		InteractableComponent->bInteractionEnabled = false;
	}
}

void AAO_PlayerCharacter::TryRegisterVoiceTalker()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (APlayerState* PS = GetPlayerState())
	{
		if (AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PS))
		{
			GetWorld()->GetTimerManager().ClearTimer(VOIPRegisterToPSTimerHandle);
			RegisterVoiceTalker();
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("Cast Failed to AO_PS"));	
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No PS Yet"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerCharacter::RegisterVoiceTalker()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (VOIPTalker)
	{
		if (AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(GetPlayerState()))
		{
			VOIPTalker->RegisterWithPlayerState(AO_PS);
			AO_LOG(LogJM, Log, TEXT("RegisterWithPlayerState Called"));
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("Cast Failed to AO_PS"));			
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No VOIPTalker"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}