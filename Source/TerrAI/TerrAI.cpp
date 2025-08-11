// TerrAI.cpp
#include "TerrAI.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

class FTerrAIModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        // Map shader directory for /Project/ virtual path
        FString ShaderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
        AddShaderSourceDirectoryMapping("/Project", ShaderDir);
        
        // Log for verification
        UE_LOG(LogTemp, Warning, TEXT("TerrAI Module: Mapped shader directory %s"), *ShaderDir);
    }
    
    virtual void ShutdownModule() override
    {
        // Cleanup if needed
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FTerrAIModule, TerrAI, "TerrAI");
