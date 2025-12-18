// HSJ : AO_GridManager.cpp
#include "Puzzle/Grid/AO_GridManager.h"
#include "Puzzle/Grid/AO_PushableRockElement.h"
#include "Puzzle/Grid/AO_GridWall.h"
#include "DrawDebugHelpers.h"

AAO_GridManager::AAO_GridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AAO_GridManager::BeginPlay()
{
    Super::BeginPlay();

#if ENABLE_DRAW_DEBUG
    if (bShowDebugGrid)
    {
        DrawDebugGrid();
    }
#endif
}

FIntPoint AAO_GridManager::WorldToGrid(const FVector& WorldLocation) const
{
	FVector LocalPos = WorldLocation - GridOrigin;
    
	// 셀 중심 기준으로 변환
	int32 X = FMath::FloorToInt((LocalPos.X + CellSize * 0.5f) / CellSize);
	int32 Y = FMath::FloorToInt((LocalPos.Y + CellSize * 0.5f) / CellSize);
    
	return FIntPoint(X, Y);
}

FVector AAO_GridManager::GridToWorld(const FIntPoint& GridCoord) const
{
	// 셀의 중심 좌표 반환
	return GridOrigin + FVector(
		GridCoord.X * CellSize + CellSize * 0.5f,
		GridCoord.Y * CellSize + CellSize * 0.5f,
		0.0f
	);
}

bool AAO_GridManager::IsRockAt(const FIntPoint& GridCoord) const
{
    return RockMap.Contains(GridCoord);
}

bool AAO_GridManager::IsValidGridCoord(const FIntPoint& GridCoord) const
{
    return GridCoord.X >= 0 && GridCoord.X < GridSize.X &&
           GridCoord.Y >= 0 && GridCoord.Y < GridSize.Y;
}

bool AAO_GridManager::HasWallBetween(const FIntPoint& From, const FIntPoint& To) const
{
	FIntPoint Delta = To - From;
    
	for (const TObjectPtr<AAO_GridWall>& Wall : Walls)
	{
		if (!Wall)
		{
			continue;
		}
        
		FIntPoint WallCell = Wall->GetGridCell();
		EWallOrientation WallOrientation = Wall->GetOrientation();
		// Vertical벽은 현재 셀의 오른쪽을 막음. (2사분면)
		// Horizontal벽은 현재 셀의 아래쪽을 막음.
		// 서쪽 이동 (X+1)(2사분면)
		if (Delta.X == 1 && Delta.Y == 0)
		{
			// 세로 벽이 사이에 있는지
			if (WallOrientation == EWallOrientation::Vertical)
			{
				// From 셀의 동쪽 경계 = X좌표가 To.X인 세로 벽
				if (WallCell.X == To.X && WallCell.Y == From.Y)
				{
					return true;
				}
			}
		}
		// 동쪽 이동 (X-1)(2사분면)
		else if (Delta.X == -1 && Delta.Y == 0)
		{
			// 세로 벽이 사이에 있는지
			if (WallOrientation == EWallOrientation::Vertical)
			{
				// From 셀의 서쪽 경계 = X좌표가 From.X인 세로 벽
				if (WallCell.X == From.X && WallCell.Y == From.Y)
				{
					return true;
				}
			}
		}
		// 북쪽 이동 (Y+1)
		else if (Delta.Y == 1 && Delta.X == 0)
		{
			// 가로 벽이 사이에 있는지
			if (WallOrientation == EWallOrientation::Horizontal)
			{
				// From 셀의 북쪽 경계 = Y좌표가 To.Y인 가로 벽
				if (WallCell.Y == To.Y && WallCell.X == From.X)
				{
					return true;
				}
			}
		}
		// 남쪽 이동 (Y-1)
		else if (Delta.Y == -1 && Delta.X == 0)
		{
			// 가로 벽이 사이에 있는지
			if (WallOrientation == EWallOrientation::Horizontal)
			{
				// From 셀의 남쪽 경계 = Y좌표가 From.Y인 가로 벽
				if (WallCell.Y == From.Y && WallCell.X == From.X)
				{
					return true;
				}
			}
		}
	}
    
	return false;
}

bool AAO_GridManager::CanRockMove(const FIntPoint& RockCoord, EGridDirection Direction) const
{
    FIntPoint TargetCoord = RockCoord + GetDirectionVector(Direction);
    
    if (!IsValidGridCoord(TargetCoord))
    {
        return false;
    }
    
    if (HasWallBetween(RockCoord, TargetCoord))
    {
        return false;
    }
    
    if (IsRockAt(TargetCoord))
    {
        return false;
    }
    
    return true;
}

void AAO_GridManager::RegisterRock(AAO_PushableRockElement* Rock, const FIntPoint& GridCoord)
{
    if (!Rock) 
    {
    	return;
    }
    RockMap.Add(GridCoord, Rock);
}

void AAO_GridManager::UnregisterRock(AAO_PushableRockElement* Rock)
{
	if (!Rock) 
	{
		return;
	}
    
    for (auto It = RockMap.CreateIterator(); It; ++It)
    {
        if (It.Value() == Rock)
        {
            It.RemoveCurrent();
            break;
        }
    }
}

void AAO_GridManager::UpdateRockPosition(AAO_PushableRockElement* Rock, const FIntPoint& OldCoord, const FIntPoint& NewCoord)
{
	if (!Rock) 
	{
		return;
	}
    
    RockMap.Remove(OldCoord);
    RockMap.Add(NewCoord, Rock);
}

void AAO_GridManager::RegisterWall(AAO_GridWall* Wall)
{
    if (Wall)
    {
        Walls.AddUnique(Wall);
    }
}

void AAO_GridManager::UnregisterWall(AAO_GridWall* Wall)
{
    Walls.Remove(Wall);
}

FIntPoint AAO_GridManager::GetDirectionVector(EGridDirection Direction)
{
    switch (Direction)
    {
    case EGridDirection::North:
	    {
    		return FIntPoint(0, 1);
	    }
    case EGridDirection::East:
	    {
    		return FIntPoint(1, 0);
	    }
    case EGridDirection::South:
	    {
    		return FIntPoint(0, -1);
	    }
    case EGridDirection::West:
	    {
    		return FIntPoint(-1, 0);
	    }
    }
    return FIntPoint(0, 0);
}

EGridDirection AAO_GridManager::GetDirectionFromVector(const FVector& WorldDirection)
{
    FVector Dir = WorldDirection.GetSafeNormal2D();
    
    float DotNorth = FVector::DotProduct(Dir, FVector(0, 1, 0));
    float DotEast = FVector::DotProduct(Dir, FVector(1, 0, 0));
    float DotSouth = FVector::DotProduct(Dir, FVector(0, -1, 0));
    float DotWest = FVector::DotProduct(Dir, FVector(-1, 0, 0));
    
    float MaxDot = FMath::Max(FMath::Max(DotNorth, DotEast), FMath::Max(DotSouth, DotWest));
    
    if (FMath::IsNearlyEqual(MaxDot, DotNorth))
    {
    	return EGridDirection::North;
    }
    else if (FMath::IsNearlyEqual(MaxDot, DotEast))
    {
    	return EGridDirection::East;
    }
    else if (FMath::IsNearlyEqual(MaxDot, DotSouth))
    {
    	return EGridDirection::South;
    }
    else
    {
    	return EGridDirection::West;
    }
        
}

#if WITH_EDITOR
void AAO_GridManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (bShowDebugGrid)
    {
        DrawDebugGrid();
    }
}
#endif

void AAO_GridManager::DrawDebugGrid()
{
#if ENABLE_DRAW_DEBUG
    if (!GetWorld())
    {
    	return;
    }
    
    FlushPersistentDebugLines(GetWorld());
    
    if (bShowDebugGrid)
    {
        FColor GridColor = FColor::Green;
        float LineThickness = 2.0f;
        
        for (int32 X = 0; X <= GridSize.X; ++X)
        {
            FVector Start = GridOrigin + FVector(X * CellSize, 0, 0);
            FVector End = GridOrigin + FVector(X * CellSize, GridSize.Y * CellSize, 0);
            DrawDebugLine(GetWorld(), Start, End, GridColor, true, -1.0f, 0, LineThickness);
        }
        
        for (int32 Y = 0; Y <= GridSize.Y; ++Y)
        {
            FVector Start = GridOrigin + FVector(0, Y * CellSize, 0);
            FVector End = GridOrigin + FVector(GridSize.X * CellSize, Y * CellSize, 0);
            DrawDebugLine(GetWorld(), Start, End, GridColor, true, -1.0f, 0, LineThickness);
        }
    }
#endif
}