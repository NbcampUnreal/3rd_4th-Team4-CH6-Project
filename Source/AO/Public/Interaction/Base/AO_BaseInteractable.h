// HSJ : AO_BaseInteractable.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "AO_BaseInteractable.generated.h"

class AAO_PuzzleReactionActor;

/**
 * 간단한 상호작용 액터 베이스 클래스
 * 
 * 사용법 (C++):
 * 1. 이 클래스(AAO_BaseInteractable) 상속 받은 새로운 클래스 생성
 * 2. 해당 클래스에서 OnInteractionSuccess_BP_Implementation 오버라이드(AO_ExampleInteractable 참고)
 * 
 * 사용법 (블루프린트):
 * 1. 이 클래스 기반으로 블루프린트 생성
 * 2. Event Graph에서 "On Interaction Success BP" 이벤트 구현
 * 
 */
UCLASS(Abstract)
class AO_API AAO_BaseInteractable : public AAO_WorldInteractable
{
	GENERATED_BODY()

public:
	AAO_BaseInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	// BP에서 오버라이드 가능
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	bool CanInteraction_BP(const FAO_InteractionQuery& InteractionQuery) const;
	virtual bool CanInteraction_BP_Implementation(const FAO_InteractionQuery& InteractionQuery) const;

	virtual void OnInteractionSuccess(AActor* Interactor) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastNotifyInteractionSuccess(AActor* Interactor);

	FORCEINLINE bool IsToggleable() const { return bIsToggleable; }
	FORCEINLINE bool IsActivated() const { return bIsActivated; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> InteractableMeshComponent;

	UPROPERTY(ReplicatedUsing = OnRep_IsActivated, EditAnywhere, BlueprintReadWrite, Category = "Interaction|State")
	bool bIsActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interaction|State")
	bool bInteractionEnabled = true;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	void OnInteractionSuccess_BP(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractionSuccessMulticast_BP(AActor* Interactor);

	virtual UStaticMeshComponent* GetInteractableMesh() const override 
	{ 
		return InteractableMeshComponent; 
	}

	UFUNCTION()
	void OnRep_IsActivated();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction|Reaction")
	void TriggerLinkedReactions(bool bActivate);

	UFUNCTION(BlueprintCallable, Category="Interaction", meta=(DisplayName="Disable Player Interaction"))
	void AddDisabledPlayer(AActor* Player);

	UFUNCTION(BlueprintCallable, Category="Interaction", meta=(DisplayName="Enable Player Interaction"))
	void RemoveDisabledPlayer(AActor* Player);

	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsPlayerDisabled(AActor* Player) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 상호작용 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Info")
	FText InteractionTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Info")
	FText InteractionContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Info")
	float InteractionDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation")
	TObjectPtr<UAnimMontage> ActiveHoldMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation")
	TObjectPtr<UAnimMontage> ActiveMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation",
		meta=(EditCondition="bIsToggleable", EditConditionHides))
	TObjectPtr<UAnimMontage> DeactivateMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Animation")
	bool bWaitForAnimationNotify = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Toggle")
	bool bIsToggleable = false;

	virtual FTransform GetInteractionTransform() const override;
    
	UPROPERTY(EditAnywhere, Category="Interaction|MotionWarping")
	FName InteractionSocketName = "InteractionPoint";
    
	UPROPERTY(EditAnywhere, Category="Interaction|MotionWarping")
	FName WarpTargetName = "InteractionPoint";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Reaction")
	TArray<TObjectPtr<AAO_PuzzleReactionActor>> LinkedReactionActors;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interaction")
	TArray<TObjectPtr<AActor>> DisabledPlayers;

private:
	UPROPERTY()
	TObjectPtr<UMeshComponent> CachedInteractionMesh;
};