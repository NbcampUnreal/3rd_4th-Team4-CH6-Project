//KSJ : AO_Area_SpawnIntensive

#include "AI/Area/AO_Area_SpawnIntensive.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "Kismet/GameplayStatics.h"

AAO_Area_SpawnIntensive::AAO_Area_SpawnIntensive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// NavModifierVolume 기본 설정
}

int32 AAO_Area_SpawnIntensive::GetCurrentMonsterCountInArea() const
{
	if (!GetWorld())
	{
		return 0;
	}

	int32 Count = 0;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAO_AICharacterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (IsValid(Actor) && EncompassesPoint(Actor->GetActorLocation()))
		{
			Count++;
		}
	}

	return Count;
}

