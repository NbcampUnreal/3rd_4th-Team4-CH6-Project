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
			
			//UI
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
			//UI
			"Slate",
			"SlateCore",
			
			// Session
			"OnlineSubsystemSteam"
		});

		PublicIncludePaths.AddRange(new string[] { "AO", "AO/Public" });
	}
}
