// TerrAI.Build.cs - Fixed build configuration for water system
using UnrealBuildTool;

public class TerrAI : ModuleRules
{
    public TerrAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core dependencies (always needed)
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "ProceduralMeshComponent",
            "RenderCore",
            "RHI"
        });

        // Private dependencies (internal use only)
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "UMG"
        });

        // Water system requires these additional modules
        PublicDependencyModuleNames.AddRange(new string[] {
            "Engine",              // For texture creation and updates
            "RenderCore",          // For texture operations
            "RHI"                  // For low-level rendering
        });

        // Editor-only dependencies (only include if building for editor)
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "ToolMenus",
                "EditorStyle",
                "EditorWidgets",
                "UnrealEd",
                "Landscape"
            });
        }

        // Performance and threading
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core"
        });

        // Enable optimizations
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        
        // Remove the undefined identifier warnings line - it's deprecated
        // bEnableUndefinedIdentifierWarnings = false; // REMOVED - deprecated in UE5
    }
}
