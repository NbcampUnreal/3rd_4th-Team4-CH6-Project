// HSJ : AO_RollingBall.cpp
#include "Puzzle/Actor/AO_RollingBall.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Puzzle/Destructible/AO_DestructibleCacheActor.h"

AAO_RollingBall::AAO_RollingBall()
{
    bReplicates = true;
    SetReplicateMovement(true);

    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    RootComponent = BallMesh;
    
    BallMesh->SetSimulatePhysics(false);
    BallMesh->SetEnableGravity(true);
    BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BallMesh->SetCollisionObjectType(ECC_PhysicsBody);
    BallMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    BallMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BallMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BallMesh->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
    BallMesh->SetNotifyRigidBodyCollision(true);
    BallMesh->SetGenerateOverlapEvents(true);

    KnockdownHitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
    DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
}

void AAO_RollingBall::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        StartPosition = GetActorLocation();
        BallMesh->OnComponentBeginOverlap.AddDynamic(this, &AAO_RollingBall::OnBallBeginOverlap);
    }
}

void AAO_RollingBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_RollingBall, StartPosition);
    DOREPLIFETIME(AAO_RollingBall, bShouldMove);
}

void AAO_RollingBall::SetShouldMove(bool bInShouldMove)
{
    if (!HasAuthority())
    {
    	return;
    }

    bShouldMove = bInShouldMove;
    
    if (bShouldMove)
    {
        // 움직여야 할 때만 Physics 활성화
        BallMesh->SetSimulatePhysics(true);
    }
    // Reset 트리거 닿을 때만 멈춤
}

void AAO_RollingBall::ResetToStart()
{
    if (!HasAuthority())
    {
    	return;
    }

	if (LinkedDestructible)
	{
		// 연결된 파괴 액터가 파괴 되었는지
		if (!IsValid(LinkedDestructible) || LinkedDestructible->IsDestroyed())
		{
			Destroy();
			return;
		}
	}

    // 초기 위치로 순간이동
    SetActorLocation(StartPosition);
    BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    
    PlayerHitCooldowns.Empty();
    
    if (bShouldMove)
    {
        BallMesh->SetSimulatePhysics(true);
    }
    else
    {
        BallMesh->SetSimulatePhysics(false);
    }
}

void AAO_RollingBall::OnBallBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
    	return;
    }
    if (!OtherActor)
    {
    	return;
    }
	// Physics 꺼져있으면 데미지 안줌
    if (!BallMesh->IsSimulatingPhysics())
    {
    	return;
    }

    AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
    if (!Player) 
    {
        return;
    }

    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player);
    if (!ASI)
    {
    	return;
    }

    UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
    if (!TargetASC)
    {
    	return;
    }

    if (TargetASC->HasMatchingGameplayTag(DeathTag))
    {
        return;
    }

    const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
    if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
    {
        return;
    }

    float* LastHitTimePtr = PlayerHitCooldowns.Find(Player);
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (LastHitTimePtr && (CurrentTime - *LastHitTimePtr) < HitCooldown)
    {
        return;
    }

    PlayerHitCooldowns.Add(Player, CurrentTime);
    
    // 데미지 적용
    if (DamageEffectClass)
    {
        FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
        EffectContext.AddInstigator(this, this);
        EffectContext.AddHitResult(SweepResult);

        FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
            DamageEffectClass,
            1.0f,
            EffectContext
        );

        if (SpecHandle.IsValid())
        {
            const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
            SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, Damage);

            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    // 넉백 적용
    ACharacter* Character = Cast<ACharacter>(Player);
    if (Character && Character->GetCharacterMovement())
    {
        FVector KnockbackDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        KnockbackDirection.Z = 0.3f;
        KnockbackDirection.Normalize();

        FVector LaunchVelocity = KnockbackDirection * KnockbackStrength;

        Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        Character->LaunchCharacter(LaunchVelocity, true, true);

        if (KnockdownHitReactTag.IsValid())
        {
            FGameplayEventData EventData;
            EventData.Instigator = this;
            EventData.Target = Player;
            
            TargetASC->HandleGameplayEvent(KnockdownHitReactTag, &EventData);
        }
    }
}