// HSJ : AO_Valve.cpp
#include "Puzzle/Actor/AO_Valve.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAO_Valve::AAO_Valve(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsToggleable = true;
    
    InteractionTitle = FText::FromString(TEXT("Valve"));
    //InteractionContent = FText::FromString(TEXT("Open/Close"));
}

void AAO_Valve::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    CollectVFXPoints();
    CollectDamageZoneBoxes();
}

void AAO_Valve::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnDamageZones();
	}
	SpawnEffects();
}

void AAO_Valve::CollectVFXPoints()
{
    VFXPoints.Empty();

    TArray<USceneComponent*> ChildComponents;
    GetRootComponent()->GetChildrenComponents(true, ChildComponents);

    for (USceneComponent* Child : ChildComponents)
    {
        if (Child && Child->GetName().StartsWith(VFXPointPrefix))
        {
            VFXPoints.Add(Child);
        }
    }

    VFXPoints.Sort([](const USceneComponent& A, const USceneComponent& B)
    {
        return A.GetName() < B.GetName();
    });
}

void AAO_Valve::CollectDamageZoneBoxes()
{
    DamageZoneBoxes.Empty();

    TArray<UBoxComponent*> BoxComponents;
    GetComponents<UBoxComponent>(BoxComponents);

    for (UBoxComponent* Box : BoxComponents)
    {
        if (Box && Box->GetName().StartsWith(DamageZonePrefix))
        {
            Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Box->ShapeColor = FColor::Red;
            
            DamageZoneBoxes.Add(Box);
        }
    }

    DamageZoneBoxes.Sort([](const UBoxComponent& A, const UBoxComponent& B)
    {
        return A.GetName() < B.GetName();
    });
}

void AAO_Valve::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupEffects();
    DestroyDamageZones();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InteractionLockTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AAO_Valve::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_Valve, bIsValveOpen);
}

void AAO_Valve::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
    Super::OnInteractionSuccess_BP_Implementation(Interactor);

    if (!HasAuthority())
    {
        return;
    }

    // 상호작용 잠금
    bInteractionEnabled = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            InteractionLockTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                bInteractionEnabled = true;
            }),
            InteractionLockDuration,
            false
        );
    }

    bIsValveOpen = !bIsValveOpen;

    if (bIsValveOpen)
    {
        OpenValve();
    }
    else
    {
        CloseValve();
    }
}

void AAO_Valve::OpenValve()
{
    if (ValveOpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ValveOpenSound, GetActorLocation());
    }

	if (HasAuthority())
	{
		DestroyDamageZones();
	}
	CleanupEffects();

    if (HasAuthority())
    {
        SpawnDamageZones();
    }
    SpawnEffects();
}

void AAO_Valve::CloseValve()
{
    if (ValveCloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ValveCloseSound, GetActorLocation());
    }

	if (HasAuthority())
	{
		DestroyDamageZones();
	}
	CleanupEffects();

	if (HasAuthority())
	{
		SpawnDamageZones();
	}
	SpawnEffects();
}

void AAO_Valve::SpawnDamageZones()
{
    if (!DamageZoneClass || !GetWorld())
    {
        return;
    }

    // 기존 데미지존 전부 제거
    DestroyDamageZones();

    SpawnedDamageZones.Reserve(DamageZoneBoxes.Num());

	for (int32 i = 0; i < DamageZoneBoxes.Num(); ++i)
	{
		UBoxComponent* DamageBox = DamageZoneBoxes[i];
		if (!DamageBox)
		{
			continue;
		}

		bool bShouldSpawn = (i % 2 == 0) ? bIsValveOpen : !bIsValveOpen;
        
		if (!bShouldSpawn)
		{
			SpawnedDamageZones.Add(nullptr);
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedZone = GetWorld()->SpawnActor<AActor>(
			DamageZoneClass,
			DamageBox->GetComponentLocation(),
			DamageBox->GetComponentRotation(),
			SpawnParams
		);

		if (SpawnedZone)
		{
			// 크기를 Box와 동일하게 설정
			TArray<UBoxComponent*> ZoneBoxComponents;
			SpawnedZone->GetComponents<UBoxComponent>(ZoneBoxComponents);
            
			if (ZoneBoxComponents.Num() > 0)
			{
				ZoneBoxComponents[0]->SetBoxExtent(DamageBox->GetScaledBoxExtent());
			}

			SpawnedDamageZones.Add(SpawnedZone);
		}
		else
		{
			SpawnedDamageZones.Add(nullptr);
		}
	}
}

void AAO_Valve::DestroyDamageZones()
{
    for (AActor* Zone : SpawnedDamageZones)
    {
        if (Zone)
        {
            Zone->Destroy();
        }
    }

    SpawnedDamageZones.Empty();
}

void AAO_Valve::SpawnEffects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    CleanupEffects();

    const int32 NumEffects = FMath::Min(EffectInfoArray.Num(), VFXPoints.Num());
    SpawnedEffectsArray.SetNum(NumEffects);

    for (int32 i = 0; i < NumEffects; ++i)
    {
    	bool bShouldSpawn = (i % 2 == 0) ? bIsValveOpen : !bIsValveOpen;

    	if (!bShouldSpawn)
    	{
    		continue;
    	}
    	
        const FAO_ValveEffectSpawnInfo& EffectInfo = EffectInfoArray[i];
        USceneComponent* VFXPoint = VFXPoints[i];
        
        if (!VFXPoint)
        {
            continue;
        }

        FAO_ValveSpawnedActors& Spawned = SpawnedEffectsArray[i];
        
        const FVector Location = VFXPoint->GetComponentLocation();
        const FRotator Rotation = VFXPoint->GetComponentRotation();

        if (EffectInfo.NiagaraEffect)
        {
            Spawned.NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World,
                EffectInfo.NiagaraEffect,
                Location,
                Rotation,
                EffectInfo.VFXScale,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
        }
        else if (EffectInfo.CascadeEffect)
        {
            Spawned.CascadeComponent = UGameplayStatics::SpawnEmitterAtLocation(
                World,
                EffectInfo.CascadeEffect,
                Location,
                Rotation,
                EffectInfo.VFXScale,
                true,
                EPSCPoolMethod::None,
                true
            );
        }

        if (EffectInfo.LoopingSound)
        {
            Spawned.AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
                World,
                EffectInfo.LoopingSound,
                Location,
                Rotation,
                EffectInfo.SoundVolumeMultiplier,
                EffectInfo.SoundPitchMultiplier,
                0.0f,
                nullptr,
                nullptr,
                false
            );
            
            if (Spawned.AudioComponent)
            {
                Spawned.AudioComponent->Play();
            }
        }
    }
}

void AAO_Valve::CleanupEffects()
{
    for (FAO_ValveSpawnedActors& Spawned : SpawnedEffectsArray)
    {
        if (Spawned.NiagaraComponent)
        {
            Spawned.NiagaraComponent->Deactivate();
            Spawned.NiagaraComponent->DestroyComponent();
            Spawned.NiagaraComponent = nullptr;
        }

        if (Spawned.CascadeComponent)
        {
            Spawned.CascadeComponent->Deactivate();
            Spawned.CascadeComponent->DestroyComponent();
            Spawned.CascadeComponent = nullptr;
        }

        if (Spawned.AudioComponent)
        {
            Spawned.AudioComponent->Stop();
            Spawned.AudioComponent->DestroyComponent();
            Spawned.AudioComponent = nullptr;
        }
    }

    SpawnedEffectsArray.Empty();
}

void AAO_Valve::OnRep_IsValveOpen()
{
    if (!HasAuthority())
    {
        if (bIsValveOpen)
        {
            OpenValve();
        }
        else
        {
            CloseValve();
        }
    }
}