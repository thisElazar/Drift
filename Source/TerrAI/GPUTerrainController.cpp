// GPUTerrainController.cpp - COMPREHENSIVE FIX
// Fixes the "disappearing controller" issue that stops atmosphere compute

#include "GPUTerrainController.h"
#include "DynamicTerrain.h"
#include "WaterController.h"
#include "AtmosphereController.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "RenderGraphBuilder.h"
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

static FAutoConsoleCommand PipelineStatusCmd(
    TEXT("gpu.PipelineStatus"),
    TEXT("Check full GPU pipeline status"),
    FConsoleCommandDelegate::CreateStatic(&AGPUTerrainController::ConsolePipelineStatus)
);

AGPUTerrainController::AGPUTerrainController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AGPUTerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    // CRITICAL FIX 1: Clear any stale static instance from previous sessions
    if (ActiveInstance && ActiveInstance != this)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: Clearing stale static instance"));
        ActiveInstance = nullptr;
    }
    
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
    
    // CRITICAL FIX 2: Validate all pointers with IsValid
    if (IsValid(TargetTerrain) && IsValid(WaterController) && IsValid(AtmosphereController))
    {
        UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: All systems found and valid"));
        
        // Schedule pipeline initialization for next frame
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            // Re-validate before initializing
            if (IsValid(TargetTerrain) && IsValid(WaterController) && IsValid(AtmosphereController))
            {
                InitializeGPUPipeline();
                
                // Schedule atmosphere GPU enable with proper delay
                if (bEnableAtmosphereOnStart)
                {
                    GetWorld()->GetTimerManager().SetTimer(
                        AtmosphereEnableTimer,
                        this,
                        &AGPUTerrainController::EnableAtmosphereGPUDeferred,
                        2.0f,
                        false
                    );
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("GPUTerrainController: Systems became invalid after BeginPlay"));
            }
        });
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: Not all systems found yet"));
    }
}

void AGPUTerrainController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // CRITICAL FIX 3: Validate all pointers every tick with IsValid()
    // This catches when actors are destroyed or become invalid
    if (!ValidateSystemReferences())
    {
        // Systems are invalid - clear connections
        if (bGPUSystemsConnected)
        {
            UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: System references became invalid, disconnecting"));
            bGPUSystemsConnected = false;
        }
        return;
    }
    
    // Handle pending atmosphere GPU enable
    if (bPendingAtmosphereGPUEnable)
    {
        // Validate before attempting enable
        if (IsValid(AtmosphereController) && AtmosphereController->IsReadyForGPU())
        {
            EnableAtmosphereGPU();
        }
        else
        {
            static int AttemptCount = 0;
            if (++AttemptCount % 60 == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Waiting for atmosphere to be ready... (attempt %d)"),
                       AttemptCount / 60);
            }
            
            if (AttemptCount > 300)
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

// CRITICAL FIX 4: New validation function that uses IsValid()
bool AGPUTerrainController::ValidateSystemReferences() const
{
    // Check if pointers are valid UE objects (not just non-null)
    bool bTerrainValid = IsValid(TargetTerrain);
    bool bWaterValid = IsValid(WaterController);
    bool bAtmosphereValid = IsValid(AtmosphereController);
    
    if (!bTerrainValid || !bWaterValid || !bAtmosphereValid)
    {
        if (!bTerrainValid)
            UE_LOG(LogTemp, VeryVerbose, TEXT("TargetTerrain is invalid"));
        if (!bWaterValid)
            UE_LOG(LogTemp, VeryVerbose, TEXT("WaterController is invalid"));
        if (!bAtmosphereValid)
            UE_LOG(LogTemp, VeryVerbose, TEXT("AtmosphereController is invalid"));
            
        return false;
    }
    
    return true;
}

void AGPUTerrainController::ExecuteGPUWatershedPipeline(float DeltaTime)
{
    double StartTime = FPlatformTime::Seconds();
    
    // CRITICAL FIX 5: Validate before every GPU operation
    if (!ValidateSystemReferences())
    {
        UE_LOG(LogTemp, Warning, TEXT("ExecuteGPUWatershedPipeline: System references invalid, skipping"));
        return;
    }
    
    // Execute terrain compute
    if (TargetTerrain->IsGPUTerrainEnabled())
    {
        // Only pass precipitation if atmosphere GPU is enabled and resources are ready
        if (IsValid(AtmosphereController) &&
            AtmosphereController->IsGPUComputeEnabled() &&
            AtmosphereController->IsGPUResourcesInitialized() &&
            IsValid(AtmosphereController->PrecipitationTexture))
        {
            TargetTerrain->SetPrecipitationTexture(AtmosphereController->PrecipitationTexture);
        }
        
        TargetTerrain->ExecuteTerrainComputeShader(DeltaTime);
    }
    
    // Execute atmosphere compute only if fully enabled
    if (IsValid(AtmosphereController) &&
        AtmosphereController->IsGPUComputeEnabled() &&
        AtmosphereController->IsGPUResourcesInitialized())
    {
        AtmosphereController->ExecuteAtmosphericCompute(DeltaTime);
    }
    
    // Execute water compute
    if (WaterController->bUseGPUVertexDisplacement &&
        WaterController->WaterSystem &&
        IsValid(WaterController->WaterSystem))
    {
        WaterController->WaterSystem->ExecuteWaveComputeShader();
    }
    
    LastGPUComputeTime = FPlatformTime::Seconds() - StartTime;
    GPUDispatchCount++;
}

void AGPUTerrainController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: EndPlay called (reason: %d)"), (int32)EndPlayReason);
    
    // CRITICAL FIX 6: Properly clean up on EndPlay
    
    // Clear timer handles
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AtmosphereEnableTimer);
    }
    
    // Disconnect from systems gracefully
    if (IsValid(AtmosphereController))
    {
        DisableAtmosphereGPU();
    }
    
    // Clear all references
    TargetTerrain = nullptr;
    WaterController = nullptr;
    AtmosphereController = nullptr;
    
    // Reset state flags
    bGPUSystemsConnected = false;
    bPendingAtmosphereGPUEnable = false;
    
    // Clear active instance ONLY if it's this instance
    if (ActiveInstance == this)
    {
        ActiveInstance = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("GPUTerrainController: Cleared static ActiveInstance"));
    }
    
    Super::EndPlay(EndPlayReason);
}

// CRITICAL FIX 7: Console commands must validate ActiveInstance with IsValid()
void AGPUTerrainController::ConsoleEnableAtmosphereGPU()
{
    if (!ActiveInstance || !IsValid(ActiveInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid GPUTerrainController instance"));
        return;
    }
    
    // Double-check the atmosphere controller is still valid
    if (!IsValid(ActiveInstance->AtmosphereController))
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphereController is invalid - system may have been destroyed"));
        return;
    }
    
    ActiveInstance->EnableAtmosphereGPU();
    UE_LOG(LogTemp, Warning, TEXT("Console: Atmosphere GPU enable requested"));
}

void AGPUTerrainController::ConsoleCheckAtmosphereStatus()
{
    if (!ActiveInstance || !IsValid(ActiveInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid GPUTerrainController instance"));
        return;
    }
    
    if (!IsValid(ActiveInstance->AtmosphereController))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid atmosphere controller"));
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
}

// Additional helper for debugging
void AGPUTerrainController::DisplayGPUStats()
{
    if (!GEngine || !ValidateSystemReferences())
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

// ===== ADDITIONAL CONSOLE COMMAND IMPLEMENTATIONS =====

void AGPUTerrainController::ConsoleDisableAtmosphereGPU()
{
    if (!ActiveInstance || !IsValid(ActiveInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid GPUTerrainController instance"));
        return;
    }
    
    if (!IsValid(ActiveInstance->AtmosphereController))
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphereController is invalid"));
        return;
    }
    
    ActiveInstance->DisableAtmosphereGPU();
    UE_LOG(LogTemp, Warning, TEXT("Console: Atmosphere GPU disabled"));
}

void AGPUTerrainController::ConsolePipelineStatus()
{
    if (!ActiveInstance || !IsValid(ActiveInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid GPUTerrainController instance"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU PIPELINE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Pipeline Enabled: %s"),
           ActiveInstance->bEnableGPUPipeline ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Systems Connected: %s"),
           ActiveInstance->bGPUSystemsConnected ? TEXT("YES") : TEXT("NO"));
    
    // Check validation status
    bool bValid = ActiveInstance->ValidateSystemReferences();
    UE_LOG(LogTemp, Warning, TEXT("System References Valid: %s"),
           bValid ? TEXT("YES") : TEXT("NO"));
    
    if (!bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("REFERENCE STATUS:"));
        UE_LOG(LogTemp, Warning, TEXT("  Terrain: %s"),
               IsValid(ActiveInstance->TargetTerrain) ? TEXT("Valid") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("  Water: %s"),
               IsValid(ActiveInstance->WaterController) ? TEXT("Valid") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("  Atmosphere: %s"),
               IsValid(ActiveInstance->AtmosphereController) ? TEXT("Valid") : TEXT("INVALID"));
    }
}

void AGPUTerrainController::EnableAtmosphereGPU()
{
    if (!bGPUSystemsConnected)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot enable atmosphere GPU - pipeline not initialized"));
        return;
    }
    
    if (!IsValid(AtmosphereController))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot enable atmosphere GPU - controller invalid"));
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

void AGPUTerrainController::DisableAtmosphereGPU()
{
    if (IsValid(AtmosphereController))
    {
        AtmosphereController->DisableGPUCompute();
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain Controller: Atmosphere GPU compute disabled"));
    }
}

void AGPUTerrainController::EnableAtmosphereGPUDeferred()
{
    UE_LOG(LogTemp, Warning, TEXT("Auto-enabling atmosphere GPU compute..."));
    EnableAtmosphereGPU();
    
    // Verify it worked
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (IsValid(AtmosphereController) && AtmosphereController->IsGPUComputeEnabled())
            {
                UE_LOG(LogTemp, Warning, TEXT("✓ Atmosphere GPU compute ACTIVE"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("✗ Atmosphere GPU failed to enable"));
            }
        });
    }
}

void AGPUTerrainController::InitializeGPUPipeline()
{
    if (!ValidateSystemReferences())
    {
        UE_LOG(LogTemp, Error, TEXT("GPUTerrainController: Cannot initialize - invalid system references"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== Initializing GPU Watershed Pipeline ==="));
    
    // Phase 1: Initialize terrain GPU
    TargetTerrain->bUseGPUTerrain = true;
    TargetTerrain->InitializeGPUTerrain();
    
    if (!TargetTerrain->IsGPUTerrainEnabled())
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain GPU not ready yet, scheduling retry"));
        
        // Retry initialization
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
            {
                if (ValidateSystemReferences())
                {
                    InitializeGPUPipeline();
                }
            });
        }
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✓ GPU Terrain initialized"));
    
    // Phase 2: Initialize water GPU
    WaterController->bUseGPUVertexDisplacement = true;
    if (WaterController->WaterSystem && IsValid(WaterController->WaterSystem))
    {
        TargetTerrain->ConnectToGPUWaterSystem(WaterController->WaterSystem);
        WaterController->WaterSystem->EnableGPUMode(true);
        UE_LOG(LogTemp, Warning, TEXT("✓ Water system connected"));
    }
    
    // Phase 3: Connect atmosphere (but don't enable GPU yet)
    if (AtmosphereController->AtmosphericSystem)
    {
        TargetTerrain->ConnectToGPUAtmosphere(AtmosphereController->AtmosphericSystem);
        UE_LOG(LogTemp, Warning, TEXT("✓ Atmosphere system connected (GPU off)"));
    }
    
    // Initialize atmosphere controller connection
    AtmosphereController->Initialize(TargetTerrain, WaterController->WaterSystem);
    
    bGPUSystemsConnected = true;
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU Pipeline Ready ==="));
}

FString AGPUTerrainController::GetValidationStatus() const
{
    FString Status = TEXT("GPU Terrain Controller Validation:\n");
    
    Status += FString::Printf(TEXT("  ActiveInstance: %s\n"),
        ActiveInstance ? TEXT("Set") : TEXT("NULL"));
    
    if (ActiveInstance)
    {
        Status += FString::Printf(TEXT("  ActiveInstance IsValid: %s\n"),
            IsValid(ActiveInstance) ? TEXT("YES") : TEXT("NO"));
    }
    
    Status += FString::Printf(TEXT("  TargetTerrain: %s\n"),
        IsValid(TargetTerrain) ? TEXT("Valid") : TEXT("INVALID"));
    
    Status += FString::Printf(TEXT("  WaterController: %s\n"),
        IsValid(WaterController) ? TEXT("Valid") : TEXT("INVALID"));
    
    Status += FString::Printf(TEXT("  AtmosphereController: %s\n"),
        IsValid(AtmosphereController) ? TEXT("Valid") : TEXT("INVALID"));
    
    Status += FString::Printf(TEXT("  Systems Connected: %s\n"),
        bGPUSystemsConnected ? TEXT("YES") : TEXT("NO"));
    
    return Status;
}

void AGPUTerrainController::SynchronizeGPUSystems()
{
    // Synchronize GPU data back to CPU for game logic
    if (IsValid(TargetTerrain) && TargetTerrain->IsGPUTerrainEnabled())
    {
        TargetTerrain->SyncGPUToCPU();
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("GPU Systems synchronized to CPU"));
}

void AGPUTerrainController::SynchronizeGridDimensions()
{
    if (!IsValid(TargetTerrain) || !IsValid(AtmosphereController))
    {
        UE_LOG(LogTemp, Warning, TEXT("SynchronizeGridDimensions: Missing valid components"));
        return;
    }
    
    // Get actual dimensions
    int32 TerrainWidth = TargetTerrain->TerrainWidth;
    int32 TerrainHeight = TargetTerrain->TerrainHeight;
    
    // Check if atmosphere grid needs to be fixed
    int32 CurrentAtmoWidth = AtmosphereController->GetGridSizeX();
    int32 CurrentAtmoHeight = AtmosphereController->GetGridSizeY();
    
    if (CurrentAtmoWidth != TerrainWidth || CurrentAtmoHeight != TerrainHeight)
    {
        UE_LOG(LogTemp, Warning, TEXT("FIXING ATMOSPHERE GRID SIZE:"));
        UE_LOG(LogTemp, Warning, TEXT("  Current: %dx%d"), CurrentAtmoWidth, CurrentAtmoHeight);
        UE_LOG(LogTemp, Warning, TEXT("  Fixing to: %dx%d"), TerrainWidth, TerrainHeight);
        
        // FIX THE GRID SIZE
        AtmosphereController->GridSizeX = TerrainWidth;
        AtmosphereController->GridSizeY = TerrainHeight;
        
        // Force reinitialization
        AtmosphereController->InitializeGPUResources();
        
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere grid synchronized"));
    }
}

void AGPUTerrainController::UpdateErosionParameters()
{
    if (!IsValid(TargetTerrain))
    {
        return;
    }
    
    TargetTerrain->GPUErosionRate = HydraulicErosionStrength;
    TargetTerrain->GPUDepositionRate = HydraulicErosionStrength * 0.5f;
}

void AGPUTerrainController::UpdateOrographicParameters()
{
    if (!IsValid(TargetTerrain))
    {
        return;
    }
    
    TargetTerrain->OrographicLiftStrength = OrographicLiftCoefficient;
    TargetTerrain->MoistureCondensationThreshold = 1.0f - AdiabaticCoolingRate;
    
    if (IsValid(AtmosphereController) && AtmosphereController->AtmosphericSystem)
    {
        AtmosphereController->AtmosphericSystem->SetRainShadowIntensity(RainShadowIntensity);
    }
}

void AGPUTerrainController::UpdateOrographicFeedback(float DeltaTime)
{
    if (!IsValid(AtmosphereController) || !IsValid(TargetTerrain))
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

void AGPUTerrainController::ConnectSystems(ADynamicTerrain* Terrain, AWaterController* Water, AAtmosphereController* Atmosphere)
{
    TargetTerrain = Terrain;
    WaterController = Water;
    AtmosphereController = Atmosphere;
    
    if (IsValid(TargetTerrain) && IsValid(WaterController) && IsValid(AtmosphereController))
    {
        InitializeGPUPipeline();
        UE_LOG(LogTemp, Warning, TEXT("Systems connected to GPU controller"));
    }
}

bool AGPUTerrainController::IsAtmosphereGPUEnabled() const
{
    return IsValid(AtmosphereController) && AtmosphereController->IsGPUComputeEnabled();
}
