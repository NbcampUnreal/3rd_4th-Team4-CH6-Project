// HSJ : AO_GridWall.cpp
#include "Puzzle/Grid/AO_GridWall.h"

#include "AO_Log.h"
#include "Puzzle/Grid/AO_GridManager.h"
#include "EngineUtils.h"

AAO_GridWall::AAO_GridWall()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
    RootComponent = WallMesh;
}

void AAO_GridWall::BeginPlay()
{
    Super::BeginPlay();

    if (GridManager)
    {
        GridManager->RegisterWall(this);
    }
}

FIntPoint AAO_GridWall::GetGridCell() const
{
	if (GridManager)
	{
		FVector LocalPos = GetActorLocation() - GridManager->GridOrigin;
		float CellSize = GridManager->CellSize;
		
		// Vertical벽은 현재 셀의 오른쪽을 막음.
		// Horizontal벽은 현재 셀의 아래쪽을 막음.
		// 벽이 어느 셀에 속하는지 계산
		int32 X = FMath::FloorToInt(LocalPos.X / CellSize);
		int32 Y = FMath::FloorToInt(LocalPos.Y / CellSize);
		return FIntPoint(X, Y);
	}
	return FIntPoint::ZeroValue;
}

#if WITH_EDITOR
void AAO_GridWall::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);
    
    if (bFinished && !bIsUpdatingTransform)
    {
        UpdateWallTransform();
    }
}

void AAO_GridWall::UpdateWallTransform()
{
	if (bIsUpdatingTransform)
	{
		return;
	}
    
	TGuardValue<bool> UpdateGuard(bIsUpdatingTransform, true);

	if (GridManager)
	{
		float CellSize = GridManager->CellSize;
		FVector GridOrigin = GridManager->GridOrigin;
		FVector CurrentPos = GetActorLocation();
		FVector LocalPos = CurrentPos - GridOrigin;
        
		FVector SnapPosition;
        
		if (WallOrientation == EWallOrientation::Horizontal)
		{
			// 가로 벽은 현재 셀의 아래쪽에 배치
			int32 Y = FMath::RoundToInt(LocalPos.Y / CellSize);
			int32 X = FMath::FloorToInt(LocalPos.X / CellSize);
            
			SnapPosition = GridOrigin + FVector(
				X * CellSize + CellSize * 0.5f,  // 셀 중심에 X
				Y * CellSize,                    // 경계선에 Y
				CurrentPos.Z
			);
			// 가로 방향
			SetActorRotation(FRotator(0, 0, 0));  
		}
		else // Vertical인 경우
		{
			// 세로 벽은 현재 셀의 오른쪽에 배치
			int32 X = FMath::RoundToInt(LocalPos.X / CellSize);
			int32 Y = FMath::FloorToInt(LocalPos.Y / CellSize);
            
			SnapPosition = GridOrigin + FVector(
				X * CellSize,                     // 경계선 X
				Y * CellSize + CellSize * 0.5f,   // 셀 중심 Y
				CurrentPos.Z
			);
			// 세로 방향
			SetActorRotation(FRotator(0, 90, 0));  
		}
        
		SetActorLocation(SnapPosition, false);
	}
}
#endif