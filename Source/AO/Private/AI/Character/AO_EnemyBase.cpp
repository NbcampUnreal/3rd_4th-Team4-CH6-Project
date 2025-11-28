// KSJ : AO_EnemyBase.cpp

#include "AI/Character/AO_EnemyBase.h"
#include "AI/GAS/AO_Enemy_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "BrainComponent.h"
#include "AIController.h"
#include "AO_Log.h"

AAO_EnemyBase::AAO_EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAO_Enemy_AttributeSet>(TEXT("AttributeSet"));

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
}

UAbilitySystemComponent* AAO_EnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAO_EnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetMovementSpeedAttribute()).AddUObject(this, &AAO_EnemyBase::OnMovementSpeedChanged);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()).AddUObject(this, &AAO_EnemyBase::OnHealthChanged);
	}
}

void AAO_EnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority() && AbilitySystemComponent)
	{
		InitAbilitySystem();
	}
}

void AAO_EnemyBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AAO_EnemyBase::InitAbilitySystem()
{
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitializeAttributes();
	GiveDefaultAbilities();

	AO_LOG(LogKSJ, Log, TEXT("Enemy GAS Initialized: %s"), *GetName());
}

void AAO_EnemyBase::InitializeAttributes()
{
	if (!AbilitySystemComponent || !DefaultAttributes) return;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AAO_EnemyBase::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority()) return;

	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void AAO_EnemyBase::OnMovementSpeedChanged(const struct FOnAttributeChangeData& Data)
{
	float NewSpeed = Data.NewValue;
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AAO_EnemyBase::OnHealthChanged(const struct FOnAttributeChangeData& Data)
{
	float NewHealth = Data.NewValue;
	
	if (NewHealth <= 0.0f && !bIsStunned)
	{
		OnStunned();
	}
}

void AAO_EnemyBase::OnStunned()
{
	bIsStunned = true;
	AO_LOG(LogKSJ, Log, TEXT("%s is Stunned!"), *GetName());

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Stunned"));
		}
	}
}