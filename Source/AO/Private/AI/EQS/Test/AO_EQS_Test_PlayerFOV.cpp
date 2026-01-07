//KSJ : AO_EQS_Test_PlayerFOV

#include "AI/EQS/Test/AO_EQS_Test_PlayerFOV.h"
#include "EnvironmentQuery/Items/EnvQueryItemType.h"
#include "Character/AO_PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UAO_EQS_Test_PlayerFOV::UAO_EQS_Test_PlayerFOV()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType::StaticClass();
	SetWorkOnFloatValues(false);
}

void UAO_EQS_Test_PlayerFOV::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	// 모든 플레이어 가져오기
	TArray<AAO_PlayerCharacter*> AllPlayers;
	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), FoundPlayers);
	
	for (AActor* Actor : FoundPlayers)
	{
		if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor))
		{
			AllPlayers.Add(Player);
		}
	}

	// 플레이어가 없으면 모든 위치 통과
	if (AllPlayers.Num() == 0)
	{
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			It.SetScore(TestPurpose, FilterType, 1.0f, 0.0f);
		}
		return;
	}

	// FOV 코사인 값 미리 계산 (절반 각도 사용)
	const float FOVCosine = FMath::Cos(FMath::DegreesToRadians(PlayerFOVDegrees * 0.5f));

	// 각 아이템에 대해 테스트
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		bool bOutsideAllPlayersFOV = true;

		for (AAO_PlayerCharacter* Player : AllPlayers)
		{
			if (!IsValid(Player))
			{
				continue;
			}

			FVector PlayerLocation = Player->GetActorLocation();
			float DistanceToPlayer = FVector::Dist(ItemLocation, PlayerLocation);

			// 최소 거리 체크 - 너무 가까우면 FOV 무관하게 실패 (바로 옆/뒤 스폰 방지)
			if (DistanceToPlayer < MinDistanceFromPlayer)
			{
				bOutsideAllPlayersFOV = false;
				break;
			}

			// 플레이어 시선 방향 가져오기
			FVector PlayerForward = Player->GetActorForwardVector();
			
			// 카메라 방향 사용 시 (플레이어가 실제로 바라보는 방향)
			if (bUseCameraDirection)
			{
				if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
				{
					FRotator CameraRot = PC->GetControlRotation();
					PlayerForward = CameraRot.Vector();
				}
			}

			// 플레이어 -> 스폰위치 방향 계산
			FVector DirToSpawnLoc = (ItemLocation - PlayerLocation).GetSafeNormal();

			// Dot Product로 FOV 체크
			float DotProduct = FVector::DotProduct(PlayerForward, DirToSpawnLoc);

			// 시야각 내부에 있으면 실패
			if (DotProduct >= FOVCosine)
			{
				bOutsideAllPlayersFOV = false;
				break;
			}
		}

		// 모든 플레이어의 FOV 밖에 있으면 통과
		It.SetScore(TestPurpose, FilterType, bOutsideAllPlayersFOV ? 1.0f : 0.0f, 0.0f);
	}
}

