#include "GPUComputeManager.h"
#include "Engine/Engine.h"
#include "RenderingThread.h"
#include "RHIStaticStates.h"
#include "ShaderCompilerCore.h"
#include "PipelineStateCache.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GenericPlatform/GenericPlatformProperties.h"
#include "RHI.h"

void UGPUComputeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPUComputeManager Initializing ==="));
    
    // Detect GPU capabilities
    DetectGPUCapabilities();
    
    // Auto-configure based on detection
    AutoConfigureQuality();
    
    // Register console commands
    RegisterConsoleCommands();
    
    // Log final configuration
    LogConfiguration();
    
    bGPUInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPUComputeManager Ready ==="));
}

void UGPUComputeManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("GPUComputeManager: Shutting down"));
    
    // Clean up texture pool
    ReleaseAllTextures();
    
    // Unregister console commands
    UnregisterConsoleCommands();
    
    bGPUInitialized = false;
    
    Super::Deinitialize();
}

UGPUComputeManager* UGPUComputeManager::Get(const UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!World) return nullptr;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return nullptr;
    
    return GameInstance->GetSubsystem<UGPUComputeManager>();
}

void UGPUComputeManager::DetectGPUCapabilities()
{
    // FIXED: Use correct API for UE 5.5
    CurrentConfig.bSupportsCompute = GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM5;
    
    // Check async compute support
    CurrentConfig.bSupportsAsyncCompute = GSupportsEfficientAsyncCompute;
    
    // Check 16-bit operations support
    CurrentConfig.bSupports16BitOps = GPixelFormats[PF_FloatR11G11B10].Supported;
    
    // FIXED: Check wave operations - it returns an enum, not bool
    ERHIFeatureSupport WaveSupport = FDataDrivenShaderPlatformInfo::GetSupportsWaveOperations(GMaxRHIShaderPlatform);
    CurrentConfig.bSupportsWaveOps = (WaveSupport != ERHIFeatureSupport::Unsupported);
    
    // Get max texture dimensions
    CurrentConfig.MaxTextureSize = GetMax2DTextureDimension();
    
    // Get compute limits
    CurrentConfig.MaxComputeThreadsPerGroup = 1024; // Standard for most GPUs
    CurrentConfig.MaxUAVs = 8; // Conservative limit
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Capabilities Detected:"));
    UE_LOG(LogTemp, Warning, TEXT("  Compute Shaders: %s"), CurrentConfig.bSupportsCompute ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Async Compute: %s"), CurrentConfig.bSupportsAsyncCompute ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  16-bit Ops: %s"), CurrentConfig.bSupports16BitOps ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Wave Ops: %s"), CurrentConfig.bSupportsWaveOps ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Max Texture: %d"), CurrentConfig.MaxTextureSize);
}

void UGPUComputeManager::AutoConfigureQuality()
{
    EGPUQualityTier DetectedTier = EGPUQualityTier::Desktop;
    
    // FIXED: Platform-agnostic detection
    #if PLATFORM_IOS || PLATFORM_ANDROID
        DetectedTier = EGPUQualityTier::Mobile;
    #elif PLATFORM_MAC
        // Mac can be anything from laptop to desktop
        // Check available memory to determine tier
        FTextureMemoryStats MemStats;
        RHIGetTextureMemoryStats(MemStats);
        int32 VRAMMb = MemStats.TotalGraphicsMemory / (1024 * 1024);
        
        if (VRAMMb >= 8000)
        {
            DetectedTier = EGPUQualityTier::Enthusiast;
        }
        else if (VRAMMb >= 4000)
        {
            DetectedTier = EGPUQualityTier::Desktop;
        }
        else
        {
            DetectedTier = EGPUQualityTier::Balanced;
        }
    #elif WITH_EDITOR
        // In editor, default to Desktop
        DetectedTier = EGPUQualityTier::Desktop;
    #else
        // Other platforms - check VRAM
        FTextureMemoryStats MemStats;
        RHIGetTextureMemoryStats(MemStats);
        int32 VRAMMb = MemStats.TotalGraphicsMemory / (1024 * 1024);
        
        if (VRAMMb >= 8000) // 8GB+ VRAM
        {
            DetectedTier = EGPUQualityTier::Enthusiast;
        }
        else if (VRAMMb >= 4000) // 4GB+ VRAM
        {
            DetectedTier = EGPUQualityTier::Desktop;
        }
        else if (VRAMMb >= 2000) // 2GB+ VRAM
        {
            DetectedTier = EGPUQualityTier::Balanced;
        }
        else
        {
            DetectedTier = EGPUQualityTier::Mobile;
        }
    #endif
    
    ConfigureForTier(DetectedTier);
    
    UE_LOG(LogTemp, Warning, TEXT("Auto-configured GPU Quality Tier: %s"),
           *UEnum::GetValueAsString(DetectedTier));
}

void UGPUComputeManager::SetQualityTier(EGPUQualityTier NewTier)
{
    EGPUQualityTier OldTier = CurrentConfig.QualityTier;
    
    if (OldTier != NewTier)
    {
        ConfigureForTier(NewTier);
        
        UE_LOG(LogTemp, Warning, TEXT("GPU Quality Tier changed: %s -> %s"),
               *UEnum::GetValueAsString(OldTier),
               *UEnum::GetValueAsString(NewTier));
        
        // Notify all systems of config change
        OnGPUConfigChanged.Broadcast(CurrentConfig);
    }
}

void UGPUComputeManager::ConfigureForTier(EGPUQualityTier Tier)
{
    CurrentConfig.QualityTier = Tier;
    
    switch (Tier)
    {
        case EGPUQualityTier::Mobile:
            // Minimum settings for mobile/integrated GPUs
            CurrentConfig.SimulationGridSize = 128;
            CurrentConfig.RenderGridSize = 256;
            CurrentConfig.DetailGridSize = 64;
            CurrentConfig.PhysicsUpdateRate = 10.0f;
            CurrentConfig.VisualUpdateRate = 30.0f;
            CurrentConfig.TextureMemoryBudget = 128;
            CurrentConfig.ComputeMemoryBudget = 64;
            break;
            
        case EGPUQualityTier::Balanced:
            // Steam Deck, consoles, mid-range laptops
            CurrentConfig.SimulationGridSize = 192;
            CurrentConfig.RenderGridSize = 384;
            CurrentConfig.DetailGridSize = 96;
            CurrentConfig.PhysicsUpdateRate = 20.0f;
            CurrentConfig.VisualUpdateRate = 60.0f;
            CurrentConfig.TextureMemoryBudget = 256;
            CurrentConfig.ComputeMemoryBudget = 128;
            break;
            
        case EGPUQualityTier::Desktop:
            // Standard desktop GPUs
            CurrentConfig.SimulationGridSize = 256;
            CurrentConfig.RenderGridSize = 512;
            CurrentConfig.DetailGridSize = 128;
            CurrentConfig.PhysicsUpdateRate = 30.0f;
            CurrentConfig.VisualUpdateRate = 60.0f;
            CurrentConfig.TextureMemoryBudget = 512;
            CurrentConfig.ComputeMemoryBudget = 256;
            break;
            
        case EGPUQualityTier::Enthusiast:
            // High-end GPUs (RTX 3080+, 6800XT+)
            CurrentConfig.SimulationGridSize = 512;
            CurrentConfig.RenderGridSize = 1024;
            CurrentConfig.DetailGridSize = 256;
            CurrentConfig.PhysicsUpdateRate = 60.0f;
            CurrentConfig.VisualUpdateRate = 120.0f;
            CurrentConfig.TextureMemoryBudget = 1024;
            CurrentConfig.ComputeMemoryBudget = 512;
            break;
    }
}

int32 UGPUComputeManager::GetAtmosphereGridSize() const
{
    // Atmosphere can be lower resolution (volumetric, less detail needed)
    return FMath::Min(256, CurrentConfig.SimulationGridSize);
}

int32 UGPUComputeManager::GetWaterGridSize() const
{
    // Water needs full resolution for flow detail
    return CurrentConfig.SimulationGridSize;
}

int32 UGPUComputeManager::GetErosionGridSize() const
{
    // Erosion runs less frequently, can be lower res
    return CurrentConfig.SimulationGridSize / 2;
}

int32 UGPUComputeManager::GetVegetationGridSize() const
{
    // Vegetation detail layer
    return CurrentConfig.DetailGridSize;
}

UTextureRenderTarget2D* UGPUComputeManager::AllocateComputeTexture(
    const FString& Name,
    int32 Width,
    int32 Height,
    ETextureRenderTargetFormat Format,
    bool bUAVCompatible)
{
    // Check if texture already exists
    if (TexturePool.Contains(Name))
    {
        return TexturePool[Name];
    }
    
    // Create new texture
    UTextureRenderTarget2D* NewTexture = NewObject<UTextureRenderTarget2D>(
        GetTransientPackage(),
        *FString::Printf(TEXT("GPU_%s"), *Name)
    );
    
    NewTexture->RenderTargetFormat = Format;
    NewTexture->InitAutoFormat(Width, Height);
    NewTexture->bCanCreateUAV = bUAVCompatible;
    NewTexture->UpdateResourceImmediate();
    
    // Add to pool
    TexturePool.Add(Name, NewTexture);
    
    // Update memory tracking
    int32 BytesPerPixel = 4; // Simplified, would calculate from format
    float TextureMB = (Width * Height * BytesPerPixel) / (1024.0f * 1024.0f);
    MemoryUsageMB += TextureMB;
    
    UE_LOG(LogTemp, Verbose, TEXT("Allocated GPU Texture '%s': %dx%d (%.2f MB)"),
           *Name, Width, Height, TextureMB);
    
    return NewTexture;
}

void UGPUComputeManager::ReleaseComputeTexture(const FString& Name)
{
    if (UTextureRenderTarget2D** Found = TexturePool.Find(Name))
    {
        UTextureRenderTarget2D* Texture = *Found;
        if (Texture)
        {
            // Update memory tracking
            int32 BytesPerPixel = 4;
            float TextureMB = (Texture->SizeX * Texture->SizeY * BytesPerPixel) / (1024.0f * 1024.0f);
            MemoryUsageMB -= TextureMB;
            
            // Release texture
            Texture->ReleaseResource();
        }
        
        TexturePool.Remove(Name);
        
        UE_LOG(LogTemp, Verbose, TEXT("Released GPU Texture '%s'"), *Name);
    }
}

void UGPUComputeManager::ReleaseAllTextures()
{
    for (auto& Pair : TexturePool)
    {
        if (Pair.Value)
        {
            Pair.Value->ReleaseResource();
        }
    }
    
    TexturePool.Empty();
    MemoryUsageMB = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Released all GPU textures"));
}

float UGPUComputeManager::GetGPUMemoryUsageMB() const
{
    return MemoryUsageMB;
}

bool UGPUComputeManager::IsGPUBottlenecked() const
{
    return LastGPUFrameTime > GPUBudgetMS;
}

void UGPUComputeManager::UpdatePerformanceMetrics()
{
    // This would be called from render thread with actual GPU timing
    // For now, using placeholder
    FRenderCommandFence Fence;
    Fence.BeginFence();
    Fence.Wait();
    
    // In production, use RHI GPU timing queries
    LastGPUFrameTime = 8.0f; // Placeholder
}

void UGPUComputeManager::LogConfiguration() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== GPU Configuration ==="));
    UE_LOG(LogTemp, Warning, TEXT("Quality Tier: %s"), *UEnum::GetValueAsString(CurrentConfig.QualityTier));
    UE_LOG(LogTemp, Warning, TEXT("Grid Sizes:"));
    UE_LOG(LogTemp, Warning, TEXT("  Atmosphere: %d"), GetAtmosphereGridSize());
    UE_LOG(LogTemp, Warning, TEXT("  Water: %d"), GetWaterGridSize());
    UE_LOG(LogTemp, Warning, TEXT("  Erosion: %d"), GetErosionGridSize());
    UE_LOG(LogTemp, Warning, TEXT("  Vegetation: %d"), GetVegetationGridSize());
    UE_LOG(LogTemp, Warning, TEXT("Update Rates:"));
    UE_LOG(LogTemp, Warning, TEXT("  Physics: %.1f Hz"), CurrentConfig.PhysicsUpdateRate);
    UE_LOG(LogTemp, Warning, TEXT("  Visual: %.1f Hz"), CurrentConfig.VisualUpdateRate);
    UE_LOG(LogTemp, Warning, TEXT("Memory Budgets:"));
    UE_LOG(LogTemp, Warning, TEXT("  Textures: %d MB"), CurrentConfig.TextureMemoryBudget);
    UE_LOG(LogTemp, Warning, TEXT("  Compute: %d MB"), CurrentConfig.ComputeMemoryBudget);
}

void UGPUComputeManager::RegisterConsoleCommands()
{
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("GPU.SetQuality"),
        TEXT("Set GPU compute quality tier (0=Mobile, 1=Balanced, 2=Desktop, 3=Enthusiast)"),
        FConsoleCommandWithArgsDelegate::CreateLambda([this](const TArray<FString>& Args)
        {
            if (Args.Num() > 0)
            {
                int32 Quality = FCString::Atoi(*Args[0]);
                if (Quality >= 0 && Quality <= 3)
                {
                    SetQualityTier((EGPUQualityTier)Quality);
                }
            }
        }),
        ECVF_Default
    ));
    
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("GPU.ShowConfig"),
        TEXT("Display current GPU configuration"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            LogConfiguration();
        }),
        ECVF_Default
    ));
    
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("GPU.ShowMemory"),
        TEXT("Display GPU memory usage"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            UE_LOG(LogTemp, Warning, TEXT("GPU Memory Usage:"));
            UE_LOG(LogTemp, Warning, TEXT("  Allocated: %.2f MB"), MemoryUsageMB);
            UE_LOG(LogTemp, Warning, TEXT("  Budget: %d MB"), CurrentConfig.TextureMemoryBudget);
            UE_LOG(LogTemp, Warning, TEXT("  Textures in pool: %d"), TexturePool.Num());
        }),
        ECVF_Default
    ));
}

void UGPUComputeManager::UnregisterConsoleCommands()
{
    for (IConsoleObject* Command : ConsoleCommands)
    {
        IConsoleManager::Get().UnregisterConsoleObject(Command);
    }
    ConsoleCommands.Empty();
}
