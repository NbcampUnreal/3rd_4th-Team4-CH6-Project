// AO_NavArea_SpawnRestriction.cpp

#include "AI/NavArea/AO_NavArea_SpawnRestriction.h"

UAO_NavArea_SpawnRestriction::UAO_NavArea_SpawnRestriction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// NavArea 기본 설정
	DefaultCost = 0.0f;
	FixedAreaEnteringCost = 0.0f;
	DrawColor = FColor::Red; // 시각적으로 구분하기 위해 빨간색
}

