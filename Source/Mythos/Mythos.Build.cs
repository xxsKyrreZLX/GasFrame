// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Mythos : ModuleRules
{
	public Mythos(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Mythos",
			"Mythos/Core/AbilitySystem",
			"Mythos/Variant_Platforming",
			"Mythos/Variant_Combat",
			"Mythos/Variant_Combat/AI",
			"Mythos/Variant_SideScrolling",
			"Mythos/Variant_SideScrolling/Gameplay",
			"Mythos/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
