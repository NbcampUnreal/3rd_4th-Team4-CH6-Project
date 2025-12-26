// AO_NavArea_SpawnIntensive.cpp

#include "AI/NavArea/AO_NavArea_SpawnIntensive.h"

UAO_NavArea_SpawnIntensive::UAO_NavArea_SpawnIntensive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// NavArea 기본 설정
	DefaultCost = 0.0f;
	FixedAreaEnteringCost = 0.0f;
	DrawColor = FColor::Green; // 시각적으로 구분하기 위해 초록색
}

