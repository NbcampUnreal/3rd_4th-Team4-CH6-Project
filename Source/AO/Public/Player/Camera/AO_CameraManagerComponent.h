// AO_CameraManagerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AO_CameraProfile.h"
#include "Components/ActorComponent.h"
#include "AO_CameraManagerComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAO_CameraProfile;

USTRUCT()
struct FAO_CameraRequest
{
	GENERATED_BODY()

	FGameplayTag Tag;
	int32 Priority = 0;
	float Weight = 0.f;
	float BlendIn = 0.25f;
	float BlendOut = 0.25f;
	bool bActive = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_CameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_CameraManagerComponent();

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UAO_CameraProfile> ProfileDB;

public:
	void BindCameraComponents(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	void BindToASC(UAbilitySystemComponent* InASC);

	void PushCameraState(const FGameplayTag& CameraTag);
	void PopCameraState(const FGameplayTag& CameraTag);

private:
	void OnStateTagChanged(FGameplayTag Tag, int32 NewCount);

	void UpdateRequests(float DeltaTime);
	const FAO_CameraSettings* ChooseTargetProfile() const;
	void ApplyProfileBlended(const FAO_CameraSettings& Target, float DeltaTime);
	
private:
	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TMap<FGameplayTag, FAO_CameraRequest> Requests;

	FTimerHandle TimerHandle_Update;
	
	// 현재 적용 중인 세팅
	float CurArmLength = 150.f;
	FVector CurSocketOffset = FVector(0.f, 50.f, 50.f);
	float CurFOV = 90.f;
	bool bCurEnableLag = true;
	float CurLagSpeed = 10.f;

	// 캐릭터 상태 관련 태그
	FGameplayTag State_Dead;
	FGameplayTag State_Sprint;
	FGameplayTag State_Traversal;
	
	// 카메라 프로필 태그
	FGameplayTag Camera_Default;
	FGameplayTag Camera_Dead;
	FGameplayTag Camera_Sprint;
	FGameplayTag Camera_Traversal;

	// 델리게이트 핸들
	TArray<FDelegateHandle> TagEventHandles;
};
