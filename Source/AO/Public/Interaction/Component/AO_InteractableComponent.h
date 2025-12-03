// HSJ : AO_InteractableComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"
#include "AO_InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableSuccess, AActor*, Interactor);

/**
 * 컴포넌트 기반 상호작용 시스템
 * 
 * 1. 이 컴포넌트 추가
 * 2. 디테일에서 InteractionTitle, InteractionContent 등 설정
 * 3. Owner 액터의 메시 콜리전을 Interaction 채널을 Block으로 설정
 * BP 이벤트 그래프에서 OnInteractionSuccess 바인딩하여 로직 작성
 * Cpp에서 OnInteractionSuccess 델리게이트 바인딩하여 로직 작성
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InteractableComponent : public UActorComponent, public IAO_Interface_Interactable
{
    GENERATED_BODY()

public:
    UAO_InteractableComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IAO_Interface_Interactable
    virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
    virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
    virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

    // 상호작용 성공 시 호출
    void NotifyInteractionSuccess(AActor* Interactor);

	virtual FTransform GetInteractionTransform() const override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
    FText InteractionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
    FText InteractionContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation")
	TObjectPtr<UAnimMontage> ActiveHoldMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation")
	TObjectPtr<UAnimMontage> ActiveMontage;

    // 상호작용 활성화 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Interaction")
    bool bInteractionEnabled = true;

    // 상호작용 성공 시 호출되는 이벤트
    UPROPERTY(BlueprintAssignable, Category="Interaction")
    FOnInteractableSuccess OnInteractionSuccess;
    
	UPROPERTY(EditAnywhere, Category="Interaction|MotionWarping")
	FName InteractionSocketName = "InteractionPoint";
    
	UPROPERTY(EditAnywhere, Category="Interaction|MotionWarping")
	FName WarpTargetName = "InteractionPoint";
};