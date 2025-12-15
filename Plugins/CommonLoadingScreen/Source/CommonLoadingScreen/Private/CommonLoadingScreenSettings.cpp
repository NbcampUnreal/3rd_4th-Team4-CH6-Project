// Copyright Epic Games, Inc. All Rights Reserved.
// CommonLoadingScreenSettings.cpp (From Lyra, 장주만 수정)

#include "CommonLoadingScreenSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonLoadingScreenSettings)

UCommonLoadingScreenSettings::UCommonLoadingScreenSettings()
{
	CategoryName = TEXT("Game");
}

FSoftClassPath UCommonLoadingScreenSettings::GetLoadingScreenWidgetPathForMap(const FString& MapName) const
{
	// if (!ensureMsgf(!MapName.IsEmpty(), TEXT("Map Name is Empty"))) // JM : ensure할 필요는 없을듯
	if (MapName.IsEmpty())
	{
		return DefaultLoadingScreenWidget;	// 맵 이름이 비어있으면 기본값 반환
	}

	for (const FLoadingScreenMapping& Mapping : LoadingScreenMappings)
	{
		if (!Mapping.MapKeyword.IsEmpty() && MapName.Contains(Mapping.MapKeyword))
		{
			return Mapping.LoadingScreenWidget;
		}
	}

	return DefaultLoadingScreenWidget;	// 매칭되는 것이 없으면 기본 값 반환
}

