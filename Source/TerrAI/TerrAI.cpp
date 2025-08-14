// TerrAI.cpp - Main module implementation with shader registration
#include "TerrAI.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"
#include "RenderingThread.h"
#include "GlobalShader.h"

#define LOCTEXT_NAMESPACE "FTerrAIModule"

// Log category definition
DEFINE_LOG_CATEGORY_STATIC(LogTerrAI, Log, All);

class FTerrAIModule : public FDefaultGameModuleImpl
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Shader directory mapping handle */
    FString ShaderDirectory;
};

void FTerrAIModule::StartupModule()
{
    // Call parent implementation
    FDefaultGameModuleImpl::StartupModule();
    
    // ========== SHADER DIRECTORY REGISTRATION ==========
    // Get the base directory of the project
    FString ProjectDir = FPaths::ProjectDir();
    ShaderDirectory = FPaths::Combine(ProjectDir, TEXT("Shaders"));
    
    // Convert to full path
    ShaderDirectory = FPaths::ConvertRelativePathToFull(ShaderDirectory);
    
    // Verify the shader directory exists
    if (!FPaths::DirectoryExists(ShaderDirectory))
    {
        UE_LOG(LogTerrAI, Warning, TEXT("Shader directory does not exist, creating: %s"), *ShaderDirectory);
        IFileManager::Get().MakeDirectory(*ShaderDirectory, true);
    }
    
    // Map shader directory - this is where /Project/ maps to in your shaders
    AddShaderSourceDirectoryMapping(TEXT("/Project"), ShaderDirectory);
    
    UE_LOG(LogTerrAI, Log, TEXT("TerrAI Module Started - Shader directory mapped: %s"), *ShaderDirectory);
}

void FTerrAIModule::ShutdownModule()
{
    // Call parent implementation
    FDefaultGameModuleImpl::ShutdownModule();
    
    UE_LOG(LogTerrAI, Log, TEXT("TerrAI Module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

// CRITICAL: Use IMPLEMENT_PRIMARY_GAME_MODULE only once
// This should be the ONLY place in your entire project where this macro is used
IMPLEMENT_PRIMARY_GAME_MODULE(FTerrAIModule, TerrAI, "TerrAI");
