// HSJ : AO_Valve.cpp
#include "Puzzle/Actor/AO_Valve.h"
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
    
    InteractionTitle = FText::FromString(TEXT("밸브"));
    InteractionContent = FText::FromString(TEXT("열기/닫기"));
}

void AAO_Valve::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (FAO_ValveSpawnedActors& SpawnedActors : SpawnedActorsArray)
	{
		CleanupSpawnedActors(SpawnedActors);
	}
	SpawnedActorsArray.Empty();
    
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

    SpawnedActorsArray.Empty();
    SpawnedActorsArray.SetNum(SpawnInfoArray.Num());

    for (int32 i = 0; i < SpawnInfoArray.Num(); ++i)
    {
        SpawnEffects(SpawnInfoArray[i], SpawnedActorsArray[i]);
    }
}

void AAO_Valve::CloseValve()
{
    if (ValveCloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ValveCloseSound, GetActorLocation());
    }

    for (FAO_ValveSpawnedActors& SpawnedActors : SpawnedActorsArray)
    {
        CleanupSpawnedActors(SpawnedActors);
    }

    SpawnedActorsArray.Empty();
}

void AAO_Valve::SpawnEffects(const FAO_ValveSpawnInfo& SpawnInfo, FAO_ValveSpawnedActors& OutSpawned)
{
    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
    	return;
    }

    FTransform ValveTransform = GetActorTransform();

	// 데미지존 스폰
    if (SpawnInfo.DamageZoneClass)
    {
        FVector DamageZoneWorldOffset = ValveTransform.TransformVector(SpawnInfo.DamageZoneOffset);
        FVector DamageZoneLocation = GetActorLocation() + DamageZoneWorldOffset;
        FRotator DamageZoneRotationFinal = GetActorRotation() + SpawnInfo.DamageZoneRotation;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        OutSpawned.DamageZone = World->SpawnActor<AActor>(
            SpawnInfo.DamageZoneClass,
            DamageZoneLocation,
            DamageZoneRotationFinal,
            SpawnParams
        );
    }

    FVector VFXWorldOffset = ValveTransform.TransformVector(SpawnInfo.VFXOffset);
    FVector VFXLocation = GetActorLocation() + VFXWorldOffset;
    FRotator VFXRotationFinal = GetActorRotation() + SpawnInfo.VFXRotation;

    // VFX 스폰
    if (SpawnInfo.NiagaraEffect)
    {
        OutSpawned.NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            SpawnInfo.NiagaraEffect,
            VFXLocation,
            VFXRotationFinal,
            SpawnInfo.VFXScale,
            true,
            true,
            ENCPoolMethod::None,
            false
        );
    }
    else if (SpawnInfo.CascadeEffect)
    {
        OutSpawned.CascadeComponent = UGameplayStatics::SpawnEmitterAtLocation(
            World,
            SpawnInfo.CascadeEffect,
            VFXLocation,
            VFXRotationFinal,
            SpawnInfo.VFXScale,
            true,
            EPSCPoolMethod::None,
            false
        );

        if (OutSpawned.CascadeComponent)
        {
            OutSpawned.CascadeComponent->Activate(true);
        }
    }

    if (SpawnInfo.LoopingSound)
    {
        OutSpawned.AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            World,
            SpawnInfo.LoopingSound,
            VFXLocation,
            VFXRotationFinal,
            SpawnInfo.SoundVolumeMultiplier,
            SpawnInfo.SoundPitchMultiplier,
            0.0f,
            nullptr,
            nullptr,
            false
        );
    }
}

void AAO_Valve::CleanupSpawnedActors(FAO_ValveSpawnedActors& SpawnedActors)
{
    // 데미지존 제거
    if (SpawnedActors.DamageZone)
    {
        SpawnedActors.DamageZone->Destroy();
        SpawnedActors.DamageZone = nullptr;
    }

    // VFX 제거
    if (SpawnedActors.NiagaraComponent)
    {
        SpawnedActors.NiagaraComponent->DestroyComponent();
        SpawnedActors.NiagaraComponent = nullptr;
    }

    if (SpawnedActors.CascadeComponent)
    {
        SpawnedActors.CascadeComponent->DestroyComponent();
        SpawnedActors.CascadeComponent = nullptr;
    }

    if (SpawnedActors.AudioComponent)
    {
        SpawnedActors.AudioComponent->Stop();
        SpawnedActors.AudioComponent->DestroyComponent();
        SpawnedActors.AudioComponent = nullptr;
    }
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