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
                "CoreOnline",
                "CoreUObject",
                "Engine",
                "PhysicsCore",
                "InputCore",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "ModularGameplay",
                "ModularGameplayActors",
                "UMG",
                "CommonLoadingScreen",
                "Niagara",
                "GameFeatures",
                "AsyncMixin",
                // if using GAS:
                // "GameplayAbilities",
                // "GameplayTags",
                // "GameplayTasks",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // e.g. "Slate", "SlateCore" if you need them
                "InputCore",
                "Slate",           // Lyra has these
                "SlateCore",       // in its PrivateDependencies
                "EnhancedInput",
                "NetCore",
                "DeveloperSettings",
                "CommonUI",
                "CommonInput",     // needed by the CommonUI ScaleBox slot
                "CommonGame",
                "CommonUser",
                "GameplayMessageRuntime",
                "UIExtension",
                "AudioModulation",
                "EngineSettings",
            }
        );
        
        // (You can also add DynamicallyLoadedModuleNames here if needed)
        
        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
