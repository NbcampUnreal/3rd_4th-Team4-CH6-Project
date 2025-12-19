// AO_UIActionKeySubsystem.cpp

#include "UI/AO_UIActionKeySubsystem.h"

#include "InputAction.h"
#include "InputMappingContext.h"

namespace AOUIActionKeyPath
{
	static const TCHAR* IMC_UI = TEXT("/Game/AVaOut/UI/ActionKey/IMC_UI.IMC_UI");
	static const TCHAR* IA_UI_Open = TEXT("/Game/AVaOut/UI/ActionKey/IA_UI_Open.IA_UI_Open");
	static const TCHAR* IA_UI_Close = TEXT("/Game/AVaOut/UI/ActionKey/IA_UI_Close.IA_UI_Close");
}

void UAO_UIActionKeySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshCache();
}

bool UAO_UIActionKeySubsystem::IsUIOpenKey(const FKey& Key) const
{
	return CachedOpenKeys.Contains(Key);
}

bool UAO_UIActionKeySubsystem::IsUICloseKey(const FKey& Key) const
{
	return CachedCloseKeys.Contains(Key);
}

void UAO_UIActionKeySubsystem::RefreshCache()
{
	CachedOpenKeys.Reset();
	CachedCloseKeys.Reset();

	CachedIMC = LoadObject<UInputMappingContext>(nullptr, AOUIActionKeyPath::IMC_UI);
	if (!CachedIMC)
	{
		return;
	}

	CachedOpenAction = LoadObject<UInputAction>(nullptr, AOUIActionKeyPath::IA_UI_Open);
	CachedCloseAction = LoadObject<UInputAction>(nullptr, AOUIActionKeyPath::IA_UI_Close);

	if (!CachedOpenAction && !CachedCloseAction)
	{
		return;
	}

	const TArray<FEnhancedActionKeyMapping>& Mappings = CachedIMC->GetMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (!Mapping.Action)
		{
			continue;
		}

		if (CachedOpenAction && Mapping.Action == CachedOpenAction)
		{
			AddUniqueKey(CachedOpenKeys, Mapping.Key);
		}

		if (CachedCloseAction && Mapping.Action == CachedCloseAction)
		{
			AddUniqueKey(CachedCloseKeys, Mapping.Key);
		}
	}
}

void UAO_UIActionKeySubsystem::AddUniqueKey(TArray<FKey>& Keys, const FKey& Key)
{
	if (!Key.IsValid())
	{
		return;
	}

	if (Keys.Contains(Key))
	{
		return;
	}

	Keys.Add(Key);
}

UInputMappingContext* UAO_UIActionKeySubsystem::GetUIIMC() const
{
	return CachedIMC;
}

UInputAction* UAO_UIActionKeySubsystem::GetUIOpenAction() const
{
	return CachedOpenAction;
}

UInputAction* UAO_UIActionKeySubsystem::GetUICloseAction() const
{
	return CachedCloseAction;
}
