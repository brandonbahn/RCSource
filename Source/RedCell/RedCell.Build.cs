// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RedCell : ModuleRules
{
    public RedCell(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
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
        
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreOnline",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "PhysicsCore",
                "InputCore",
                "GameplayCameras",
                "MotionWarping",
                "AnimGraphRuntime",
                "AnimationWarpingRuntime",
                "Chooser",
                "PoseSearch",
                "StructUtils",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "AIModule",
                "ModularGameplay",
                "ModularGameplayActors",
                "DataRegistry",
                "UMG",
                "CommonLoadingScreen",
                "Niagara",
                "GameFeatures",
                "SignificanceManager",
                "AsyncMixin",
                "PropertyPath",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "InputCore",
                "Slate",
                "SlateCore",
                "EnhancedInput",
                "MotionWarping",
                "NetCore",
                "Projects",
                "Gauntlet",
                "DeveloperSettings",
                "RHI",
                "CommonUI",
                "CommonInput",
                "GameSettings",
                "CommonGame",
                "CommonUser",
                "GameplayMessageRuntime",
                "UIExtension",
                "ClientPilot",
                "AudioModulation",
                "EngineSettings",
                "DTLSHandlerComponent",
                "Json",
            }
        );
        
        // (You can also add DynamicallyLoadedModuleNames here if needed)
        
        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
