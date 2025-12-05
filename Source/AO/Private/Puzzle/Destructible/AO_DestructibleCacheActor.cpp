// HSJ : AO_DestructibleCacheActor.cpp
#include "Puzzle/Destructible/AO_DestructibleCacheActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"

AAO_DestructibleCacheActor::AAO_DestructibleCacheActor()
{
	bReplicates = true;
	
	GeoComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeoComp"));
	RootComponent = GeoComp;
	GeoComp->SetSimulatePhysics(false);
	GeoComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// 기본 TriggerTag 설정
	TriggerTag = FGameplayTag::RequestGameplayTag(FName("Effect.Destruction.Triggered"));
}

void AAO_DestructibleCacheActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAO_DestructibleCacheActor, bIsDestroyed);
}

UAbilitySystemComponent* AAO_DestructibleCacheActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAO_DestructibleCacheActor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (!GeoComp)
	{
		AO_LOG_NET(LogHSJ, Error, TEXT("GeoComp is null"));
		return;
	}
	
	// 카오스 캐시 매니저가 GeoComp 관찰 시작
	FindOrAddObservedComponent(GeoComp, NAME_None, false);

	// ASC 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// TriggerTag 변경 감지 등록
		if (TriggerTag.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(
				TriggerTag,
				EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &AAO_DestructibleCacheActor::OnTagChanged);
		}
		else
		{
			AO_LOG_NET(LogHSJ, Warning, TEXT("TriggerTag is invalid"));
		}
	}
}

void AAO_DestructibleCacheActor::TriggerDestruction()
{
	if (bIsDestroyed)
	{
		return;
	}

	if (HasAuthority())
	{
		// 서버: 즉시 파괴 실행
		bIsDestroyed = true;
		ExecuteDestruction();
	}
	else
	{
		// 클라이언트: 서버에 요청
		ServerTriggerDestruction();
	}
}

void AAO_DestructibleCacheActor::ServerTriggerDestruction_Implementation()
{
	TriggerDestruction();
}

void AAO_DestructibleCacheActor::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// TriggerTag가 추가되고 아직 파괴 안 됐으면 실행
	if (HasAuthority() && Tag.MatchesTagExact(TriggerTag) && NewCount > 0 && !bIsDestroyed)
	{
		//AO_LOG_NET(LogHSJ, Warning, TEXT("TriggerTag added, executing destruction: %s"), *Tag.ToString());
		bIsDestroyed = true;
		ExecuteDestruction();
	}
}

void AAO_DestructibleCacheActor::OnRep_IsDestroyed()
{
	// 클라이언트에서 파괴 상태 복제 받음
	if (bIsDestroyed)
	{
		//AO_LOG_NET(LogHSJ, Log, TEXT("Replicated destruction state, executing"));
		ExecuteDestruction();
	}
}

void AAO_DestructibleCacheActor::ExecuteDestruction()
{
	checkf(GeoComp, TEXT("GeoComp is null in ExecuteDestruction"));

	// 카오스 캐시 매니저의 Trigger 모드 실행 (녹화된 파괴 재생)
	TriggerComponent(GeoComp);

	// 삭제 타이머 설정
	if (HasAuthority() && DestroyDelay > 0.0f)
	{
		TObjectPtr<UWorld> World = GetWorld();
		checkf(World, TEXT("World is null in ExecuteDestruction"));
        
		TWeakObjectPtr<AAO_DestructibleCacheActor> WeakThis(this);
        
		World->GetTimerManager().SetTimer(
			DestroyTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
			{
				if (TObjectPtr<AAO_DestructibleCacheActor> StrongThis = WeakThis.Get())
				{
					StrongThis->Destroy();
				}
			}),
			DestroyDelay,
			false
		);
	}
}

void AAO_DestructibleCacheActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}