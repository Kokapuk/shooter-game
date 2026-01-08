// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ShooterGame : ModuleRules
{
	public ShooterGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG" });

    RuntimeDependencies.Add("$(TargetOutputDir)/steam_appid.txt", Path.Combine(ModuleDirectory, "../../RuntimeDependencies/steam_appid.txt"));
  }
}
