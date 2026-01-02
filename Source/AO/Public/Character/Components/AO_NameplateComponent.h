// AO_NameplateComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_NameplateComponent.generated.h"

class UAO_CustomizingComponent;
class UAO_NameTagWidget;
class UWidgetComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_NameplateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_NameplateComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void HandlePlayerStateChanged(APlayerState* NewPlayerState);

protected:
	UPROPERTY(EditAnywhere, Category = "Nameplate|Widget")
	TSubclassOf<UUserWidget> NameplateWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Distance")
	float MinScaleDistance = 200.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Distance")
	float MaxScaleDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Distance")
	float MinScale = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Distance")
	float MaxScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Distance")
	float HideDistance = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Offset")
	float CapsuleHeight = 176.f;
	
	UPROPERTY(EditAnywhere, Category = "Nameplate|Offset")
	float BaseZOffset = 5.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Offset")
	float ExtraZOffset = 10.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Visibility")
	bool bHideForSelf = true;

private:
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAO_NameTagWidget> WidgetInstance;

	UPROPERTY(ReplicatedUsing = OnRep_DisplayName)
	FText DisplayName;

	TWeakObjectPtr<UAO_CustomizingComponent> CachedCustomizingComponent;

	UFUNCTION()
	void OnRep_DisplayName();

	UFUNCTION()
	void HandleCapsuleSizeChanged(float NewHalfHeight);

	// 서버에서 이름 갱신
	void SetDisplayName_Server(const FText& InName);

	// UI 반영
	void ApplyDisplayNameToWidget();
	
	// 유틸용 함수
	void EnsureWidgetComponent();
	void TryInitNameFromOwner();
	void ApplyDistanceVisuals();
	bool ShouldHideForSelf() const;
};
