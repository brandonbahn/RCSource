// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RedCell : ModuleRules
{
    public RedCell(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // === Path setup exactly like Lyra ===
        // Tells UBT to look in Source/RedCell/RedCell for all headers
        PublicIncludePaths.AddRange(
            new string[] {
                "RedCell"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                // (keep empty for now)
            }
        );

        // === dependencies ===
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "ModularGameplay",
                "ModularGameplayActors",
                "UMG",
                "CommonLoadingScreen",
                "GameFeatures",
                // if using GAS:
                // "GameplayAbilities",
                // "GameplayTags",
                // "GameplayTasks",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // e.g. "Slate", "SlateCore" if you need them
                "Slate",           // Lyra has these
                "SlateCore",       // in its PrivateDependencies
                "CommonUI",
                "CommonInput",     // needed by the CommonUI ScaleBox slot
                "CommonGame",
                "CommonUser",
                "UIExtension",
                "AudioModulation",
            }
        );
        
        // (You can also add DynamicallyLoadedModuleNames here if needed)
    }
}
