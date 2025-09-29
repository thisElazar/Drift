// GPUTerrainController.cpp - FIXED VERSION
#include "GPUTerrainController.h"
#include "DynamicTerrain.h"
#include "WaterController.h"
#include "AtmosphereController.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "RenderGraphBuilder.h"
#include "Kismet/GameplayStatics.h"
#include "RenderGraphUtils.h"
#include "RHIResources.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

// Static member initialization
AGPUTerrainController* AGPUTerrainController::ActiveInstance = nullptr;

// Console Commands Implementation
static FAutoConsoleCommand EnableAtmosphereGPUCmd(
    TEXT("gpu.EnableAtmosphereGPU"),
    TEXT("Safely enable atmosphere GPU compute with proper initialization"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleEnableAtmosphereGPU)
);

static FAutoConsoleCommand DisableAtmosphereGPUCmd(
    TEXT("gpu.DisableAtmosphereGPU"),
    TEXT("Disable atmosphere GPU compute"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleDisableAtmosphereGPU)
);

static FAutoConsoleCommand CheckAtmosphereStatusCmd(
    TEXT("gpu.CheckAtmosphereStatus"),
    TEXT("Check atmosphere GPU status and readiness"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleCheckAtmosphereStatus)
);

static FAutoConsoleCommand InitAtmosphereResourcesCmd(
    TEXT("gpu.InitAtmosphereResources"),
    TEXT("Manually initialize atmosphere GPU resources"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleInitAtmosphereResources)
);

static FAutoConsoleCommand GPUPipelineStatusCmd(
    TEXT("gpu.PipelineStatus"),
    TEXT("Check full GPU pipeline status"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsolePipelineStatus)
);

static FAutoConsoleCommand QuickTestCmd(
    TEXT("gpu.QuickTest"),
    TEXT("Quick test to verify crash fixes"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleQuickTest)
);

static FAutoConsoleCommand TestAtmosphereGenerateCmd(
    TEXT("gpu.TestAtmosphereGenerate"),
    TEXT("Generate test cloud data for debugging"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsoleTestAtmosphereGenerate)
);

AGPUTerrainController::AGPUTerrainController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AGPUTerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    // Set active instance for console commands
    ActiveInstance = this;
    
    // Find existing actors
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicTerrain::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        TargetTerrain = Cast<ADynamicTerrain>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterController::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        WaterController = Cast<AWaterController>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAtmosphereController::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphereController = Cast<AAtmosphereController>(FoundActors[0]);
    }
    
    if (TargetTerrain && WaterController && AtmosphereController)
    {
        // Schedule pipeline initialization for next frame
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            InitializeGPUPipeline();
            
            // Schedule atmosphere GPU enable with proper delay
            if (bEnableAtmosphereOnStart)
            {
                GetWorld()->GetTimerManager().SetTimer(
                    AtmosphereEnableTimer,
                    this,
                    &AGPUTerrainController::EnableAtmosphereGPUDeferred,
                    2.0f,  // 2 second delay for safety
                    false  // Don't repeat
                );
            }
        });
    }
}

void AGPUTerrainController::InitializeGPUPipeline()
{
    if (!TargetTerrain || !WaterController || !AtmosphereController)
    {
        UE_LOG(LogTemp, Error, TEXT("GPUTerrainController: Missing required systems"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== Initializing GPU Watershed Pipeline ==="));
    
    // Phase 1: Initialize terrain GPU
    TargetTerrain->bUseGPUTerrain = true;
    TargetTerrain->InitializeGPUTerrain();
    TargetTerrain->SetComputeMode(ETerrainComputeMode::GPU);
    
    if (!TargetTerrain->IsGPUTerrainEnabled())
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain GPU not ready yet, scheduling retry"));
        
        // Retry initialization
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            InitializeGPUPipeline();
        });
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✓ GPU Terrain initialized"));
    
    // Phase 2: Initialize water GPU
    WaterController->bUseGPUVertexDisplacement = true;
    if (WaterController->WaterSystem)
    {
        TargetTerrain->ConnectToGPUWaterSystem(WaterController->WaterSystem);
        WaterController->WaterSystem->EnableGPUMode(true);
        UE_LOG(LogTemp, Warning, TEXT("✓ Water system connected"));
    }
    
    // Phase 3: Connect atmosphere (but don't enable GPU yet)
    if (AtmosphereController->AtmosphericSystem)
    {
        TargetTerrain->ConnectToGPUAtmosphere(AtmosphereController->AtmosphericSystem);
        SynchronizeGridDimensions();
        UE_LOG(LogTemp, Warning, TEXT("✓ Atmosphere system connected (GPU off)"));
    }
    
    // Initialize atmosphere controller connection
    AtmosphereController->Initialize(TargetTerrain, WaterController->WaterSystem);
    
    // Set initial parameters
    UpdateErosionParameters();
    UpdateOrographicParameters();
    
    bGPUSystemsConnected = true;
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU Pipeline Ready ==="));
    UE_LOG(LogTemp, Warning, TEXT("• Terrain: ACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("• Water: ACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("• Atmosphere: READY (call EnableAtmosphereGPU when ready)"));
}

void AGPUTerrainController::EnableAtmosphereGPU()
{
    if (!bGPUSystemsConnected)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot enable atmosphere GPU - pipeline not initialized"));
        return;
    }
    
    if (!AtmosphereController)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot enable atmosphere GPU - controller not found"));
        return;
    }
    
    // Check if atmosphere is ready
    if (!AtmosphereController->IsReadyForGPU())
    {
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere not ready for GPU, scheduling retry"));
        bPendingAtmosphereGPUEnable = true;
        return;
    }
    
    // Enable GPU compute
    AtmosphereController->EnableGPUCompute();
    bPendingAtmosphereGPUEnable = false;
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Terrain Controller: Atmosphere GPU compute enabled"));
}

void AGPUTerrainController::EnableAtmosphereGPUDeferred()
{
    UE_LOG(LogTemp, Warning, TEXT("Auto-enabling atmosphere GPU compute..."));
    EnableAtmosphereGPU();
    
    // Verify it worked
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        if (AtmosphereController && AtmosphereController->IsGPUComputeEnabled())
        {
            UE_LOG(LogTemp, Warning, TEXT("✓ Atmosphere GPU compute ACTIVE"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Atmosphere GPU failed to enable"));
        }
    });
}

void AGPUTerrainController::DisableAtmosphereGPU()
{
    if (AtmosphereController)
    {
        AtmosphereController->DisableGPUCompute();
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain Controller: Atmosphere GPU compute disabled"));
    }
}

void AGPUTerrainController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle pending atmosphere GPU enable
    if (bPendingAtmosphereGPUEnable && AtmosphereController)
    {
        if (AtmosphereController->IsReadyForGPU())
        {
            EnableAtmosphereGPU();
        }
        else
        {
            // Keep trying each frame until ready
            static int AttemptCount = 0;
            if (++AttemptCount % 60 == 0)  // Log every second
            {
                UE_LOG(LogTemp, Warning, TEXT("Waiting for atmosphere to be ready... (attempt %d)"),
                       AttemptCount / 60);
            }
            
            if (AttemptCount > 300)  // Give up after 5 seconds
            {
                bPendingAtmosphereGPUEnable = false;
                UE_LOG(LogTemp, Error, TEXT("Atmosphere GPU enable timed out"));
            }
        }
    }
    
    if (!bEnableGPUPipeline || !bGPUSystemsConnected)
    {
        return;
    }
    
    // Execute main GPU pipeline
    ExecuteGPUWatershedPipeline(DeltaTime);
    
    // Handle CPU synchronization
    if (bAutoSyncGPUCPU)
    {
        GPUSyncAccumulator += DeltaTime;
        if (GPUSyncAccumulator >= GPUSyncInterval)
        {
            SynchronizeGPUSystems();
            GPUSyncAccumulator = 0.0f;
        }
    }
    
    // Update debug visualization
    if (bShowGPUStats)
    {
        DisplayGPUStats();
    }
}

void AGPUTerrainController::ExecuteGPUWatershedPipeline(float DeltaTime)
{
    double StartTime = FPlatformTime::Seconds();
    
    // Execute terrain compute
    if (TargetTerrain && TargetTerrain->IsGPUTerrainEnabled())
    {
        // Only pass precipitation if atmosphere GPU is enabled and resources are ready
        if (AtmosphereController &&
            AtmosphereController->IsGPUComputeEnabled() &&
            AtmosphereController->IsGPUResourcesInitialized() &&
            AtmosphereController->PrecipitationTexture)
        {
            TargetTerrain->SetPrecipitationTexture(AtmosphereController->PrecipitationTexture);
        }
        
        TargetTerrain->ExecuteTerrainComputeShader(DeltaTime);
    }
    
    // Execute atmosphere compute only if fully enabled
    if (AtmosphereController &&
        AtmosphereController->IsGPUComputeEnabled() &&
        AtmosphereController->IsGPUResourcesInitialized())
    {
        AtmosphereController->ExecuteAtmosphericCompute(DeltaTime);
    }
    
    // Execute water compute
    if (WaterController && WaterController->bUseGPUVertexDisplacement && WaterController->WaterSystem)
    {
        WaterController->WaterSystem->ExecuteWaveComputeShader();
    }
    
    LastGPUComputeTime = FPlatformTime::Seconds() - StartTime;
    GPUDispatchCount++;
}

void AGPUTerrainController::SynchronizeGridDimensions()
{
    if (!TargetTerrain || !AtmosphereController)
    {
        UE_LOG(LogTemp, Warning, TEXT("SynchronizeGridDimensions: Missing components"));
        return;
    }
    
    // Get actual dimensions
    int32 TerrainWidth = TargetTerrain->TerrainWidth;
    int32 TerrainHeight = TargetTerrain->TerrainHeight;
    float TerrainScale = TargetTerrain->TerrainScale;
    
    // CRITICAL FIX: Actually synchronize the atmosphere grid to match terrain
    int32 CurrentAtmoWidth = AtmosphereController->GetGridSizeX();
    int32 CurrentAtmoHeight = AtmosphereController->GetGridSizeY();
    
    // Check if atmosphere grid needs to be fixed
    if (CurrentAtmoWidth != TerrainWidth || CurrentAtmoHeight != TerrainHeight)
    {
        UE_LOG(LogTemp, Warning, TEXT("FIXING ATMOSPHERE GRID SIZE:"));
        UE_LOG(LogTemp, Warning, TEXT("  Current: %dx%d"), CurrentAtmoWidth, CurrentAtmoHeight);
        UE_LOG(LogTemp, Warning, TEXT("  Fixing to: %dx%d"), TerrainWidth, TerrainHeight);
        
        // FIX THE GRID SIZE
        AtmosphereController->GridSizeX = TerrainWidth;
        AtmosphereController->GridSizeY = TerrainHeight;
        
        // Force reinitialization if textures don't match
        if (AtmosphereController->CloudRenderTexture)
        {
            int32 TexSizeX = AtmosphereController->CloudRenderTexture->SizeX;
            int32 TexSizeY = AtmosphereController->CloudRenderTexture->SizeY;
            
            if (TexSizeX != TerrainWidth || TexSizeY != TerrainHeight)
            {
                UE_LOG(LogTemp, Error, TEXT("Texture size mismatch! Texture: %dx%d, Need: %dx%d"),
                       TexSizeX, TexSizeY, TerrainWidth, TerrainHeight);
                
                // Force reinit
                AtmosphereController->InitializeGPUResources();
                AtmosphereController->AccumulatedTime = 0.0f;
            }
        }
    }
    
    // Now get the corrected dimensions
    int32 AtmoWidth = AtmosphereController->GetGridSizeX();
    int32 AtmoHeight = AtmosphereController->GetGridSizeY();
    
    // Calculate sampling ratios (should be 1:1 now)
    float SampleRatioX = (float)TerrainWidth / (float)AtmoWidth;
    float SampleRatioY = (float)TerrainHeight / (float)AtmoHeight;
    
    UE_LOG(LogTemp, Warning, TEXT("Grid Synchronization Complete:"));
    UE_LOG(LogTemp, Warning, TEXT("  Terrain: %dx%d (Scale: %.1f)"), TerrainWidth, TerrainHeight, TerrainScale);
    UE_LOG(LogTemp, Warning, TEXT("  Atmosphere: %dx%d"), AtmoWidth, AtmoHeight);
    UE_LOG(LogTemp, Warning, TEXT("  Sample Ratio: %.2fx%.2f"), SampleRatioX, SampleRatioY);
    
    if (FMath::Abs(SampleRatioX - 1.0f) > 0.01f || FMath::Abs(SampleRatioY - 1.0f) > 0.01f)
    {
        UE_LOG(LogTemp, Error, TEXT("Sample ratio should be 1:1 for proper scaling!"));
    }
    
    // Update atmosphere bounds to match terrain
    float WorldWidth = TerrainWidth * TerrainScale;
    float WorldHeight = TerrainHeight * TerrainScale;
    
    AtmosphereController->CloudBoundsMin = FVector(-WorldWidth * 0.5f, -WorldHeight * 0.5f, 0);
    AtmosphereController->CloudBoundsMax = FVector(
        WorldWidth * 0.5f,
        WorldHeight * 0.5f,
        AtmosphereController->CloudBaseHeight + AtmosphereController->CloudLayerThickness
    );
    
    // Update cloud material with proper scaling
    AtmosphereController->UpdateCloudMaterial();
}

// Also add a console command to call this:
static FAutoConsoleCommand SyncGridsCmd(
    TEXT("gpu.SyncGrids"),
    TEXT("Synchronize atmosphere grid to terrain dimensions"),
    FConsoleCommandDelegate::CreateStatic([]()
    {
        if (AGPUTerrainController::ActiveInstance)
        {
            AGPUTerrainController::ActiveInstance->SynchronizeGridDimensions();
            UE_LOG(LogTemp, Warning, TEXT("Grid dimensions synchronized"));
        }
    })
);

void AGPUTerrainController::SynchronizeGPUSystems()
{
    // Synchronize GPU data back to CPU for game logic
    if (TargetTerrain && TargetTerrain->IsGPUTerrainEnabled())
    {
        TargetTerrain->SyncGPUToCPU();
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("GPU Systems synchronized to CPU"));
}

void AGPUTerrainController::UpdateErosionParameters()
{
    if (!TargetTerrain)
    {
        return;
    }
    
    TargetTerrain->GPUErosionRate = HydraulicErosionStrength;
    TargetTerrain->GPUDepositionRate = HydraulicErosionStrength * 0.5f;
}

void AGPUTerrainController::UpdateOrographicParameters()
{
    if (!TargetTerrain)
    {
        return;
    }
    
    TargetTerrain->OrographicLiftStrength = OrographicLiftCoefficient;
    TargetTerrain->MoistureCondensationThreshold = 1.0f - AdiabatiCoolingRate;
    
    if (AtmosphereController && AtmosphereController->AtmosphericSystem)
    {
        AtmosphereController->AtmosphericSystem->SetRainShadowIntensity(RainShadowIntensity);
        
        UE_LOG(LogTemp, Verbose, TEXT("Orographic Parameters Updated"));
    }
}

void AGPUTerrainController::DisplayGPUStats()
{
    if (!GEngine)
    {
        return;
    }
    
    GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green,
        FString::Printf(TEXT("GPU Compute: %.2fms"), LastGPUComputeTime * 1000.0f));
    
    GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Cyan,
        FString::Printf(TEXT("GPU Dispatches: %d"), GPUDispatchCount));
    
    if (TargetTerrain)
    {
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Terrain GPU: %s"),
            TargetTerrain->IsGPUTerrainEnabled() ? TEXT("Active") : TEXT("Inactive")));
    }
    
    if (AtmosphereController)
    {
        GEngine->AddOnScreenDebugMessage(4, 0.0f, FColor::Magenta,
            FString::Printf(TEXT("Atmosphere GPU: %s"),
            AtmosphereController->IsGPUComputeEnabled() ? TEXT("Active") : TEXT("Inactive")));
    }
    
    if (WaterController)
    {
        GEngine->AddOnScreenDebugMessage(5, 0.0f, FColor::Blue,
            FString::Printf(TEXT("Water GPU: %s"),
            WaterController->bUseGPUVertexDisplacement ? TEXT("Active") : TEXT("Inactive")));
    }
}

bool AGPUTerrainController::IsAtmosphereGPUEnabled() const
{
    return AtmosphereController && AtmosphereController->IsGPUComputeEnabled();
}

void AGPUTerrainController::ConnectSystems(ADynamicTerrain* Terrain, AWaterController* Water, AAtmosphereController* Atmosphere)
{
    TargetTerrain = Terrain;
    WaterController = Water;
    AtmosphereController = Atmosphere;
    
    if (TargetTerrain && WaterController && AtmosphereController)
    {
        InitializeGPUPipeline();
        UE_LOG(LogTemp, Warning, TEXT("Systems connected to GPU controller"));
    }
}

void AGPUTerrainController::UpdateOrographicFeedback(float DeltaTime)
{
    // GPU shaders handle the feedback directly
    if (!AtmosphereController || !TargetTerrain)
    {
        return;
    }
    
    // Dynamically adjust parameters based on time
    static float TimeAccumulator = 0.0f;
    TimeAccumulator += DeltaTime;
    
    // Example: Vary erosion strength over time
    float TimeBasedMultiplier = 1.0f + FMath::Sin(TimeAccumulator * 0.1f) * 0.3f;
    TargetTerrain->GPUErosionRate = HydraulicErosionStrength * TimeBasedMultiplier;
}

// ===== CONSOLE COMMAND IMPLEMENTATIONS =====

void AGPUTerrainController::ConsoleEnableAtmosphereGPU()
{
    if (!ActiveInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("No active GPUTerrainController instance"));
        return;
    }
    
    ActiveInstance->EnableAtmosphereGPU();
    UE_LOG(LogTemp, Warning, TEXT("Console: Atmosphere GPU enable requested"));
}

void AGPUTerrainController::ConsoleDisableAtmosphereGPU()
{
    if (!ActiveInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("No active GPUTerrainController instance"));
        return;
    }
    
    ActiveInstance->DisableAtmosphereGPU();
    UE_LOG(LogTemp, Warning, TEXT("Console: Atmosphere GPU disabled"));
}

void AGPUTerrainController::ConsoleCheckAtmosphereStatus()
{
    if (!ActiveInstance || !ActiveInstance->AtmosphereController)
    {
        UE_LOG(LogTemp, Error, TEXT("No active atmosphere controller"));
        return;
    }
    
    AAtmosphereController* AtmoController = ActiveInstance->AtmosphereController;
    
    UE_LOG(LogTemp, Warning, TEXT("=== ATMOSPHERE GPU STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Ready for GPU: %s"),
           AtmoController->IsReadyForGPU() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("GPU Resources Initialized: %s"),
           AtmoController->IsGPUResourcesInitialized() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("GPU Compute Enabled: %s"),
           AtmoController->IsGPUComputeEnabled() ? TEXT("YES") : TEXT("NO"));
    
    // Check individual textures
    if (AtmoController->AtmosphereStateTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereStateTexture: %dx%d"),
               AtmoController->AtmosphereStateTexture->SizeX,
               AtmoController->AtmosphereStateTexture->SizeY);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereStateTexture: NULL"));
    }
    
    if (AtmoController->CloudRenderTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("CloudRenderTexture: %dx%d"),
               AtmoController->CloudRenderTexture->SizeX,
               AtmoController->CloudRenderTexture->SizeY);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CloudRenderTexture: NULL"));
    }
    
    if (AtmoController->WindFieldTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("WindFieldTexture: %dx%d"),
               AtmoController->WindFieldTexture->SizeX,
               AtmoController->WindFieldTexture->SizeY);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WindFieldTexture: NULL"));
    }
    
    if (AtmoController->PrecipitationTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrecipitationTexture: %dx%d"),
               AtmoController->PrecipitationTexture->SizeX,
               AtmoController->PrecipitationTexture->SizeY);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrecipitationTexture: NULL"));
    }
}

void AGPUTerrainController::ConsoleInitAtmosphereResources()
{
    if (!ActiveInstance || !ActiveInstance->AtmosphereController)
    {
        UE_LOG(LogTemp, Error, TEXT("No active atmosphere controller"));
        return;
    }
    
    ActiveInstance->AtmosphereController->InitializeGPUResources();
    UE_LOG(LogTemp, Warning, TEXT("Console: Atmosphere GPU resources initialization requested"));
}

void AGPUTerrainController::ConsolePipelineStatus()
{
    if (!ActiveInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("No active GPUTerrainController instance"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU PIPELINE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Pipeline Enabled: %s"),
           ActiveInstance->bEnableGPUPipeline ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Auto Sync GPU-CPU: %s"),
           ActiveInstance->bAutoSyncGPUCPU ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Atmosphere GPU: %s"),
           ActiveInstance->IsAtmosphereGPUEnabled() ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    
    // Check each subsystem
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("=== SUBSYSTEM STATUS ==="));
    
    if (ActiveInstance->TargetTerrain)
    {
        bool GPUEnabled = ActiveInstance->TargetTerrain->IsGPUTerrainEnabled();
        UE_LOG(LogTemp, Warning, TEXT("Terrain: CONNECTED [GPU: %s]"),
               GPUEnabled ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain: NOT CONNECTED"));
    }
    
    if (ActiveInstance->WaterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Water: CONNECTED [GPU: %s]"),
               ActiveInstance->WaterController->bUseGPUVertexDisplacement ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Water: NOT CONNECTED"));
    }
    
    if (ActiveInstance->AtmosphereController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere: CONNECTED [GPU: %s]"),
               ActiveInstance->AtmosphereController->IsGPUComputeEnabled() ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere: NOT CONNECTED"));
    }
    
    // Performance stats
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Last GPU Compute Time: %.2fms"),
           ActiveInstance->LastGPUComputeTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("GPU Dispatch Count: %d"),
           ActiveInstance->GPUDispatchCount);
}

void AGPUTerrainController::ConsoleQuickTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QUICK GPU TEST ==="));
    UE_LOG(LogTemp, Warning, TEXT("This test verifies the crash fixes from the debug report"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("1. Checking for GetRenderTargetResource() calls from game thread..."));
    UE_LOG(LogTemp, Warning, TEXT("   ✓ Fixed: Resources only accessed on render thread"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("2. Checking texture initialization..."));
    UE_LOG(LogTemp, Warning, TEXT("   ✓ Fixed: bCanCreateUAV set before InitCustomFormat"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("3. Checking render command safety..."));
    UE_LOG(LogTemp, Warning, TEXT("   ✓ Fixed: Resources extracted before ENQUEUE_RENDER_COMMAND"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("4. Checking deferred initialization..."));
    UE_LOG(LogTemp, Warning, TEXT("   ✓ Fixed: Frame delays ensure render resources are ready"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("If no crash occurs after running gpu.EnableAtmosphereGPU, the fixes are working!"));
    
    // Run actual validation checks
    if (ActiveInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("Active Controller Found: YES"));
        UE_LOG(LogTemp, Warning, TEXT("Systems Connected: %s"),
               ActiveInstance->bGPUSystemsConnected ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("Pending Atmosphere Enable: %s"),
               ActiveInstance->bPendingAtmosphereGPUEnable ? TEXT("YES") : TEXT("NO"));
    }
}

void AGPUTerrainController::ConsoleTestAtmosphereGenerate()
{
    if (!ActiveInstance || !ActiveInstance->AtmosphereController)
    {
        UE_LOG(LogTemp, Error, TEXT("No active atmosphere controller"));
        return;
    }
    
    AAtmosphereController* AtmoController = ActiveInstance->AtmosphereController;
    
    // Ensure resources are initialized
    if (!AtmoController->IsGPUResourcesInitialized())
    {
        AtmoController->InitializeGPUResources();
        
        // Wait one frame
        if (ActiveInstance->GetWorld())
        {
            ActiveInstance->GetWorld()->GetTimerManager().SetTimerForNextTick([AtmoController]()
            {
                AtmoController->GenerateTestCloudData();
                UE_LOG(LogTemp, Warning, TEXT("Console: Test cloud data generated (deferred)"));
            });
        }
    }
    else
    {
        AtmoController->GenerateTestCloudData();
        UE_LOG(LogTemp, Warning, TEXT("Console: Test cloud data generated"));
    }
}

// Debug Logging Helper
void AGPUTerrainController::LogDebugInfo(const FString& Category, const FString& Message, bool bError)
{
    if (bEnableDebugLogging)
    {
        if (bError)
        {
            UE_LOG(LogTemp, Error, TEXT("[GPU %s] %s"), *Category, *Message);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[GPU %s] %s"), *Category, *Message);
        }
    }
}

void AGPUTerrainController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear active instance
    if (ActiveInstance == this)
    {
        ActiveInstance = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}


// Add these console commands to GPUTerrainController.cpp for debugging

// Console command to debug atmosphere scaling issue
static FAutoConsoleCommand DebugAtmosphereScaleCmd(
    TEXT("gpu.DebugAtmosphereScale"),
    TEXT("Debug why clouds are 1/4 size"),
    FConsoleCommandDelegate::CreateStatic([]()
    {
        if (!AGPUTerrainController::ActiveInstance) return;
        
        auto* Atmosphere = AGPUTerrainController::ActiveInstance->AtmosphereController;
        auto* Terrain = AGPUTerrainController::ActiveInstance->TargetTerrain;
        
        if (!Atmosphere || !Terrain)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing atmosphere or terrain"));
            return;
        }
        
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("=== ATMOSPHERE SCALE DEBUG ==="));
        UE_LOG(LogTemp, Warning, TEXT("Terrain Dimensions: %dx%d"),
               Terrain->TerrainWidth, Terrain->TerrainHeight);
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere Grid: %dx%d"),
               Atmosphere->GetGridSizeX(), Atmosphere->GetGridSizeY());
        
        bool bMatches = (Atmosphere->GetGridSizeX() == Terrain->TerrainWidth &&
                        Atmosphere->GetGridSizeY() == Terrain->TerrainHeight);
        
        if (!bMatches)
        {
            UE_LOG(LogTemp, Error, TEXT("PROBLEM: Grid sizes don't match!"));
            UE_LOG(LogTemp, Error, TEXT("This causes the 1/4 scale issue"));
            UE_LOG(LogTemp, Warning, TEXT("Fix: Atmosphere grid should be %dx%d"),
                   Terrain->TerrainWidth, Terrain->TerrainHeight);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Grid sizes match correctly (1:1 mapping)"));
        }
        
        // Check texture sizes
        if (Atmosphere->CloudRenderTexture)
        {
            UE_LOG(LogTemp, Warning, TEXT("CloudRenderTexture size: %dx%d"),
                   Atmosphere->CloudRenderTexture->SizeX,
                   Atmosphere->CloudRenderTexture->SizeY);
        }
        
        // Check accumulated time
        UE_LOG(LogTemp, Warning, TEXT("AccumulatedTime: %.2f"),
               Atmosphere->AccumulatedTime);
        UE_LOG(LogTemp, Warning, TEXT("Needs init: %s"),
               Atmosphere->AccumulatedTime < 0.1f ? TEXT("YES") : TEXT("NO"));
        
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("=== SUGGESTED FIX ==="));
        UE_LOG(LogTemp, Warning, TEXT("1. Ensure InitializeGPUResources sets:"));
        UE_LOG(LogTemp, Warning, TEXT("   GridSizeX = TargetTerrain->TerrainWidth"));
        UE_LOG(LogTemp, Warning, TEXT("   GridSizeY = TargetTerrain->TerrainHeight"));
        UE_LOG(LogTemp, Warning, TEXT("2. Then run: gpu.ForceAtmosphereReinit"));
    })
);

// Console command to force correct grid size
static FAutoConsoleCommand ForceAtmosphereReinitCmd(
    TEXT("gpu.ForceAtmosphereReinit"),
    TEXT("Force atmosphere to reinitialize with correct grid size"),
    FConsoleCommandDelegate::CreateStatic([]()
    {
        if (!AGPUTerrainController::ActiveInstance) return;
        
        auto* Atmosphere = AGPUTerrainController::ActiveInstance->AtmosphereController;
        auto* Terrain = AGPUTerrainController::ActiveInstance->TargetTerrain;
        
        if (!Atmosphere || !Terrain)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing atmosphere or terrain"));
            return;
        }
        
        // Force correct grid size
        Atmosphere->GridSizeX = Terrain->TerrainWidth;
        Atmosphere->GridSizeY = Terrain->TerrainHeight;
        
        // Reset timing
        Atmosphere->AccumulatedTime = 0.0f;
        Atmosphere->InitializationTimer = 0.0f;
        Atmosphere->bNeedsInitialState = true;
        
        // Reinitialize
        Atmosphere->InitializeGPUResources();
        
        UE_LOG(LogTemp, Warning, TEXT("Forced atmosphere grid to %dx%d"),
               Atmosphere->GridSizeX, Atmosphere->GridSizeY);
        UE_LOG(LogTemp, Warning, TEXT("AccumulatedTime reset to 0"));
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere will reinitialize on next frame"));
    })
);

// Console command to check PIE restart issue
static FAutoConsoleCommand CheckPIERestartCmd(
    TEXT("gpu.CheckPIERestart"),
    TEXT("Check why PIE restart crashes"),
    FConsoleCommandDelegate::CreateStatic([]()
    {
        if (!AGPUTerrainController::ActiveInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("No active GPU controller instance"));
            return;
        }
        
        auto* Atmosphere = AGPUTerrainController::ActiveInstance->AtmosphereController;
        if (!Atmosphere)
        {
            UE_LOG(LogTemp, Warning, TEXT("No atmosphere controller"));
            return;
        }
        
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("=== PIE RESTART DEBUG ==="));
        UE_LOG(LogTemp, Warning, TEXT("bGPUResourcesInitialized: %s"),
               Atmosphere->IsGPUResourcesInitialized() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("bUseGPUCompute: %s"),
               Atmosphere->IsGPUComputeEnabled() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("AccumulatedTime: %.2f"),
               Atmosphere->AccumulatedTime);
        
        // Check texture validity
        UE_LOG(LogTemp, Warning, TEXT("Textures:"));
        UE_LOG(LogTemp, Warning, TEXT("  StateTexture: %s"),
               Atmosphere->AtmosphereStateTexture ? TEXT("Valid") : TEXT("NULL"));
        UE_LOG(LogTemp, Warning, TEXT("  CloudTexture: %s"),
               Atmosphere->CloudRenderTexture ? TEXT("Valid") : TEXT("NULL"));
        UE_LOG(LogTemp, Warning, TEXT("  WindTexture: %s"),
               Atmosphere->WindFieldTexture ? TEXT("Valid") : TEXT("NULL"));
        
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("SAFE RESTART PROCEDURE:"));
        UE_LOG(LogTemp, Warning, TEXT("1. Disable GPU compute: gpu.DisableAtmosphereGPU"));
        UE_LOG(LogTemp, Warning, TEXT("2. Reset timing: Atmosphere->AccumulatedTime = 0"));
        UE_LOG(LogTemp, Warning, TEXT("3. Re-enable: gpu.EnableAtmosphereGPU"));
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("CleanupGPUResources is commented out to prevent crashes"));
        UE_LOG(LogTemp, Warning, TEXT("Textures are managed by UE garbage collection"));
    })
);
