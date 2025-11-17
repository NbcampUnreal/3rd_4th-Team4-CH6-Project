// HSJ : AO_InspectionPuzzle.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"
#include "AO_InspectionPuzzle.generated.h"

class UAO_InspectableComponent;
class AAO_PuzzleConditionChecker;
class UStaticMeshComponent;
class UGameplayAbility;

/**
 * Inspection 요소 매핑 구조체
 * - 어떤 메시를 클릭하면 어떤 태그를 발생시킬지에 대한 정보
 */

USTRUCT(BlueprintType)
struct FAO_InspectionElementMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
    FName MeshComponentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
    FGameplayTag ActivatedTag;

    UPROPERTY(BlueprintReadOnly, Category = "Inspection")
    bool bIsActivated = false;
};

/**
 * Inspection 퍼즐 액터
 * 
 * - F키로 상호작용 (IAO_Interface_Interactable)
 * - Inspection 모드에서 메시 클릭 가능 (IAO_Interface_Inspectable)
 * - 클릭된 메시에 대응하는 GameplayTag를 Checker에 전송
 * 
 * 1. 플레이어 F키 -> GA_Inspect_Enter 활성화 -> EnterInspectionMode()
 * 2. 플레이어 마우스 클릭 -> GA_Inspect_Click 활성화
 * 3. ServerProcessInspectionClick() -> OnInspectionMeshClicked()
 * 4. FindTagForComponent() -> BroadcastTag()
 * 5. LinkedChecker->OnPuzzleEvent(Tag) -> 조건 체크
 * 
 */

UCLASS()
class AO_API AAO_InspectionPuzzle : public AActor, public IAO_Interface_Interactable, public IAO_Interface_Inspectable
{
    GENERATED_BODY()

public:
    AAO_InspectionPuzzle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IAO_Interface_Interactable
    virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
    virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
    virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

    // IAO_Interface_Inspectable
    virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    FGameplayTag FindTagForComponent(UPrimitiveComponent* Component) const;
    void BroadcastTag(const FGameplayTag& Tag);
    void UpdateActivationState(const FName& ComponentName, bool bActivated);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAO_InspectableComponent> InspectableComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Info")
    FText InspectionTitle = FText::FromString("Inspection");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Info")
    FText InspectionContent = FText::FromString(": Press F");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Info")
    float InteractionDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Info")
    TSubclassOf<UGameplayAbility> AbilityToGrant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Elements")
    TArray<FAO_InspectionElementMapping> ElementMappings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Elements")
    TObjectPtr<AAO_PuzzleConditionChecker> LinkedChecker;

protected:
    UPROPERTY(Replicated)
    TArray<FAO_InspectionElementMapping> ReplicatedMappings;
};