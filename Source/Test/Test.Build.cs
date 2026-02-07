using UnrealBuildTool;
using System.Collections.Generic;

public class Test : ModuleRules
{
    public Test(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "AIModule",         // ensure AI APIs are available
                "NavigationSystem",  // <-- add this so UNavigationSystemV1 links
                "GameplayStateTreeModule", // ensure StateTree AI component API is available
                "EnhancedInput" // Enhanced Input plugin for advanced input handling
            }
        );

        // Slate/SlateCore are required for SVirtualJoystick and other Slate UI widgets
        // UMG is required for CreateWidget / UUserWidget usage
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });

        // Uncomment if you use Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // If you need online features:
        // DynamicallyLoadedModuleNames.Add("OnlineSubsystem");
    }
}