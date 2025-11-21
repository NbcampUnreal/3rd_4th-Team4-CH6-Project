#include "Item/AO_Sample_Fuel.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "Train/AO_Train.h"


// Sets default values
AAO_Sample_Fuel::AAO_Sample_Fuel()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	FuelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuelMesh"));
	SetRootComponent(FuelMesh);

	FuelMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	FuelMesh->SetGenerateOverlapEvents(true);
	FuelMesh->OnComponentBeginOverlap.AddDynamic(this, &AAO_Sample_Fuel::OnOverlap);
}

void AAO_Sample_Fuel::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Sample_Fuel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAO_Sample_Fuel::OnOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)

{
    if (!HasAuthority()) return;
    if (bConsumed) return;
    bConsumed = true;

    UE_LOG(LogTemp, Warning, TEXT("🔥 OnOverlap Fired! OtherActor = %s"), *GetNameSafe(OtherActor));

    AAO_Train* Train = TargetTrain;
    if (!Train)
    {
        for (TActorIterator<AAO_Train> It(GetWorld()); It; ++It)
        {
            Train = *It;
            break;
        }
    }
    if (!Train) return;

    UAbilitySystemComponent* ASC = Train->GetAbilitySystemComponent();
    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Train ASC is null"));
        return;
    }

    // 1) 시도: 트레인에 에디터에서 지정한 AddEnergyAbilityClass가 있다면 그걸로 먼저 찾는다
    FGameplayAbilitySpec* FoundSpec = nullptr;
    if (Train->AddEnergyAbilityClass)
    {
        FoundSpec = ASC->FindAbilitySpecFromClass(Train->AddEnergyAbilityClass);
    }

    // 2) 못 찾았으면, 모든 AbilitySpec을 순회하면서 베이스 클래스의 파생 여부로 찾기
    if (!FoundSpec)
    {
        for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) // 엔진 버전에 따라 접근자명 다를 수 있음
        {
            if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(UAO_AddFuel_GameplayAbility::StaticClass()))
            {
                FoundSpec = &Spec;
                break;
            }
        }
    }

    if (!FoundSpec)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Train doesn't have AddEnergy ability (after thorough search)"));
        return;
    }

    // 3) Primary instance (인스턴싱 정책에 따라 null일 수 있음)
    UAO_AddFuel_GameplayAbility* AbilityInst = nullptr;
    if (FoundSpec->GetPrimaryInstance()) // 안전하게 검사
    {
        AbilityInst = Cast<UAO_AddFuel_GameplayAbility>(FoundSpec->GetPrimaryInstance());
    }

    // 인스턴스가 없다면(예: non-instanced), PendingAmount을 Ability에 직접 쓰는 방식은 불가능.
    // 그 경우 SetByCaller를 사용하거나 스펙에 직접 값을 주입해야 함.
    if (AbilityInst)
    {
        AbilityInst->PendingAmount = AddFuelAmount;
        UE_LOG(LogTemp, Warning, TEXT("✅ Item: PendingAmount set to ability instance = %f"), AddFuelAmount);
    }
    /*else
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Ability instance not available. Will try to apply via Spec SetByCaller fallback."));

        // Fallback: Make a GE Spec directly and SetByCaller; 이 방식은 ability 인스턴스 없이도 동작
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UGAS_Effect_AddEnergy::StaticClass(), 1.f, ASC->MakeEffectContext());
        if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
        {
            FGameplayTag AmountTag = FGameplayTag::RequestGameplayTag(FName("Data.EnergyAmount"));
            SpecHandle.Data->SetSetByCallerMagnitude(AmountTag, AddFuelAmount);
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("✅ Applied direct GE Spec with SetByCaller amount = %f"), AddFuelAmount);
        }
    }*/

    // 4) Ability 활성화 (가능하면 Spec->Handle 사용)
    if (FoundSpec)
    {
        ASC->TryActivateAbility(FoundSpec->Handle);
    }

    Destroy();
}