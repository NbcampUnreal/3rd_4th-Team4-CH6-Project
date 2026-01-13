//KSJ : AO_AICharacterBase

#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "AI/GAS/AO_AIAttributeSet.h"
#include "AI/GAS/Ability/AO_GA_AI_Stun.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"

AAO_AICharacterBase::AAO_AICharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	NetUpdateFrequency = 30.0f;
	MinNetUpdateFrequency = 15.0f;
	NetPriority = 2.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAO_AIAttributeSet>(TEXT("AttributeSet"));

	MemoryComponent = CreateDefaultSubobject<UAO_AIMemoryComponent>(TEXT("MemoryComponent"));
}

void AAO_AICharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetIsReplicated(true);
		MovementComp->bEnablePhysicsInteraction = false;
		MovementComp->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
		MovementComp->NetworkMaxSmoothUpdateDistance = 92.0f;
		MovementComp->NetworkNoSmoothUpdateDistance = 140.0f;
		
		if (HasAuthority())
		{
			MovementComp->bRunPhysicsWithNoController = true;
		}
		else
		{
			MovementComp->bRunPhysicsWithNoController = false;
			MovementComp->bAlwaysCheckFloor = false;
			MovementComp->bSweepWhileNavWalking = false;
			MovementComp->MaxDepenetrationWithGeometry = 0.0f;
			MovementComp->MaxDepenetrationWithPawn = 0.0f;
		}
	}

	if (!HasAuthority())
	{
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
	}

	if (HasAuthority())
	{
		InitializeAbilitySystem();
		
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
		}
	}
}

void AAO_AICharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAbilitySystemComponent* AAO_AICharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AAO_AICharacterBase::IsStunned() const
{
	if (!ensureMsgf(AbilitySystemComponent, TEXT("AbilitySystemComponent is null on %s"), *GetName()))
	{
		return false;
	}

	const FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned"));
	return AbilitySystemComponent->HasMatchingGameplayTag(StunnedTag);
}

void AAO_AICharacterBase::OnStunBegin()
{
	HandleStunBegin();
}

void AAO_AICharacterBase::OnStunEnd()
{
	HandleStunEnd();
}

void AAO_AICharacterBase::HandleStunBegin()
{
	// 서버에서만 이동 중지 처리
	if (HasAuthority())
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
		}
	}
	// 기절 시작 시 추가 처리는 자식 클래스에서 오버라이드
}

void AAO_AICharacterBase::HandleStunEnd()
{
	// 기절 종료 시 추가 처리는 자식 클래스에서 오버라이드
}

void AAO_AICharacterBase::Multicast_PlayStunMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// 이미 재생 중이면 무시
	if (!AnimInstance->Montage_IsPlaying(Montage))
	{
		AnimInstance->Montage_Play(Montage, PlayRate);
		CurrentStunMontage = Montage;
	}
}

void AAO_AICharacterBase::Multicast_StopStunMontage_Implementation(float BlendOutTime)
{
	if (!CurrentStunMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (AnimInstance->Montage_IsPlaying(CurrentStunMontage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, CurrentStunMontage);
	}

	CurrentStunMontage = nullptr;
}

void AAO_AICharacterBase::TestStun()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayTag StunEventTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = this;
	
	AbilitySystemComponent->HandleGameplayEvent(StunEventTag, &EventData);
}

void AAO_AICharacterBase::TestStunEnd()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayTagContainer StunAbilityTags;
	StunAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Stunned")));
	
	AbilitySystemComponent->CancelAbilities(&StunAbilityTags);
}

FEnemyAttackConfig AAO_AICharacterBase::GetCurrentAttackConfig_Implementation() const
{
	return FEnemyAttackConfig();
}

void AAO_AICharacterBase::SetIsAttacking(bool bAttacking)
{
	bIsAttacking = bAttacking;
}

void AAO_AICharacterBase::InitializeAbilitySystem()
{
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	BindDefaultAbilities();
	BindDefaultEffects();
}

void AAO_AICharacterBase::BindDefaultAbilities()
{
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void AAO_AICharacterBase::BindDefaultEffects()
{
	if (!ensure(AbilitySystemComponent))
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			Context.AddInstigator(this, this);

			FGameplayEffectSpecHandle Handle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Handle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
			}
		}
	}
}

