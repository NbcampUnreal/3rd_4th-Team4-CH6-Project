// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AO : ModuleRules
{
	public AO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			// Initial Dependencies
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"UMG",
			"GameplayTags", 
			"GameplayAbilities", 
			"GameplayTasks", 
			"ChaosCaching", 
			"GeometryCollectionEngine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
		});

		PublicIncludePaths.AddRange(new string[] { "AO", "AO/Public" });
	}
}
