// AO_DamageVolume.cpp

#include "Maps/Volumes/AO_DamageVolume.h"

#include "Character/AO_PlayerCharacter.h"
#include "Components/BoxComponent.h"

AAO_DamageVolume::AAO_DamageVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAO_DamageVolume::OnBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AAO_DamageVolume::OnEndOverlap);
}

void AAO_DamageVolume::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ensure(OtherActor) || ActiveHandles.Contains(OtherActor))
	{
		return;
	}

	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	checkf(ASC, TEXT("ASC is NULL!"));

	checkf(DamageEffectClass, TEXT("DamageEffectClass is NULL!"));

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	checkf(SpecHandle.IsValid(), TEXT("Handle is invalid!"));

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamagePerSecond")), DamagePerSecond);

	FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	ActiveHandles.Add(OtherActor, ActiveHandle);
}

void AAO_DamageVolume::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ensure(OtherActor))
	{
		return;
	}

	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	checkf(ASC, TEXT("ASC is NULL!"));

	if (FActiveGameplayEffectHandle* Handle = ActiveHandles.Find(OtherActor))
	{
		ASC->RemoveActiveGameplayEffect(*Handle);
		ActiveHandles.Remove(OtherActor);
	}
}
