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
			
			// Session
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			
			// Pose Search
			"PoseSearch",
			
			// GAS
			"GameplayTags",
			"GameplayAbilities",
			"GameplayTasks",
			
			// 카오스 디스트럭션
			"ChaosCaching",
			"GeometryCollectionEngine",
			
			// AI
			"AIModule",
			"NavigationSystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"OnlineSubsystemSteam"
		});

		PublicIncludePaths.AddRange(new string[] { "AO", "AO/Public" });
	}
}
