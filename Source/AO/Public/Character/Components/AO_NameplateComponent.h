// AO_NameplateComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_NameplateComponent.generated.h"

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetDisplayName(const FText& InName);
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
	float BaseZOffset = 80.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Offset")
	float ExtraZOffset = 10.f;

	UPROPERTY(EditAnywhere, Category = "Nameplate|Visibility")
	bool bHideForSelf = true;

private:
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAO_NameTagWidget> WidgetInstance;

	TWeakObjectPtr<APlayerState> CachedPlayerState;

	void EnsureWidgetComponent();
	void RefreshNameFromPlayerState();
	void ApplyDistanceVisuals();
	bool ShouldHideForSelf() const;
};
