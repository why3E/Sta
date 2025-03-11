// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Statistic : ModuleRules
{
	public Statistic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
