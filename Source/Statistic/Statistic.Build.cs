// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Statistic : ModuleRules
{
	public Statistic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTasks",
			"Niagara",
			"UMG",
			"ProceduralMeshComponent",
			"Json", "JsonUtilities",
			"SlateCore", "Paper2D",
			"CinematicCamera" // Cinematic Camera 모듈 추가
		});
	}
}
