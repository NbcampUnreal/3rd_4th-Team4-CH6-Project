// AO_TrollWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AO_TrollWeapon.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AAO_Troll;

/**
 * Troll 전용 무기 액터
 * 
 * 특징:
 * - StaticMesh로 무기 외형 표시
 * - Troll만 줍기/드롭 가능 (플레이어 상호작용 불가)
 * - 드롭 시 물리 시뮬레이션
 * - 소켓에 부착/분리 가능
 */
UCLASS()
class AO_API AAO_TrollWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AAO_TrollWeapon();

	// 무기 줍기 (Troll 전용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|TrollWeapon")
	bool PickupByTroll(AAO_Troll* Troll);

	// 무기 드롭
	UFUNCTION(BlueprintCallable, Category = "AO|AI|TrollWeapon")
	void Drop();

	// 현재 소지 중인 Troll
	UFUNCTION(BlueprintCallable, Category = "AO|AI|TrollWeapon")
	AAO_Troll* GetOwningTroll() const { return OwningTroll.Get(); }

	// 소지 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|TrollWeapon")
	bool IsPickedUp() const { return OwningTroll.IsValid(); }

	// 무기 메시 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|TrollWeapon")
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 물리 시뮬레이션 활성화/비활성화
	void SetPhysicsEnabled(bool bEnabled);

	// 충돌 설정
	void SetCollisionForPickedUp();
	void SetCollisionForDropped();

protected:
	// 무기 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|TrollWeapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// 감지용 콜리전 (Troll이 무기를 찾을 때 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|TrollWeapon")
	TObjectPtr<USphereComponent> DetectionSphere;

	// 현재 무기를 소지한 Troll
	UPROPERTY(ReplicatedUsing = OnRep_OwningTroll)
	TWeakObjectPtr<AAO_Troll> OwningTroll;

	// 부착할 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|TrollWeapon")
	FName WeaponSocketName = FName("WeaponSocket");

	UFUNCTION()
	void OnRep_OwningTroll();
};
