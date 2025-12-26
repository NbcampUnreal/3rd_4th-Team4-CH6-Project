// HSJ : AO_GasRoomPuzzle.cpp
#include "Puzzle/Actor/AO_GasRoomPuzzle.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Interaction/Actor/AO_BunkerDoor.h"
#include "Puzzle/Element/AO_PasswordPanelInspectable.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_GasRoomPuzzle::AAO_GasRoomPuzzle()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));

    DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
    
    for (int32 i = 0; i < 4; ++i)
    {
        ActiveDecalIndices[i] = -1;
    }
}

void AAO_GasRoomPuzzle::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    CollectCandidateDecals();
}

void AAO_GasRoomPuzzle::CollectCandidateDecals()
{
    CandidateDecals.Empty();

    TArray<UDecalComponent*> AllDecals;
    GetComponents<UDecalComponent>(AllDecals);

    for (UDecalComponent* Decal : AllDecals)
    {
        if (Decal && Decal->GetName().StartsWith(CandidateDecalPrefix))
        {
            Decal->SetVisibility(false);
            Decal->SetHiddenInGame(true);
            CandidateDecals.Add(Decal);
        }
    }

    CandidateDecals.Sort([](const UDecalComponent& A, const UDecalComponent& B)
    {
        return A.GetName() < B.GetName();
    });
}

void AAO_GasRoomPuzzle::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAO_GasRoomPuzzle::OnBoxBeginOverlap);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAO_GasRoomPuzzle::OnBoxEndOverlap);

        if (PasswordPanel)
        {
            PasswordPanel->OnPasswordCorrectEvent.AddDynamic(this, &AAO_GasRoomPuzzle::OnPasswordCorrect);
        }
    }
}

void AAO_GasRoomPuzzle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllTimers();
    MulticastCleanupGasEffects();
    Super::EndPlay(EndPlayReason);
}

void AAO_GasRoomPuzzle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_GasRoomPuzzle, CurrentState);
    DOREPLIFETIME(AAO_GasRoomPuzzle, RemainingTime);
    DOREPLIFETIME(AAO_GasRoomPuzzle, ActiveDecalIndices);
}

void AAO_GasRoomPuzzle::OnBoxBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor || CurrentState == EGasRoomState::Completed)
    {
        return;
    }

    if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(OtherActor))
    {
        OverlappedCharacters.Add(TWeakObjectPtr<AActor>(Player));

        if (CurrentState == EGasRoomState::Idle && !bHasEverStarted)
        {
            StartPuzzleDelayed();
        }
    }
}

void AAO_GasRoomPuzzle::OnBoxEndOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (HasAuthority() && OtherActor)
    {
        OverlappedCharacters.Remove(TWeakObjectPtr<AActor>(OtherActor));
    }
}

void AAO_GasRoomPuzzle::StartPuzzleDelayed()
{
    if (CurrentState != EGasRoomState::Idle)
    {
        return;
    }

    CurrentState = EGasRoomState::WaitingToStart;
    bHasEverStarted = true;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            StartDelayTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                if (GetAliveCharacterCount() == 0)
                {
                    ResetPuzzle();
                }
                else
                {
                    StartPuzzle();
                }
            }),
            DelayBeforeStart,
            false
        );
    }
}

void AAO_GasRoomPuzzle::StartPuzzle()
{
    if (CurrentState != EGasRoomState::WaitingToStart)
    {
        return;
    }

    CurrentState = EGasRoomState::Active;
    RemainingTime = CountdownDuration;

    if (HasAuthority())
    {
        if (PasswordPanel)
        {
            PasswordPanel->GenerateRandomPassword();
            
            int32 Digit1, Digit2, Digit3, Digit4;
            PasswordPanel->GetPasswordDigits(Digit1, Digit2, Digit3, Digit4);
            
            SelectRandomCandidates();
        	MulticastUpdatePasswordHints(Digit1, Digit2, Digit3, Digit4,
				ActiveDecalIndices[0], ActiveDecalIndices[1], 
				ActiveDecalIndices[2], ActiveDecalIndices[3]);
        }

        LockDoors();
        ActivateDamageZone();
    }

    MulticastCloseDoors();
    MulticastSpawnGasEffects();

    if (UWorld* World = GetWorld(); World && HasAuthority())
    {
        World->GetTimerManager().SetTimer(
            CharacterCheckTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]() { CheckAliveCharacters(); }),
            CharacterCheckInterval,
            true
        );

        World->GetTimerManager().SetTimer(
            CountdownTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]() { OnCountdownTick(); }),
            CountdownTickInterval,
            true
        );
    }
}

void AAO_GasRoomPuzzle::SelectRandomCandidates()
{
    const int32 NumCandidates = CandidateDecals.Num();
    if (NumCandidates < 4)
    {
        return;
    }

    TArray<int32> AvailableIndices;
    AvailableIndices.Reserve(NumCandidates);
    for (int32 i = 0; i < NumCandidates; ++i)
    {
        AvailableIndices.Add(i);
    }

    for (int32 i = 0; i < 4; ++i)
    {
        const int32 RandomIdx = FMath::RandRange(0, AvailableIndices.Num() - 1);
        ActiveDecalIndices[i] = AvailableIndices[RandomIdx];
        AvailableIndices.RemoveAtSwap(RandomIdx);
    }
}

void AAO_GasRoomPuzzle::MulticastUpdatePasswordHints_Implementation(
	int32 Digit1, int32 Digit2, int32 Digit3, int32 Digit4,
	int32 Idx1, int32 Idx2, int32 Idx3, int32 Idx4)
{
	ActiveDecalIndices[0] = Idx1;
	ActiveDecalIndices[1] = Idx2;
	ActiveDecalIndices[2] = Idx3;
	ActiveDecalIndices[3] = Idx4;
	
    const int32 Digits[4] = {Digit1, Digit2, Digit3, Digit4};
    const FLinearColor Colors[4] = {Digit1Color, Digit2Color, Digit3Color, Digit4Color};
	const int32 Indices[4] = {Idx1, Idx2, Idx3, Idx4};

    for (int32 i = 0; i < 4; ++i)
    {
    	const int32 Idx = Indices[i];
        
        // 배열 범위 체크
        if (Idx < 0 || Idx >= CandidateDecals.Num())
        {
            continue;
        }

        UDecalComponent* Decal = CandidateDecals[Idx];
        if (!Decal)
        {
        	continue;
        }

        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(NumberDecalMaterial, this);
        if (DynMat)
        {
            DynMat->SetTextureParameterValue(FName("NumberTexture"), NumberTextures[Digits[i]]);
            DynMat->SetVectorParameterValue(FName("TintColor"), Colors[i]);
            
            Decal->SetDecalMaterial(DynMat);
            Decal->SetVisibility(true);
            Decal->SetHiddenInGame(false);
        }
    }
}

void AAO_GasRoomPuzzle::MulticastClearPasswordHints_Implementation()
{
    for (int32 i = 0; i < 4; ++i)
    {
        const int32 Idx = ActiveDecalIndices[i];
        if (Idx >= 0 && Idx < CandidateDecals.Num() && CandidateDecals[Idx])
        {
            CandidateDecals[Idx]->SetVisibility(false);
            CandidateDecals[Idx]->SetHiddenInGame(true);
        }
        ActiveDecalIndices[i] = -1;
    }
}

void AAO_GasRoomPuzzle::OnCountdownTick()
{
    if (CurrentState != EGasRoomState::Active)
    {
        return;
    }

    RemainingTime -= CountdownTickInterval;
    
    if (RemainingTime <= 0.0f)
    {
        RemainingTime = 0.0f;
        ResetPuzzle();
    }
}

void AAO_GasRoomPuzzle::CheckAliveCharacters()
{
    if (CurrentState != EGasRoomState::Active)
    {
        return;
    }

    if (GetAliveCharacterCount() == 0)
    {
        ResetPuzzle();
    }
}

void AAO_GasRoomPuzzle::OnPasswordCorrect()
{
    if (HasAuthority())
    {
        CompletePuzzle();
    }
}

void AAO_GasRoomPuzzle::CompletePuzzle()
{
    if (CurrentState == EGasRoomState::Completed)
    {
        return;
    }
	
    CurrentState = EGasRoomState::Completed;
    ClearAllTimers();
    
    MulticastOpenDoors();
    MulticastCleanupGasEffects();
    MulticastClearPasswordHints();

    if (HasAuthority())
    {
        DeactivateDamageZone();
        UnlockDoors();
        
        if (LinkedChecker && CompletionTag.IsValid())
        {
            LinkedChecker->OnPuzzleEvent(CompletionTag, GetInstigator());
        }
    }
}

void AAO_GasRoomPuzzle::ResetPuzzle()
{
    if (CurrentState == EGasRoomState::Completed)
    {
        return;
    }

    CurrentState = EGasRoomState::Idle;
    RemainingTime = 0.0f;
    bHasEverStarted = false;

    ClearAllTimers();
    MulticastOpenDoors();
    MulticastCleanupGasEffects();
    MulticastClearPasswordHints();
    
    if (HasAuthority())
    {
        DeactivateDamageZone();
        UnlockDoors();
    }
}

void AAO_GasRoomPuzzle::ActivateDamageZone()
{
    if (!DamageZoneClass || !GetWorld())
    {
        return;
    }

    if (SpawnedDamageZone)
    {
        SpawnedDamageZone->Destroy();
        SpawnedDamageZone = nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SpawnedDamageZone = GetWorld()->SpawnActor<AActor>(
        DamageZoneClass,
        TriggerBox->GetComponentLocation(),
        TriggerBox->GetComponentRotation(),
        SpawnParams
    );

    if (SpawnedDamageZone)
    {
        TArray<UBoxComponent*> BoxComponents;
        SpawnedDamageZone->GetComponents<UBoxComponent>(BoxComponents);
        
        if (BoxComponents.Num() > 0)
        {
            BoxComponents[0]->SetBoxExtent(TriggerBox->GetScaledBoxExtent());
        }
    }
}

void AAO_GasRoomPuzzle::DeactivateDamageZone()
{
    if (SpawnedDamageZone)
    {
        SpawnedDamageZone->Destroy();
        SpawnedDamageZone = nullptr;
    }
}

int32 AAO_GasRoomPuzzle::GetAliveCharacterCount() const
{
    int32 Count = 0;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : OverlappedCharacters)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
        {
        	continue;
        }

        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                if (!ASC->HasMatchingGameplayTag(DeathTag))
                {
                    Count++;
                }
            }
        }
    }
    
    return Count;
}

void AAO_GasRoomPuzzle::ClearAllTimers()
{
    if (UWorld* World = GetWorld())
    {
        FTimerManager& TimerManager = World->GetTimerManager();
        TimerManager.ClearTimer(StartDelayTimer);
        TimerManager.ClearTimer(CharacterCheckTimer);
        TimerManager.ClearTimer(CountdownTimer);
    }
}

void AAO_GasRoomPuzzle::LockDoors()
{
    for (AAO_BunkerDoor* Door : BunkerDoors)
    {
        if (Door)
        {
            Door->LockInteraction();
        }
    }
}

void AAO_GasRoomPuzzle::UnlockDoors()
{
    for (AAO_BunkerDoor* Door : BunkerDoors)
    {
        if (Door)
        {
            Door->UnlockInteraction();
        }
    }
}

void AAO_GasRoomPuzzle::MulticastCloseDoors_Implementation()
{
    for (AAO_BunkerDoor* Door : BunkerDoors)
    {
        if (Door)
        {
            Door->SetDoorState(false);
        }
    }
}

void AAO_GasRoomPuzzle::MulticastOpenDoors_Implementation()
{
    for (AAO_BunkerDoor* Door : BunkerDoors)
    {
        if (Door)
        {
            Door->SetDoorState(true);
        }
    }
}

void AAO_GasRoomPuzzle::MulticastSpawnGasEffects_Implementation()
{
    if (!GetWorld())
    {
    	return;
    }

    for (FAO_GasEffectSpawnedActors& SpawnedActor : SpawnedEffects)
    {
        if (SpawnedActor.NiagaraComponent)
        {
        	SpawnedActor.NiagaraComponent->DestroyComponent();
        }
        if (SpawnedActor.CascadeComponent)
        {
        	SpawnedActor.CascadeComponent->DestroyComponent();
        }
        if (SpawnedActor.AudioComponent)
        {
        	SpawnedActor.AudioComponent->DestroyComponent();
        }
    }
    SpawnedEffects.Empty();

    for (const FAO_GasEffectSpawnInfo& SpawnInfo : GasEffectSpawnInfos)
    {
        FAO_GasEffectSpawnedActors SpawnedActor;
        
        const FTransform SpawnTransform = GetActorTransform();
        const FVector WorldLocation = SpawnTransform.TransformPosition(SpawnInfo.RelativeLocation);
        const FRotator WorldRotation = (SpawnTransform.GetRotation() * SpawnInfo.RelativeRotation.Quaternion()).Rotator();

        if (SpawnInfo.NiagaraEffect)
        {
            SpawnedActor.NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), SpawnInfo.NiagaraEffect, WorldLocation, WorldRotation, SpawnInfo.VFXScale);
        }
        else if (SpawnInfo.CascadeEffect)
        {
            SpawnedActor.CascadeComponent = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), SpawnInfo.CascadeEffect, WorldLocation, WorldRotation, SpawnInfo.VFXScale);
        }

        if (SpawnInfo.LoopingSound)
        {
            SpawnedActor.AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
                GetWorld(), SpawnInfo.LoopingSound, WorldLocation, WorldRotation, SpawnInfo.SoundVolumeMultiplier);
        }

        SpawnedEffects.Add(SpawnedActor);
    }
}

void AAO_GasRoomPuzzle::MulticastCleanupGasEffects_Implementation()
{
    for (FAO_GasEffectSpawnedActors& SpawnedActor : SpawnedEffects)
    {
        if (SpawnedActor.NiagaraComponent)
        {
            SpawnedActor.NiagaraComponent->Deactivate();
            SpawnedActor.NiagaraComponent->DestroyComponent();
        }
        if (SpawnedActor.CascadeComponent)
        {
            SpawnedActor.CascadeComponent->Deactivate();
            SpawnedActor.CascadeComponent->DestroyComponent();
        }
        if (SpawnedActor.AudioComponent)
        {
            SpawnedActor.AudioComponent->Stop();
            SpawnedActor.AudioComponent->DestroyComponent();
        }
    }
    
    SpawnedEffects.Empty();
}