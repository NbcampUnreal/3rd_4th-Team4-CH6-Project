// KSJ : AO_Enemy_Insect.cpp

#include "AI/Character/AO_Enemy_Insect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AI/Manager/AO_Manager_InsectKidnapManager.h"
#include "AI/GAS/Effect/AO_GameplayEffect_InsectDamage.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

AAO_Enemy_Insect::AAO_Enemy_Insect()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAO_Enemy_Insect::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAO_Enemy_Insect, KidnappedPlayer);
}

void AAO_Enemy_Insect::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Enemy_Insect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && bIsKidnapping)
	{
		InternalEndKidnap(false);
	}
	
	UWorld* World = GetWorld();
	if (World && KidnapDamageTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(KidnapDamageTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AAO_Enemy_Insect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (KidnappedPlayer)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (ACharacter* PlayerChar = Cast<ACharacter>(KidnappedPlayer))
			{
				if (!PlayerChar->GetAttachParentActor())
				{
					bool bReAttached = PlayerChar->AttachToComponent(
						MeshComp,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,
						PickupSocketName
					);
					
					if (bReAttached)
					{
						AO_LOG(LogKSJ, Warning, TEXT("[Insect] [%s] Re-attached player to socket"), 
							HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
					}
					else
					{
						AO_LOG(LogKSJ, Error, TEXT("[Insect] [%s] Failed to re-attach player to socket"), 
							HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
						
						if (HasAuthority())
						{
							EndKidnap(false);
							return;
						}
					}
				}
			}
		}
	}
	
	if (HasAuthority())
	{
		if (bIsKidnapping && KidnappedPlayer)
		{
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - KidnapStartTime >= MaxKidnapDuration)
			{
				AO_LOG(LogKSJ, Log, TEXT("[Insect] Max kidnap duration reached, forcing end"));
				EndKidnap(true);
			}
		}
	}
}

void AAO_Enemy_Insect::StartKidnap(AActor* PlayerActor)
{
	if (!HasAuthority() || !PlayerActor || bIsKidnapping) return;
	
	if (UWorld* World = GetWorld())
	{
		if (UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>())
		{
			if (!Manager->CanKidnapPlayer(PlayerActor))
			{
				AO_LOG(LogKSJ, Warning, TEXT("[Insect] Cannot kidnap %s (cooldown or already kidnapped)"), *PlayerActor->GetName());
				return;
			}
			
			Manager->RegisterKidnap(PlayerActor);
		}
	}
	
	KidnappedPlayer = PlayerActor;
	bIsKidnapping = true;
	KidnapStartLocation = GetActorLocation();
	KidnapStartTime = FPlatformTime::Seconds();
	
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerActor))
		{
			if (!MeshComp->DoesSocketExist(PickupSocketName))
			{
				AO_LOG(LogKSJ, Error, TEXT("[Insect] Socket '%s' does not exist on mesh!"), *PickupSocketName.ToString());
				bIsKidnapping = false;
				KidnappedPlayer = nullptr;
				return;
			}
			
			if (UCharacterMovementComponent* PlayerMovement = PlayerChar->GetCharacterMovement())
			{
				PlayerMovement->MaxWalkSpeed = 0.0f;
				PlayerMovement->NavAgentProps.bCanJump = false;
				PlayerMovement->StopMovementImmediately();
				
				AO_LOG(LogKSJ, Log, TEXT("[Insect] Movement and jump disabled for %s (camera rotation allowed)"), *PlayerActor->GetName());
			}
			
			if (AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(PlayerChar))
			{
				if (USpringArmComponent* SpringArm = PlayerCharacter->GetSpringArm())
				{
					SpringArm->bDoCollisionTest = false;
					AO_LOG(LogKSJ, Log, TEXT("[Insect] Camera collision disabled for %s"), *PlayerActor->GetName());
				}
			}
			
			bool bAttached = PlayerChar->AttachToComponent(
				MeshComp,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				PickupSocketName
			);
			
			if (bAttached)
			{
				AO_LOG(LogKSJ, Log, TEXT("[Insect] Player attached to socket: %s"), *PickupSocketName.ToString());
			}
			else
			{
				AO_LOG(LogKSJ, Error, TEXT("[Insect] Failed to attach player to socket: %s"), *PickupSocketName.ToString());
				if (UCharacterMovementComponent* PlayerMovement = PlayerChar->GetCharacterMovement())
				{
					PlayerMovement->MaxWalkSpeed = 500.0f;
					PlayerMovement->NavAgentProps.bCanJump = true;
				}
				bIsKidnapping = false;
				KidnappedPlayer = nullptr;
				return;
			}
		}
	}
	
	SetBackwardMovement(true);
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			KidnapDamageTimerHandle,
			this,
			&AAO_Enemy_Insect::ApplyKidnapDamage,
			KidnapDamageInterval,
			true
		);
	}
	
	AO_LOG(LogKSJ, Log, TEXT("[Insect] Started kidnapping %s"), *PlayerActor->GetName());
}

void AAO_Enemy_Insect::EndKidnap(bool bApplyKnockback)
{
	if (!HasAuthority() || !bIsKidnapping) return;
	
	InternalEndKidnap(bApplyKnockback);
}

void AAO_Enemy_Insect::InternalEndKidnap(bool bApplyKnockback)
{
	if (!KidnappedPlayer) return;
	
	AActor* PlayerToDrop = KidnappedPlayer;
	
	if (UWorld* World = GetWorld())
	{
		if (UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>())
		{
			Manager->UnregisterKidnap(PlayerToDrop);
			Manager->RegisterKidnapCooldown(PlayerToDrop);
		}
	}
	
	UWorld* World = GetWorld();
	if (World && KidnapDamageTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(KidnapDamageTimerHandle);
	}
	
	if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerToDrop))
	{
		PlayerChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		if (UCharacterMovementComponent* PlayerMovement = PlayerChar->GetCharacterMovement())
		{
			PlayerMovement->MaxWalkSpeed = 500.0f;
			PlayerMovement->NavAgentProps.bCanJump = true;
			
			if (bApplyKnockback)
			{
				FVector KnockbackDirection = (PlayerToDrop->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				KnockbackDirection.Z = 0.3f;
				PlayerMovement->Velocity = KnockbackDirection * KnockbackStrength;
			}
			else
			{
				PlayerMovement->Velocity = FVector::ZeroVector;
			}
		}
		
		if (AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(PlayerToDrop))
		{
			if (USpringArmComponent* SpringArm = PlayerCharacter->GetSpringArm())
			{
				SpringArm->bDoCollisionTest = true;
				AO_LOG(LogKSJ, Log, TEXT("[Insect] Camera collision restored for %s"), *PlayerToDrop->GetName());
			}
		}
		
		AO_LOG(LogKSJ, Log, TEXT("[Insect] Player physics restored for %s"), *PlayerToDrop->GetName());
	}
	
	SetBackwardMovement(false);
	
	KidnappedPlayer = nullptr;
	bIsKidnapping = false;
	
	AO_LOG(LogKSJ, Log, TEXT("[Insect] Ended kidnapping %s"), *PlayerToDrop->GetName());
}

void AAO_Enemy_Insect::SetBackwardMovement(bool bIsBackward)
{
	if (bIsBackward)
	{
		GetCharacterMovement()->MaxWalkSpeed = KidnapMoveSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}
}

void AAO_Enemy_Insect::OnRep_KidnappedPlayer()
{
	if (KidnappedPlayer)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (ACharacter* PlayerChar = Cast<ACharacter>(KidnappedPlayer))
			{
				if (!PlayerChar->GetAttachParentActor())
				{
					bool bAttached = PlayerChar->AttachToComponent(
						MeshComp,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,
						PickupSocketName
					);
					
					if (bAttached)
					{
						AO_LOG(LogKSJ, Log, TEXT("[Insect] [CLIENT] Player attached to socket: %s"), *PickupSocketName.ToString());
					}
					else
					{
						AO_LOG(LogKSJ, Warning, TEXT("[Insect] [CLIENT] Failed to attach player to socket: %s"), *PickupSocketName.ToString());
					}
				}
			}
		}
	}
	else
	{
		AO_LOG(LogKSJ, Log, TEXT("[Insect] [CLIENT] KidnappedPlayer cleared"));
	}
}

void AAO_Enemy_Insect::ApplyKidnapDamage()
{
	if (!HasAuthority() || !KidnappedPlayer || !AbilitySystemComponent) return;
	
	if (!KidnapDamageEffectClass)
	{
		AO_LOG(LogKSJ, Error, TEXT("[Insect] KidnapDamageEffectClass is NULL!"));
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(KidnappedPlayer);
	if (!TargetASC) return;
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(KidnapDamageEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(FName("Data.Damage")), 
			-KidnapDamageAmount
		);
		
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		
		AO_LOG(LogKSJ, Verbose, TEXT("[Insect] Applied kidnap damage to %s"), *KidnappedPlayer->GetName());
	}
}
