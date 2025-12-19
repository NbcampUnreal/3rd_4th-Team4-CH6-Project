// AO_AICharacterBase.cpp

#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "AI/GAS/AO_AIAttributeSet.h"
#include "AI/GAS/Ability/AO_GA_AI_Stun.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

AAO_AICharacterBase::AAO_AICharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Attribute Set 생성
	AttributeSet = CreateDefaultSubobject<UAO_AIAttributeSet>(TEXT("AttributeSet"));

	// Memory 컴포넌트 생성
	MemoryComponent = CreateDefaultSubobject<UAO_AIMemoryComponent>(TEXT("MemoryComponent"));
}

void AAO_AICharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeAbilitySystem();
	}

	// 기본 이동 속도 설정
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
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
	// AbilitySystemComponent가 없으면 기절 상태가 아닌 것으로 처리
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
	// 기본 기절 처리: 이동 멈춤
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	AO_LOG(LogKSJ, Log, TEXT("AI Stunned: %s"), *GetName());
}

void AAO_AICharacterBase::HandleStunEnd()
{
	// 기절 해제 처리
	AO_LOG(LogKSJ, Log, TEXT("AI Stun Ended: %s"), *GetName());
}

void AAO_AICharacterBase::TestStun()
{
	if (!AbilitySystemComponent)
	{
		AO_LOG(LogKSJ, Warning, TEXT("TestStun: AbilitySystemComponent is null on %s"), *GetName());
		return;
	}

	// Event.AI.Stunned 이벤트 발생
	FGameplayTag StunEventTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = this;
	
	AbilitySystemComponent->HandleGameplayEvent(StunEventTag, &EventData);
	
	AO_LOG(LogKSJ, Log, TEXT("TestStun: Triggered stun event on %s"), *GetName());
}

void AAO_AICharacterBase::TestStunEnd()
{
	if (!AbilitySystemComponent)
	{
		AO_LOG(LogKSJ, Warning, TEXT("TestStunEnd: AbilitySystemComponent is null on %s"), *GetName());
		return;
	}

	// 기절 Ability 태그로 취소 (프로젝트 내 다른 코드와 동일한 패턴)
	FGameplayTagContainer StunAbilityTags;
	StunAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Stunned")));
	
	AbilitySystemComponent->CancelAbilities(&StunAbilityTags);
	
	AO_LOG(LogKSJ, Log, TEXT("TestStunEnd: Cancelled stun ability on %s"), *GetName());
}

void AAO_AICharacterBase::InitializeAbilitySystem()
{
	// GAS 초기화 - AbilitySystemComponent가 반드시 존재해야 함
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
	// 기본 Ability 바인딩 - AbilitySystemComponent 필수
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
	// 기본 Effect 바인딩 - AbilitySystemComponent 필수
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
