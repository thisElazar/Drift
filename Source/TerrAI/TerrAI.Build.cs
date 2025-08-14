// TerrAI.Build.cs - Complete build configuration with shader support
using UnrealBuildTool;
using System.IO;

public class TerrAI : ModuleRules
{
    public TerrAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // ========== SHADER DIRECTORY SETUP ==========
        // Map the project's Shaders folder for custom shaders
        string ProjectShaderDir = Path.Combine(ModuleDirectory, "..", "..", "Shaders");
        if (Directory.Exists(ProjectShaderDir))
        {
            PublicIncludePaths.Add(Path.GetFullPath(ProjectShaderDir));
        }

        // ========== CORE DEPENDENCIES ==========
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

        // ========== RENDERING DEPENDENCIES FOR SHADERS ==========
        PublicDependencyModuleNames.AddRange(new string[] {
            "Renderer",
            "RenderCore",
            "RHI",
            "Projects",
            "Slate",
            "SlateCore"
        });

        // ========== PRIVATE DEPENDENCIES ==========
        PrivateDependencyModuleNames.AddRange(new string[] {
            "UMG",
            "ToolMenus"
        });

        // ========== EDITOR-ONLY DEPENDENCIES ==========
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "Landscape",
                "EditorSubsystem"
            });
        }

        // ========== OPTIMIZATION SETTINGS ==========
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bEnableExceptions = false;
        
        // ========== PREPROCESSOR DEFINITIONS ==========
        PublicDefinitions.Add("WITH_TERRAI_SHADERS=1");
        
        // Add shader directory to includes
        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Public"),
            Path.Combine(ModuleDirectory, "Public/Shaders")
        });
        
        PrivateIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Private"),
            Path.Combine(ModuleDirectory, "Private/Shaders")
        });
    }
}