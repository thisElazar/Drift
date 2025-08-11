// TerrAI.Build.cs - Fixed build configuration with shader support
using UnrealBuildTool;
using System.IO;  // Add this for Path class

public class TerrAI : ModuleRules
{
    public TerrAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // ========== SHADER DIRECTORY SETUP ==========
        // Map the /Project/ shader directory to your project's Shaders folder
        string ProjectShaderDir = Path.Combine(ModuleDirectory, "..", "..", "Shaders");
        if (Directory.Exists(ProjectShaderDir))
        {
            PublicIncludePaths.Add(Path.GetFullPath(ProjectShaderDir));
        }

        // Core dependencies (always needed)
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "ProceduralMeshComponent",
            "Niagara",
            "NiagaraCore"
        });

        // Rendering dependencies for shader support
        PublicDependencyModuleNames.AddRange(new string[] {
            "Renderer",        // Add this for shader support
            "RenderCore",
            "RHI",
            "RHICore",
            "Projects"         // Add this for shader mapping
        });

        // Private dependencies (internal use only)
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "UMG"
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

        // Enable optimizations
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        
    }
}
