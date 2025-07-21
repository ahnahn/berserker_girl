// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Berserker_girl : ModuleRules
{
	public Berserker_girl(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "GeometryCollectionEngine" , "Niagara", "UMG", "AIModule" });
	}
}
