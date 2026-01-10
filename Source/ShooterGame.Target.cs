// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShooterGameTarget : TargetRules
{
	public ShooterGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.Add("ShooterGame");
	}
}