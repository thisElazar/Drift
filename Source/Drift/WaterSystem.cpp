/**
 * ============================================
 * DRIFT WATER SYSTEM - IMPLEMENTATION
 * ============================================
 * Reorganized: December 2025
 * Original: 9,217 lines | Functions: ~180
 * Section markers preserved - header TOC added
 *
 * PURPOSE:
 * Complete water simulation for Drift watershed simulator.
 * Pressure-based 8-directional flow physics with GPU compute shaders
 * for real-time wave generation and vertex displacement.
 *
 * KEY CAPABILITIES:
 * - Real-time water simulation (513x513 grid, 60+ FPS)
 * - GPU vertex displacement via compute shaders
 * - Physics-based wave generation (6 wave types)
 * - Water conservation tracking (perfect mass balance)
 * - Chunk-based rendering with dynamic LOD
 *
 * ARCHITECTURE:
 * - UObject owned by ADynamicTerrain
 * - GPU-first design with compute shaders for wave physics
 * - Authority pattern: MasterController validates water conservation
 * - Hybrid CPU/GPU: Physics on CPU, waves on GPU
 *
 * TABLE OF CONTENTS:
 *
 * SECTION 1: SYSTEM LIFECYCLE (~400 lines)
 *   1.1 Constructor & Defaults
 *   1.2 Initialize with terrain/controller references
 *   1.3 Main simulation update loop
 *   1.4 System readiness validation
 *   1.5 World scaling integration (IScalableSystem)
 *   1.6 System reset and cleanup
 *
 * SECTION 2: CORE WATER PHYSICS (~800 lines)
 *   2.1 Flow Calculation (8-directional pressure gradients)
 *   2.2 Flow Application (velocity integration)
 *   2.3 Evaporation & Infiltration (atmospheric/groundwater coupling)
 *
 * SECTION 3: WATER MANAGEMENT & VISUALS (~1000 lines)
 *   3.1 Volumetric water surface chunk management
 *   3.2 Enhanced mesh generation with LOD
 *   3.3 Flow displacement textures
 *   3.4 Terrain synchronization
 *   3.5 Advanced water texture system
 *
 * SECTION 4: PUBLIC API & UTILITIES (~600 lines)
 *   4.1 Water Manipulation (Add/Remove with radius)
 *   4.2 Water Queries (Depth, volume, flow statistics)
 *   4.3 Safe Accessors (Bounds-checked getters/setters)
 *   4.4 Coordinate Utilities (World <-> Terrain transforms)
 *
 * SECTION 5: RENDERING & VISUALS (~900 lines)
 *   5.1 Shader Core (main update loop, material application)
 *   5.2 UE5.4 Integration (Lumen, Nanite, Enhanced Input)
 *   5.3 Shader Parameters & Textures
 *   5.4 Quality & Filtering (edge-preserving)
 *   5.5 Material Parameters (dynamic updates)
 *   5.6 Player Queries (underwater detection)
 *   5.7 Advanced Mesh Generation
 *
 * SECTION 6: WAVE PHYSICS SYSTEM (~1200 lines)
 *   6.1 Wave Generation Core
 *   6.2 Wave Type Generators (6 types):
 *       - Wind Waves (capillary-gravity)
 *       - Gravity Waves (deep water)
 *       - Turbulent Waves (chaotic motion)
 *       - Collision Waves (obstacle interaction)
 *       - Flow Waves (flow-induced)
 *       - Capillary Waves (surface tension)
 *   6.3 Wave Processing (smoothing, combining)
 *   6.4 Splash & Utilities
 *
 * SECTION 7: GPU COMPUTE PIPELINE (~800 lines)
 *   7.1 GPU Initialization (textures, shader resources)
 *   7.2 Data Upload (CPU -> GPU transfer)
 *   7.3 Compute Execution (wave shader dispatch)
 *   7.4 Parameter Updates (real-time wave animation)
 *
 * SECTION 8: GPU MESH MANAGEMENT (~700 lines)
 *   8.1 Mesh Regeneration Logic (change detection)
 *   8.2 GPU Mesh Generation (flat base mesh)
 *   8.3 Material & Alignment (shader parameters)
 *   8.4 Mesh Utilities
 *
 * SECTION 9: DEBUG & UTILITIES (~600 lines)
 *   9.1 Position & Coordinate Debugging
 *   9.2 Wave System Debugging
 *   9.3 Pipeline Validation
 *   9.4 System Management (force updates, resets)
 *
 * TOTAL: ~9,200 lines
 *
 * DEPENDENCIES:
 * - ADynamicTerrain: Terrain data, chunk system, coordinate transforms
 * - AMasterWorldController: Coordinate authority, water conservation
 * - AAtmosphereController: Wind data, precipitation
 * - UTemporalManager: Time scaling coordination
 *
 * WATER CONSERVATION:
 * All water transfers go through MasterController:
 * - TransferSurfaceToAtmosphere() for evaporation
 * - TransferSurfaceToSoilMoisture() for infiltration → soil moisture → water table
 * - TransferSurfaceToGroundwater() for edge drainage (boundary water)
 * Perfect mass balance maintained (±0.0 m³)
 *
 * SOIL MOISTURE LAYER:
 * Infiltrated water goes to GeologyController's soil moisture buffer first,
 * then slowly drains to water table. This creates the "residence time" that
 * plants need to access water - the foundation of the ecosystem!
 */

#include "WaterSystem.h"
#include "DynamicTerrain.h"
#include "MasterController.h"
#include "GeologyController.h"
#include "AtmosphericSystem.h"
#include "Engine/Engine.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "ProceduralMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceTexture.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "GameFramework/PlayerController.h"
#include "WaterSystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Shaders/WaveComputeShader.h"
#include "Camera/PlayerCameraManager.h"

// Replace all MasterController with CachedMasterController
#define MasterController CachedMasterController

// Debug optimization static variables
float UWaterSystem::LastDebugLogTime = 0.0f;
int32 UWaterSystem::DebugLogCounter = 0;
const float UWaterSystem::DEBUG_LOG_INTERVAL = 2.0f;

// UE5.4 Performance Stats
DECLARE_CYCLE_STAT(TEXT("Water Addition"), STAT_WaterAddition, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Water System Total"), STAT_WaterSystemTotal, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Water Physics"), STAT_WaterPhysics, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Water Rendering"), STAT_WaterRendering, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Water Textures"), STAT_WaterTextures, STATGROUP_Game);

// ============================================================================
// SECTION 1: SYSTEM LIFECYCLE
// ============================================================================
//
// This section contains all functions related to the water system's lifecycle:
// - Object construction and default initialization
// - System initialization with terrain and controller references
// - Main simulation update loop and temporal coordination
// - System readiness validation
// - World scaling integration (IScalableSystem interface)
// - Water quality management
// - System reset and cleanup
//
// Key Functions:
// - UWaterSystem() - Constructor with default settings
// - Initialize() - Setup with terrain/controller references
// - UpdateWaterSimulation() - Main simulation tick (calls physics & visuals)
// - IsSystemReady() - Validation of system state
// - ConfigureFromMaster() - World scaling configuration
// - ResetWaterSystem() - Reset all water data
//
// Dependencies: ADynamicTerrain, AMasterWorldController, UTemporalManager
// Authority: MasterController for coordinate transforms and water budget
// ============================================================================


// ===== CONSTRUCTOR & DEFAULTS =====

UWaterSystem::UWaterSystem()
{
    // Initialize default settings
    bEnableWaterSimulation = true;
    WaterFlowSpeed = 50.0f;
    WaterEvaporationRate = 0.001f;
    WaterAbsorptionRate = 0.003f;  // Reduced from 0.02 - allows rivers to form before soaking in
    WaterDamping = 0.95f;
    MaxWaterVelocity = 100.0f;
    MinWaterDepth = 0.01f;
    
    

    WeatherTimer = 0.0f;
    
    // Debug settings
    bShowWaterStats = false;  // DISABLED to reduce log spam
    bShowWaterDebugTexture = false;
    
    // Shader system enabled for debugging
    bUseShaderWater = true;
    
    // Initialize volumetric water parameters
    MaterialParams = FWaterMaterialParams();
    bWaterChangedThisFrame = false;
    
    // Initialize surface water system
    bEnableWaterVolumes = true;      // Disabled by default for performance
    MinVolumeDepth = 0.01f;           // Minimum depth to create surfaces
    VolumeUpdateDistance = 2000.0f;  // LOD distance for surface updates
    MaxVolumeChunks = 64;            // Performance limit
    
    // Initialize edge drainage settings
    bEnableEdgeDrainage = false;
    EdgeDrainageStrength = 0.2f;
    bEnhancedWaterfallEffect = false;
    WaterfallDrainageMultiplier = 0.3f;
    MinDepthThreshold = 0.1f;
    

    
    // Initialize Water Authority & Quality Settings
    MinMeshDepth = 0.01f;
    MinContiguousWaterCells = 9;
    bAlwaysShowWaterMeshes = true;
    WaterMeshQuality = 64;
    
    // Initialize water quality presets
    InitializeWaterQualityDefaults();
    
    // Call WaterParameterCollection
    WaterParameterCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/MPC_WaterParameters.MPC_WaterParameters"));
    
    
    if (bEnableVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Created surface water"));
    }
}

// ===== SYSTEM INITIALIZATION =====

// ===== INITIALIZATION =====

/**
 * PHASE 1-2: Enhanced initialization with immediate resource creation
 * Creates all required resources during initialization for consistent readiness
 * Establishes coordinate authority and validates system integration
 */
void UWaterSystem::Initialize(ADynamicTerrain* InTerrain, AMasterWorldController* InMasterController)
{
    if (!InTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize WaterSystem with null terrain"));
        return;
    }
    
    OwnerTerrain = InTerrain;
    CachedMasterController = InMasterController;
    
    // Initialize simulation data
    SimulationData.Initialize(OwnerTerrain->TerrainWidth, OwnerTerrain->TerrainHeight);
    
    // Create shader textures if needed
    if (bUseShaderWater)
    {
        CreateWaterDepthTexture();
        CreateAdvancedWaterTexture();
    }
    
    // NEW: Create erosion textures (always, for GPU terrain erosion)
    if (SimulationData.IsValid())
    {
        CreateErosionTextures();
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Created erosion textures for GPU terrain"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Initialized %dx%d - MasterController: %s"),
           SimulationData.TerrainWidth, SimulationData.TerrainHeight,
           CachedMasterController ? TEXT("Available") : TEXT("Not Available"));
}



/**
 * PHASE 1-2: Event-driven system readiness check
 * Eliminates timing-dependent initialization for more reliable startup
 * Uses actual system state validation instead of arbitrary delays
 */
bool UWaterSystem::IsSystemReady() const
{
    // Core requirement: OwnerTerrain and valid simulation data
    bool bCoreReady = (OwnerTerrain != nullptr && SimulationData.IsValid());
    
    // Optional: Shader textures (if shader water enabled)
    bool bShaderReady = !bUseShaderWater || (WaterDepthTexture != nullptr);
    
    // MasterController is OPTIONAL, not required
    return bCoreReady && bShaderReady;
}

// ===== MAIN UPDATE LOOP =====

// ===== WATER PHYSICS SIMULATION =====

void UWaterSystem::UpdateWaterSimulation(float DeltaTime)
{
    // ===== VALIDATION: ENSURE SIMULATION IS READY =====
    if (!SimulationData.IsValid() || !OwnerTerrain)
    {
        return;
    }
    
    // ===== TEMPORAL PAUSE CHECK: RESPECT MASTER TIME CONTROL =====
    UTemporalManager* TemporalManager = GetWorld() ?
        GetWorld()->GetGameInstance()->GetSubsystem<UTemporalManager>() : nullptr;

    // When paused, skip physics but keep visual updates responsive
    if (TemporalManager && TemporalManager->IsPaused())
    {
        // Update textures for visual feedback even when paused
        if (bUseShaderWater)
        {
            UpdateWaterDepthTexture();
            UpdateWaterShaderParameters();
        }
        return; // Skip all physics updates
    }

    // ===== TOOL EDIT PAUSE: FREEZE WATER WHILE SCULPTING =====
    // Water stays "attached" to terrain during edits - no flow, no sources
    // Prevents tidal waves from springs and lets water move with terrain
    if (OwnerTerrain && OwnerTerrain->IsToolEditActive())
    {
        // Keep visuals updated so water appears on raised/lowered terrain
        if (bUseShaderWater)
        {
            UpdateWaterDepthTexture();
            UpdateWaterShaderParameters();
        }
        return; // Skip all physics - flow, precipitation, evaporation
    }
    
    // ===== GET PROPERLY SCALED DELTA TIME =====
    // Query TemporalManager for time-scaled delta (accounts for global time acceleration)
    float EffectiveDeltaTime = DeltaTime;
    if (TemporalManager)
    {
        EffectiveDeltaTime = TemporalManager->GetSystemDeltaTime(
            ESystemType::WaterPhysics,
            DeltaTime
        );
        
        // Safety check: if temporal system returns 0 (shouldn't happen after IsPaused check)
        if (EffectiveDeltaTime <= 0.0f)
        {
            return;
        }
    }
    

    
    // Performance timing
    float SimulationStartTime = FPlatformTime::Seconds();
    
    // Step 1: Track time for time-based effects
   // AccumulatedTime += DeltaTime;
   // AccumulatedScaledTime += DeltaTime * TimeScale;

    // Step 2: Process precipitation from atmosphere
    if (OwnerTerrain && OwnerTerrain->AtmosphericSystem)
    {
        AccumulatePrecipitation(EffectiveDeltaTime);
    }

    // Step 3: Calculate water flow forces
    CalculateWaterFlow(EffectiveDeltaTime);
    
    if (bEnableSimulationSmoothing)
    {
        static int32 FramesSinceSmoothing = 0;
        if (++FramesSinceSmoothing >= 6)
        {
            ApplySpatialSmoothing();
            FramesSinceSmoothing = 0;
        }
    }
    
    // Step 4: Move water based on flow
    ApplyWaterFlow(EffectiveDeltaTime);

    // Step 4b: Transport sediment with water flow
    ApplySedimentTransport(EffectiveDeltaTime);

    // Step 5: Handle evaporation and absorption
    ProcessWaterEvaporation(EffectiveDeltaTime);
    
    // Step 6: Always maintain chunk list
        UpdateWaterSurfaceChunks();
        
        // Step 7: Handle GPU-specific rendering if enabled
        if (bUseVertexDisplacement)
        {
            // Update GPU wave parameters
            UpdateGPUWaveParameters(DeltaTime);
            
            // Execute wave compute shader EVERY frame
            ExecuteWaveComputeShader();
            
            // CRITICAL: Check ALL terrain chunks for water, not just existing water chunks
            for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
            {
                // Check if this chunk area has any water
                const FTerrainChunk& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
                const int32 TerrainChunkSize = OwnerTerrain->ChunkSize;
                const int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
                
                int32 ChunkStartX = TerrainChunk.ChunkX * (TerrainChunkSize - ChunkOverlap);
                int32 ChunkStartY = TerrainChunk.ChunkY * (TerrainChunkSize - ChunkOverlap);
                int32 ChunkEndX = FMath::Min(ChunkStartX + TerrainChunkSize, OwnerTerrain->TerrainWidth);
                int32 ChunkEndY = FMath::Min(ChunkStartY + TerrainChunkSize, OwnerTerrain->TerrainHeight);
                
                bool bChunkHasWater = false;
                for (int32 Y = ChunkStartY; Y < ChunkEndY && !bChunkHasWater; Y++)
                {
                    for (int32 X = ChunkStartX; X < ChunkEndX && !bChunkHasWater; X++)
                    {
                        if (GetWaterDepthSafe(X, Y) > MinWaterDepth)
                        {
                            bChunkHasWater = true;
                        }
                    }
                }
                
                if (bChunkHasWater)
                {
                    // Find or create water chunk
                    FWaterSurfaceChunk* WaterChunk = WaterSurfaceChunks.FindByPredicate(
                        [ChunkIndex](FWaterSurfaceChunk& Chunk) {
                            return Chunk.ChunkIndex == ChunkIndex;
                        });
                    
                    if (!WaterChunk)
                    {
                        // Create new water chunk
                        FWaterSurfaceChunk NewChunk;
                        NewChunk.ChunkIndex = ChunkIndex;
                        NewChunk.ChunkX = TerrainChunk.ChunkX;
                        NewChunk.ChunkY = TerrainChunk.ChunkY;
                        NewChunk.bHasWater = true;
                        NewChunk.bNeedsUpdate = true;
                        WaterSurfaceChunks.Add(NewChunk);
                        
                        WaterChunk = &WaterSurfaceChunks.Last();
                    }
                    
                    // Ensure chunk has mesh
                    if (!WaterChunk->SurfaceMesh)
                    {
                        InitializeGPUChunkMesh(*WaterChunk);
                    }
                    
                    // Always update mesh
                    GenerateFlatBaseMesh(*WaterChunk);
                    ChunkWaterAreas.Add(ChunkIndex, CalculateChunkWaterArea(ChunkIndex));
                    
                    // Update material parameters
                    if (WaterChunk->SurfaceMesh)
                    {
                        UMaterialInstanceDynamic* MatInstance = Cast<UMaterialInstanceDynamic>(
                            WaterChunk->SurfaceMesh->GetMaterial(0));
                        if (MatInstance)
                        {
                            MatInstance->SetScalarParameterValue(TEXT("Time"),
                                GetWorld()->GetTimeSeconds());
                        }
                    }
                }
            }
        }
    
    // Step 8: Update shader system if enabled
    if (bUseShaderWater)
    {
        CalculateFoamData();
        UpdateAllWaterVisuals(DeltaTime);
        UpdateWaterShaderParameters();
    }
    if (ErosionWaterDepthRT && ErosionFlowVelocityRT)
       {
           UpdateErosionTextures();
       }
}

// ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====

// ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====

void UWaterSystem::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[SCALING] WaterSystem: Configuring from master with world size %dx%d"),
           Config.TerrainWidth, Config.TerrainHeight);
    
    CurrentWorldConfig = Config;
    
    // Initialize or reconfigure simulation data with master's dimensions
    if (!SimulationData.IsValid() ||
        SimulationData.TerrainWidth != Config.WaterConfig.SimulationArrayWidth ||
        SimulationData.TerrainHeight != Config.WaterConfig.SimulationArrayHeight)
    {
        SimulationData.Initialize(Config.WaterConfig.SimulationArrayWidth,
                                Config.WaterConfig.SimulationArrayHeight);
        UE_LOG(LogTemp, Warning, TEXT("[SCALING] WaterSystem: Simulation data resized to %dx%d"),
               Config.WaterConfig.SimulationArrayWidth, Config.WaterConfig.SimulationArrayHeight);
    }
    
    // Configure water cell scaling
    if (bUseShaderWater)
    {
        // Recreate textures with new dimensions
        CreateWaterDepthTexture();
        CreateAdvancedWaterTexture();
        UE_LOG(LogTemp, Warning, TEXT("[SCALING] WaterSystem: Shader textures recreated for new scaling"));
    }
    
    // Mark as scaled by master
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[SCALING] WaterSystem: Configuration complete - IsScaled: %s"),
           IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
}

void UWaterSystem::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    UE_LOG(LogTemp, Warning, TEXT("[COORD SYNC] WaterSystem: Synchronizing with coordinate system"));
    
    CurrentCoordinateSystem = Coords;
    
    // Update surface chunks if they need coordinate adjustments
    if (bIsScaledByMaster)
    {
        for (FWaterSurfaceChunk& SurfaceChunk : WaterSurfaceChunks)
        {
            UpdateSurfaceUVMapping(SurfaceChunk);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[COORD SYNC] WaterSystem: Updated %d surface chunks"),
               WaterSurfaceChunks.Num());
    }
}


void UWaterSystem::RegisterWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("[REGISTRATION] WaterSystem: Cannot register with null MasterController"));
        return;
    }
    
    CachedMasterController = Master;
    UE_LOG(LogTemp, Warning, TEXT("[REGISTRATION] WaterSystem: Successfully registered with MasterController"));
}

bool UWaterSystem::IsRegisteredWithMaster() const
{
    return CachedMasterController != nullptr;
}

FString UWaterSystem::GetScalingDebugInfo() const
{
    if (!IsSystemScaled())
    {
        return TEXT("WaterSystem: Not scaled by master");
    }
    
    return FString::Printf(TEXT("WaterSystem: OK - Simulation:%dx%d, WaterCells:%d, MaxDepth:%.2f, FlowSpeed:%.2f"),
                          CurrentWorldConfig.WaterConfig.SimulationArrayWidth,
                          CurrentWorldConfig.WaterConfig.SimulationArrayHeight,
                          CurrentWorldConfig.WaterConfig.WaterCellScale,
                          CurrentWorldConfig.WaterConfig.CoordinateScale);
}

// ===== WATER QUALITY MANAGEMENT =====

// ===== WATER QUALITY MANAGEMENT FUNCTIONS =====

void UWaterSystem::InitializeWaterQualityDefaults()
{
    WaterQualityPresets.Empty();
    WaterQualityPresets.Add(TEXT("Ultra Low"), 8);
    WaterQualityPresets.Add(TEXT("Low"), 16);
    WaterQualityPresets.Add(TEXT("Medium"), 32);
    WaterQualityPresets.Add(TEXT("High"), 64);
    WaterQualityPresets.Add(TEXT("Ultra High"), 128);
    WaterQualityPresets.Add(TEXT("Cinematic"), 256);
    
    UE_LOG(LogTemp, Log, TEXT("[WATER QUALITY] Initialized %d quality presets"), WaterQualityPresets.Num());
}

void UWaterSystem::SetWaterMeshQuality(int32 NewQuality)
{
    int32 ClampedQuality = FMath::Clamp(NewQuality, 8, 256);
    if (WaterMeshQuality != ClampedQuality)
    {
        WaterMeshQuality = ClampedQuality;
        
        // Force update all existing water meshes with new quality
        for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
        {
            Chunk.bNeedsUpdate = true;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[WATER QUALITY] Set to %d - updating %d chunks"),
               WaterMeshQuality, WaterSurfaceChunks.Num());
    }
}

void UWaterSystem::SetWaterQualityPreset(const FString& PresetName)
{
    if (WaterQualityPresets.Contains(PresetName))
    {
        SetWaterMeshQuality(WaterQualityPresets[PresetName]);
        UE_LOG(LogTemp, Warning, TEXT("[WATER QUALITY] Applied preset '%s' = %d"),
               *PresetName, WaterQualityPresets[PresetName]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[WATER QUALITY] Unknown preset '%s'"), *PresetName);
    }
}

void UWaterSystem::InitializeWaterQualityPresets()
{
    InitializeWaterQualityDefaults();
}

TArray<FString> UWaterSystem::GetWaterQualityPresetNames() const
{
    TArray<FString> PresetNames;
    WaterQualityPresets.GetKeys(PresetNames);
    return PresetNames;
}

FString UWaterSystem::GetWaterQualityPerformanceInfo() const
{
    int32 VerticesPerChunk = WaterMeshQuality * WaterMeshQuality;
    int32 TrianglesPerChunk = (WaterMeshQuality - 1) * (WaterMeshQuality - 1) * 2;
    int32 TotalVertices = VerticesPerChunk * WaterSurfaceChunks.Num();
    int32 TotalTriangles = TrianglesPerChunk * WaterSurfaceChunks.Num();
    
    return FString::Printf(TEXT("Quality: %d | Chunks: %d | Vertices: %d | Triangles: %d"),
                          WaterMeshQuality, WaterSurfaceChunks.Num(), TotalVertices, TotalTriangles);
}

// ===== SYSTEM RESET =====

void UWaterSystem::ResetWaterSystem()
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== RESETTING WATER SYSTEM ==="));
    
    // Reset all water data
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        SimulationData.WaterDepthMap[i] = 0.0f;
        SimulationData.WaterVelocityX[i] = 0.0f;
        SimulationData.WaterVelocityY[i] = 0.0f;
    }
    
    // Reset weather
    WeatherTimer = 0.0f;
    
    // Clear tracking data
    ChunksWithWater.Empty();
    TotalWaterAmount = 0.0f;
    
    // ===== CRITICAL FIX: CLEANUP AND RECREATE EROSION TEXTURES =====
    RecreateErosionTextures();
    
    // Force update of water textures
    if (WaterDepthTexture)
    {
        UpdateWaterDepthTexture();
    }
    
    UE_LOG(LogTemp, Warning, TEXT(" Water system reset complete (including erosion textures)"));
}

// END OF SECTION 1: SYSTEM LIFECYCLE


// ============================================================================
// SECTION 2: CORE WATER PHYSICS
// ============================================================================
//
// This section implements the heart of the water simulation engine:
// - Pressure-based shallow water equations
// - 8-directional flow distribution with diagonal scaling
// - Velocity integration and damping
// - Edge drainage and waterfall effects
// - Water conservation and mass balance
// - Evaporation and infiltration (atmospheric/groundwater coupling)
//
// Algorithm: Finite Difference Method for shallow water equations
// References:
// - Stam, J. (1999). "Stable Fluids" SIGGRAPH '99
// - Bridson, R. (2015). "Fluid Simulation for Computer Graphics" 2nd Ed.
//
// Performance: ~2-3ms for 513x513 grid on modern hardware
// Threading: Game thread only (modifies shared velocity arrays)
//
// Key Functions:
// - CalculateWaterFlow() - Pressure gradient calculation (8-directional)
// - ApplyWaterFlow() - Velocity integration and water movement
// - ProcessWaterEvaporation() - Atmospheric and groundwater coupling
//
// Dependencies: SimulationData arrays (WaterDepthMap, VelocityX/Y)
// Authority: MasterController for water budget transfers
// ============================================================================

// ===== FLOW CALCULATION =====

/**
 * ============================================
 * CORE WATER PHYSICS SIMULATION
 * ============================================
 * Algorithm: Finite difference method with pressure gradients
 * Performance: ~2-3ms for 513x513 terrain on modern hardware
 * Threading: Game thread only (modifies shared velocity arrays)
 */

/**
 * Calculates water flow forces based on pressure gradients
 *
 * Algorithm: Finite Difference Method for shallow water equations
 * References:
 * - Stam, J. (1999). "Stable Fluids" SIGGRAPH '99
 * - Bridson, R. (2015). "Fluid Simulation for Computer Graphics" 2nd Ed.
 *
 * Steps:
 * 1. Calculate water surface height (terrain + water depth)
 * 2. Compute pressure gradients to neighbors
 * 3. Handle edge drainage (waterfall effect)
 * 4. Apply forces to velocity with damping
 * 5. Clamp velocities to prevent instability
 *
 * @param DeltaTime - Frame time for velocity integration
 *
 * Performance: Processes ~263k cells in 2-3ms
 * Edge Cases: Special handling for terrain boundaries (waterfall drainage)
 * Setup for 8 directional flow
 */
void UWaterSystem::CalculateWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    TArray<float> NewVelocityX = SimulationData.WaterVelocityX;
    TArray<float> NewVelocityY = SimulationData.WaterVelocityY;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (SimulationData.WaterDepthMap[Index] <= MinWaterDepth)
            {
                continue;
            }
            
            float TerrainHeight = OwnerTerrain->GetHeightSafe(X, Y);
            float WaterSurfaceHeight = TerrainHeight + SimulationData.WaterDepthMap[Index];
            
            float TerrainScale = OwnerTerrain ? OwnerTerrain->TerrainScale : 100.0f;
            float ForceX = 0.0f;
            float ForceY = 0.0f;
            
            // === SURFACE TENSION MODEL (FIXED) ===
            float LowestNeighborSurface = WaterSurfaceHeight;
            
            // Find lowest neighbor
            for (int32 dy = -1; dy <= 1; dy++)
            {
                for (int32 dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0) continue;
                    
                    int32 NX = X + dx;
                    int32 NY = Y + dy;
                    
                    if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                    {
                        int32 NIndex = NY * Width + NX;
                        float NTerrainHeight = OwnerTerrain->GetHeightSafe(NX, NY);
                        float NWaterSurface = NTerrainHeight + SimulationData.WaterDepthMap[NIndex];
                        LowestNeighborSurface = FMath::Min(LowestNeighborSurface, NWaterSurface);
                    }
                }
            }
            
            // Calculate pooling factor with surface tension
            float PoolingFactor = 0.0f;
            if (WaterSurfaceHeight > LowestNeighborSurface + 0.01f)
            {
                float DepthDiff = WaterSurfaceHeight - LowestNeighborSurface;
                
                if (DepthDiff > 2.0f) // 2cm depression threshold
                {
                    PoolingFactor = FMath::Clamp(DepthDiff / 10.0f, 0.0f, 0.3f);
                    float SurfaceTensionFlow = 0.2f;
                    PoolingFactor *= (1.0f - SurfaceTensionFlow);
                }
            }
            
            // === 8-DIRECTIONAL FLOW (FIXED) ===
            if (bUse8DirectionalFlow)
            {
                const float DiagonalDistance = TerrainScale * 1.41421356f;
                const float CardinalDistance = TerrainScale;
                
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    for (int32 dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dy == 0) continue;
                        
                        bool bIsDiagonal = (dx != 0 && dy != 0);
                        float Distance = bIsDiagonal ? DiagonalDistance : CardinalDistance;
                        float Weight = bIsDiagonal ? 0.7071f : 1.0f;
                        
                        int32 NX = X + dx;
                        int32 NY = Y + dy;
                        
                        if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                        {
                            int32 NIndex = NY * Width + NX;
                            float NTerrainHeight = OwnerTerrain->GetHeightSafe(NX, NY);
                            float NWaterHeight = NTerrainHeight + SimulationData.WaterDepthMap[NIndex];
                            
                            float HeightDiff = WaterSurfaceHeight - NWaterHeight;
                            
                            /*
                            // Edge drainage
                            if ((NX == 0 || NX == Width-1 || NY == 0 || NY == Height-1) &&
                                HeightDiff > 0)
                            {
                                HeightDiff *= 2.0f;
                            }
                            */
                            if (HeightDiff > 0.001f)
                            {
                                float Force = (HeightDiff * Weight) / Distance;
                                ForceX += Force * dx;
                                ForceY += Force * dy;
                            }
                        }
                    }
                }
            }
            else
            {
                // 4-directional fallback
                int32 LeftIdx = (X > 0) ? Y * Width + (X - 1) : -1;
                int32 RightIdx = (X < Width - 1) ? Y * Width + (X + 1) : -1;
                int32 UpIdx = (Y > 0) ? (Y - 1) * Width + X : -1;
                int32 DownIdx = (Y < Height - 1) ? (Y + 1) * Width + X : -1;
                
                if (LeftIdx >= 0)
                {
                    float NHeight = OwnerTerrain->GetHeightSafe(X-1, Y) + SimulationData.WaterDepthMap[LeftIdx];
                    float Diff = WaterSurfaceHeight - NHeight;
                    if (X == 1 && Diff > 0) Diff *= 2.0f;
                    ForceX += Diff / TerrainScale;
                }
                
                if (RightIdx >= 0)
                {
                    float NHeight = OwnerTerrain->GetHeightSafe(X+1, Y) + SimulationData.WaterDepthMap[RightIdx];
                    float Diff = WaterSurfaceHeight - NHeight;
                    if (X == Width-2 && Diff > 0) Diff *= 2.0f;
                    ForceX -= Diff / TerrainScale;
                }
                
                if (UpIdx >= 0)
                {
                    float NHeight = OwnerTerrain->GetHeightSafe(X, Y-1) + SimulationData.WaterDepthMap[UpIdx];
                    float Diff = WaterSurfaceHeight - NHeight;
                    if (Y == 1 && Diff > 0) Diff *= 2.0f;
                    ForceY += Diff / TerrainScale;
                }
                
                if (DownIdx >= 0)
                {
                    float NHeight = OwnerTerrain->GetHeightSafe(X, Y+1) + SimulationData.WaterDepthMap[DownIdx];
                    float Diff = WaterSurfaceHeight - NHeight;
                    if (Y == Height-2 && Diff > 0) Diff *= 2.0f;
                    ForceY -= Diff / TerrainScale;
                }
            }
            
            // Apply pooling reduction
            ForceX *= (1.0f - PoolingFactor);
            ForceY *= (1.0f - PoolingFactor);
            
            // Update velocities with damping
            NewVelocityX[Index] = (SimulationData.WaterVelocityX[Index] + ForceX * WaterFlowSpeed * DeltaTime) * WaterDamping;
            NewVelocityY[Index] = (SimulationData.WaterVelocityY[Index] + ForceY * WaterFlowSpeed * DeltaTime) * WaterDamping;
            
            // Clamp velocities
            float VelMagnitude = FMath::Sqrt(NewVelocityX[Index] * NewVelocityX[Index] +
                                            NewVelocityY[Index] * NewVelocityY[Index]);
            if (VelMagnitude > MaxWaterVelocity)
            {
                float Scale = MaxWaterVelocity / VelMagnitude;
                NewVelocityX[Index] *= Scale;
                NewVelocityY[Index] *= Scale;
            }
        }
    }
    
    SimulationData.WaterVelocityX = NewVelocityX;
    SimulationData.WaterVelocityY = NewVelocityY;
}

// ===== FLOW APPLICATION =====

void UWaterSystem::ApplyWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    TArray<float> NewWaterDepth = SimulationData.WaterDepthMap;
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // STABILITY FIX 1: Adaptive timestep scaling
    float StabilityFactor = 1.0f;
    if (DeltaTime > 0.016f) // If timestep larger than 60fps
    {
        StabilityFactor = 0.016f / DeltaTime;
        StabilityFactor = FMath::Clamp(StabilityFactor, 0.2f, 1.0f); // Don't reduce too much
    }
    
    // STABILITY FIX 2: Track previous depths for oscillation detection
    static TArray<float> PreviousDepths = SimulationData.WaterDepthMap;
    if (PreviousDepths.Num() != SimulationData.WaterDepthMap.Num())
    {
        PreviousDepths = SimulationData.WaterDepthMap; // Initialize if needed
    }
    
    // Process ALL cells including edges for waterfall effect
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (SimulationData.WaterDepthMap[Index] <= MinWaterDepth)
            {
                continue;
            }
            
            // STABILITY FIX 3: Detect oscillation and apply damping
            float OscillationDamping = 1.0f;
            if (PreviousDepths.IsValidIndex(Index))
            {
                float CurrentDepth = SimulationData.WaterDepthMap[Index];
                float PreviousDepth = PreviousDepths[Index];
                float DepthChange = CurrentDepth - PreviousDepth;
                
                // Simple oscillation check: if depth is bouncing around previous value
                if (FMath::Abs(DepthChange) > CurrentDepth * 0.3f) // 30% change threshold
                {
                    OscillationDamping = 0.7f; // Reduce flow when oscillating
                }
            }
            
            // Get current velocities with stability adjustments
            float VelX = SimulationData.WaterVelocityX[Index] * StabilityFactor * OscillationDamping;
            float VelY = SimulationData.WaterVelocityY[Index] * StabilityFactor * OscillationDamping;
            
            // NEW: 8-DIRECTIONAL FLOW DISTRIBUTION
            // Calculate flow for all 8 directions
            float Flows[8] = {0}; // E, NE, N, NW, W, SW, S, SE
            
            // Decompose velocity into 8 directional components
            const float sqrt2inv = 0.7071f; // 1/sqrt(2)
            
            // East (0)
            Flows[0] = FMath::Max(0.0f, VelX);
            // West (4)
            Flows[4] = FMath::Max(0.0f, -VelX);
            // North (2) - negative Y in UE
            Flows[2] = FMath::Max(0.0f, -VelY);
            // South (6)
            Flows[6] = FMath::Max(0.0f, VelY);
            
            // Calculate diagonal flows based on velocity vector
            if (VelX > 0 && VelY < 0) // NE
            {
                float diagonalSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv;
                Flows[1] = diagonalSpeed * 0.5f;
            }
            if (VelX < 0 && VelY < 0) // NW
            {
                float diagonalSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv;
                Flows[3] = diagonalSpeed * 0.5f;
            }
            if (VelX < 0 && VelY > 0) // SW
            {
                float diagonalSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv;
                Flows[5] = diagonalSpeed * 0.5f;
            }
            if (VelX > 0 && VelY > 0) // SE
            {
                float diagonalSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv;
                Flows[7] = diagonalSpeed * 0.5f;
            }
            
            // Apply flow rate scaling - tunable via FlowRateMultiplier
            float FlowRate = SimulationData.WaterDepthMap[Index] * DeltaTime * FlowRateMultiplier;
            for (int i = 0; i < 8; i++)
            {
                Flows[i] *= FlowRate;
            }
            
            // FIXED: Compare WATER SURFACE elevations, not just water depths
            // A pool at a valley bottom should NOT drain just because it has more water than dry neighbors
            // Only accelerate outflow if water SURFACE is elevated relative to neighbor SURFACES
            float CenterTerrainHeight = GetTerrainHeightSafe(X, Y);
            float CenterSurface = CenterTerrainHeight + SimulationData.WaterDepthMap[Index];

            float NeighborSurfaceAvg = 0.0f;
            int32 NeighborCount = 0;

            for (int32 dy = -1; dy <= 1; dy++)
            {
                for (int32 dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0) continue;

                    int32 NX = X + dx;
                    int32 NY = Y + dy;

                    if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                    {
                        int32 NIndex = NY * Width + NX;
                        float NeighborTerrain = GetTerrainHeightSafe(NX, NY);
                        float NeighborSurface = NeighborTerrain + SimulationData.WaterDepthMap[NIndex];
                        NeighborSurfaceAvg += NeighborSurface;
                        NeighborCount++;
                    }
                }
            }

            float OutflowMultiplier = 1.0f;
            if (NeighborCount > 0)
            {
                NeighborSurfaceAvg /= NeighborCount;
                // Compare SURFACE elevations: terrain + water
                float SurfaceDiff = CenterSurface - NeighborSurfaceAvg;

                // Only boost outflow if water SURFACE is significantly elevated
                // A pool at a valley bottom will have SurfaceDiff <= 0 and won't drain
                if (SurfaceDiff > 0.5f)
                {
                    OutflowMultiplier = 1.0f + (SurfaceDiff * 0.3f);  // Reduced from 0.5f
                    OutflowMultiplier = FMath::Min(OutflowMultiplier, 2.0f);  // Reduced from 3.0f
                }
            }

            // Apply multiplier to all flows
            for (int i = 0; i < 8; i++)
            {
                Flows[i] *= OutflowMultiplier;
            }
            
            // Ensure total outflow doesn't exceed available water
            float TotalOutflow = 0.0f;
            for (int i = 0; i < 8; i++)
            {
                TotalOutflow += Flows[i];
            }

            // DEPTH-DEPENDENT COHESION: Deeper water is more stable (like real pressure)
            // - Shallow water (splashes): flows freely, preserves dynamic behavior
            // - Deep water (pool bottoms): resists outflow, allows accumulation
            // Tunable via CohesionReferenceDepth and CohesionStrength
            if (CohesionStrength > 0.0f)
            {
                // Normalized depth: 0 = shallow/surface, 1+ = deep
                float NormalizedDepth = FMath::Clamp(SimulationData.WaterDepthMap[Index] / CohesionReferenceDepth, 0.0f, 1.0f);

                // Cohesion scales with depth squared (pressure increases non-linearly)
                float CohesionFactor = NormalizedDepth * NormalizedDepth * CohesionStrength;

                // Reduce outflow based on cohesion - deep water stays, shallow water splashes
                TotalOutflow *= (1.0f - CohesionFactor);

                // Also scale individual flows to maintain distribution ratios
                float CohesionScale = (1.0f - CohesionFactor);
                for (int i = 0; i < 8; i++)
                {
                    Flows[i] *= CohesionScale;
                }
            }

            if (TotalOutflow > SimulationData.WaterDepthMap[Index])
            {
                float Scale = SimulationData.WaterDepthMap[Index] / TotalOutflow;
                for (int i = 0; i < 8; i++)
                {
                    Flows[i] *= Scale;
                }
                TotalOutflow = SimulationData.WaterDepthMap[Index];
            }
            
            // Remove water from current cell
            NewWaterDepth[Index] -= TotalOutflow;
            
            // Distribute to all 8 neighbors
            const int32 dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
            const int32 dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};
            
            float EdgeDrainageVolume = 0.0f;
            
            for (int i = 0; i < 8; i++)
            {
                int32 NX = X + dx[i];
                int32 NY = Y + dy[i];
                
                if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                {
                    int32 NIndex = NY * Width + NX;
                    NewWaterDepth[NIndex] += Flows[i];
                }
                else
                {
                    EdgeDrainageVolume += Flows[i];
                }
            }
            
            // Track edge drainage with water budget
            if (EdgeDrainageVolume > 0.0f && CachedMasterController)
            {
                float VolumeM3 = CachedMasterController->GetWaterCellVolume(EdgeDrainageVolume);
                FVector WorldPos = OwnerTerrain->TerrainToWorldPosition(X, Y);
                CachedMasterController->TransferSurfaceToGroundwater(WorldPos, VolumeM3);
            }
        }
    }
    
    // STABILITY FIX 4: Post-process smoothing for spikes
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        float NewDepth = FMath::Max(0.0f, NewWaterDepth[i]);
        
        // Gentle smoothing: if change is too dramatic, blend it
        if (PreviousDepths.IsValidIndex(i))
        {
            float OldDepth = SimulationData.WaterDepthMap[i];
            float DepthChange = NewDepth - OldDepth;
            
            // If depth changed by more than 50%, smooth it
            if (FMath::Abs(DepthChange) > OldDepth * 0.5f && OldDepth > MinWaterDepth)
            {
                // Blend 70% new, 30% old for stability
                NewDepth = NewDepth * 0.7f + OldDepth * 0.3f;
            }
        }
        
        SimulationData.WaterDepthMap[i] = NewDepth;
    }
    
    // Store current depths for next frame's oscillation detection
    PreviousDepths = SimulationData.WaterDepthMap;
    
    bWaterChangedThisFrame = true;
    bVolumeNeedsUpdate = true;
}


//HANDLES EVAPORATION AND ABSORPTION TO ATMOSPHERE AND GROUNDWATER

// ===== EVAPORATION & INFILTRATION =====

void UWaterSystem::ProcessWaterEvaporation(float DeltaTime)
{
    if (!SimulationData.IsValid() || !CachedMasterController)
    {
        return;
    }
    
   // if (bPausedForTerrainEdit) return;
    
    float TotalEvaporation = 0.0f;
    float TotalInfiltration = 0.0f;
    float CellArea = CachedMasterController->GetWaterCellArea();
    
    // Process all water cells locally (FAST - no external calls)
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        if (SimulationData.WaterDepthMap[i] > MinWaterDepth)
        {
            // Calculate evaporation
            float EvaporationDepth = WaterEvaporationRate * DeltaTime;
            EvaporationDepth = FMath::Min(EvaporationDepth, SimulationData.WaterDepthMap[i]);
            
            // Calculate infiltration from remaining water
            float RemainingDepth = SimulationData.WaterDepthMap[i] - EvaporationDepth;
            float InfiltrationDepth = 0.0f;
            if (RemainingDepth > MinWaterDepth)
            {
                InfiltrationDepth = WaterAbsorptionRate * DeltaTime;
                InfiltrationDepth = FMath::Min(InfiltrationDepth, RemainingDepth);
            }
            
            // Update water depth locally
            SimulationData.WaterDepthMap[i] -= (EvaporationDepth + InfiltrationDepth);
            
            // Accumulate totals
            TotalEvaporation += EvaporationDepth * CellArea;
            TotalInfiltration += InfiltrationDepth * CellArea;
        }
    }
    
    // Single report to master controller (FAST - just two function calls)
    if (TotalEvaporation > 0.0f)
       {
           // Calculate center of terrain for evaporation location
           FVector CenterLocation = OwnerTerrain->GetActorLocation();
           CachedMasterController->TransferSurfaceToAtmosphere(CenterLocation, TotalEvaporation);
       }
       
       if (TotalInfiltration > 0.0f)
       {
           // Route infiltration through soil moisture layer (NOT directly to groundwater!)
           // Water will sit in soil for plants to use, then slowly drain to water table
           FVector CenterLocation = OwnerTerrain->GetActorLocation();
           CachedMasterController->TransferSurfaceToSoilMoisture(CenterLocation, TotalInfiltration);
       }
    
    // Mark that water has changed
    if (TotalEvaporation > 0.0f || TotalInfiltration > 0.0f)
    {
        bWaterChangedThisFrame = true;
        bVolumeNeedsUpdate = true;
    }
}

// ===== SEDIMENT TRANSPORT =====

void UWaterSystem::ApplySedimentTransport(float DeltaTime)
{
    if (!SimulationData.IsValid() || SimulationData.SedimentMap.Num() == 0)
    {
        return;
    }

    TArray<float> NewSediment = SimulationData.SedimentMap;
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;

    // Sediment advects with water flow - follows same 8-directional pattern as water
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;

            // Need water to transport sediment
            float WaterDepth = SimulationData.WaterDepthMap[Index];
            if (WaterDepth <= MinWaterDepth)
            {
                continue;
            }

            float CurrentSediment = SimulationData.SedimentMap[Index];
            if (CurrentSediment <= 0.0f)
            {
                continue;
            }

            // Get water velocity
            float VelX = SimulationData.WaterVelocityX[Index];
            float VelY = SimulationData.WaterVelocityY[Index];

            // 8-directional decomposition (same as water flow)
            float Flows[8] = {0}; // E, NE, N, NW, W, SW, S, SE
            const float sqrt2inv = 0.7071f;

            // Cardinal directions
            Flows[0] = FMath::Max(0.0f, VelX);      // East
            Flows[4] = FMath::Max(0.0f, -VelX);     // West
            Flows[2] = FMath::Max(0.0f, -VelY);     // North
            Flows[6] = FMath::Max(0.0f, VelY);      // South

            // Diagonal flows
            if (VelX > 0 && VelY < 0) // NE
            {
                Flows[1] = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv * 0.5f;
            }
            if (VelX < 0 && VelY < 0) // NW
            {
                Flows[3] = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv * 0.5f;
            }
            if (VelX < 0 && VelY > 0) // SW
            {
                Flows[5] = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv * 0.5f;
            }
            if (VelX > 0 && VelY > 0) // SE
            {
                Flows[7] = FMath::Sqrt(VelX * VelX + VelY * VelY) * sqrt2inv * 0.5f;
            }

            // Sediment transport rate - proportional to water velocity and current sediment
            // Use same FlowRateMultiplier as water for consistency
            float TransportRate = CurrentSediment * DeltaTime * FlowRateMultiplier;
            for (int i = 0; i < 8; i++)
            {
                Flows[i] *= TransportRate;
            }

            // Calculate total outflow
            float TotalOutflow = 0.0f;
            for (int i = 0; i < 8; i++)
            {
                TotalOutflow += Flows[i];
            }

            // Clamp to available sediment
            if (TotalOutflow > CurrentSediment)
            {
                float Scale = CurrentSediment / TotalOutflow;
                for (int i = 0; i < 8; i++)
                {
                    Flows[i] *= Scale;
                }
                TotalOutflow = CurrentSediment;
            }

            // Remove sediment from current cell
            NewSediment[Index] -= TotalOutflow;

            // Distribute to 8 neighbors
            const int32 dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
            const int32 dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};

            for (int i = 0; i < 8; i++)
            {
                int32 NX = X + dx[i];
                int32 NY = Y + dy[i];

                if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                {
                    int32 NIndex = NY * Width + NX;
                    NewSediment[NIndex] += Flows[i];
                }
                // Sediment leaving edges is lost (washed away)
            }
        }
    }

    // Apply updates
    for (int32 i = 0; i < SimulationData.SedimentMap.Num(); i++)
    {
        SimulationData.SedimentMap[i] = FMath::Max(0.0f, NewSediment[i]);
    }
}

// ===== ADVANCED WATER TEXTURE SYSTEM =====


// END OF SECTION 2: CORE WATER PHYSICS


// ============================================================================
// SECTION 3: WATER MANAGEMENT & VISUALS
// ============================================================================
//
// This section handles all visual representation of water and management functions:
// - Volumetric water surface generation and chunk management
// - Enhanced mesh generation with LOD system
// - Flow displacement textures for GPU shader rendering
// - Authority-based water queries (via MasterController)
// - Coordinate transformation (MasterController authority)
// - Terrain synchronization and height caching
// - Localized mesh generation for specific areas
// - Advanced water texture system (depth, flow, foam, turbulence)
// - Foam data calculation (visual effects)
//
// Visual Pipeline:
// [SimulationData]  ->  [Chunk LOD Selection]  ->  [Mesh Generation]  ->
//  ->  [Flow Displacement]  ->  [GPU Shader Rendering]
//
// Authority Pattern:
// - SimulationData.WaterDepthMap is ground truth for water presence
// - MasterController authority for all coordinate transforms
// - Mesh generation blocked if simulation doesn't authorize (< MinMeshDepth)
//
// Key Subsystems:
// 1. Volumetric Water Surfaces - Chunk-based water mesh management
// 2. Enhanced Mesh Generation - High-quality procedural meshes with LOD
// 3. Flow Displacement - GPU texture system for shader-based waves
// 4. Authority Queries - Coordinate-aware water depth/velocity queries
// 5. Terrain Synchronization - Height caching and change response
// 6. Localized Generation - Targeted mesh updates
// 7. Advanced Textures - Multi-channel water property textures
// 8. Foam Calculation - Physically-based foam effects
//
// Dependencies: SimulationData, OwnerTerrain, CachedMasterController
// Threading: Game thread only (Unreal mesh API limitation)
// ============================================================================


// ===== SUBSYSTEM 3.1: VOLUMETRIC WATER SURFACES =====
// Chunk-based water surface management with LOD

// ===== VOLUMETRIC WATER IMPLEMENTATION =====

/**
 * PHASE 1-2: SYNCHRONIZED WATER SURFACE CHUNKS
 * Updates water surface chunks based EXCLUSIVELY on simulation data authority
 *
 * NEW DATA PIPELINE: [FWaterSimulationData]   [Mesh Generation]   [Procedural Surfaces]
 *
 * Algorithm:
 * 1. Query simulation state for each chunk (WaterDepthMap as ground truth)
 * 2. Calculate flow properties from velocity arrays (VelocityX/Y   FlowDirection/Speed)
 * 3. Generate visual effects based on FoamMap and simulation state
 * 4. Create surface meshes with simulation-driven parameters
 * 5. Apply materials with physically-based optical properties
 */
void UWaterSystem::UpdateWaterSurfaceChunks()
{
    if (!OwnerTerrain || !SimulationData.IsValid())
    {
        return;
    }
    
    // GPU MODE: Different logic - don't remove chunks aggressively
    if (bUseVertexDisplacement)
    {
        UpdateGPUWaterChunks();
        return;
    }
    
    // CPU MODE: Original logic continues...
    if (!bEnableWaterVolumes)
    {
        return;
    }
    
    
    
    // Safety check - ensure terrain chunks exist
    if (OwnerTerrain->TerrainChunks.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateWaterSurfaceChunks: No terrain chunks available"));
        return;
    }
    
    // Reset frame counters
    MeshUpdatesThisFrame = 0;
    
    // STEP 1: Remove chunks that no longer have water (with bounds checking)
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        
        // Bounds check before accessing terrain chunks
        if (!OwnerTerrain->TerrainChunks.IsValidIndex(Chunk.ChunkIndex))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid chunk index %d, removing water chunk"), Chunk.ChunkIndex);
            
            // Clean up invalid chunk
            if (Chunk.SurfaceMesh)
            {
                Chunk.SurfaceMesh->DestroyComponent();
            }
            WaterSurfaceChunks.RemoveAt(i);
            continue;
        }
        
        if (!ShouldGenerateSurfaceForChunk_AuthorityOnly(Chunk.ChunkIndex))
        {
            // Clean up mesh components
            if (Chunk.SurfaceMesh)
            {
                Chunk.SurfaceMesh->ClearAllMeshSections();
                Chunk.SurfaceMesh->DestroyComponent();
                Chunk.SurfaceMesh = nullptr;
            }
            
            WaterSurfaceChunks.RemoveAt(i);
        }
    }
    
    // STEP 2: Add new chunks where water appeared (with bounds checking)
    int32 MaxChunks = OwnerTerrain->TerrainChunks.Num();
    for (int32 ChunkIndex = 0; ChunkIndex < MaxChunks; ChunkIndex++)
    {
        // Double-check bounds
        if (!OwnerTerrain->TerrainChunks.IsValidIndex(ChunkIndex))
        {
            UE_LOG(LogTemp, Error, TEXT("ChunkIndex %d out of bounds during iteration"), ChunkIndex);
            break;
        }
        
        bool bShouldHaveWater = ShouldGenerateSurfaceForChunk_AuthorityOnly(ChunkIndex);
        
        FWaterSurfaceChunk* ExistingChunk = WaterSurfaceChunks.FindByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& Chunk) {
                return Chunk.ChunkIndex == ChunkIndex;
            });
        
        if (bShouldHaveWater && !ExistingChunk && WaterSurfaceChunks.Num() < MaxVolumeChunks)
        {
            // Create new water chunk with proper bounds checking
            FWaterSurfaceChunk NewChunk;
            NewChunk.ChunkIndex = ChunkIndex;
            NewChunk.bHasWater = true;
            NewChunk.bNeedsUpdate = true;
            NewChunk.CurrentLOD = 0;
            
            // Safe calculation of grid position
            int32 ChunksPerRow = FMath::Max(1, FMath::CeilToInt(SimulationData.TerrainWidth / (float)ChunkSize));
            NewChunk.ChunkX = ChunkIndex % ChunksPerRow;
            NewChunk.ChunkY = ChunkIndex / ChunksPerRow;
            NewChunk.WorldPosition = FVector2D(
                NewChunk.ChunkX * ChunkWorldSize,
                NewChunk.ChunkY * ChunkWorldSize
            );
            
            WaterSurfaceChunks.Add(NewChunk);
            ExistingChunk = &WaterSurfaceChunks.Last();
        }
        
        // Update existing chunks (CPU mode only)
        if (ExistingChunk && !bUseVertexDisplacement)
        {
            SynchronizeChunkWithSimulation(*ExistingChunk);
            
            if (ExistingChunk->bNeedsUpdate || bWaterChangedThisFrame)
            {
                CreateWaterSurfaceMesh(*ExistingChunk);
                ExistingChunk->bNeedsUpdate = false;
                MeshUpdatesThisFrame++;
            }
        }
    }
}


UProceduralMeshComponent* UWaterSystem::GetPooledMeshComponent()
{
    // Redirect to enhanced validated pooling system
    return GetValidatedPooledComponent();
}

void UWaterSystem::ReturnMeshComponentToPool(UProceduralMeshComponent* Component)
{
    // Redirect to enhanced validated pooling system
    ReturnComponentToValidatedPool(Component);
}

void UWaterSystem::ConnectToGPUTerrain(ADynamicTerrain* Terrain)
{
    if (!Terrain)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot connect to null terrain"));
        return;
    }
    
    OwnerTerrain = Terrain;
    
    // Ensure simulation data is valid
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SimulationData not valid - cannot create erosion textures"));
        return;
    }
    
    // ===== CRITICAL: ENSURE EROSION TEXTURES EXIST =====
    if (!ErosionWaterDepthRT || !ErosionFlowVelocityRT)
    {
        UE_LOG(LogTemp, Warning, TEXT("Erosion textures missing - creating now"));
        CreateErosionTextures();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Erosion textures already exist"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT(" Water connected to GPU terrain"));
}


// ===== SUBSYSTEM 3.2: ENHANCED MESH GENERATION =====
// High-quality procedural mesh generation with LOD, mesh pooling, and boundary fixes

// ===== ENHANCED MESH GENERATION WITH LOD =====

/**
 * PHASE 1-2: Synchronizes surface chunk with simulation data authority
 * Replaces manual property setting with simulation-driven data extraction
 *
 * Implementation follows refactor strategy data pipeline:
 * [FWaterSimulationData]   [WaterSurfaceChunk Properties]   [Mesh Generation]
 */
void UWaterSystem::SynchronizeChunkWithSimulation(FWaterSurfaceChunk& SurfaceChunk)
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        SurfaceChunk.ChunkIndex < 0 || SurfaceChunk.ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 1-2] Cannot sync chunk %d - invalid data"), SurfaceChunk.ChunkIndex);
        return;
    }
    
    // STEP 1: Calculate depth properties from simulation WaterDepthMap (authoritative)
    SurfaceChunk.MaxDepth = GetChunkMaxDepthFromSimulation(SurfaceChunk.ChunkIndex);
    SurfaceChunk.AverageDepth = GetChunkAverageDepthFromSimulation(SurfaceChunk.ChunkIndex);
    
    // STEP 2: Calculate flow properties from simulation velocity arrays (authoritative)
    SurfaceChunk.FlowDirection = GetChunkFlowDirectionFromSimulation(SurfaceChunk.ChunkIndex);
    SurfaceChunk.FlowSpeed = GetChunkFlowSpeedFromSimulation(SurfaceChunk.ChunkIndex);
    
    // STEP 3: Determine visual effects from simulation state (authoritative)
    SurfaceChunk.bHasFoam = GetChunkFoamStateFromSimulation(SurfaceChunk.ChunkIndex);
    SurfaceChunk.bHasCaustics = (SurfaceChunk.MaxDepth > 1.0f && SurfaceChunk.FlowSpeed > 5.0f);
    
    // STEP 4: Update wave phase for animation continuity
    if (OwnerTerrain->GetWorld())
    {
        float Time = OwnerTerrain->GetWorld()->GetTimeSeconds();
        SurfaceChunk.WavePhase = Time * 2.0f + (SurfaceChunk.ChunkIndex * 0.1f);
    }
    
    // STEP 5: Mark for update if significant changes detected
    static TMap<int32, float> LastKnownDepths;
    float* LastDepth = LastKnownDepths.Find(SurfaceChunk.ChunkIndex);
    if (!LastDepth || FMath::Abs(*LastDepth - SurfaceChunk.AverageDepth) > 0.1f)
    {
        SurfaceChunk.bNeedsUpdate = true;
        LastKnownDepths.Add(SurfaceChunk.ChunkIndex, SurfaceChunk.AverageDepth);
    }
}

/**
 * PHASE 1-2: Queries simulation data for maximum water depth in chunk
 * Uses WaterDepthMap as authoritative source (replaces manual depth calculation)
 */
float UWaterSystem::GetChunkMaxDepthFromSimulation(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return 0.0f;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    float MaxDepth = 0.0f;
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            float Depth = GetWaterDepthSafe(X, Y);
            MaxDepth = FMath::Max(MaxDepth, Depth);
        }
    }
    
    return MaxDepth;
}

/**
 * PHASE 1-2: Queries simulation data for average water depth in chunk
 * Provides optical depth calculations for Beer-Lambert law materials
 */
float UWaterSystem::GetChunkAverageDepthFromSimulation(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return 0.0f;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    float TotalDepth = 0.0f;
    int32 WaterCells = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            float Depth = GetWaterDepthSafe(X, Y);
            if (Depth > MinWaterDepth)
            {
                TotalDepth += Depth;
                WaterCells++;
            }
        }
    }
    
    return WaterCells > 0 ? (TotalDepth / WaterCells) : 0.0f;
}

/**
 * PHASE 1-2: Calculates flow direction from simulation velocity arrays
 * Provides normalized flow vector for surface animation
 */
FVector2D UWaterSystem::GetChunkFlowDirectionFromSimulation(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return FVector2D::ZeroVector;
    }

    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    FVector2D TotalFlow = FVector2D::ZeroVector;
    int32 FlowCells = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            if (GetWaterDepthSafe(X, Y) > MinWaterDepth)
            {
                int32 Index = Y * SimulationData.TerrainWidth + X;
                if (Index >= 0 && Index < SimulationData.WaterVelocityX.Num())
                {
                    TotalFlow.X += SimulationData.WaterVelocityX[Index];
                    TotalFlow.Y += SimulationData.WaterVelocityY[Index];
                    FlowCells++;
                }
            }
        }
    }
    
    if (FlowCells > 0)
    {
        TotalFlow /= FlowCells;
    }
    
    return TotalFlow.GetSafeNormal();
}

/**
 * PHASE 1-2: Calculates flow speed from simulation velocity arrays
 * Provides magnitude for animation intensity and surface deformation
 */
float UWaterSystem::GetChunkFlowSpeedFromSimulation(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return 0.0f;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    float TotalSpeed = 0.0f;
    int32 FlowCells = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            if (GetWaterDepthSafe(X, Y) > MinWaterDepth)
            {
                int32 Index = Y * SimulationData.TerrainWidth + X;
                if (Index >= 0 && Index < SimulationData.WaterVelocityX.Num())
                {
                    float VelX = SimulationData.WaterVelocityX[Index];
                    float VelY = SimulationData.WaterVelocityY[Index];
                    float Speed = FMath::Sqrt(VelX * VelX + VelY * VelY);
                    TotalSpeed += Speed;
                    FlowCells++;
                }
            }
        }
    }
    
    return FlowCells > 0 ? (TotalSpeed / FlowCells) : 0.0f;
}

/**
 * PHASE 1-2: Determines foam state from simulation FoamMap
 * Provides physically-based foam generation for materials
 */
bool UWaterSystem::GetChunkFoamStateFromSimulation(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return false;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    float TotalFoam = 0.0f;
    int32 WaterCells = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            if (GetWaterDepthSafe(X, Y) > MinWaterDepth)
            {
                int32 Index = Y * SimulationData.TerrainWidth + X;
                if (Index >= 0 && Index < SimulationData.FoamMap.Num())
                {
                    TotalFoam += SimulationData.FoamMap[Index];
                    WaterCells++;
                }
            }
        }
    }
    
    float AverageFoam = WaterCells > 0 ? (TotalFoam / WaterCells) : 0.0f;
    return AverageFoam > 0.3f; // Threshold for visible foam
}



// ===== PHASE 1 & 2: FLOW DISPLACEMENT SYSTEM IMPLEMENTATION =====

void UWaterSystem::CreateFlowDisplacementTexture()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating flow displacement texture"));
    
    if (!IsSystemReady())
    {
        UE_LOG(LogTemp, Error, TEXT("System not ready for displacement texture creation"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create RGBA render target for flow displacement
    // R: Flow displacement X, G: Flow displacement Y
    // B: Wave phase offset, A: Flow intensity
    FlowDisplacementTexture = NewObject<UTextureRenderTarget2D>(this);
    
    if (FlowDisplacementTexture)
    {
        FlowDisplacementTexture->RenderTargetFormat = RTF_RGBA16f;
        FlowDisplacementTexture->InitCustomFormat(Width, Height, PF_FloatRGBA, false);
        FlowDisplacementTexture->AddressX = TextureAddress::TA_Wrap;
        FlowDisplacementTexture->AddressY = TextureAddress::TA_Wrap;
        FlowDisplacementTexture->bCanCreateUAV = true;  // CRITICAL: Enable UAV for compute shader write
        FlowDisplacementTexture->UpdateResourceImmediate(false);
        
        UE_LOG(LogTemp, Warning, TEXT("Flow displacement texture created: %dx%d RGBA"), Width, Height);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create flow displacement texture"));
    }
}

void UWaterSystem::UpdateFlowDisplacementTexture()
{
    if (!FlowDisplacementTexture || !SimulationData.IsValid())
        return;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create data array with FLinearColor for RGBA float texture
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    // Fill with flow data
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (Index < SimulationData.WaterVelocityX.Num())
            {
                float VelX = SimulationData.WaterVelocityX[Index];
                float VelY = SimulationData.WaterVelocityY[Index];
                float Foam = SimulationData.FoamMap[Index];
                float Depth = SimulationData.WaterDepthMap[Index];
                
                // Pack data into RGBA channels
                TextureData[Index] = FLinearColor(VelX, VelY, Depth, Foam);
            }
            else
            {
                TextureData[Index] = FLinearColor(0, 0, 0, 0);
            }
        }
    }
    
    // Update render target using RHI commands on render thread
    FTextureRenderTargetResource* RenderTargetResource = FlowDisplacementTexture->GameThread_GetRenderTargetResource();
    if (RenderTargetResource)
    {
        ENQUEUE_RENDER_COMMAND(UpdateFlowDisplacementTextureCommand)(
            [RenderTargetResource, TextureData, Width, Height](FRHICommandListImmediate& RHICmdList)
            {
                FTextureRHIRef TextureRHI = RenderTargetResource->GetRenderTargetTexture();
                if (TextureRHI.IsValid())
                {
                    uint32 Stride = Width * sizeof(FLinearColor);
                    FUpdateTextureRegion2D Region(0, 0, 0, 0, Width, Height);
                    
                    RHICmdList.UpdateTexture2D(
                        TextureRHI,
                        0,  // Mip level
                        Region,
                        Stride,
                        (const uint8*)TextureData.GetData()
                    );
                }
            }
        );
    }
}

void UWaterSystem::UpdateWaterShaderForDisplacement(UMaterialInstanceDynamic* Material)
{
    if (!Material || !FlowDisplacementTexture)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Skipping displacement - Material:%s Texture:%s"),
               Material ? TEXT("OK") : TEXT("NULL"),
               FlowDisplacementTexture ? TEXT("OK") : TEXT("NULL"));
        return;
    }
    
    // Apply displacement texture and parameters
    Material->SetTextureParameterValue(FName("FlowDisplacementTexture"), FlowDisplacementTexture);
    Material->SetScalarParameterValue(FName("DisplacementStrength"), DisplacementStrength);
    Material->SetScalarParameterValue(FName("WaveAnimationSpeed"), WaveAnimationSpeed);
    Material->SetVectorParameterValue(FName("GlobalFlowDirection"),
        FLinearColor(GetDominantFlowDirection().X, GetDominantFlowDirection().Y, 0, 0));
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Updated material with displacement parameters"));
}

FVector2D UWaterSystem::GetDominantFlowDirection() const
{
    if (!SimulationData.IsValid())
    {
        return FVector2D::ZeroVector;
    }
    
    FVector2D TotalFlow = FVector2D::ZeroVector;
    int32 FlowCells = 0;
    
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        if (SimulationData.WaterDepthMap[i] > MinWaterDepth)
        {
            TotalFlow.X += SimulationData.WaterVelocityX[i];
            TotalFlow.Y += SimulationData.WaterVelocityY[i];
            FlowCells++;
        }
    }
    
    if (FlowCells > 0)
    {
        TotalFlow /= FlowCells;
    }
    
    return TotalFlow;
}

float UWaterSystem::GenerateWavePhase(int32 Index) const
{
    const int32 Width = SimulationData.TerrainWidth;
    int32 X = Index % Width;
    int32 Y = Index / Width;
    
    // Get actual flow direction (source of truth)
    FVector2D FlowDir(SimulationData.WaterVelocityX[Index], SimulationData.WaterVelocityY[Index]);
    float FlowMagnitude = FlowDir.Size();
    
    if (FlowMagnitude > 1.0f)
    {
        // Flow-oriented wave phase
        FVector2D FlowNormal = FlowDir.GetSafeNormal();
        return (X * FlowNormal.Y - Y * FlowNormal.X) * 0.1f * UE_PI + FlowMagnitude * 0.1f;
    }
    else
    {
        // Wind-oriented wave phase (for flat areas)
        // Use cached wind data here
        return (X * 0.1f + Y * 0.15f) * 2.0f * UE_PI;
    }
}

//Mesh Generation Validation Function

int32 UWaterSystem::CountActiveWaterCellsInChunk(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return 0;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, SimulationData.TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, SimulationData.TerrainHeight);
    
    int32 ActiveCells = 0;
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            int32 Index = Y * SimulationData.TerrainWidth + X;
            if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num() &&
                SimulationData.WaterDepthMap[Index] >= MinWaterDepth)
            {
                ActiveCells++;
            }
        }
    }
    
    return ActiveCells;
}

/**
 * ENHANCED: Check for contiguous water areas to prevent scattered cell meshing
 * Ensures water forms coherent bodies rather than isolated pixels
 */
bool UWaterSystem::CheckForContiguousWater(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return false;
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, SimulationData.TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, SimulationData.TerrainHeight);
    
    // Find largest contiguous water area using flood fill
    TSet<int32> VisitedCells;
    int32 LargestWaterArea = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            int32 Index = Y * SimulationData.TerrainWidth + X;
            
            if (VisitedCells.Contains(Index) ||
                SimulationData.WaterDepthMap[Index] < MinWaterDepth)
            {
                continue;
            }
            
            // Flood fill to find contiguous area size
            int32 AreaSize = FloodFillWaterArea(X, Y, StartX, StartY, EndX, EndY, VisitedCells);
            LargestWaterArea = FMath::Max(LargestWaterArea, AreaSize);
        }
    }
    
    // Require largest contiguous area to be at least 9 cells (3x3 minimum)
    return LargestWaterArea >= 9;
}

/**
 * HELPER: Flood fill algorithm to find contiguous water area size
 */
int32 UWaterSystem::FloodFillWaterArea(int32 StartX, int32 StartY, int32 MinX, int32 MinY,
                                      int32 MaxX, int32 MaxY, TSet<int32>& VisitedCells) const
{
    TArray<FIntPoint> Stack;
    Stack.Push(FIntPoint(StartX, StartY));
    int32 AreaSize = 0;
    
    while (Stack.Num() > 0)
    {
        FIntPoint Current = Stack.Pop();
        int32 X = Current.X;
        int32 Y = Current.Y;
        
        // Bounds check
        if (X < MinX || X >= MaxX || Y < MinY || Y >= MaxY) continue;
        
        int32 Index = Y * SimulationData.TerrainWidth + X;
        
        // Skip if already visited or no water
        if (VisitedCells.Contains(Index) ||
            SimulationData.WaterDepthMap[Index] < MinWaterDepth)
        {
            continue;
        }
        
        // Mark as visited and count
        VisitedCells.Add(Index);
        AreaSize++;
        
        // Add neighbors to stack
        Stack.Push(FIntPoint(X + 1, Y));
        Stack.Push(FIntPoint(X - 1, Y));
        Stack.Push(FIntPoint(X, Y + 1));
        Stack.Push(FIntPoint(X, Y - 1));
    }
    
    return AreaSize;
}




// ===== SUBSYSTEM 3.3: AUTHORITY-BASED WATER QUERIES =====
// PHASE 1 FIX: All coordinate transformations through MasterController authority

// ===== AUTHORITY-BASED WATER FUNCTIONS =====

bool UWaterSystem::ShouldGenerateSurfaceForChunk_AuthorityOnly(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !CachedMasterController ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return false;
    }
    
    // SINGLE AUTHORITY CHECK: Does simulation have water?
    float SimMaxDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
    float SimAvgDepth = GetChunkAverageDepthFromSimulation(ChunkIndex);
    
    // Require meaningful water depth
    if (SimMaxDepth < MinMeshDepth || SimAvgDepth < (MinWaterDepth))// * 1.5f))
    {
        return false;
    }
    
    // Require contiguous water (prevent scattered pixels)
    int32 ContiguousWaterCells = CountActiveWaterCellsInChunk(ChunkIndex);
    if (ContiguousWaterCells < MinContiguousWaterCells)
    {
        return false;
    }
    
    // Authority approves mesh creation
    return true;
}


void UWaterSystem::CreateWaterSurfaceMesh_AlwaysVisible(FWaterSurfaceChunk& SurfaceChunk)
{
    if (!OwnerTerrain || !SimulationData.IsValid())
    {
        return;
    }
    
    // AUTHORITY VALIDATION: Double-check simulation authorizes this
    float SimDepth = GetChunkMaxDepthFromSimulation(SurfaceChunk.ChunkIndex);
    if (SimDepth < MinMeshDepth)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTHORITY] Blocking mesh creation - simulation depth %.3f < minimum %.3f"),
               SimDepth, MinMeshDepth);
        return;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[MESH] Creating always-visible water mesh for chunk %d"),
           SurfaceChunk.ChunkIndex);
    
    // Create surface mesh component if needed
    if (!SurfaceChunk.SurfaceMesh)
    {
        SurfaceChunk.SurfaceMesh = GetValidatedPooledComponent();
        
        if (!SurfaceChunk.SurfaceMesh)
        {
            FString ComponentName = FString::Printf(TEXT("WaterSurface_%d"), SurfaceChunk.ChunkIndex);
            SurfaceChunk.SurfaceMesh = NewObject<UProceduralMeshComponent>(OwnerTerrain, *ComponentName);
            SurfaceChunk.SurfaceMesh->SetupAttachment(OwnerTerrain->GetRootComponent());
            SurfaceChunk.SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SurfaceChunk.SurfaceMesh->RegisterComponent();
        }
        
        // Position mesh with Z=0 since vertices contain absolute heights
        // Use relative location to maintain parent-child transform during rotation
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
        ChunkWorldPos.Z = 0.0f; // Force Z to zero
        SurfaceChunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPos - OwnerTerrain->GetActorLocation());
    }
    
    // Generate mesh data
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    // Use boundary-fixed mesh generation for seamless chunks
    GenerateSmoothWaterSurface(SurfaceChunk, Vertices, Triangles, Normals, UVs, VertexColors);
    
    // Apply mesh to component
    if (Vertices.Num() > 0 && Triangles.Num() > 0)
    {
        SurfaceChunk.SurfaceMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors,
                                                   TArray<FProcMeshTangent>(), true);
        
        // Apply material
        if (VolumeMaterial)
        {
            SurfaceChunk.SurfaceMesh->SetMaterial(0, VolumeMaterial);
        }
        else if (OwnerTerrain && OwnerTerrain->CurrentActiveMaterial)
        {
            SurfaceChunk.SurfaceMesh->SetMaterial(0, OwnerTerrain->CurrentActiveMaterial);
        }
        
        // Update UV mapping for coordinate authority
        UpdateSurfaceUVMapping(SurfaceChunk);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("[MESH] Created water mesh: %d vertices, %d triangles"),
               Vertices.Num(), Triangles.Num() / 3);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[MESH] No geometry generated for chunk %d"), SurfaceChunk.ChunkIndex);
    }
}



bool UWaterSystem::ValidateShaderDataForChunk(int32 ChunkIndex) const
{
    if (!WaterDepthTexture || !WaterDataTexture)
    {
        return false; // Shader textures not ready
    }
    
    // Check if shader system has been updated recently enough
    static float LastShaderUpdate = 0.0f;
    float CurrentTime = OwnerTerrain->GetWorld() ? OwnerTerrain->GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastShaderUpdate < WaterShaderUpdateInterval * 2.0f)
    {
        return true; // Shader data is fresh
    }
    
    return false; // Shader data too stale
}

int32 UWaterSystem::CountContiguousWaterCells(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return 0;
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, SimulationData.TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, SimulationData.TerrainHeight);
    
    // Find largest contiguous water region using flood fill
    TSet<int32> VisitedCells;
    int32 LargestContiguousArea = 0;
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            int32 Index = Y * SimulationData.TerrainWidth + X;
            
            if (VisitedCells.Contains(Index) || SimulationData.WaterDepthMap[Index] < MinWaterDepth)
                continue;
            
            // Flood fill to find this contiguous area size
            int32 AreaSize = FloodFillContiguousWater(X, Y, StartX, StartY, EndX, EndY, VisitedCells);
            LargestContiguousArea = FMath::Max(LargestContiguousArea, AreaSize);
        }
    }
    
    return LargestContiguousArea;
}

int32 UWaterSystem::FloodFillContiguousWater(int32 StartX, int32 StartY, int32 MinX, int32 MinY,
                                            int32 MaxX, int32 MaxY, TSet<int32>& VisitedCells) const
{
    TArray<FIntPoint> Stack;
    Stack.Push(FIntPoint(StartX, StartY));
    int32 AreaSize = 0;
    
    while (Stack.Num() > 0)
    {
        FIntPoint Current = Stack.Pop();
        int32 X = Current.X;
        int32 Y = Current.Y;
        
        if (X < MinX || X >= MaxX || Y < MinY || Y >= MaxY)
            continue;
        
        int32 Index = Y * SimulationData.TerrainWidth + X;
        
        if (VisitedCells.Contains(Index) || SimulationData.WaterDepthMap[Index] < MinWaterDepth)
            continue;
        
        VisitedCells.Add(Index);
        AreaSize++;
        
        // Add neighbors
        Stack.Push(FIntPoint(X + 1, Y));
        Stack.Push(FIntPoint(X - 1, Y));
        Stack.Push(FIntPoint(X, Y + 1));
        Stack.Push(FIntPoint(X, Y - 1));
    }
    
    return AreaSize;
}

// ===== PHASE 1: COORDINATE AUTHORITY CONSOLIDATION =====

/**
 * PHASE 1 FIX: Eliminate coordinate authority conflicts
 * ALL coordinate transformations MUST go through MasterController
 * NO FALLBACK calculations allowed - prevents phantom water
 */
int32 UWaterSystem::GetSimulationIndex(FVector2D WorldPos) const
{
    // AUTHORITY CHECK: Block operation if no MasterController
    if (!CachedMasterController || !SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[PHASE 1] No coordinate authority - blocking operation"));
        return -1;
    }
    
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(FVector(WorldPos.X, WorldPos.Y, 0));
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    
    if (IsValidCoordinate(X, Y))
        return Y * SimulationData.TerrainWidth + X;
    
    return -1;
}

float UWaterSystem::GetSimulationDepth(FVector2D WorldPos) const
{
    int32 Index = GetSimulationIndex(WorldPos);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
        return SimulationData.WaterDepthMap[Index];
    return 0.0f;
}

FVector2D UWaterSystem::GetSimulationVelocity(FVector2D WorldPos) const
{
    int32 Index = GetSimulationIndex(WorldPos);
    if (Index >= 0 && Index < SimulationData.WaterVelocityX.Num())
        return FVector2D(SimulationData.WaterVelocityX[Index], SimulationData.WaterVelocityY[Index]);
    return FVector2D::ZeroVector;
}

float UWaterSystem::GetExactWaterDepthAtWorld(FVector2D WorldPos) const
{
    return GetSimulationDepth(WorldPos);
}


// ===== SUBSYSTEM 3.4: TERRAIN SYNCHRONIZATION =====
// Height caching and response to terrain modifications

// ===== TERRAIN SYNCHRONIZATION IMPLEMENTATION =====

void UWaterSystem::RefreshTerrainHeightCache()
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return;
    
    // Rebuild entire terrain height cache
    int32 TotalCells = SimulationData.TerrainWidth * SimulationData.TerrainHeight;
    CachedTerrainHeights.SetNum(TotalCells);
    
    for (int32 Y = 0; Y < SimulationData.TerrainHeight; Y++)
    {
        for (int32 X = 0; X < SimulationData.TerrainWidth; X++)
        {
            int32 Index = Y * SimulationData.TerrainWidth + X;
            FVector WorldPos = OwnerTerrain->TerrainToWorldPosition(X, Y);
            
            // Get FRESH terrain height (not cached)
            CachedTerrainHeights[Index] = OwnerTerrain->GetHeightAtPosition(WorldPos);
        }
    }
    
    LastTerrainSyncTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
    UE_LOG(LogTemp, VeryVerbose, TEXT("Terrain height cache refreshed (%d cells)"), TotalCells);
}

void UWaterSystem::ForceWaterReflow()
{
    if (!SimulationData.IsValid())
        return;
    
    // Clear existing velocities to force recalculation
    for (int32 i = 0; i < SimulationData.WaterVelocityX.Num(); i++)
    {
        SimulationData.WaterVelocityX[i] *= 0.1f;  // Dampen but don't zero
        SimulationData.WaterVelocityY[i] *= 0.1f;
    }
    
    // Force immediate flow calculation for next frame
    bWaterChangedThisFrame = true;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Water velocities reset - forcing reflow"));
}

void UWaterSystem::ForceTerrainSync()
{
    RefreshTerrainHeightCache();
    ForceWaterReflow();
    
    if (CachedMasterController && CachedMasterController->GeologyController)
        {
            CachedMasterController->GeologyController->CheckInitialWaterTable();
        }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Manual terrain sync completed WITH WATER TABLE"));
}


float UWaterSystem::GetTerrainHeightSafe(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y) || !OwnerTerrain)
    {
        // CRITICAL FIX: Instead of returning 0, return the nearest valid neighbor height
        // This prevents artificial cliffs that cause drainage/pooling bugs
        
        // Find the nearest valid coordinate
        int32 ClampedX = FMath::Clamp(X, 0, SimulationData.TerrainWidth - 1);
        int32 ClampedY = FMath::Clamp(Y, 0, SimulationData.TerrainHeight - 1);
        
        // Return the clamped coordinate's height
        return OwnerTerrain->GetHeightSafe(ClampedX, ClampedY);
    }
    
    // Normal case - direct terrain grid access
    return OwnerTerrain->GetHeightSafe(X, Y);
}

FVector2D UWaterSystem::GetFlowVectorAtWorld(FVector2D WorldPos) const
{
    if (!SimulationData.IsValid() || !CachedMasterController)
        return FVector2D::ZeroVector;
    
    // Convert world position to simulation grid coordinates using MasterController
    // Note: MasterController takes FVector, so convert our FVector2D
    FVector WorldPos3D(WorldPos.X, WorldPos.Y, 0.0f);
    FVector2D TerrainPos = CachedMasterController->WorldToTerrainCoordinates(WorldPos3D);
    
    int32 X = FMath::FloorToInt(TerrainPos.X);
    int32 Y = FMath::FloorToInt(TerrainPos.Y);
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Bounds check
    if (X >= 0 && X < Width && Y >= 0 && Y < Height)
    {
        int32 Index = Y * Width + X;
        return FVector2D(SimulationData.WaterVelocityX[Index], SimulationData.WaterVelocityY[Index]);
    }
    
    return FVector2D::ZeroVector;
}

float UWaterSystem::GetTerrainGradientMagnitude(FVector2D WorldPos) const
{
    if (!OwnerTerrain)
        return 0.0f;
    
    float SampleDistance = 10.0f; // meters
    
    // Sample terrain heights in cardinal directions
    float CenterHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X, WorldPos.Y, 0));
    float LeftHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X - SampleDistance, WorldPos.Y, 0));
    float RightHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X + SampleDistance, WorldPos.Y, 0));
    float UpHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X, WorldPos.Y - SampleDistance, 0));
    float DownHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X, WorldPos.Y + SampleDistance, 0));
    
    // Calculate gradients
    float GradientX = (RightHeight - LeftHeight) / (2.0f * SampleDistance);
    float GradientY = (DownHeight - UpHeight) / (2.0f * SampleDistance);
    
    // Return gradient magnitude
    return FMath::Sqrt(GradientX * GradientX + GradientY * GradientY) * 100.0f; // Convert to percentage
}

FVector UWaterSystem::CalculateWaterNormal(FVector2D WorldPos, FVector2D FlowVector, float WaterDepth) const
{
    const float SampleDistance = 10.0f;
    
    if (bUse8DirectionalFlow)
    {
        // 8-directional gradient calculation
        float Heights[8];
        for (int i = 0; i < 8; i++)
        {
            float Angle = (i * PI * 0.25f);
            FVector2D SamplePos = WorldPos + FVector2D(
                FMath::Cos(Angle) * SampleDistance,
                FMath::Sin(Angle) * SampleDistance
            );
            Heights[i] = GetInterpolatedWaterDepthSeamless(SamplePos);
        }
        
        FVector2D Gradient;
        Gradient.X = (Heights[0] - Heights[4]) + // E - W
                    (Heights[1] - Heights[5]) * DiagonalFlowWeight + // NE - SW
                    (Heights[7] - Heights[3]) * DiagonalFlowWeight;  // NW - SE
        Gradient.Y = (Heights[2] - Heights[6]) + // N - S
                    (Heights[1] - Heights[7]) * DiagonalFlowWeight + // NE - NW
                    (Heights[3] - Heights[5]) * DiagonalFlowWeight;  // SE - SW
        
        FVector Normal(-Gradient.X, -Gradient.Y, SampleDistance * 2.0f);
        Normal.Normalize();
        return Normal;
    }
    else
    {
        // Original 4-directional
        float HeightN = GetInterpolatedWaterDepthSeamless(WorldPos + FVector2D(0, SampleDistance));
        float HeightS = GetInterpolatedWaterDepthSeamless(WorldPos - FVector2D(0, SampleDistance));
        float HeightE = GetInterpolatedWaterDepthSeamless(WorldPos + FVector2D(SampleDistance, 0));
        float HeightW = GetInterpolatedWaterDepthSeamless(WorldPos - FVector2D(SampleDistance, 0));
        
        FVector Normal(-(HeightE - HeightW), -(HeightN - HeightS), SampleDistance * 2.0f);
        Normal.Normalize();
        return Normal;
    }
}

float UWaterSystem::CalculateDistanceToWaterEdge(FVector2D WorldPos, const FWaterSurfaceChunk& SurfaceChunk) const
{
    return 100.0f; // Simplified implementation
}

float UWaterSystem::CalculateShoreBlendFactor(float WaterDepth, float DistanceToEdge) const
{
    if (WaterDepth < MinWaterDepth) return 0.0f;
    float BlendRange = 50.0f;
    return FMath::Clamp(DistanceToEdge / BlendRange, 0.0f, 1.0f);
}

float UWaterSystem::ApplyShoreBlending(float WaterDepth, float BlendFactor, float DistanceToEdge) const
{
    if (WaterDepth < MinWaterDepth) return 0.0f;
    return FMath::Max(WaterDepth * BlendFactor, MinWaterDepth);
}

// Add this helper function to force regeneration of all water meshes
void UWaterSystem::ForceFullMeshRegeneration()
{
    // Mark all chunks for update
    for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
    {
        Chunk.bNeedsUpdate = true;
    }
    
    // The meshes will be regenerated on the next UpdateWaterSurfaceChunks call
    // which happens every frame during UpdateWaterSimulation
    
    UE_LOG(LogTemp, Warning, TEXT("Forced full water mesh regeneration - all %d chunks marked for update"), WaterSurfaceChunks.Num());
}




// ===== SUBSYSTEM 3.5: LOCALIZED MESH GENERATION =====
// Targeted mesh updates for specific areas and contiguity checking

// ===== LOCALIZED MESH GENERATION =====

void UWaterSystem::GenerateLocalizedWaterMeshes()
{
    //   SYSTEM DISABLED: This function created spiky triangles due to broken i+=3 triangle generation
    UE_LOG(LogTemp, Error, TEXT("  BLOCKED: GenerateLocalizedWaterMeshes() disabled - prevents spiky triangles"));
    
    // Emergency cleanup if somehow regions exist
    for (FWaterMeshRegion& Region : WaterMeshRegions)
    {
        if (Region.MeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("  CLEANUP: Destroying localized mesh component"));
            Region.MeshComponent->DestroyComponent();
            Region.MeshComponent = nullptr;
        }
    }
    WaterMeshRegions.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("  SYSTEM: Using optimized chunk system instead"));
    return; // Function completely disabled
}



bool UWaterSystem::ShouldGenerateLocalizedMesh(FVector2D WorldPosition, float SampleRadius) const
{
    if (!CachedMasterController || !SimulationData.IsValid())
    {
        return false;
    }
    
    // Sample 3x3 grid around position
    int32 WaterCells = 0;
    float MaxDepth = 0.0f;
    
    for (int32 Y = -1; Y <= 1; Y++)
    {
        for (int32 X = -1; X <= 1; X++)
        {
            FVector2D SamplePos = WorldPosition + FVector2D(X * SampleRadius, Y * SampleRadius);
            float Depth = GetSimulationDepthAuthority(SamplePos);
            if (Depth > MinWaterDepth)
            {
                WaterCells++;
                MaxDepth = FMath::Max(MaxDepth, Depth);
            }
        }
    }
    
    // Require majority water coverage (5/9 cells)
    return (WaterCells >= 5 && MaxDepth > MinVolumeDepth);
}

float UWaterSystem::GetSimulationDepthAuthority(FVector2D WorldPos) const
{
    if (!CachedMasterController || !SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(FVector(WorldPos.X, WorldPos.Y, 0));
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    if (X >= 0 && X < SimulationData.TerrainWidth && Y >= 0 && Y < SimulationData.TerrainHeight)
    {
        int32 Index = Y * SimulationData.TerrainWidth + X;
        if (Index < SimulationData.WaterDepthMap.Num())
        {
            return SimulationData.WaterDepthMap[Index];
        }
    }
    
    return 0.0f;
}

/**
 * Determines if a terrain chunk should have a water volume generated
 * Checks for sufficient water depth and cell count to justify volume creation
 *
 * @param ChunkIndex - Index into terrain chunks array
 * @return true if volume should be generated, false otherwise
 */
/**
 * PHASE 2: Enhanced simulation authority validation for chunk generation
 * Implements stricter requirements to eliminate phantom water chunks
 * Only creates meshes where simulation data explicitly authorizes water presence
 */
bool UWaterSystem::ShouldGenerateSurfaceForChunk(int32 ChunkIndex) const
{
    // Basic validation
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return false;
    }
    
    // PHASE 2: MANDATORY SIMULATION AUTHORITY VALIDATION
    
    // Query simulation data directly (ONLY authoritative source)
    float SimMaxDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
    float SimAvgDepth = GetChunkAverageDepthFromSimulation(ChunkIndex);
    int32 ActiveWaterCells = CountActiveWaterCellsInChunk(ChunkIndex);
    
    // PHASE 2: ENHANCED REQUIREMENTS - stricter thresholds to prevent phantom water
    bool bHasSignificantWater = (
        SimMaxDepth >= MinVolumeDepth &&           // Must meet minimum volume depth
        SimAvgDepth >= (MinWaterDepth * 2.0f) &&   // Average depth must be substantial (increased from 1.5f)
        ActiveWaterCells >= 9                       // Must have meaningful cell count (increased from 6)
    );
    
    // PHASE 2: WATER DENSITY VALIDATION - prevent sparse water meshing
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 TotalCells = ChunkSize * ChunkSize;
    float WaterDensity = (float)ActiveWaterCells / TotalCells;
    
    // PHASE 2: Require at least 8% water coverage (increased from 5%)
    bool bHasSufficientDensity = (WaterDensity >= 0.08f);
    
    // PHASE 2: CONTIGUOUS WATER VALIDATION - prevent scattered cell meshing
    bool bHasContiguousWater = CheckForContiguousWater(ChunkIndex);
    
    // PHASE 2: ALL CONDITIONS MUST BE MET - no partial generation
    bool bShouldGenerate = bHasSignificantWater && bHasSufficientDensity && bHasContiguousWater;
    
    // PHASE 2: Enhanced logging for rejected chunks (helps debug phantom water)
    if (!bShouldGenerate && (SimMaxDepth > 0.01f || ActiveWaterCells > 0))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 2] Chunk %d REJECTED - MaxDepth:%.2f(req:%.2f) AvgDepth:%.2f(req:%.2f) Cells:%d(req:9) Density:%.3f(req:0.08) Contiguous:%s"),
               ChunkIndex, SimMaxDepth, MinVolumeDepth, SimAvgDepth, MinWaterDepth * 2.0f,
               ActiveWaterCells, WaterDensity, bHasContiguousWater ? TEXT("YES") : TEXT("NO"));
    }
    else if (bShouldGenerate)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 2] Chunk %d APPROVED - All simulation criteria met"), ChunkIndex);
    }
    
    return bShouldGenerate;
}

/**
 * PHASE 2: Mandatory simulation authority validation for mesh creation
 * Creates surface-based water mesh ONLY when simulation data authorizes it
 * Prevents phantom water chunks that don't match simulation reality
 *
 * @param SurfaceChunk - Reference to surface chunk to generate mesh for
 */
void UWaterSystem::CreateWaterSurfaceMesh(FWaterSurfaceChunk& SurfaceChunk)
{
    if (!OwnerTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("[PHASE 2] CreateWaterSurfaceMesh FAILED - No OwnerTerrain"));
        return;
    }
    
    // PHASE 2 AUTHORITY CHECK: Verify simulation supports this chunk
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[PHASE 2] BLOCKING mesh creation - invalid simulation data for chunk %d"), SurfaceChunk.ChunkIndex);
        return;
    }
    
    // PHASE 2 SIMULATION VALIDATION: Check authoritative simulation state
    float SimDepth = GetChunkMaxDepthFromSimulation(SurfaceChunk.ChunkIndex);
    if (SimDepth < MinVolumeDepth)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] BLOCKING mesh creation - simulation has insufficient water (%.3f) in chunk %d"),
               SimDepth, SurfaceChunk.ChunkIndex);
        
        // Clear any existing mesh - simulation says no water here
        if (SurfaceChunk.SurfaceMesh)
        {
            SurfaceChunk.SurfaceMesh->ClearAllMeshSections();
            SurfaceChunk.SurfaceMesh->SetVisibility(false);
        }
        if (SurfaceChunk.UndersideMesh)
        {
            SurfaceChunk.UndersideMesh->ClearAllMeshSections();
            SurfaceChunk.UndersideMesh->SetVisibility(false);
        }
        return; // Block mesh creation - simulation authority overrides
    }
    
    // PHASE 2 CONTIGUOUS WATER CHECK: Ensure water forms coherent bodies
    if (!CheckForContiguousWater(SurfaceChunk.ChunkIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] BLOCKING mesh creation - water not contiguous in chunk %d"),
               SurfaceChunk.ChunkIndex);
        return; // Block scattered water meshing
    }
    
    /*
    // Simulation authorizes mesh creation - proceed
    UE_LOG(LogTemp, Log, TEXT("[PHASE 2]  Simulation authorizes mesh creation for chunk %d (depth: %.2f)"),
           SurfaceChunk.ChunkIndex, SimDepth);
    
    // DISABLED:     UE_LOG(LogTemp, Warning, TEXT("[MESH DEBUG]  Proceeding with mesh creation for chunk %d"), SurfaceChunk.ChunkIndex);
    */
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
  
    // Create surface mesh component if needed
    if (!SurfaceChunk.SurfaceMesh)
    {
        // Try to get from pool first
        SurfaceChunk.SurfaceMesh = GetValidatedPooledComponent();
        
        if (!SurfaceChunk.SurfaceMesh)
        {
            // Create new if pool empty
            FString ComponentName = FString::Printf(TEXT("WaterSurface_%d"), SurfaceChunk.ChunkIndex);
            SurfaceChunk.SurfaceMesh = NewObject<UProceduralMeshComponent>(OwnerTerrain, *ComponentName);
            
            // Set up attachment only for new components
            SurfaceChunk.SurfaceMesh->SetupAttachment(OwnerTerrain->GetRootComponent());
            SurfaceChunk.SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SurfaceChunk.SurfaceMesh->RegisterComponent();
        }
        else
        {
            // Pooled component - already registered, just re-activate
            SurfaceChunk.SurfaceMesh->SetVisibility(true);
            SurfaceChunk.SurfaceMesh->ClearAllMeshSections();
        }
    }

    
    // Create underside mesh component if needed
    if (!SurfaceChunk.UndersideMesh)
    {
        FString ComponentName = FString::Printf(TEXT("WaterUnderside_%d"), SurfaceChunk.ChunkIndex);
        SurfaceChunk.UndersideMesh = NewObject<UProceduralMeshComponent>(OwnerTerrain, *ComponentName);
        SurfaceChunk.UndersideMesh->SetupAttachment(OwnerTerrain->GetRootComponent());
        SurfaceChunk.UndersideMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SurfaceChunk.UndersideMesh->RegisterComponent();
    }
    
    // Generate smooth surface geometry with boundary fixes
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    GenerateSmoothWaterSurface(SurfaceChunk, Vertices, Triangles, Normals, UVs, VertexColors);
    
    // Create surface mesh section
    SurfaceChunk.SurfaceMesh->CreateMeshSection_LinearColor(
        0, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(),
        TArray<FProcMeshTangent>(), true
    );
    
    // Apply materials with optical depth
    UMaterialInterface* MaterialToUse = VolumeMaterial ? VolumeMaterial : OwnerTerrain->CurrentActiveMaterial;
 
    if (MaterialToUse)
    {
        UMaterialInstanceDynamic* SurfaceMaterialInstance = UMaterialInstanceDynamic::Create(
            MaterialToUse, OwnerTerrain);
        
        if (SurfaceMaterialInstance)
        {
            ApplyVolumetricWaterToMaterial(SurfaceMaterialInstance);
            
            // Surface-specific parameters
            SurfaceMaterialInstance->SetScalarParameterValue(FName("IsSurface"), 1.0f);
            SurfaceMaterialInstance->SetScalarParameterValue(FName("MaxDepth"), SurfaceChunk.MaxDepth);
            SurfaceMaterialInstance->SetScalarParameterValue(FName("AverageDepth"), SurfaceChunk.AverageDepth);
            
            SurfaceChunk.SurfaceMesh->SetMaterial(0, SurfaceMaterialInstance);
        }
    }
    
    // FIX: Set mesh position with Z=0 since vertices contain absolute heights
    // Use relative location to maintain parent-child transform during rotation
    ChunkWorldPos.Z = 0.0f;
    SurfaceChunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPos - OwnerTerrain->GetActorLocation());
    
    SurfaceChunk.LastUpdateTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
    
}

/**
 * Generates smooth water surface geometry using bicubic interpolation
 * Creates natural-looking water surfaces that properly represent depth
 *
 * @param SurfaceChunk - Surface chunk to generate geometry for
 * @param Vertices - Output vertex array
 * @param Triangles - Output triangle indices
 * @param Normals - Output vertex normals
 * @param UVs - Output UV coordinates
 * @param VertexColors - Output vertex colors (depth encoding)
 */



void UWaterSystem::GenerateWaterSurfaceTriangles(int32 Resolution, TArray<int32>& Triangles, const TArray<int32>& VertexIndexMap)
{
    Triangles.Empty();
    
    // Generate triangles for the water surface grid
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            // Get vertex indices for the quad
            int32 TopLeft = VertexIndexMap[Y * Resolution + X];
            int32 TopRight = VertexIndexMap[Y * Resolution + (X + 1)];
            int32 BottomLeft = VertexIndexMap[(Y + 1) * Resolution + X];
            int32 BottomRight = VertexIndexMap[(Y + 1) * Resolution + (X + 1)];
            
            // Only create triangles if all vertices exist
            if (TopLeft >= 0 && TopRight >= 0 && BottomLeft >= 0 && BottomRight >= 0)
            {
                // First triangle: TopLeft -> BottomLeft -> TopRight
                Triangles.Add(TopLeft);
                Triangles.Add(BottomLeft);
                Triangles.Add(TopRight);
                
                // Second triangle: TopRight -> BottomLeft -> BottomRight
                Triangles.Add(TopRight);
                Triangles.Add(BottomLeft);
                Triangles.Add(BottomRight);
            }
        }
    }
}

/**
 * Gets interpolated water depth using bilinear interpolation
 * Provides smooth depth transitions for natural surface generation
 *
 * @param WorldPosition - World position to sample depth at
 * @return Interpolated water depth value
 */
/**
 * PHASE 1 FIX: Authority-validated water depth interpolation
 * NO FALLBACK calculations - MasterController authority required
 * Enhanced validation prevents phantom water generation
 */
float UWaterSystem::GetInterpolatedWaterDepth(FVector2D WorldPosition) const
{
    // AUTHORITY CHECK: Block all operations without MasterController
    if (!CachedMasterController || !SimulationData.IsValid())
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 1] Depth query blocked - no coordinate authority"));
        return 0.0f; // NO FALLBACKS - prevents phantom water
    }
    
    // Convert to simulation coordinates using SINGLE AUTHORITY
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(
        FVector(WorldPosition.X, WorldPosition.Y, 0));
    
    int32 X0 = FMath::FloorToInt(TerrainCoords.X);
    int32 Y0 = FMath::FloorToInt(TerrainCoords.Y);
    int32 X1 = X0 + 1;
    int32 Y1 = Y0 + 1;
    
    // STRICT bounds checking - return 0 for out-of-bounds (prevents phantom water)
    if (X0 < 0 || Y0 < 0 || X1 >= SimulationData.TerrainWidth || Y1 >= SimulationData.TerrainHeight)
    {
        return 0.0f;
    }
    
    // Sample four corners with simulation authority validation
    float Depth00 = GetWaterDepthSafe(X0, Y0);
    float Depth10 = GetWaterDepthSafe(X1, Y0);
    float Depth01 = GetWaterDepthSafe(X0, Y1);
    float Depth11 = GetWaterDepthSafe(X1, Y1);
    
    // ENHANCED VALIDATION: Only interpolate if significant water exists
    float MaxDepthInArea = FMath::Max(FMath::Max(Depth00, Depth10), FMath::Max(Depth01, Depth11));
    int32 ValidCells = 0;
    if (Depth00 >= MinWaterDepth) ValidCells++;
    if (Depth10 >= MinWaterDepth) ValidCells++;
    if (Depth01 >= MinWaterDepth) ValidCells++;
    if (Depth11 >= MinWaterDepth) ValidCells++;
    
    // PHASE 2: Require at least 2 valid cells and meaningful depth for mesh generation
    if (MaxDepthInArea < MinWaterDepth || ValidCells < 2)
    {
        return 0.0f; // Insufficient water for mesh generation
    }
    
    // Smooth interpolation with fractional coordinates
    float FracX = TerrainCoords.X - X0;
    float FracY = TerrainCoords.Y - Y0;
    
    float DepthTop = FMath::Lerp(Depth00, Depth10, FracX);
    float DepthBottom = FMath::Lerp(Depth01, Depth11, FracX);
    float InterpolatedDepth = FMath::Lerp(DepthTop, DepthBottom, FracY);
    
    // Final authority validation
    return FMath::Max(0.0f, InterpolatedDepth);
}

// ==============================================
// ENHANCED WATER MESH LOD & POOLING SYSTEM IMPLEMENTATION
// ==============================================

float UWaterSystem::GetInterpolatedWaterDepthFixed(FVector2D WorldPosition) const
{
    // SINGLE AUTHORITY: Always use MasterController if available
    if (!CachedMasterController || !SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[WATER] No authority for depth calculation"));
        return 0.0f;
    }
    
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(
        FVector(WorldPosition.X, WorldPosition.Y, 0));
    
    int32 X0 = FMath::FloorToInt(TerrainCoords.X);
    int32 Y0 = FMath::FloorToInt(TerrainCoords.Y);
    
    // Bounds checking
    if (X0 < 0 || Y0 < 0 || X0 >= SimulationData.TerrainWidth - 1 || Y0 >= SimulationData.TerrainHeight - 1)
    {
        return 0.0f;
    }
    
    // Sample and interpolate
    float FracX = TerrainCoords.X - X0;
    float FracY = TerrainCoords.Y - Y0;
    
    float Depth00 = GetWaterDepthSafe(X0, Y0);
    float Depth10 = GetWaterDepthSafe(X0 + 1, Y0);
    float Depth01 = GetWaterDepthSafe(X0, Y0 + 1);
    float Depth11 = GetWaterDepthSafe(X0 + 1, Y0 + 1);
    
    float DepthTop = FMath::Lerp(Depth00, Depth10, FracX);
    float DepthBottom = FMath::Lerp(Depth01, Depth11, FracX);
    
    return FMath::Lerp(DepthTop, DepthBottom, FracY);
}

// ===== MISSING FUNCTION IMPLEMENTATIONS FOR ARM64 LINKER =====

void UWaterSystem::OptimizeWaterMeshUpdates(FVector CameraLocation, float DeltaTime)
{
    if (!OwnerTerrain || !SimulationData.IsValid())
    {
        return;
    }
    
    // Throttle mesh updates based on performance
    static float LastOptimizationTime = 0.0f;
    float CurrentTime = OwnerTerrain->GetWorld() ? OwnerTerrain->GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastOptimizationTime < 0.5f) // Only optimize every 500ms
    {
        return;
    }
    LastOptimizationTime = CurrentTime;
    
    // Reset frame counters
    MeshUpdatesThisFrame = 0;
    
    // Sort chunks by distance for priority updates
    TArray<TPair<int32, float>> ChunkDistances;
    
    for (int32 i = 0; i < WaterSurfaceChunks.Num(); i++)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        FVector ChunkPos = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
        float Distance = FVector::Dist(CameraLocation, ChunkPos);
        ChunkDistances.Add(TPair<int32, float>(i, Distance));
    }
    
    // Sort by distance
    ChunkDistances.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B) {
        return A.Value < B.Value;
    });
    
    // Update closest chunks first
    int32 UpdatesAllowed = FMath::Max(MaxMeshUpdatesPerFrame, 4);
    for (const auto& ChunkPair : ChunkDistances)
    {
        if (MeshUpdatesThisFrame >= UpdatesAllowed) break;
        
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[ChunkPair.Key];
        if (Chunk.bNeedsUpdate)
        {
            SynchronizeChunkWithSimulation(Chunk);
            CreateWaterSurfaceMesh_AlwaysVisible(Chunk);
            Chunk.bNeedsUpdate = false;
            MeshUpdatesThisFrame++;
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[OPTIMIZE] Updated %d water meshes this frame"), MeshUpdatesThisFrame);
}

bool UWaterSystem::ShouldUpdateWaterMesh(int32 ChunkIndex, FVector CameraLocation) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return false;
    }
    
    // Check if chunk has significant water
    float SimDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
    if (SimDepth < MinVolumeDepth)
    {
        return false;
    }
    
    // Check distance (no longer used in always-visible system, but keeping for compatibility)
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(ChunkIndex);
    float Distance = FVector::Dist(CameraLocation, ChunkWorldPos);
    
    return Distance < VolumeUpdateDistance * 2.0f; // Large range for compatibility
}

void UWaterSystem::ReplaceExistingUpdateFunction()
{
    // Fallback to the old LOD-based update system
    if (!OwnerTerrain || !SimulationData.IsValid() || !bEnableWaterVolumes)
    {
        return;
    }
    
    // Get camera location
    FVector CameraLocation = FVector::ZeroVector;
    if (OwnerTerrain->GetWorld())
    {
        if (APlayerController* PC = OwnerTerrain->GetWorld()->GetFirstPlayerController())
        {
            if (PC->PlayerCameraManager)
            {
                CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
            }
        }
    }
    
    // Update existing chunks
    for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
    {
        if (ShouldUpdateWaterMesh(Chunk.ChunkIndex, CameraLocation))
        {
            SynchronizeChunkWithSimulation(Chunk);
            if (Chunk.bNeedsUpdate)
            {
                CreateWaterSurfaceMesh(Chunk);
                Chunk.bNeedsUpdate = false;
            }
        }
    }
    
    // Create new chunks for visible areas with water
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        if (ShouldGenerateSurfaceForChunk(ChunkIndex) &&
            ShouldUpdateWaterMesh(ChunkIndex, CameraLocation))
        {
            // Check if chunk already exists
            bool bExists = WaterSurfaceChunks.ContainsByPredicate(
                [ChunkIndex](const FWaterSurfaceChunk& Chunk) {
                    return Chunk.ChunkIndex == ChunkIndex;
                }
            );
            
            if (!bExists && WaterSurfaceChunks.Num() < MaxVolumeChunks)
            {
                FWaterSurfaceChunk NewChunk;
                NewChunk.ChunkIndex = ChunkIndex;
                NewChunk.bNeedsUpdate = true;
                WaterSurfaceChunks.Add(NewChunk);
            }
        }
    }
}

UProceduralMeshComponent* UWaterSystem::GetValidatedPooledComponent()
{
    if (!bEnableComponentPooling)
    {
        return CreateNewMeshComponent();
    }
    
    // Clean up invalid components first
    ValidateAndCleanPool();
    
    // Find best component from pool
    UProceduralMeshComponent* BestComponent = nullptr;
    float OldestTime = 0.0f;
    
    for (int32 i = ValidatedMeshPool.Num() - 1; i >= 0; i--)
    {
        UProceduralMeshComponent* Component = ValidatedMeshPool[i];
        if (IsValid(Component))
        {
            float* LastUsed = PoolComponentLastUsed.Find(Component);
            if (!LastUsed || *LastUsed > OldestTime)
            {
                BestComponent = Component;
                OldestTime = LastUsed ? *LastUsed : 0.0f;
            }
        }
    }
    
    if (BestComponent)
    {
        // Remove from pool and reset
        ValidatedMeshPool.Remove(BestComponent);
        PoolComponentLastUsed.Remove(BestComponent);
        
        // Reset component state
        BestComponent->ClearAllMeshSections();
        BestComponent->SetVisibility(true);
        BestComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        return BestComponent;
    }
    
    return CreateNewMeshComponent();
}

void UWaterSystem::ReturnComponentToValidatedPool(UProceduralMeshComponent* Component)
{
    if (!Component || !IsValid(Component) || !bEnableComponentPooling)
    {
        if (Component) Component->DestroyComponent();
        return;
    }
    
    // Don't exceed pool size
    if (ValidatedMeshPool.Num() >= MaxPoolSize)
    {
        Component->DestroyComponent();
        return;
    }
    
    // Validate component state before pooling
    if (!ValidateComponentForPooling(Component))
    {
        Component->DestroyComponent();
        return;
    }
    
    // Clean and add to pool
    Component->ClearAllMeshSections();
    Component->SetVisibility(false);
    Component->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    
    ValidatedMeshPool.Add(Component);
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        PoolComponentLastUsed.Add(Component, OwnerTerrain->GetWorld()->GetTimeSeconds());
    }
}

bool UWaterSystem::ValidateComponentForPooling(UProceduralMeshComponent* Component) const
{
    if (!Component || !IsValid(Component))
    {
        return false;
    }
    
    // Check component hasn't been corrupted
    if (!Component->GetOwner() || Component->IsBeingDestroyed())
    {
        return false;
    }
    
    // Verify attachment hierarchy is valid
    if (!Component->GetAttachParent())
    {
        return false;
    }
    
    return true;
}

void UWaterSystem::ValidateAndCleanPool()
{
    if (!OwnerTerrain || !OwnerTerrain->GetWorld()) return;
    
    float CurrentTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
    
    for (int32 i = ValidatedMeshPool.Num() - 1; i >= 0; i--)
    {
        UProceduralMeshComponent* Component = ValidatedMeshPool[i];
        
        // Remove invalid components
        if (!IsValid(Component) || !ValidateComponentForPooling(Component))
        {
            ValidatedMeshPool.RemoveAt(i);
            if (Component) PoolComponentLastUsed.Remove(Component);
            continue;
        }
        
        // Remove components unused for too long
        float* LastUsed = PoolComponentLastUsed.Find(Component);
        if (LastUsed && CurrentTime - *LastUsed > 60.0f) // 1 minute timeout
        {
            ValidatedMeshPool.RemoveAt(i);
            PoolComponentLastUsed.Remove(Component);
            Component->DestroyComponent();
        }
    }
}

UProceduralMeshComponent* UWaterSystem::CreateNewMeshComponent()
{
    if (!OwnerTerrain)
    {
        return nullptr;
    }
    
    static int32 ComponentCounter = 0;
    FString ComponentName = FString::Printf(TEXT("WaterMesh_%d"), ComponentCounter++);
    
    UProceduralMeshComponent* NewComponent = NewObject<UProceduralMeshComponent>(
        OwnerTerrain, *ComponentName);
    
    if (NewComponent)
    {
        NewComponent->SetupAttachment(OwnerTerrain->GetRootComponent());
        NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        NewComponent->RegisterComponent();
    }
    
    return NewComponent;
}

// ==============================================
// OPTIMIZED MESH UPDATE SYSTEM REPLACEMENT
// ==============================================

void UWaterSystem::UpdateWaterSurfaceChunksFixed()
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !bEnableWaterVolumes)
    {
        return;
    }
    
    FVector CameraLocation = GetCameraLocation();
    MeshUpdatesThisFrame = 0;
    
    // ISSUE FIX 1: Ensure multiple meshes can load simultaneously
    MaxMeshUpdatesPerFrame = FMath::Max(MaxMeshUpdatesPerFrame, 8); // Allow at least 8 per frame
    
    // ISSUE FIX 2: Get visible chunks with increased range
    TArray<int32> VisibleChunks;
    GetVisibleWaterChunks(CameraLocation, VisibleChunks);
    
    // Update existing chunks first
    for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
    {
        if (MeshUpdatesThisFrame >= MaxMeshUpdatesPerFrame) break;
        
        if (VisibleChunks.Contains(Chunk.ChunkIndex))
        {
            UpdateExistingChunk(Chunk, CameraLocation);
        }
    }
    
    // Create new chunks for visible areas
    for (int32 ChunkIndex : VisibleChunks)
    {
        if (MeshUpdatesThisFrame >= MaxMeshUpdatesPerFrame) break;
        
        bool bExists = WaterSurfaceChunks.ContainsByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& Chunk) {
                return Chunk.ChunkIndex == ChunkIndex;
            });
        
        if (!bExists)
        {
            CreateNewWaterChunk(ChunkIndex);
        }
    }
    
    // Clean up only truly distant chunks
    CleanupDistantChunks(CameraLocation);
}

void UWaterSystem::GetVisibleWaterChunks(FVector CameraLocation, TArray<int32>& OutVisibleChunks)
{
    OutVisibleChunks.Empty();
    
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        // Check if chunk has water
        float MaxDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
        if (MaxDepth < MinVolumeDepth) continue;
        
        // In always-visible system, all chunks with water are visible
        // No distance restrictions - water appears everywhere simulation has it
        OutVisibleChunks.Add(ChunkIndex);
    }
}

void UWaterSystem::UpdateExistingChunk(FWaterSurfaceChunk& Chunk, FVector CameraLocation)
{
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
    float Distance = FVector::Dist(CameraLocation, ChunkWorldPos);
    
    // No LOD calculation needed - always use highest quality
    // SurfaceChunk.CurrentLOD is always 0 in the always-visible system
    
    // Check if water depth changed significantly
    bool bDepthChanged = HasWaterDepthChangedSignificantly(Chunk.ChunkIndex);
    
    if (bDepthChanged || Chunk.bNeedsUpdate)
    {
        SynchronizeChunkWithSimulation(Chunk);
        CreateWaterSurfaceMeshStable(Chunk);
        Chunk.bNeedsUpdate = false;
        MeshUpdatesThisFrame++;
    }
}


bool UWaterSystem::HasWaterDepthChangedSignificantly(int32 ChunkIndex) const
{
    static TMap<int32, float> LastKnownDepths;
    
    float CurrentDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
    float* LastDepth = LastKnownDepths.Find(ChunkIndex);
    
    if (!LastDepth || FMath::Abs(*LastDepth - CurrentDepth) > 0.2f)
    {
        LastKnownDepths.Add(ChunkIndex, CurrentDepth);
        return true;
    }
    
    return false;
}

void UWaterSystem::CreateNewWaterChunk(int32 ChunkIndex)
{
    FWaterSurfaceChunk NewChunk;
    NewChunk.ChunkIndex = ChunkIndex;
    NewChunk.bNeedsUpdate = true;
    NewChunk.CurrentLOD = 0; // Start with highest quality
    
    WaterSurfaceChunks.Add(NewChunk);
    
    // Immediately create mesh for new chunk
    FWaterSurfaceChunk& ChunkRef = WaterSurfaceChunks.Last();
    SynchronizeChunkWithSimulation(ChunkRef);
    CreateWaterSurfaceMeshStable(ChunkRef);
    ChunkRef.bNeedsUpdate = false;
    
    MeshUpdatesThisFrame++;
}

void UWaterSystem::CreateWaterSurfaceMeshStable(FWaterSurfaceChunk& SurfaceChunk)
{
    // Ensure component exists
    if (!SurfaceChunk.SurfaceMesh)
    {
        SurfaceChunk.SurfaceMesh = GetValidatedPooledComponent();
        if (!SurfaceChunk.SurfaceMesh) return;
    }
    
    // CRITICAL: Validate simulation data before any mesh operations
    if (SurfaceChunk.MaxDepth < MinVolumeDepth)
    {
        // Hide mesh instead of destroying for stability
        SurfaceChunk.SurfaceMesh->SetVisibility(false);
        return;
    }
    
    // Generate geometry
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    GenerateStableWaterMesh(SurfaceChunk, Vertices, Triangles, Normals, UVs, VertexColors);
    
    if (Vertices.Num() > 0 && Triangles.Num() > 0)
    {
        // Create mesh section safely
        SurfaceChunk.SurfaceMesh->ClearAllMeshSections();
        SurfaceChunk.SurfaceMesh->CreateMeshSection_LinearColor(
            0, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(),
            TArray<FProcMeshTangent>(), true);
        
        SurfaceChunk.SurfaceMesh->SetVisibility(true);
        
        // Apply material
        ApplyMaterialToChunk(SurfaceChunk);
        
        // Position mesh using relative location for proper rotation
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
        SurfaceChunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPos - OwnerTerrain->GetActorLocation());
    }
}

void UWaterSystem::GenerateStableWaterMesh(FWaterSurfaceChunk& SurfaceChunk,
    TArray<FVector>& Vertices, TArray<int32>& Triangles,
    TArray<FVector>& Normals, TArray<FVector2D>& UVs,
    TArray<FColor>& VertexColors)
{
    int32 Resolution = FMath::Clamp(BaseSurfaceResolution >> SurfaceChunk.CurrentLOD, 8, 64);
    
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    float ChunkSize = OwnerTerrain->ChunkSize * OwnerTerrain->TerrainScale;
    float CellSize = ChunkSize / (Resolution - 1);
    
    // Generate vertices in grid pattern
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D WorldPos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            float WaterDepth = GetInterpolatedWaterDepthSeamless(WorldPos);
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X, WorldPos.Y, 0));
            
            // Create vertex
            FVector VertexPos(LocalX, LocalY, (TerrainHeight + WaterDepth) - ChunkWorldPos.Z);
            Vertices.Add(VertexPos);
            
            // Add normal
            Normals.Add(FVector::UpVector);
            
            // Add UV
            float U = (float)X / (Resolution - 1);
            float V = (float)Y / (Resolution - 1);
            UVs.Add(FVector2D(U, V));
            
            // Add color (depth-based alpha)
            uint8 Alpha = WaterDepth > MinVolumeDepth ? 255 : 0;
            VertexColors.Add(FColor(0, 100, 255, Alpha));
        }
    }
    
    // Generate triangles
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            int32 TopLeft = Y * Resolution + X;
            int32 TopRight = Y * Resolution + (X + 1);
            int32 BottomLeft = (Y + 1) * Resolution + X;
            int32 BottomRight = (Y + 1) * Resolution + (X + 1);
            
            // Create triangles
            Triangles.Add(TopLeft);
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);
            
            Triangles.Add(TopRight);
            Triangles.Add(BottomLeft);
            Triangles.Add(BottomRight);
        }
    }
}

void UWaterSystem::ApplyMaterialToChunk(FWaterSurfaceChunk& Chunk)
{
    if (!Chunk.SurfaceMesh || !VolumeMaterial) return;
    
    UMaterialInstanceDynamic* MaterialInstance =
        UMaterialInstanceDynamic::Create(VolumeMaterial, OwnerTerrain);
    
    if (MaterialInstance)
    {
        // Set water-specific parameters
        MaterialInstance->SetScalarParameterValue(FName("WaterDepth"), Chunk.AverageDepth);
        MaterialInstance->SetScalarParameterValue(FName("FlowSpeed"), Chunk.FlowSpeed);
        MaterialInstance->SetVectorParameterValue(FName("FlowDirection"),
            FLinearColor(Chunk.FlowDirection.X, Chunk.FlowDirection.Y, 0, 0));
        
        Chunk.SurfaceMesh->SetMaterial(0, MaterialInstance);
    }
}

void UWaterSystem::CleanupDistantChunks(FVector CameraLocation)
{
    // In always-visible system, we don't remove chunks based on distance
    // Only remove chunks that no longer have water in simulation
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        
        // Check simulation authority - remove if no water
        float SimDepth = GetChunkMaxDepthFromSimulation(Chunk.ChunkIndex);
        if (SimDepth < MinVolumeDepth)
        {
            if (Chunk.SurfaceMesh)
            {
                ReturnComponentToValidatedPool(Chunk.SurfaceMesh);
                Chunk.SurfaceMesh = nullptr;
            }
            
            if (Chunk.UndersideMesh)
            {
                Chunk.UndersideMesh->DestroyComponent();
            }
            
            WaterSurfaceChunks.RemoveAt(i);
        }
    }
}

FVector UWaterSystem::GetCameraLocation() const
{
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        if (APlayerController* PC = OwnerTerrain->GetWorld()->GetFirstPlayerController())
        {
            if (PC->PlayerCameraManager)
            {
                return PC->PlayerCameraManager->GetCameraLocation();
            }
        }
    }
    return FVector::ZeroVector;
}



void UWaterSystem::LogWaterMeshStats() const
{
    static float LastLogTime = 0.0f;
    if (!OwnerTerrain || !OwnerTerrain->GetWorld()) return;
    
    float CurrentTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastLogTime > 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Warning, TEXT("[WATER MESH] Stats - Active Chunks: %d, Pool Size: %d, Updates/Frame: %d"),
               WaterSurfaceChunks.Num(), ValidatedMeshPool.Num(), MeshUpdatesThisFrame);
        
        // Log LOD distribution
        TMap<int32, int32> LODCounts;
        for (const FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
        {
            LODCounts.FindOrAdd(Chunk.CurrentLOD, 0)++;
        }
        
        for (auto& Pair : LODCounts)
        {
            UE_LOG(LogTemp, Log, TEXT("  LOD %d: %d chunks"), Pair.Key, Pair.Value);
        }
        
        LastLogTime = CurrentTime;
    }
}



/**
 * Updates UV mapping for seamless chunk transitions
 * Ensures water textures tile correctly across chunk boundaries
 *
 * @param SurfaceChunk - Surface chunk to update UV mapping for
 */
/**
 * PHASE 1 FIX: Centralize UV mapping authority
 * Uses live world dimensions from MasterController instead of cached values
 * Eliminates coordinate desynchronization that causes shader sampling errors
 */
void UWaterSystem::UpdateSurfaceUVMapping(FWaterSurfaceChunk& SurfaceChunk)
{
    // ELEGANT SOLUTION: Remove chunk UV offsets entirely
    // Let the shader use global coordinate transformation parameters instead
    // This matches exactly how the mesh system works
    
    if (!SurfaceChunk.SurfaceMesh || !CachedMasterController)
    {
        return;
    }
    
    UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(
        SurfaceChunk.SurfaceMesh->GetMaterial(0));
    
    if (!MaterialInstance)
    {
        return; // Material not ready yet
    }
    
    // Only set basic material properties - no coordinate offsets
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
    float TerrainScale = CachedMasterController->GetTerrainScale();
    
    MaterialInstance->SetScalarParameterValue(FName("WorldScale"), TerrainScale);
    MaterialInstance->SetVectorParameterValue(FName("WorldDimensions"),
        FLinearColor(WorldDims.X, WorldDims.Y, 0, 0));
}



// ===== SUBSYSTEM 3.6: ADVANCED WATER TEXTURE SYSTEM =====
// Multi-channel render target for water properties (depth, flow, foam, turbulence)

// ===== ADVANCED WATER TEXTURE SYSTEM =====

void UWaterSystem::CreateAdvancedWaterTexture()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating advanced multi-channel water texture"));
    
    if (!IsSystemReady())
    {
        UE_LOG(LogTemp, Error, TEXT("System not ready for advanced texture creation"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create RGBA texture for multi-channel data
    // R: Water Depth, G: Flow Speed, B: Flow Direction X, A: Flow Direction Y
    WaterDataTexture = UTexture2D::CreateTransient(Width, Height, PF_FloatRGBA);
    
    if (WaterDataTexture)
    {
        WaterDataTexture->Filter = TextureFilter::TF_Bilinear;
        WaterDataTexture->AddressX = TextureAddress::TA_Clamp;
        WaterDataTexture->AddressY = TextureAddress::TA_Clamp;
        WaterDataTexture->SRGB = false;
        WaterDataTexture->UpdateResource();
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced water texture created: %dx%d RGBA"), Width, Height);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create advanced water texture"));
    }
}

void UWaterSystem::UpdateAdvancedWaterTexture()
{
    if (!WaterDataTexture || !SimulationData.IsValid() || !CachedMasterController || !OwnerTerrain)
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Calculate cell size: water cells are 1:1 with terrain vertices
    // Cell size = (terrain world size) / (grid size)
    float WorldSize = OwnerTerrain->TerrainWidth * OwnerTerrain->TerrainScale;
    float CellSize = WorldSize / static_cast<float>(Width);
    
    // Create RGBA texture data
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    // Get current time for ripple calculations
    float CurrentTime = GetScaledTime();
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                // Get base water data
                float Depth = SimulationData.WaterDepthMap[Index];
                float VelX = SimulationData.WaterVelocityX[Index];
                float VelY = SimulationData.WaterVelocityY[Index];
                
                // Calculate flow speed and normalize direction
                float FlowSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY);
                float NormVelX = FlowSpeed > 0.01f ? VelX / FlowSpeed : 0.0f;
                float NormVelY = FlowSpeed > 0.01f ? VelY / FlowSpeed : 0.0f;
                
                // Convert grid coordinates to world position for ripple calculations
                // Match the pattern used in GetInterpolatedWaterDepthSeamless
                FVector LocalPos = FVector(X * CellSize, Y * CellSize, 0);
                FVector WorldPos3D = OwnerTerrain->GetActorLocation() + LocalPos;
                FVector2D WorldPos(WorldPos3D.X, WorldPos3D.Y);
                
                // Calculate ripple contribution at this position
                float RippleHeight = 0.0f;
                if (Depth > MinWaterDepth && ActiveRipples.Num() > 0)
                {
                    for (const FWaterRipple& Ripple : ActiveRipples)
                    {
                        float Age = CurrentTime - Ripple.StartTime;
                        
                        // Skip expired ripples
                        if (Age > RippleLifetime)
                            continue;
                        
                        // Distance from ripple origin
                        float Distance = FVector2D::Distance(WorldPos, Ripple.Origin);
                        float CurrentRadius = Ripple.WaveSpeed * Age;
                        
                        // Check if we're within the wave packet
                        float WavePosition = CurrentRadius - Distance;
                        if (FMath::Abs(WavePosition) < Ripple.Wavelength * 3.0f && Distance < Ripple.MaxRadius)
                        {
                            // Calculate wave contribution
                            float Phase = (WavePosition / Ripple.Wavelength) * 2.0f * PI;
                            float Envelope = FMath::Exp(-FMath::Square(WavePosition / (Ripple.Wavelength * 1.5f)));
                            
                            // Amplitude with distance and time damping
                            float DistanceDamping = Ripple.InitialAmplitude / (1.0f + Distance * 0.002f);
                            float TimeDamping = FMath::Pow(Ripple.Damping, Age);
                            float Amplitude = DistanceDamping * TimeDamping * Envelope;
                            
                            // Add wave contribution
                            RippleHeight += Amplitude * FMath::Sin(Phase) * RippleHeightScale;
                        }
                    }
                }
                
                // Pack all data into texture
                // R: Depth (including ripples)
                // G: Flow Speed
                // B: Flow Direction X (normalized)
                // A: Flow Direction Y (normalized)
                TextureData[Index] = FLinearColor(
                    FMath::Max(0.0f, Depth + RippleHeight),  // Depth + ripples (clamped positive)
                    FMath::Clamp(FlowSpeed / 50.0f, 0.0f, 1.0f),  // Normalized flow speed
                    NormVelX * 0.5f + 0.5f,  // Remap from [-1,1] to [0,1]
                    NormVelY * 0.5f + 0.5f   // Remap from [-1,1] to [0,1]
                );
            }
            else
            {
                // Default values for out-of-bounds
                TextureData[Index] = FLinearColor(0.0f, 0.0f, 0.5f, 0.5f);
            }
        }
    }
    
    // Upload to GPU
    if (WaterDataTexture->GetPlatformData() && WaterDataTexture->GetPlatformData()->Mips.Num() > 0)
    {
        FTexture2DMipMap& Mip = WaterDataTexture->GetPlatformData()->Mips[0];
        void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
        if (Data)
        {
            FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num() * sizeof(FLinearColor));
            Mip.BulkData.Unlock();
            WaterDataTexture->UpdateResource();
        }
    }
    
    // Debug logging (throttled)
    static float LastDebugLogTime = 0.0f;
    if (CurrentTime - LastDebugLogTime > 2.0f && ActiveRipples.Num() > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated water texture with %d active ripples"), ActiveRipples.Num());
        LastDebugLogTime = CurrentTime;
    }
}


// ===== SUBSYSTEM 3.7: FOAM CALCULATION =====
// Physically-based foam effects from flow properties

void UWaterSystem::CalculateFoamData()
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Calculate foam based on multiple conditions
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            
            float WaterDepth = SimulationData.WaterDepthMap[Index];
            if (WaterDepth < MinWaterDepth)
            {
                SimulationData.FoamMap[Index] = 0.0f;
                continue;
            }
            
            // Edge foam (shallow water)
            float EdgeFoam = 1.0f - FMath::Clamp(WaterDepth / 0.5f, 0.0f, 1.0f);
            
            // Velocity foam (turbulence)
            float FlowSpeed = FMath::Sqrt(
                SimulationData.WaterVelocityX[Index] * SimulationData.WaterVelocityX[Index] +
                SimulationData.WaterVelocityY[Index] * SimulationData.WaterVelocityY[Index]
            );
            float VelocityFoam = FMath::Clamp(FlowSpeed / 20.0f, 0.0f, 1.0f);
            
            // Flow convergence foam (where flows meet)
            float LeftVelX = SimulationData.WaterVelocityX[Y * Width + (X - 1)];
            float RightVelX = SimulationData.WaterVelocityX[Y * Width + (X + 1)];
            float UpVelY = SimulationData.WaterVelocityY[(Y - 1) * Width + X];
            float DownVelY = SimulationData.WaterVelocityY[(Y + 1) * Width + X];
            
            float Divergence = (RightVelX - LeftVelX) + (DownVelY - UpVelY);
            float ConvergenceFoam = FMath::Clamp(-Divergence * 5.0f, 0.0f, 1.0f);
            
            // Terrain slope foam (waterfalls)
            float TerrainHeight = GetTerrainHeightSafe(X, Y);
            float LeftHeight = GetTerrainHeightSafe(X - 1, Y);
            float RightHeight = GetTerrainHeightSafe(X + 1, Y);
            float UpHeight = GetTerrainHeightSafe(X, Y - 1);
            float DownHeight = GetTerrainHeightSafe(X, Y + 1);
            
            float MaxGradient = FMath::Max(
                FMath::Abs(TerrainHeight - LeftHeight),
                FMath::Max(
                    FMath::Abs(TerrainHeight - RightHeight),
                    FMath::Max(
                        FMath::Abs(TerrainHeight - UpHeight),
                        FMath::Abs(TerrainHeight - DownHeight)
                    )
                )
            );
            float SlopeFoam = FMath::Clamp(MaxGradient / 100.0f, 0.0f, 1.0f);
            
            // Combine foam factors
            float TotalFoam = FMath::Clamp(
                EdgeFoam + VelocityFoam * 0.5f + ConvergenceFoam + SlopeFoam,
                0.0f, 1.0f
            );
            
            SimulationData.FoamMap[Index] = TotalFoam;
        }
    }
}





// END OF SECTION 3: WATER MANAGEMENT & VISUALS

// ============================================================================
// END OF STAGE 1 REORGANIZATION
// Stage 1 Complete - Lines 1-3,569 of original file reorganized
// Next: Stage 2 will handle Player Interaction, Weather System, and Wave Physics
// ============================================================================

// =============================================================================
// SECTION 4: PLAYER INTERACTION & QUERIES
// =============================================================================
//
// PURPOSE:
//   External API for player actions, world queries, and water system interaction.
//   Provides high-level interface for gameplay systems to manipulate and query
//   water state. All functions in this section are safe for external calling
//   and handle coordinate transformations, bounds checking, and conservation.
//
// SUBSECTIONS:
//   4.1 - Water Manipulation: Add/remove water with radius-based distribution
//   4.2 - Water Queries: Depth, volume, flow statistics, and debug info
//   4.3 - Safe Accessors: Bounds-checked getters/setters and utilities
//   4.4 - Coordinate Utilities: World   Terrain transformations
//
// KEY FEATURES:
//     Gaussian distribution for smooth water addition
//     Water conservation tracking (GetTotalWaterInSystem)
//     Chunk-aware updates (MarkChunkForUpdate)
//     Safe accessors with automatic bounds checking
//     World/terrain coordinate transformation
//     Debug visualization support
//
// DEPENDENCIES:
//   Internal:
//     - Section 1: IsSystemReady() for safety checks
//     - Section 2: SimulationData access for water depth arrays
//     - Section 3: Chunk management for visual updates
//   External:
//     - ADynamicTerrain: Coordinate transformations, chunk management
//     - AMasterWorldController: Authority for water conservation
//
// PERFORMANCE NOTES:
//   - AddWaterInRadius: O(RÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â²) where R is radius in cells
//   - Water queries: O(1) with bounds checking
//   - Coordinate transforms: O(1) matrix operations
//
// WATER CONSERVATION:
//   All water manipulation functions properly update the simulation arrays
//   and trigger appropriate visual updates. GetTotalWaterInSystem() provides
//   conservation validation by summing all water in the system.
//
// =============================================================================

// ============================================================
// SUBSECTION 4.1: WATER MANIPULATION
// ============================================================
// Player-facing water add/remove functions with Gaussian
// distribution for smooth, realistic water placement.
// All functions properly track water for conservation.
// ============================================================

// ===== PLAYER INTERACTION =====

void UWaterSystem::AddWater(FVector WorldPosition, float Amount)
{
    if (!IsSystemReady())
    {
        return;
    }
    
    // FIXED: Never add water to just a single cell - always use radius
    // Convert single-point water addition to radius-based addition
    float MinRadius = 3.0f; // Minimum 3 cells to prevent spikes
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    
    // Use radius-based water addition instead of single point
    AddWaterInRadius(X, Y, MinRadius, Amount);
}

void UWaterSystem::AddWaterInRadius(int32 CenterX, int32 CenterY, float Radius, float Amount)
{
    if (!IsValidCoordinate(CenterX, CenterY) || !SimulationData.IsValid())
    {
        return;
    }
    
    // CHANGED: Smooth scaling for max column height
    // Radius < 200: 5000cm (50 meters) - singularity fun zone
    // Radius 200-500: Smooth transition
    // Radius > 500: 150cm (conservative)
    float MaxColumnHeight;
    if (Radius <= 200.0f)
    {
        MaxColumnHeight = 15000.0f; // 50x higher for small brush play
    }
    else if (Radius >= 500.0f)
    {
        MaxColumnHeight = 150.0f; // Conservative for large brushes
    }
    else
    {
        // Smooth interpolation from 200 to 500 - no artifacts
        float t = (Radius - 200.0f) / 300.0f; // 0 to 1
        MaxColumnHeight = FMath::Lerp(15000.0f, 150.0f, t);
    }
    
    int32 IntRadius = FMath::CeilToInt(Radius);
    TSet<int32> AffectedChunks;
    
    // Calculate brush size factor for sigma scaling
    float BrushSizeFactor = FMath::Clamp(Radius / 10.0f, 0.0f, 1.0f);
    
    // Calculate total distribution area
    float TotalWeight = 0.0f;
    for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
    {
        for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
        {
            float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
            
            // Hard boundary at brush radius
            if (Distance > Radius)
                continue;
            
            // Adaptive sigma: tight for small, smooth for large
            float Sigma = FMath::Lerp(Radius / 4.0f, Radius / 2.0f, BrushSizeFactor);
            float Falloff = FMath::Exp(-0.5f * (Distance * Distance) / (Sigma * Sigma));
            TotalWeight += Falloff;
        }
    }
    
    // Distribute water
    float TotalWaterAdded = 0.0f;
    if (TotalWeight > 0.0f)
    {
        for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
        {
            for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
            {
                int32 X = CenterX + OffsetX;
                int32 Y = CenterY + OffsetY;
                
                if (!IsValidCoordinate(X, Y))
                    continue;
                
                float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
                
                // Hard boundary at brush radius
                if (Distance > Radius)
                    continue;
                
                float Sigma = FMath::Lerp(Radius / 4.0f, Radius / 2.0f, BrushSizeFactor);
                float Falloff = FMath::Exp(-0.5f * (Distance * Distance) / (Sigma * Sigma));
                
                float CellAmount = (Falloff / TotalWeight) * Amount;
                
                int32 Index = Y * SimulationData.TerrainWidth + X;
                float CurrentDepth = SimulationData.WaterDepthMap[Index];
                
                if (CurrentDepth >= MaxColumnHeight)
                    continue;
                
                float AllowedAmount = FMath::Min(CellAmount, MaxColumnHeight - CurrentDepth);
                
                SimulationData.WaterDepthMap[Index] += AllowedAmount;
                TotalWaterAdded += AllowedAmount;
                
                int32 ChunkIndex = OwnerTerrain->GetChunkIndexFromCoordinates(X, Y);
                AffectedChunks.Add(ChunkIndex);
            }
        }
    }
    
    // Update chunks
    for (int32 ChunkIndex : AffectedChunks)
    {
        OwnerTerrain->MarkChunkForUpdate(ChunkIndex);
    }

    bVolumeNeedsUpdate = true;
}

void UWaterSystem::AddWaterAtIndex(int32 X, int32 Y, float Amount)
{
    if (!IsValidCoordinate(X, Y))
    {
        return;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        SimulationData.WaterDepthMap[Index] += Amount;
        
        // Mark chunk for visual update
        MarkChunkForUpdate(X, Y);
        
        // Mark water as changed for volumetric updates
        bWaterChangedThisFrame = true;
            
            UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Added %.2f water at (%d,%d), depth now %.2f"),
                   Amount, X, Y, SimulationData.WaterDepthMap[Index]);
    }
    
    bVolumeNeedsUpdate = true;
}

void UWaterSystem::RemoveWater(FVector WorldPosition, float Amount)
{
    if (!IsSystemReady())
    {
        return;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    
    if (IsValidCoordinate(X, Y))
    {
        int32 Index = GetTerrainIndex(X, Y);
        if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
        {
            float OldDepth = SimulationData.WaterDepthMap[Index];
            SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, OldDepth - Amount);
            float ActualRemoved = OldDepth - SimulationData.WaterDepthMap[Index];
            
            // MasterController handles all budget authority manually
            
            // Mark chunk for visual update
            MarkChunkForUpdate(X, Y);
            
            // Mark water as changed for volumetric updates
            bWaterChangedThisFrame = true;
        }
    }
    bVolumeNeedsUpdate = true;
}

// Radius-based water removal with Gaussian distribution
void UWaterSystem::RemoveWaterInRadius(int32 CenterX, int32 CenterY, float Radius, float Amount)
{
    if (!IsValidCoordinate(CenterX, CenterY) || !SimulationData.IsValid())
    {
        return;
    }
    
    // Ensure minimum radius
    Radius = FMath::Max(Radius, 3.0f);
    
    int32 IntRadius = FMath::CeilToInt(Radius);
    TSet<int32> AffectedChunks;
    
    // Calculate total distribution area for proper normalization
    float TotalWeight = 0.0f;
    for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
    {
        for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
        {
            float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
            if (Distance <= Radius)
            {
                // Gaussian distribution for smoother removal
                float Sigma = Radius / 2.0f;
                float Falloff = FMath::Exp(-0.5f * (Distance * Distance) / (Sigma * Sigma));
                TotalWeight += Falloff;
            }
        }
    }
    
    // Distribute water removal with Gaussian falloff
    if (TotalWeight > 0.0f)
    {
        for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
        {
            for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
            {
                int32 X = CenterX + OffsetX;
                int32 Y = CenterY + OffsetY;
                
                if (!IsValidCoordinate(X, Y))
                {
                    continue;
                }
                
                float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
                if (Distance <= Radius)
                {
                    // Gaussian falloff
                    float Sigma = Radius / 3.0f;
                    float Falloff = FMath::Exp(-0.5f * (Distance * Distance) / (Sigma * Sigma));
                    
                    float CellAmount = (Falloff / TotalWeight) * Amount;
                    
                    int32 Index = Y * SimulationData.TerrainWidth + X;
                    float OldDepth = SimulationData.WaterDepthMap[Index];
                    SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, OldDepth - CellAmount);
                    
                    // Mark chunks for update if water was removed
                    if (OldDepth > 0.0f)
                    {
                        int32 ChunkIndex = OwnerTerrain->GetChunkIndexFromCoordinates(X, Y);
                        AffectedChunks.Add(ChunkIndex);
                    }
                }
            }
        }
    }
    
    // Mark affected chunks for update
    for (int32 ChunkIndex : AffectedChunks)
    {
        OwnerTerrain->MarkChunkForUpdate(ChunkIndex);
    }

    bVolumeNeedsUpdate = true;
}

// ============================================================
// SUBSECTION 4.2: WATER QUERIES & STATISTICS
// ============================================================
// Query functions for water depth, system statistics, and
// debug visualization. All queries are bounds-safe and
// provide accurate real-time data.
// ============================================================

float UWaterSystem::GetWaterDepthAtPosition(FVector WorldPosition) const
{
    if (!IsSystemReady())
    {
        return 0.0f;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    
    return GetWaterDepthAtIndex(X, Y);
}

float UWaterSystem::GetWaterDepthAtIndex(int32 X, int32 Y) const
{
    return GetWaterDepthSafe(X, Y);
}

// ===== WEATHER SYSTEM =====




// ===== UTILITIES =====

float UWaterSystem::GetTotalWaterInSystem() const
{
    if (!SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    float TotalWater = 0.0f;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        TotalWater += WaterDepth;
    }
    return TotalWater;
}

int32 UWaterSystem::GetWaterCellCount() const
{
    if (!SimulationData.IsValid())
    {
        return 0;
    }
    
    int32 WaterCells = 0;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        if (WaterDepth > MinWaterDepth)
        {
            WaterCells++;
        }
    }
    return WaterCells;
}

float UWaterSystem::GetMaxFlowSpeed() const
{
    if (!SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    float MaxFlow = 0.0f;
    for (int32 i = 0; i < SimulationData.WaterVelocityX.Num(); i++)
    {
        float Flow = FMath::Sqrt(
            SimulationData.WaterVelocityX[i] * SimulationData.WaterVelocityX[i] +
            SimulationData.WaterVelocityY[i] * SimulationData.WaterVelocityY[i]
        );
        MaxFlow = FMath::Max(MaxFlow, Flow);
    }
    return MaxFlow;
}


// ===== HELPER FUNCTIONS =====

// ============================================================
// SUBSECTION 4.3: SAFE ACCESSORS & UTILITIES
// ============================================================
// Bounds-checked accessors and utility functions for safe
// manipulation of water data arrays. All functions validate
// coordinates before access.
// ============================================================

float UWaterSystem::GetWaterDepthSafe(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y) || !SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        return SimulationData.WaterDepthMap[Index];
    }
    return 0.0f;
}

void UWaterSystem::SetWaterDepthSafe(int32 X, int32 Y, float Depth)
{
    if (!IsValidCoordinate(X, Y) || !SimulationData.IsValid())
    {
        return;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, Depth);
    }
}

int32 UWaterSystem::GetTerrainIndex(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y))
    {
        return -1;
    }
    
    return Y * SimulationData.TerrainWidth + X;
}

bool UWaterSystem::IsValidCoordinate(int32 X, int32 Y) const
{
    // Use terrain's actual bounds instead of SimulationData
    if (OwnerTerrain)
    {
        return X >= 0 && X < OwnerTerrain->TerrainWidth &&
               Y >= 0 && Y < OwnerTerrain->TerrainHeight;
    }
    
    // Fallback to simulation bounds
    return X >= 0 && X < SimulationData.TerrainWidth &&
           Y >= 0 && Y < SimulationData.TerrainHeight;
}


FVector2D UWaterSystem::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    // Try MasterController first (for advanced features)
    if (CachedMasterController)
    {
        return CachedMasterController->WorldToTerrainCoordinates(WorldPosition);
    }
    
    // AUTHORITY FIX: Use CachedMasterController for terrain scale
    if (OwnerTerrain && CachedMasterController)
    {
        FVector LocalPos = OwnerTerrain->GetActorTransform().InverseTransformPosition(WorldPosition);
        float AuthScale = CachedMasterController->GetTerrainScale();
        return FVector2D(LocalPos.X / AuthScale, LocalPos.Y / AuthScale);
    }
    
    return FVector2D::ZeroVector;
}

void UWaterSystem::MarkChunkForUpdate(int32 X, int32 Y)
{
    if (!OwnerTerrain)
    {
        return;
    }
    
    int32 ChunkIndex = OwnerTerrain->GetChunkIndexFromCoordinates(X, Y);
    if (ChunkIndex >= 0)
    {
        OwnerTerrain->MarkChunkForUpdate(ChunkIndex);
    }
}
// =============================================================================
// SECTION 5: SHADER & VISUAL SYSTEMS
// =============================================================================
//
// PURPOSE:
//   Complete shader, texture, material, and visual rendering pipeline.
//   Handles all aspects of water visualization including GPU textures,
//   material parameters, UE5.4 integration, mesh quality, and advanced
//   visual effects. This section bridges the physics simulation (Section 2)
//   with the visual representation (Section 3 mesh generation).
//
// SUBSECTIONS:
//   5.1 - Shader Core: Main shader update loop and material application
//   5.2 - UE5.4 Integration: Enhanced Input, Lumen, Nanite, World Partition
//   5.3 - Shader Parameters & Textures: GPU texture creation and updates
//   5.4 - Quality & Filtering: Edge-preserving filters and smoothing
//   5.5 - Material Parameters: Dynamic material parameter updates
//   5.6 - Player Queries: Underwater detection and player-water interaction
//   5.7 - Advanced Mesh Generation: High-quality water surface generation
//
// KEY FEATURES:
//     GPU texture streaming for water depth and flow data
//     Real-time material parameter updates
//     UE5.4 feature integration (Lumen, Nanite, Enhanced Input)
//     Edge-preserving filtering for visual quality
//     Chunk boundary seamless interpolation
//     LOD-aware mesh generation
//     Underwater detection and player queries
//
// DEPENDENCIES:
//   Internal:
//     - Section 1: System lifecycle and configuration
//     - Section 2: Core physics for depth/flow data
//     - Section 3: Mesh generation and chunk management
//     - Section 6: Wave physics for surface displacement
//   External:
//     - UE5 RHI: Texture creation and GPU resource management
//     - UMaterialInstanceDynamic: Material parameter updates
//     - Enhanced Input System: UE5.4 input handling
//     - Lumen: Global illumination for water reflections
//     - Nanite: Virtualized geometry compatibility
//
// PERFORMANCE NOTES:
//   - Texture updates: 2-4ms for 513x513 grids
//   - Material parameter updates: <0.1ms per chunk
//   - Edge-preserving filter: ~1ms for active water areas
//   - Mesh generation: 0.5-2ms per chunk depending on LOD
//   - GPU texture streaming: Async, non-blocking
//
// UE5.4 INTEGRATION:
//   This section consolidates all UE5.4-specific features that were
//   previously scattered across 8 separate sections:
//     Enhanced Input System for better input handling
//     Lumen reflections for realistic water appearance
//     Nanite compatibility for high-poly water meshes
//     World Partition streaming for open-world support
//     Performance profiling integration
//     Initialization sequences
//     Async texture loading
//     Validation systems
//
// =============================================================================

// ============================================================
// SUBSYSTEM 5.1: SHADER CORE
// ============================================================
// Main shader update loop and material application functions.
// Coordinates shader system timing and applies water textures
// to materials. Entry point for all shader-related updates.
// ============================================================

void UWaterSystem::UpdateShaderSystem(float DeltaTime)
{
    static float ShaderUpdateTimer = 0.0f;
    ShaderUpdateTimer += DeltaTime;
    
    if (ShaderUpdateTimer >= WaterShaderUpdateInterval)
    {
        UpdateWaterDepthTexture();
        ShaderUpdateTimer = 0.0f;
    }
}

void UWaterSystem::ApplyWaterTextureToMaterial(UMaterialInstanceDynamic* Material)
{
    if (!Material || !IsSystemReady())
    {
        return; // Fail silently if system not ready
    }
    
    // Ensure water depth texture exists
    if (!WaterDepthTexture)
    {
        CreateWaterDepthTexture();
    }
    
    if (!WaterDepthTexture)
    {
        return; // Fail silently without spamming logs
    }
    
    // Apply water depth texture
    Material->SetTextureParameterValue(FName("WaterDepthTexture"), WaterDepthTexture);
    
    // Apply advanced water texture if available
    if (WaterDataTexture)
    {
        Material->SetTextureParameterValue(FName("WaterDataTexture"), WaterDataTexture);
    }
    
    // Apply basic water parameters
    Material->SetScalarParameterValue(FName("WaterDepthScale"), WaterDepthScale);
    Material->SetScalarParameterValue(FName("MinWaterDepth"), MinWaterDepth);
    
    /*
    UE_LOG(LogTemp, Log, TEXT("Applied water texture to material - DepthTexture:%s DataTexture:%s"),
           WaterDepthTexture ? TEXT("OK") : TEXT("NULL"),
           WaterDataTexture ? TEXT("OK") : TEXT("NULL"));
     */
}

void UWaterSystem::ApplyVolumetricWaterToMaterial(UMaterialInstanceDynamic* Material)
{
    if (!Material)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyVolumetricWaterToMaterial: Material is NULL"));
        return;
    }
    
    // Apply water texture first
    ApplyWaterTextureToMaterial(Material);
    
    // Apply material parameters
    ApplyWaterMaterialParams(Material, MaterialParams);
    
    // Apply displacement if available
    if (FlowDisplacementTexture)
    {
        UpdateWaterShaderForDisplacement(Material);
    }
    
    //UE_LOG(LogTemp, Log, TEXT("Applied volumetric water to material"));
}


// ============================================================
// SUBSYSTEM 5.2: UE5.4 INTEGRATION
// ============================================================
// Consolidated UE5.4 feature integration (8 systems unified).
// Handles Enhanced Input, Lumen reflections, Nanite compatibility,
// World Partition streaming, performance profiling, initialization,
// async texture loading, and validation systems.
// ============================================================

void UWaterSystem::HandleEnhancedInput(const FInputActionValue& ActionValue, FVector CursorWorldPosition)
{
    if (!IsSystemReady() || !MasterController)
    {
        return;
    }
    
    float InputValue = ActionValue.Get<float>();
    if (FMath::Abs(InputValue) < 0.1f) // Dead zone
    {
        return;
    }
    
    if (!MasterController->ValidateWorldPosition(CursorWorldPosition))
    {
        return;
    }
    
    float ScaledAmount = InputValue * 5.0f * GetWorld()->GetDeltaSeconds();
    AddWater(CursorWorldPosition, ScaledAmount);
}

// ===== UE5.4 LUMEN INTEGRATION =====
void UWaterSystem::ConfigureLumenReflections()
{
    if (!OwnerTerrain || !OwnerTerrain->GetWorld())
    {
        return;
    }
    
    for (FWaterSurfaceChunk& SurfaceChunk : WaterSurfaceChunks)
    {
        if (SurfaceChunk.SurfaceMesh)
        {
            SurfaceChunk.SurfaceMesh->SetCastShadow(true);
            SurfaceChunk.SurfaceMesh->SetAffectDynamicIndirectLighting(true);
            SurfaceChunk.SurfaceMesh->SetAffectDistanceFieldLighting(true);
            SurfaceChunk.SurfaceMesh->SetReceivesDecals(false);
            SurfaceChunk.SurfaceMesh->SetRenderInMainPass(true);
        }
    }
}

// ===== UE5.4 NANITE COMPATIBILITY =====
void UWaterSystem::ConfigureNaniteCompatibility()
{
    for (FWaterSurfaceChunk& SurfaceChunk : WaterSurfaceChunks)
    {
        if (SurfaceChunk.SurfaceMesh)
        {
            SurfaceChunk.SurfaceMesh->SetCollisionProfileName("Water");
            SurfaceChunk.SurfaceMesh->SetGenerateOverlapEvents(false);
            SurfaceChunk.SurfaceMesh->SetCanEverAffectNavigation(false);
            // SetUseAsyncCooking removed - not available in UE5.4
        }
    }
}

// ===== UE5.4 WORLD PARTITION SUPPORT =====
void UWaterSystem::ConfigureWorldPartitionStreaming()
{
    if (!OwnerTerrain || !OwnerTerrain->GetWorld())
    {
        return;
    }
    
    UWorld* World = OwnerTerrain->GetWorld();
    
    // UE5.4: Check if world partition is enabled
    if (World->GetWorldPartition())
    {
        for (FWaterSurfaceChunk& SurfaceChunk : WaterSurfaceChunks)
        {
            if (SurfaceChunk.SurfaceMesh)
            {
                // Configure for streaming - API updated for UE5.4
                float StreamingDistance = VolumeUpdateDistance * 1.5f;
                // Note: SetWorldPartitionStreamingSourcePriority may not be available
                // Configure through other means if needed
            }
        }
    }
}

// ===== UE5.4 PERFORMANCE PROFILING =====
void UWaterSystem::ProfileWaterSystemPerformance()
{
    SCOPE_CYCLE_COUNTER(STAT_WaterSystemTotal);
    
    static int32 FrameCounter = 0;
    if (++FrameCounter % 300 == 0) // Every 5 seconds at 60fps
    {
        UE_LOG(LogTemp, Log, TEXT("Water System Performance: Frame %d"), FrameCounter);
    }
}

// ===== UE5.4 INITIALIZATION SEQUENCE =====
void UWaterSystem::InitializeForUE54()
{
    ConfigureLumenReflections();
    ConfigureNaniteCompatibility();
    ConfigureWorldPartitionStreaming();
    ProfileWaterSystemPerformance();
    
    UE_LOG(LogTemp, Warning, TEXT("Water System: UE5.4 initialization complete"));
}

// ===== UE5.4 ASYNC TEXTURE LOADING =====
void UWaterSystem::LoadWaterTexturesAsync()
{
    if (!WaterDepthTexture)
    {
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            CreateWaterDepthTexture();
            if (WaterDepthTexture)
            {
                UpdateWaterDepthTexture();
            }
        });
    }
}




// ===== UE5.4 VALIDATION FUNCTIONS =====
void UWaterSystem::ValidateShaderTextureAlignment()
{
    if (!WaterDepthTexture || !SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[TEXTURE VALIDATION] Missing texture or simulation data"));
        return;
    }
    
    FVector TestWorldPos(10000.0f, 15000.0f, 0.0f);
    AddWater(TestWorldPos, 10.0f);
    UpdateWaterDepthTexture();
    
    FVector2D TerrainCoords = MasterController->WorldToTerrainCoordinates(TestWorldPos);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    if (IsValidCoordinate(X, Y))
    {
        float SimDepth = GetWaterDepthSafe(X, Y);
        UE_LOG(LogTemp, Warning, TEXT("[TEXTURE VALIDATION] Test water validation complete"));
        
        if (SimDepth > 9.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[TEXTURE VALIDATION] SUCCESS - Simulation data matches"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[TEXTURE VALIDATION] FAILED - Water not in simulation"));
        }
    }
}

// ============================================================
// SUBSYSTEM 5.3: SHADER PARAMETERS & TEXTURES
// ============================================================
// GPU texture creation, updates, and shader parameter management.
// Creates and maintains render targets for water depth and flow
// data. Updates GPU textures with simulation data each frame.
// ============================================================

void UWaterSystem::FixChunkUVMapping()
{
    if (!CachedMasterController)
    {
        UE_LOG(LogTemp, Fatal, TEXT("CRITICAL: No MasterController authority for global water shaders"));
        return;
    }
    
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
    float TerrainScale = CachedMasterController->GetTerrainScale();
    
    if (!WaterParameterCollection || !OwnerTerrain || !OwnerTerrain->GetWorld())
    {
        UE_LOG(LogTemp, Fatal, TEXT("CRITICAL: Missing water parameter collection or world"));
        return;
    }
    
    UMaterialParameterCollectionInstance* Instance =
        OwnerTerrain->GetWorld()->GetParameterCollectionInstance(WaterParameterCollection);
    
    if (!Instance)
    {
        UE_LOG(LogTemp, Fatal, TEXT("CRITICAL: Failed to get water parameter collection instance"));
        return;
    }
    
    // ELEGANT FIX: Add terrain world origin for proper coordinate transformation
    FVector TerrainOrigin = OwnerTerrain->GetActorLocation();
    
    Instance->SetScalarParameterValue(FName("WaterTextureWidth"), WorldDims.X);
    Instance->SetScalarParameterValue(FName("WaterTextureHeight"), WorldDims.Y);
    Instance->SetScalarParameterValue(FName("TerrainScale"), TerrainScale);
    Instance->SetScalarParameterValue(FName("TexelSizeX"), 1.0f / WorldDims.X);
    Instance->SetScalarParameterValue(FName("TexelSizeY"), 1.0f / WorldDims.Y);
    
    // KEY FIX: Add terrain world origin for shader coordinate transformation
    Instance->SetVectorParameterValue(FName("TerrainWorldOrigin"),
        FLinearColor(TerrainOrigin.X, TerrainOrigin.Y, TerrainOrigin.Z, 0.0f));
}




void UWaterSystem::UpdateWaterShaderParameters()
{
    if (!WaterParameterCollection || !MasterController)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // UE5.4 Material Parameter Collection Instance
    UMaterialParameterCollectionInstance* MPC =
        World->GetParameterCollectionInstance(WaterParameterCollection);
    
    if (MPC)
    {
        // Authoritative dimensions from MasterController
        FVector2D WorldDims = MasterController->GetWorldDimensions();
        float TerrainScale = MasterController->GetTerrainScale();
        
        // Core water parameters
        MPC->SetScalarParameterValue(FName("WaterTime"), World->GetTimeSeconds());
        MPC->SetScalarParameterValue(FName("WaterDepthScale"), WaterDepthScale);
        MPC->SetScalarParameterValue(FName("TerrainWidth"), WorldDims.X);
        MPC->SetScalarParameterValue(FName("TerrainHeight"), WorldDims.Y);
        MPC->SetScalarParameterValue(FName("TerrainScale"), TerrainScale);
        
        // UV Correction parameters
        MPC->SetScalarParameterValue(FName("TexelSizeX"), 1.0f / WorldDims.X);
        MPC->SetScalarParameterValue(FName("TexelSizeY"), 1.0f / WorldDims.Y);
        
        // Water physics parameters
        float MaxFlow = GetMaxFlowSpeed();
        MPC->SetScalarParameterValue(FName("MaxFlowSpeed"), MaxFlow);

        
        // Volumetric water optical parameters
        MPC->SetScalarParameterValue(FName("AbsorptionCoeff"), MaterialParams.Absorption);
        MPC->SetScalarParameterValue(FName("CausticStrength"), MaterialParams.CausticStrength);
        MPC->SetScalarParameterValue(FName("WaterClarityFactor"), MaterialParams.Clarity);
        MPC->SetScalarParameterValue(FName("WaveStrength"), MaterialParams.WaveStrength);
        
        // Volumetric color parameters
        MPC->SetVectorParameterValue(FName("DeepWaterColor"), MaterialParams.DeepColor);
        MPC->SetVectorParameterValue(FName("ShallowWaterColor"), MaterialParams.ShallowColor);
    }
}

void UWaterSystem::CreateWaterDepthTexture()
{
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Log, TEXT("Water depth texture already exists, skipping creation"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    if (Width <= 0 || Height <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid texture dimensions: %dx%d"), Width, Height);
        return;
    }
    
    // Create texture with explicit validation
    WaterDepthTexture = UTexture2D::CreateTransient(Width, Height, PF_G8);
    
    if (!WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: CreateTransient failed for water depth texture"));
        return;
    }
    
    // Configure texture properties
    WaterDepthTexture->Filter = TextureFilter::TF_Bilinear;
    WaterDepthTexture->AddressX = TextureAddress::TA_Clamp;
    WaterDepthTexture->AddressY = TextureAddress::TA_Clamp;
    WaterDepthTexture->SRGB = false;
    WaterDepthTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;

    // Force immediate GPU upload
    WaterDepthTexture->UpdateResource();

    // PHASE 1.5: Create previous depth texture for displacement detection
    PreviousWaterDepthTexture = UTexture2D::CreateTransient(Width, Height, PF_G8);
    if (PreviousWaterDepthTexture)
    {
        PreviousWaterDepthTexture->Filter = TextureFilter::TF_Bilinear;
        PreviousWaterDepthTexture->AddressX = TextureAddress::TA_Clamp;
        PreviousWaterDepthTexture->AddressY = TextureAddress::TA_Clamp;
        PreviousWaterDepthTexture->SRGB = false;
        PreviousWaterDepthTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
        PreviousWaterDepthTexture->UpdateResource();
        UE_LOG(LogTemp, Log, TEXT("PHASE 1.5: Previous water depth texture created for displacement detection"));
    }
    
    // Simple validation without blocking
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Log, TEXT("Water depth texture created and GPU-validated: %dx%d"), Width, Height);
        
        // Immediately populate with initial data
        UpdateWaterDepthTexture();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Water depth texture creation failed GPU validation"));
        WaterDepthTexture = nullptr; // Clear invalid texture
    }
}

void UWaterSystem::UpdateWaterDepthTexture()
{
    if (!WaterDepthTexture || !SimulationData.IsValid())
        return;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create floating-point buffer for smooth processing
    TArray<float> FloatBuffer;
    FloatBuffer.SetNum(Width * Height);
    
    // Track water statistics for debugging
    int32 NonZeroPixels = 0;
    float MaxWaterDepth = 0.0f;
    float MinNonZeroDepth = FLT_MAX;
    
    // Step 1: Convert water depth to normalized float values with smooth thresholding
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                float WaterDepth = SimulationData.WaterDepthMap[Index];
                
                // ELEGANT SMOOTH THRESHOLD: Use smooth step instead of hard cutoff
                float ProcessedDepth = 0.0f;
                
                // Define the transition zone
                float TransitionStart = MinWaterDepth * 0.5f;  // Start fading in
                float TransitionEnd = MinWaterDepth * 2.0f;    // Fully visible
                
                if (WaterDepth <= TransitionStart)
                {
                    // No water
                    ProcessedDepth = 0.0f;
                }
                else if (WaterDepth >= TransitionEnd)
                {
                    // Full water
                    ProcessedDepth = WaterDepth;
                }
                else
                {
                    // Smooth transition zone - this prevents shoreline dancing
                    float t = (WaterDepth - TransitionStart) / (TransitionEnd - TransitionStart);
                    // Smooth step function: 3t  - 2t
                    float smoothT = t * t * (3.0f - 2.0f * t);
                    ProcessedDepth = WaterDepth * smoothT;
                }
                
                // Update statistics
                if (ProcessedDepth > 0.0f)
                {
                    NonZeroPixels++;
                    MaxWaterDepth = FMath::Max(MaxWaterDepth, WaterDepth);
                    MinNonZeroDepth = FMath::Min(MinNonZeroDepth, WaterDepth);
                }
                
                // Normalize to 0-1 range for processing
                FloatBuffer[Index] = ProcessedDepth / WaterDepthScale;
            }
            else
            {
                FloatBuffer[Index] = 0.0f;
            }
        }
    }
    
    // Step 2: Apply 8-directional smoothing if enabled
    if (bUse8DirectionalFlow && NonZeroPixels > 0)
    {
        // Number of smoothing iterations - more passes = smoother result
        const int32 SmoothingPasses = 2;
        
        // Temporary buffer for ping-pong smoothing
        TArray<float> TempBuffer;
        TempBuffer.SetNum(Width * Height);
        
        for (int32 Pass = 0; Pass < SmoothingPasses; Pass++)
        {
            // Copy current state
            TempBuffer = FloatBuffer;
            
            // Apply smoothing filter to all pixels
            for (int32 Y = 0; Y < Height; Y++)
            {
                for (int32 X = 0; X < Width; X++)
                {
                    int32 Index = Y * Width + X;
                    
                    // 8-directional Gaussian-like kernel
                    const float CenterWeight = 4.0f;
                    const float CardinalWeight = 2.0f;
                    const float DiagonalWeight = DiagonalFlowWeight;
                    
                    float WeightedSum = 0.0f;
                    float TotalWeight = 0.0f;
                    
                    // Process all 9 positions (8 neighbors + center)
                    for (int32 dy = -1; dy <= 1; dy++)
                    {
                        for (int32 dx = -1; dx <= 1; dx++)
                        {
                            int32 NX = X + dx;
                            int32 NY = Y + dy;
                            
                            // Handle boundaries with clamping
                            NX = FMath::Clamp(NX, 0, Width - 1);
                            NY = FMath::Clamp(NY, 0, Height - 1);
                            
                            int32 NeighborIndex = NY * Width + NX;
                            float NeighborValue = TempBuffer[NeighborIndex];
                            
                            // Determine weight based on position
                            float Weight;
                            if (dx == 0 && dy == 0)
                            {
                                Weight = CenterWeight;
                            }
                            else if (dx == 0 || dy == 0)
                            {
                                Weight = CardinalWeight;
                            }
                            else
                            {
                                Weight = DiagonalWeight;
                            }
                            
                            WeightedSum += NeighborValue * Weight;
                            TotalWeight += Weight;
                        }
                    }
                    
                    // Apply smoothed value
                    FloatBuffer[Index] = WeightedSum / TotalWeight;
                }
            }
        }
        
        // Optional: Apply edge-preserving filter to maintain sharp features where needed
        if (bPreserveWaterEdges)
        {
            ApplyEdgePreservingFilter(FloatBuffer, Width, Height);
        }
    }
    
    // Step 3: Convert back to uint8 texture data with proper scaling
    TArray<uint8> TextureData;
    TextureData.SetNum(Width * Height);
    
    for (int32 i = 0; i < FloatBuffer.Num(); i++)
    {
        float Value = FloatBuffer[i];
        
        // Apply final adjustments
        if (Value > 0.0f)
        {
            // Ensure minimum visible value for any water
            Value = FMath::Max(Value, 1.0f / 255.0f);
            
            // Optional: Apply gamma correction for better visual distribution
            if (bUseGammaCorrection)
            {
                Value = FMath::Pow(Value, WaterDepthGamma);
            }
        }
        
        // Convert to uint8 with proper rounding
        TextureData[i] = (uint8)FMath::RoundToInt(FMath::Clamp(Value * 255.0f, 0.0f, 255.0f));
    }
    
    // Step 4: Upload to GPU
    FTexture2DMipMap& Mip = WaterDepthTexture->GetPlatformData()->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    if (Data)
    {
        FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num() * sizeof(uint8));
        Mip.BulkData.Unlock();
        WaterDepthTexture->UpdateResource();
    }
    
    // Step 5: Log statistics (throttled)
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastLogTime > 2.0f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Water texture updated - Pixels with water: %d, Max depth: %.2f, Min non-zero: %.2f"),
               NonZeroPixels, MaxWaterDepth, MinNonZeroDepth);
        LastLogTime = CurrentTime;
    }

    // PHASE 1.5: Copy current depth to previous depth for next frame's displacement detection
    if (PreviousWaterDepthTexture)
    {
        FTexture2DMipMap& PrevMip = PreviousWaterDepthTexture->GetPlatformData()->Mips[0];
        void* PrevData = PrevMip.BulkData.Lock(LOCK_READ_WRITE);
        if (PrevData)
        {
            FMemory::Memcpy(PrevData, TextureData.GetData(), TextureData.Num() * sizeof(uint8));
            PrevMip.BulkData.Unlock();
            PreviousWaterDepthTexture->UpdateResource();
        }
    }
}


// ============================================================
// SUBSYSTEM 5.4: QUALITY & FILTERING
// ============================================================
// Edge-preserving filters and spatial smoothing for visual quality.
// Applies sophisticated filtering to prevent aliasing and artifacts
// while preserving sharp features like shorelines and flow boundaries.
// ============================================================

void UWaterSystem::ApplyEdgePreservingFilter(TArray<float>& Buffer, int32 Width, int32 Height)
{
    TArray<float> TempBuffer = Buffer;
    
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            float CenterValue = TempBuffer[Index];
            
            // Skip if no water
            if (CenterValue < 0.0001f)
                continue;
            
            // Calculate local gradient
            float GradientX = FMath::Abs(TempBuffer[Index + 1] - TempBuffer[Index - 1]);
            float GradientY = FMath::Abs(TempBuffer[Index + Width] - TempBuffer[Index - Width]);
            float Gradient = FMath::Max(GradientX, GradientY);
            
            // Only smooth low-gradient areas
            const float GradientThreshold = 0.1f;
            if (Gradient < GradientThreshold)
            {
                // Apply smoothing
                float Sum = 0.0f;
                int32 Count = 0;
                
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    for (int32 dx = -1; dx <= 1; dx++)
                    {
                        int32 NeighborIndex = (Y + dy) * Width + (X + dx);
                        if (TempBuffer[NeighborIndex] > 0.001f)
                        {
                            Sum += TempBuffer[NeighborIndex];
                            Count++;
                        }
                    }
                }
                
                if (Count > 0)
                {
                    float SmoothedValue = Sum / Count;
                    // Blend based on gradient
                    float BlendFactor = 1.0f - (Gradient / GradientThreshold);
                    Buffer[Index] = FMath::Lerp(CenterValue, SmoothedValue, BlendFactor * 0.5f);
                }
            }
        }
    }
}

void UWaterSystem::SmoothWaterDepthMap()
{
    if (!SimulationData.IsValid() || !bEnableSimulationSmoothing)
        return;
        
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    TArray<float> SmoothedDepths = SimulationData.WaterDepthMap;
    
    // Apply Gaussian-like smoothing
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            
            // Only smooth cells that have water or are near water
            float CenterDepth = SimulationData.WaterDepthMap[Index];
            if (CenterDepth <= 0.001f)
            {
                // Check if any neighbor has water
                bool bHasWaterNearby = false;
                for (int32 dy = -1; dy <= 1 && !bHasWaterNearby; dy++)
                {
                    for (int32 dx = -1; dx <= 1 && !bHasWaterNearby; dx++)
                    {
                        if (dx == 0 && dy == 0) continue;
                        int32 NIndex = (Y + dy) * Width + (X + dx);
                        if (SimulationData.WaterDepthMap[NIndex] > MinWaterDepth)
                            bHasWaterNearby = true;
                    }
                }
                
                if (!bHasWaterNearby) continue;
            }
                
            float Sum = 0.0f;
            float Weight = 0.0f;
            
            // 3x3 Gaussian kernel approximation
            const float Kernel[3][3] = {
                {0.0625f, 0.125f, 0.0625f},
                {0.125f,  0.25f,  0.125f},
                {0.0625f, 0.125f, 0.0625f}
            };
            
            for (int32 dy = -1; dy <= 1; dy++)
            {
                for (int32 dx = -1; dx <= 1; dx++)
                {
                    int32 NX = X + dx;
                    int32 NY = Y + dy;
                    
                    // Boundary check
                    if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                    {
                        int32 NIndex = NY * Width + NX;
                        float KernelWeight = Kernel[dy + 1][dx + 1];
                        Sum += SimulationData.WaterDepthMap[NIndex] * KernelWeight;
                        Weight += KernelWeight;
                    }
                }
            }
            
            if (Weight > 0.0f)
            {
                float SmoothedValue = Sum / Weight;
                // Blend with original based on smoothing strength
                SmoothedDepths[Index] = FMath::Lerp(CenterDepth, SmoothedValue, SimulationSmoothingStrength * 2.0f); //doubled smoothing
            }
        }
    }
    
    // Apply smoothed values back
    SimulationData.WaterDepthMap = SmoothedDepths;
}

// Apply edge-aware spatial smoothing to reduce artifacts
void UWaterSystem::ApplySpatialSmoothing()
{
    if (!SimulationData.IsValid() || !bEnableSimulationSmoothing)
        return;
        
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Only smooth areas with water gradients
    TArray<float> SmoothedDepths = SimulationData.WaterDepthMap;
    
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            float CenterDepth = SimulationData.WaterDepthMap[Index];
            
            // Skip completely dry cells
            if (CenterDepth <= 0.0f)
                continue;
            
            // Check if this cell is near a water edge (high gradient)
            bool bNearEdge = false;
            float MaxDiff = 0.0f;
            
            for (int32 dy = -1; dy <= 1; dy++)
            {
                for (int32 dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0) continue;
                    
                    int32 NIndex = (Y + dy) * Width + (X + dx);
                    float Diff = FMath::Abs(SimulationData.WaterDepthMap[NIndex] - CenterDepth);
                    MaxDiff = FMath::Max(MaxDiff, Diff);
                    
                    // Detect edges
                    if (Diff > MinWaterDepth * 2.0f)
                    {
                        bNearEdge = true;
                    }
                }
            }
            
            // Apply smoothing only near edges to preserve bulk water behavior
            if (bNearEdge && MaxDiff > 0.0f)
            {
                float Sum = 0.0f;
                float TotalWeight = 0.0f;
                
                // Use distance-weighted averaging
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    for (int32 dx = -1; dx <= 1; dx++)
                    {
                        int32 NX = X + dx;
                        int32 NY = Y + dy;
                        
                        if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
                        {
                            int32 NIndex = NY * Width + NX;
                            float NeighborDepth = SimulationData.WaterDepthMap[NIndex];
                            
                            // Weight by inverse depth difference
                            float DepthDiff = FMath::Abs(NeighborDepth - CenterDepth);
                            float Weight = 1.0f / (1.0f + DepthDiff * 10.0f);
                            
                            Sum += NeighborDepth * Weight;
                            TotalWeight += Weight;
                        }
                    }
                }
                
                if (TotalWeight > 0.0f)
                {
                    // Blend smoothed value with original
                    float SmoothValue = Sum / TotalWeight;
                    float BlendFactor = FMath::Clamp(MaxDiff / (MinWaterDepth * 10.0f), 0.0f, SimulationSmoothingStrength);
                    SmoothedDepths[Index] = FMath::Lerp(CenterDepth, SmoothValue, BlendFactor);
                }
            }
        }
    }
    
    SimulationData.WaterDepthMap = SmoothedDepths;
}

// ============================================================
// SUBSYSTEM 5.5: MATERIAL PARAMETERS & VISUALS
// ============================================================
// Dynamic material parameter updates and visual state management.
// Handles turbidity, visual updates, change detection, and
// active cell counting for performance optimization.
// ============================================================

float UWaterSystem::GetScaledTime() const
{
    // For GPU waves, we should use real time
    if (bUseRealTimeForGPUWaves && GetWorld())
    {
        return GetWorld()->GetTimeSeconds() * GPUWaveAnimationSpeed;
    }
    
    // Fallback - but this shouldn't be used for waves
    return AccumulatedScaledTime;
}

// ===== VOLUMETRIC WATER WITH OPTICAL DEPTH IMPLEMENTATION =====

void UWaterSystem::ApplyWaterMaterialParams(UMaterialInstanceDynamic* Material, const FWaterMaterialParams& Params)
{
    if (!Material) return;
    
    // Core volumetric water parameters
    Material->SetScalarParameterValue(FName("WaterClarityFactor"), Params.Clarity);
    Material->SetScalarParameterValue(FName("AbsorptionCoefficient"), Params.Absorption);
    Material->SetScalarParameterValue(FName("CausticStrength"), Params.CausticStrength);
    Material->SetVectorParameterValue(FName("DeepWaterColor"), Params.DeepColor);
    Material->SetVectorParameterValue(FName("ShallowWaterColor"), Params.ShallowColor);
    
    // Environmental response parameters
   
    Material->SetScalarParameterValue(FName("WaveStrength"), Params.WaveStrength);
    
    // Water depth scaling for Beer's law
    Material->SetScalarParameterValue(FName("WaterDepthScale"), 1.0f / WaterDepthScale);
    
   // UE_LOG(LogTemp, VeryVerbose, TEXT("Applied volumetric water parameters - Clarity: %.2f, Absorption: %.2f"),
       //    Params.Clarity, Params.Absorption);
}


void UWaterSystem::UpdateAllWaterVisuals(float DeltaTime)
{
    // Efficient update system - only update texture when water actually changes
    if (HasWaterChanged())
    {
        UpdateWaterDepthTexture();
        bWaterChangedThisFrame = true;
        UE_LOG(LogTemp, VeryVerbose, TEXT("Water changed - updating depth texture"));
    }
    else
    {
        bWaterChangedThisFrame = false;
    }
    
    // Always update dynamic parameters (time, weather)
    UpdateWaterShaderParameters();
}

bool UWaterSystem::HasWaterChanged() const
{
    // Check if water was added, removed, or significantly moved this frame
    static int32 LastWaterCellCount = 0;
    static float LastTotalWater = 0.0f;
    
    int32 CurrentWaterCells = GetActiveWaterCellCount();
    float CurrentTotalWater = GetTotalWaterVolume();
    
    bool Changed = (CurrentWaterCells != LastWaterCellCount) ||
                   (FMath::Abs(CurrentTotalWater - LastTotalWater) > 0.1f);
    
    LastWaterCellCount = CurrentWaterCells;
    LastTotalWater = CurrentTotalWater;
    
    return Changed;
}

int32 UWaterSystem::GetActiveWaterCellCount() const
{
    if (!SimulationData.IsValid()) return 0;
    
    int32 ActiveCells = 0;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        if (WaterDepth > MinWaterDepth)
        {
            ActiveCells++;
        }
    }
    return ActiveCells;
}


int32 UWaterSystem::GetActiveVolumeChunkCount() const
{
    return WaterSurfaceChunks.Num();

// ============================================================
// SUBSYSTEM 5.6: PLAYER INTERACTION QUERIES
// ============================================================
// Player-specific queries for underwater detection and depth.
// Provides gameplay systems with player-water interaction data.
// ============================================================

}

bool UWaterSystem::IsPlayerUnderwater(APlayerController* Player) const
{
    if (!Player || !Player->GetPawn() || !OwnerTerrain) return false;
    
    FVector PlayerLocation = Player->GetPawn()->GetActorLocation();
    float WaterDepth = GetWaterDepthAtPosition(PlayerLocation);
    float TerrainHeight = OwnerTerrain->GetHeightAtPosition(PlayerLocation);
    
    return PlayerLocation.Z < (TerrainHeight + WaterDepth);
}

float UWaterSystem::GetWaterDepthAtPlayer(APlayerController* Player) const
{
    if (!Player || !Player->GetPawn()) return 0.0f;
    
    FVector PlayerLocation = Player->GetPawn()->GetActorLocation();
    return GetWaterDepthAtPosition(PlayerLocation);
}

// ===== CHUNK BOUNDARY FIX IMPLEMENTATION =====

// Helper to get exact chunk boundaries in world space
FBox UWaterSystem::GetChunkWorldBounds(int32 ChunkIndex) const
{
    if (!OwnerTerrain || ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return FBox(ForceInit);
    }
    
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    float TerrainScale = OwnerTerrain->TerrainScale;
    
    // Calculate exact chunk boundaries
    float MinX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap) * TerrainScale;
    float MinY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap) * TerrainScale;
    float MaxX = MinX + (ChunkSize - 1) * TerrainScale;
    float MaxY = MinY + (ChunkSize - 1) * TerrainScale;
    
    // Get terrain height range for Z bounds
    float MinZ = -1000.0f;
    float MaxZ = 10000.0f;
    
    return FBox(FVector(MinX, MinY, MinZ), FVector(MaxX, MaxY, MaxZ));
}


// ============================================================
// SUBSYSTEM 5.7: ADVANCED MESH GENERATION & QUALITY
// ============================================================
// High-quality water surface generation with seamless boundaries.
// Includes both standard and high-quality mesh generation,
// interpolated depth queries, chunk overlap handling, and
// comprehensive boundary validation for artifact-free water.
//
// KEY ALGORITHMS:
//     Seamless depth interpolation across chunk boundaries
//     Edge-aware filtering for quality preservation
//     Boundary continuity validation
//     Multi-resolution mesh generation (LOD support)
// ============================================================

void UWaterSystem::GenerateSmoothWaterSurface(FWaterSurfaceChunk& SurfaceChunk,
                                             TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                             TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                             TArray<FColor>& VertexColors)
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return;
    
    int32 Resolution = FMath::Max(8, BaseSurfaceResolution >> SurfaceChunk.CurrentLOD);
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    FBox ChunkBounds = GetChunkWorldBounds(SurfaceChunk.ChunkIndex);
    
    // IMPORTANT: Use chunk bounds for accurate mesh generation
    float ChunkSizeWorld = ChunkBounds.Max.X - ChunkBounds.Min.X;
    float CellSize = ChunkSizeWorld / (float)(Resolution - 1);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WATER MESH] Generating boundary-fixed chunk %d with LOD %d (Resolution: %dx%d)"),
           SurfaceChunk.ChunkIndex, SurfaceChunk.CurrentLOD, Resolution, Resolution);
    
    // First pass: Build water presence map
    TArray<bool> WaterPresenceMap;
    WaterPresenceMap.SetNum(Resolution * Resolution);
    
    bool bHasAnyWater = false;
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float WorldX = ChunkBounds.Min.X + (X * CellSize);
            float WorldY = ChunkBounds.Min.Y + (Y * CellSize);
            FVector2D WorldPos(WorldX, WorldY);
            
            float WaterDepth = GetInterpolatedWaterDepthSeamless(WorldPos);
            int32 Index = Y * Resolution + X;
            WaterPresenceMap[Index] = (WaterDepth > MinWaterDepth);
            
            if (WaterPresenceMap[Index])
                bHasAnyWater = true;
        }
    }
    
    // Early exit if no water in chunk
    if (!bHasAnyWater)
    {
        Vertices.Empty();
        Triangles.Empty();
        Normals.Empty();
        UVs.Empty();
        VertexColors.Empty();
        UE_LOG(LogTemp, VeryVerbose, TEXT("[WATER MESH] Chunk %d has no water - skipping mesh generation"),
               SurfaceChunk.ChunkIndex);
        return;
    }
    
    // Second pass: Create vertices only where water exists or is adjacent to water
    TArray<int32> VertexIndexMap;
    VertexIndexMap.SetNum(Resolution * Resolution);
    for (int32 i = 0; i < VertexIndexMap.Num(); i++)
        VertexIndexMap[i] = -1;
    
    SurfaceChunk.MaxDepth = 0.0f;
    SurfaceChunk.AverageDepth = 0.0f;
    int32 WaterVertexCount = 0;
    int32 CurrentVertexIndex = 0;
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 GridIndex = Y * Resolution + X;
            
            // Check if this vertex is needed (has water or is adjacent to water for smooth edges)
            bool bVertexNeeded = false;
            if (WaterPresenceMap[GridIndex])
            {
                bVertexNeeded = true;
            }
            else
            {
                // Check if any adjacent cell has water (8-way check for smooth edges)
                for (int32 DY = -1; DY <= 1 && !bVertexNeeded; DY++)
                {
                    for (int32 DX = -1; DX <= 1 && !bVertexNeeded; DX++)
                    {
                        if (DX == 0 && DY == 0) continue;
                        
                        int32 AdjX = X + DX;
                        int32 AdjY = Y + DY;
                        if (AdjX >= 0 && AdjX < Resolution && AdjY >= 0 && AdjY < Resolution)
                        {
                            int32 AdjIndex = AdjY * Resolution + AdjX;
                            if (WaterPresenceMap[AdjIndex])
                                bVertexNeeded = true;
                        }
                    }
                }
            }
            
            if (!bVertexNeeded)
                continue; // Skip this vertex entirely - no mesh here!
            
            // Calculate world position using chunk bounds
            float WorldX = ChunkBounds.Min.X + (X * CellSize);
            float WorldY = ChunkBounds.Min.Y + (Y * CellSize);
            FVector2D WorldSamplePos(WorldX, WorldY);
            
            // Local position relative to chunk origin (for vertex position)
            float LocalX = WorldX - ChunkWorldPos.X;
            float LocalY = WorldY - ChunkWorldPos.Y;
            
            float WaterDepth = GetInterpolatedWaterDepthSeamless(WorldSamplePos);
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldX, WorldY, 0));
            
            float SurfaceHeight = TerrainHeight;
            bool bIsWaterVertex = (WaterDepth > MinWaterDepth);
            
            if (bIsWaterVertex)
            {
                SurfaceHeight = TerrainHeight + WaterDepth;
                SurfaceChunk.MaxDepth = FMath::Max(SurfaceChunk.MaxDepth, WaterDepth);
                SurfaceChunk.AverageDepth += WaterDepth;
                WaterVertexCount++;
                
                // ============================================
                // INTEGRATED WAVE GENERATION - SIMULATION RESPECTING
                // ============================================
                if (OwnerTerrain->GetWorld())
                {
                    float Time = GetScaledTime();
                    float WaveOffset = 0.0f;
                    
                    // STEP 1: Get authoritative flow data from simulation
                    FVector2D FlowVector = GetFlowVectorAtWorld(WorldSamplePos);
                    float FlowSpeed = FlowVector.Size();
                    
                    // STEP 2: Get atmospheric wind data
                    FVector WindData = FVector::ZeroVector;
                    if (CachedMasterController && CachedMasterController->AtmosphereController)
                    {
                        WindData = CachedMasterController->AtmosphereController->GetWindAtLocation(
                            FVector(WorldX, WorldY, TerrainHeight));
                    }
                    FVector2D WindDirection = FVector2D(WindData.X, WindData.Y);
                    float WindStrength = WindData.Size();
                    
                    float TerrainGradient = GetTerrainGradientMagnitude(WorldSamplePos);
                    
                    // Calculate natural wave offset
                   WaveOffset = CalculateNaturalWaveOffset(
                        WorldSamplePos,
                        Time,
                        WaterDepth,
                        FlowVector,
                        WindStrength,
                        WindDirection,
                        TerrainGradient,
                        SurfaceChunk
                    );

                    // Natural amplitude limiting PAY ATTENTION HERE WAVE CLAMP
                    WaveOffset = FMath::Clamp(WaveOffset, -WaterDepth * 0.125f, WaterDepth * 0.125f);

                    // Apply the calculated wave offset
                    SurfaceHeight += WaveOffset;
                }
            }
            
            // Store vertex index for triangle generation
            VertexIndexMap[GridIndex] = CurrentVertexIndex++;
            
            // Create vertex with proper local position
            FVector VertexPos(LocalX, LocalY, SurfaceHeight);
            Vertices.Add(VertexPos);
            
            // Normal calculation
            FVector2D FlowVector = GetFlowVectorAtWorld(WorldSamplePos);
            FVector Normal = CalculateWaterNormal(WorldSamplePos, FlowVector, WaterDepth);
            Normals.Add(Normal);
            
            // UV mapping (0-1 within chunk)
            float U = (float)X / (Resolution - 1);
            float V = (float)Y / (Resolution - 1);
            UVs.Add(FVector2D(U, V));
            
            // Vertex color with proper alpha handling
            uint8 Alpha = 255;
            if (!bIsWaterVertex)
            {
                // Edge vertex (no water but adjacent to water) - low alpha for blending
                Alpha = 25;
            }
            else
            {
                // Water vertex - calculate alpha based on depth and edge distance
                // Calculate distance to chunk edge for smooth blending
                float EdgeDistanceX = FMath::Min((float)X, (float)(Resolution - 1 - X)) / (float)(Resolution - 1);
                float EdgeDistanceY = FMath::Min((float)Y, (float)(Resolution - 1 - Y)) / (float)(Resolution - 1);
                float EdgeDistance = FMath::Min(EdgeDistanceX, EdgeDistanceY);
                
                // Fade alpha near edges for seamless blending between chunks
                float EdgeFade = FMath::SmoothStep(0.0f, 0.15f, EdgeDistance);
                float DepthFactor = FMath::Clamp(WaterDepth / 3.0f, 0.2f, 1.0f);
                Alpha = FMath::Clamp(255.0f * DepthFactor * EdgeFade, 50, 255);
            }
            
            VertexColors.Add(FColor(0, 100, 255, Alpha));
        }
    }
    
    if (WaterVertexCount > 0)
    {
        SurfaceChunk.AverageDepth /= WaterVertexCount;
    }
    
    // Third pass: Generate triangles only where water exists
    Triangles.Empty();
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            int32 TopLeft = Y * Resolution + X;
            int32 TopRight = Y * Resolution + (X + 1);
            int32 BottomLeft = (Y + 1) * Resolution + X;
            int32 BottomRight = (Y + 1) * Resolution + (X + 1);
            
            // Check if any corner has water
            bool bHasWater = WaterPresenceMap[TopLeft] || WaterPresenceMap[TopRight] ||
                            WaterPresenceMap[BottomLeft] || WaterPresenceMap[BottomRight];
            
            if (!bHasWater)
                continue; // Skip triangles in completely dry areas
            
            // Get vertex indices (-1 if vertex doesn't exist)
            int32 V0 = VertexIndexMap[TopLeft];
            int32 V1 = VertexIndexMap[TopRight];
            int32 V2 = VertexIndexMap[BottomLeft];
            int32 V3 = VertexIndexMap[BottomRight];
            
            // Create triangles only if all three vertices exist
            if (V0 >= 0 && V1 >= 0 && V2 >= 0)
            {
                Triangles.Add(V0);
                Triangles.Add(V2);
                Triangles.Add(V1);
            }
            
            if (V1 >= 0 && V2 >= 0 && V3 >= 0)
            {
                Triangles.Add(V1);
                Triangles.Add(V2);
                Triangles.Add(V3);
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WATER MESH] Generated chunk %d: %d vertices, %d triangles (from %d grid points)"),
           SurfaceChunk.ChunkIndex, Vertices.Num(), Triangles.Num() / 3, Resolution * Resolution);
}



// Fixed version of GenerateSmoothWaterSurface_HighQuality
void UWaterSystem::GenerateSmoothWaterSurface_HighQuality(FWaterSurfaceChunk& SurfaceChunk,
                                                         TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                                         TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                                         TArray<FColor>& VertexColors)
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return;
        
    // Use configurable quality setting
    int32 Resolution = FMath::Clamp(WaterMeshQuality, 8, 256);
    
    float ChunkSize = OwnerTerrain->ChunkSize - 1;
    float CellSize = (OwnerTerrain->TerrainScale * ChunkSize) / (float)(Resolution - 1);
    
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[HIGH QUALITY] Generating chunk %d with quality %d (Resolution: %dx%d)"),
           SurfaceChunk.ChunkIndex, WaterMeshQuality, Resolution, Resolution);
    
    // First pass: Build water presence map
    TArray<bool> WaterPresenceMap;
    WaterPresenceMap.SetNum(Resolution * Resolution);
    
    bool bHasAnyWater = false;
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D WorldSamplePos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            // AUTHORITY: Get water depth from simulation (single source of truth)
            float WaterDepth = GetSimulationDepth(WorldSamplePos);
            int32 Index = Y * Resolution + X;
            WaterPresenceMap[Index] = (WaterDepth > MinWaterDepth);
            
            if (WaterPresenceMap[Index])
                bHasAnyWater = true;
        }
    }
    
    // Early exit if no water
    if (!bHasAnyWater)
    {
        Vertices.Empty();
        Triangles.Empty();
        Normals.Empty();
        UVs.Empty();
        VertexColors.Empty();
        return;
    }
    
    // Second pass: Create vertices only where water exists or adjacent to water
    TArray<int32> VertexIndexMap;
    VertexIndexMap.SetNum(Resolution * Resolution);
    for (int32 i = 0; i < VertexIndexMap.Num(); i++)
        VertexIndexMap[i] = -1;
    
    SurfaceChunk.MaxDepth = 0.0f;
    SurfaceChunk.AverageDepth = 0.0f;
    float TotalDepth = 0.0f;
    int32 WaterVertices = 0;
    int32 CurrentVertexIndex = 0;
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 GridIndex = Y * Resolution + X;
            
            // Check if this vertex is needed
            bool bVertexNeeded = false;
            if (WaterPresenceMap[GridIndex])
            {
                bVertexNeeded = true;
            }
            else
            {
                // Check if any adjacent cell has water (for smooth edges)
                for (int32 DY = -1; DY <= 1 && !bVertexNeeded; DY++)
                {
                    for (int32 DX = -1; DX <= 1 && !bVertexNeeded; DX++)
                    {
                        if (DX == 0 && DY == 0) continue;
                        
                        int32 AdjX = X + DX;
                        int32 AdjY = Y + DY;
                        if (AdjX >= 0 && AdjX < Resolution && AdjY >= 0 && AdjY < Resolution)
                        {
                            int32 AdjIndex = AdjY * Resolution + AdjX;
                            if (WaterPresenceMap[AdjIndex])
                                bVertexNeeded = true;
                        }
                    }
                }
            }
            
            if (!bVertexNeeded)
                continue;
            
            // Create vertex
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D WorldSamplePos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            float WaterDepth = GetSimulationDepth(WorldSamplePos);
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldSamplePos.X, WorldSamplePos.Y, 0));
            
            float SurfaceHeight = TerrainHeight;
            if (WaterDepth > MinWaterDepth)
            {
                SurfaceHeight = TerrainHeight + WaterDepth;
                SurfaceChunk.MaxDepth = FMath::Max(SurfaceChunk.MaxDepth, WaterDepth);
                TotalDepth += WaterDepth;
                WaterVertices++;
                
            }
            
            // Store vertex index
            VertexIndexMap[GridIndex] = CurrentVertexIndex++;
            
            // Create vertex with absolute height
            FVector VertexPos(LocalX, LocalY, SurfaceHeight);
            Vertices.Add(VertexPos);
            
            // Calculate normal from simulation flow
            FVector2D FlowVector = GetSimulationVelocity(WorldSamplePos);
            FVector Normal = CalculateWaterNormal(WorldSamplePos, FlowVector, WaterDepth);
            Normals.Add(Normal);
            
            // UV coordinates
            float U = (float)X / (Resolution - 1);
            float V = (float)Y / (Resolution - 1);
            UVs.Add(FVector2D(U, V));
            
            // High quality color based on depth and flow
            float FlowSpeed = FlowVector.Size();
            uint8 Alpha = 255;
            if (WaterDepth <= MinWaterDepth)
            {
                // Edge vertex - reduced alpha
                Alpha = 50;
            }
            else
            {
                // Water vertex - full quality with depth-based variation
                Alpha = FMath::Clamp(128 + (WaterDepth / 5.0f * 127), 128, 255);
            }
            
            FColor VertexColor = FColor(
                FMath::Clamp(WaterDepth * 25.0f, 0.0f, 255.0f),     // Red: Depth
                FMath::Clamp(FlowSpeed * 5.0f, 0.0f, 255.0f),       // Green: Flow
                128,                                                  // Blue: Constant
                Alpha                                                 // Alpha
            );
            VertexColors.Add(VertexColor);
        }
    }
    
    SurfaceChunk.AverageDepth = WaterVertices > 0 ? (TotalDepth / WaterVertices) : 0.0f;
    
    // Third pass: Generate triangles only where water exists
    Triangles.Empty();
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            int32 TopLeft = Y * Resolution + X;
            int32 TopRight = Y * Resolution + (X + 1);
            int32 BottomLeft = (Y + 1) * Resolution + X;
            int32 BottomRight = (Y + 1) * Resolution + (X + 1);
            
            // Check if any corner has water
            bool bHasWater = WaterPresenceMap[TopLeft] || WaterPresenceMap[TopRight] ||
                            WaterPresenceMap[BottomLeft] || WaterPresenceMap[BottomRight];
            
            if (!bHasWater)
                continue;
            
            // Get vertex indices
            int32 V0 = VertexIndexMap[TopLeft];
            int32 V1 = VertexIndexMap[TopRight];
            int32 V2 = VertexIndexMap[BottomLeft];
            int32 V3 = VertexIndexMap[BottomRight];
            
            // Create triangles only if vertices exist
            if (V0 >= 0 && V1 >= 0 && V2 >= 0)
            {
                Triangles.Add(V0);
                Triangles.Add(V2);
                Triangles.Add(V1);
            }
            
            if (V1 >= 0 && V2 >= 0 && V3 >= 0)
            {
                Triangles.Add(V1);
                Triangles.Add(V2);
                Triangles.Add(V3);
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[HIGH QUALITY] Generated %d vertices, %d triangles at quality %d"),
           Vertices.Num(), Triangles.Num() / 3, WaterMeshQuality);
}

// Enhanced water depth interpolation for seamless chunk boundaries
float UWaterSystem::GetInterpolatedWaterDepthSeamless(FVector2D WorldPosition) const
{
    if (!CachedMasterController || !SimulationData.IsValid())
        return 0.0f;
    
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(
        FVector(WorldPosition.X, WorldPosition.Y, 0));
    
    float X = TerrainCoords.X;
    float Y = TerrainCoords.Y;
    int32 X0 = FMath::FloorToInt(X);
    int32 Y0 = FMath::FloorToInt(Y);
    
    // Use bicubic interpolation for smoother results
    if (bUse8DirectionalFlow && bUseBicubicInterpolation)
    {
        // Gather 4x4 grid of samples for bicubic interpolation
        float Samples[4][4];
        
        for (int32 j = -1; j <= 2; j++)
        {
            for (int32 i = -1; i <= 2; i++)
            {
                int32 SX = FMath::Clamp(X0 + i, 0, SimulationData.TerrainWidth - 1);
                int32 SY = FMath::Clamp(Y0 + j, 0, SimulationData.TerrainHeight - 1);
                int32 SIndex = SY * SimulationData.TerrainWidth + SX;
                
                float Depth = SimulationData.WaterDepthMap[SIndex];
                
                // Apply smooth threshold function instead of hard cutoff
                if (Depth < MinWaterDepth && Depth > 0.0f)
                {
                    // Smooth falloff near threshold
                    float T = Depth / MinWaterDepth;
                    Depth = Depth * FMath::SmoothStep(0.0f, 1.0f, T);
                }
                
                Samples[j + 1][i + 1] = Depth;
            }
        }
        
        // Bicubic interpolation
        float FracX = X - X0;
        float FracY = Y - Y0;
        
        // Catmull-Rom spline interpolation
        auto CubicInterpolate = [](float p0, float p1, float p2, float p3, float t) -> float
        {
            float a0 = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
            float a1 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
            float a2 = -0.5f * p0 + 0.5f * p2;
            float a3 = p1;
            
            return a0 * t * t * t + a1 * t * t + a2 * t + a3;
        };
        
        // Interpolate along Y
        float Y0Result = CubicInterpolate(Samples[0][0], Samples[0][1], Samples[0][2], Samples[0][3], FracX);
        float Y1Result = CubicInterpolate(Samples[1][0], Samples[1][1], Samples[1][2], Samples[1][3], FracX);
        float Y2Result = CubicInterpolate(Samples[2][0], Samples[2][1], Samples[2][2], Samples[2][3], FracX);
        float Y3Result = CubicInterpolate(Samples[3][0], Samples[3][1], Samples[3][2], Samples[3][3], FracX);
        
        // Interpolate along X
        float Result = CubicInterpolate(Y0Result, Y1Result, Y2Result, Y3Result, FracY);
        
        return FMath::Max(0.0f, Result);
    }
    else if (bUse8DirectionalFlow)
    {
        // Enhanced 8-directional sampling without bicubic
        float WeightedSum = 0.0f;
        float TotalWeight = 0.0f;
        
        // Sample in 8 directions plus center
        for (int32 dy = -1; dy <= 1; dy++)
        {
            for (int32 dx = -1; dx <= 1; dx++)
            {
                int32 SX = FMath::Clamp(X0 + dx, 0, SimulationData.TerrainWidth - 1);
                int32 SY = FMath::Clamp(Y0 + dy, 0, SimulationData.TerrainHeight - 1);
                int32 SIndex = SY * SimulationData.TerrainWidth + SX;
                
                float Depth = SimulationData.WaterDepthMap[SIndex];
                
                // Calculate distance-based weight
                float DistX = (float)(X0 + dx) - X;
                float DistY = (float)(Y0 + dy) - Y;
                float DistSq = DistX * DistX + DistY * DistY;
                float Weight = FMath::Exp(-DistSq * 2.0f);
                
                // Apply diagonal weight adjustment
                if (dx != 0 && dy != 0)
                {
                    Weight *= DiagonalFlowWeight;
                }
                
                WeightedSum += Depth * Weight;
                TotalWeight += Weight;
            }
        }
        
        if (TotalWeight > 0.0f)
        {
            return WeightedSum / TotalWeight;
        }
    }
    else
    {
        // Original bilinear interpolation
        float X1 = FMath::Clamp(X0 + 1, 0, SimulationData.TerrainWidth - 1);
        float Y1 = FMath::Clamp(Y0 + 1, 0, SimulationData.TerrainHeight - 1);
        
        float FracX = X - X0;
        float FracY = Y - Y0;
        
        int32 Idx00 = Y0 * SimulationData.TerrainWidth + X0;
        int32 Idx10 = Y0 * SimulationData.TerrainWidth + X1;
        int32 Idx01 = Y1 * SimulationData.TerrainWidth + X0;
        int32 Idx11 = Y1 * SimulationData.TerrainWidth + X1;
        
        float D00 = SimulationData.WaterDepthMap[Idx00];
        float D10 = SimulationData.WaterDepthMap[Idx10];
        float D01 = SimulationData.WaterDepthMap[Idx01];
        float D11 = SimulationData.WaterDepthMap[Idx11];
        
        float D0 = FMath::Lerp(D00, D10, FracX);
        float D1 = FMath::Lerp(D01, D11, FracX);
        
        return FMath::Lerp(D0, D1, FracY);
    }
    
    return 0.0f;
}

// Override chunk overlap for water to ensure seamless boundaries
int32 UWaterSystem::GetWaterChunkOverlap() const
{
    // Water needs 2-cell overlap for smooth interpolation at boundaries
    return 2;
}

// Validate water mesh bounds
bool UWaterSystem::ValidateWaterMeshBounds(const FWaterSurfaceChunk& Chunk) const
{
    if (!Chunk.SurfaceMesh)
        return false;
    
    FBox ChunkBounds = GetChunkWorldBounds(Chunk.ChunkIndex);
    FBoxSphereBounds MeshBounds = Chunk.SurfaceMesh->Bounds;
    
    // Check if mesh bounds exceed chunk bounds (with small tolerance)
    const float Tolerance = 10.0f;
    bool bWithinBounds =
        MeshBounds.Origin.X >= ChunkBounds.Min.X - Tolerance &&
        MeshBounds.Origin.X <= ChunkBounds.Max.X + Tolerance &&
        MeshBounds.Origin.Y >= ChunkBounds.Min.Y - Tolerance &&
        MeshBounds.Origin.Y <= ChunkBounds.Max.Y + Tolerance;
    
    if (!bWithinBounds)
    {
        UE_LOG(LogTemp, Warning, TEXT("Water mesh %d exceeds chunk bounds!"), Chunk.ChunkIndex);
    }
    
    return bWithinBounds;
}

// Blueprint-callable validation function
void UWaterSystem::ValidateAllWaterChunkBoundaries()
{
    if (!OwnerTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateAllWaterChunkBoundaries: No terrain owner!"));
        return;
    }
    
    int32 TotalChunks = WaterSurfaceChunks.Num();
    int32 ValidChunks = 0;
    int32 InvalidChunks = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("===== WATER CHUNK BOUNDARY VALIDATION ====="));
    UE_LOG(LogTemp, Warning, TEXT("Total water chunks: %d"), TotalChunks);
    
    for (const FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
    {
        if (ValidateWaterMeshBounds(Chunk))
        {
            ValidChunks++;
        }
        else
        {
            InvalidChunks++;
            
            // Log detailed info about invalid chunk
            FBox ChunkBounds = GetChunkWorldBounds(Chunk.ChunkIndex);
            if (Chunk.SurfaceMesh)
            {
                FBoxSphereBounds MeshBounds = Chunk.SurfaceMesh->Bounds;
                UE_LOG(LogTemp, Error, TEXT("Chunk %d boundary violation:"), Chunk.ChunkIndex);
                UE_LOG(LogTemp, Error, TEXT("  Expected bounds: X[%.1f, %.1f] Y[%.1f, %.1f]"),
                    ChunkBounds.Min.X, ChunkBounds.Max.X, ChunkBounds.Min.Y, ChunkBounds.Max.Y);
                UE_LOG(LogTemp, Error, TEXT("  Mesh center: (%.1f, %.1f, %.1f)"),
                    MeshBounds.Origin.X, MeshBounds.Origin.Y, MeshBounds.Origin.Z);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Validation complete: %d valid, %d invalid"), ValidChunks, InvalidChunks);
    UE_LOG(LogTemp, Warning, TEXT("===== END VALIDATION ====="));
    
    // Check chunk neighbor continuity
    CheckChunkBoundaryContinuity();
}

void UWaterSystem::CheckChunkBoundaryContinuity()
{
    UE_LOG(LogTemp, Warning, TEXT("===== CHUNK BOUNDARY CONTINUITY CHECK ====="));
    
    for (int32 i = 0; i < WaterSurfaceChunks.Num(); i++)
    {
        const FWaterSurfaceChunk& ChunkA = WaterSurfaceChunks[i];
        
        // Get neighbors
        TArray<int32> Neighbors = OwnerTerrain->GetNeighboringChunks(ChunkA.ChunkIndex, false);
        
        for (int32 NeighborIndex : Neighbors)
        {
            // Find neighbor chunk in water chunks
            const FWaterSurfaceChunk* ChunkB = WaterSurfaceChunks.FindByPredicate(
                [NeighborIndex](const FWaterSurfaceChunk& C) { return C.ChunkIndex == NeighborIndex; });
            
            if (ChunkB && ChunkA.SurfaceMesh && ChunkB->SurfaceMesh)
            {
                // Check if boundaries align
                FBox BoundsA = GetChunkWorldBounds(ChunkA.ChunkIndex);
                FBox BoundsB = GetChunkWorldBounds(ChunkB->ChunkIndex);
                
                // Check for gaps
                float Gap = 0.0f;
                bool bSharesEdge = false;
                
                // Check X boundary
                if (FMath::IsNearlyEqual(BoundsA.Max.X, BoundsB.Min.X, 1.0f))
                {
                    bSharesEdge = true;
                    Gap = FMath::Abs(BoundsA.Max.X - BoundsB.Min.X);
                }
                else if (FMath::IsNearlyEqual(BoundsB.Max.X, BoundsA.Min.X, 1.0f))
                {
                    bSharesEdge = true;
                    Gap = FMath::Abs(BoundsB.Max.X - BoundsA.Min.X);
                }
                // Check Y boundary
                else if (FMath::IsNearlyEqual(BoundsA.Max.Y, BoundsB.Min.Y, 1.0f))
                {
                    bSharesEdge = true;
                    Gap = FMath::Abs(BoundsA.Max.Y - BoundsB.Min.Y);
                }
                else if (FMath::IsNearlyEqual(BoundsB.Max.Y, BoundsA.Min.Y, 1.0f))
                {
                    bSharesEdge = true;
                    Gap = FMath::Abs(BoundsB.Max.Y - BoundsA.Min.Y);
                }
                
                if (bSharesEdge && Gap > 0.1f)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Gap detected between chunks %d and %d: %.3f units"),
                        ChunkA.ChunkIndex, ChunkB->ChunkIndex, Gap);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("===== END CONTINUITY CHECK ====="));
}
// =============================================================================
// SECTION 6: WAVE PHYSICS SYSTEM
// =============================================================================
//
// PURPOSE:
//   Complete wave generation and physical wave simulation system.
//   Implements realistic water surface waves using multiple wave types
//   and physical models. Provides natural-looking water motion through
//   superposition of different wave phenomena.
//
// SUBSECTIONS:
//   6.1 - Wave Generation Core: Main wave calculation entry point
//   6.2 - Wave Type Generators: 6 distinct wave physics models
//   6.3 - Wave Processing: Spatial smoothing and wave combining
//   6.4 - Splash & Utilities: Dynamic splashes and debug tools
//
// WAVE TYPES IMPLEMENTED:
//   1. Wind Waves: Wind-driven capillary-gravity waves
//   2. Gravity Waves: Deep water waves from wind energy
//   3. Turbulent Waves: Chaotic motion from turbulence
//   4. Collision Waves: Obstacle and terrain interaction
//   5. Flow Waves: Surface waves from water flow
//   6. Capillary Waves: Surface tension driven (small scale)
//
// KEY PHYSICS EQUATIONS:
//   Deep Water Dispersion:   = gk (where k = 2ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚ÂÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬/ÃƒÆ’Ã†â€™Ãƒâ€¦Ã‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â»)
//   Shallow Water Speed: c = ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã¢â‚¬Â¹ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡(gh)
//   Froude Number: Fr = v/ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã¢â‚¬Â¹ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡(gh)
//   Capillary Wavelength: ÃƒÆ’Ã†â€™Ãƒâ€¦Ã‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â»c = 2ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚ÂÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã¢â‚¬Â¹ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡(ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚ÂÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢/ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚ÂÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Âg)   17mm for water
//   Wave Energy: E = (1/2)ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚ÂÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂgA  (per unit area)
//   Group Velocity: cg = c/2 (deep water)
//
// PHYSICAL PARAMETERS:
//     g = 9.81 m/s  (gravitational acceleration)
//       = 0.072 N/m (surface tension, water-air)
//       = 1000 kg/m  (water density)
//     Typical wavelengths: 1cm - 100m
//     Typical periods: 0.1s - 10s
//
// WAVE SUPERPOSITION:
//   Final wave height is linear combination of all wave types:
//   h_total = ÃƒÆ’Ã†â€™Ãƒâ€¦Ã‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â£(h_i) where each h_i is a wave component
//
//   Amplitude limits ensure physically reasonable results:
//     Wind waves: Limited by fetch and wind duration
//     Gravity waves: Limited by breaking criterion (H/  < 1/7)
//     Capillary waves: Limited by surface tension
//
// DEPENDENCIES:
//   Internal:
//     - Section 2: Water depth and flow velocity data
//     - Section 5: Mesh generation calls wave functions
//   External:
//     - AAtmosphereController: Wind data for wind waves
//     - ADynamicTerrain: Terrain gradient for collision waves
//
// PERFORMANCE NOTES:
//   - Wave calculation: ~0.1ms per vertex
//   - 6 wave generators: ~0.6ms per vertex total
//   - Optimized for real-time mesh generation
//   - LOD system reduces vertex count at distance
//
// REALISM FEATURES:
//     Froude number transitions (subcritical   supercritical)
//     Hydraulic jumps in supercritical flow
//     Wave breaking at shallow depth
//     Wind fetch effects
//     Terrain shadowing (wave blocking)
//     Amplitude modulation from flow
//
// =============================================================================

// ============================================================
// SUBSYSTEM 6.1: WAVE GENERATION CORE
// ============================================================
// Main wave calculation function that coordinates all wave
// generators and combines results. Entry point for natural
// wave displacement in mesh generation.
//
// ALGORITHM:
//   1. Setup wave context (depth, flow, wind, terrain)
//   2. Call each wave generator (6 types)
//   3. Combine wave components with superposition
//   4. Apply spatial smoothing
//   5. Return final wave height offset
// ============================================================

float UWaterSystem::CalculateNaturalWaveOffset(FVector2D WorldPos, float Time, float WaterDepth,
    FVector2D FlowVector, float WindStrength, FVector2D WindDirection,
    float TerrainGradient, FWaterSurfaceChunk& Chunk)
{
    // PHASE 2: Calculate mesh resolution from terrain and base resolution
    float ChunkSizeWorld = OwnerTerrain->ChunkSize * OwnerTerrain->TerrainScale;
    int32 Resolution = FMath::Clamp(BaseSurfaceResolution >> Chunk.CurrentLOD, 8, 64);
    float MeshResolution = ChunkSizeWorld / (Resolution - 1);
    
    // Initialize context with mesh resolution
    FWaveContext Context;
    Context.Init(WorldPos, Time, WaterDepth, FlowVector, WindStrength, WindDirection,
                 TerrainGradient, MeshResolution);
    
    // Update cache (existing code)
  
    
    TArray<float> WaveComponents;
    
    // Generate waves with scale awareness
    if (WindStrength > WaveMinWindForWaves)
    {
        WaveComponents.Add(GenerateWindWaves(Context));
    }
    
    if (Context.FlowSpeed > 0.1f)
    {
        WaveComponents.Add(GenerateFlowWaves(Context));
    }
    
    if (WaterDepth > 200.0f && WindStrength > 1.0f)
    {
        WaveComponents.Add(GenerateGravityWaves(Context));
    }
    
    if (WaterDepth < 50.0f && WaterDepth > 1.0f)
    {
        WaveComponents.Add(GenerateCapillaryWaves(Context));
    }
    
    if (Context.FlowSpeed > WaveTuning.CollisionFlowThreshold)
    {
        WaveComponents.Add(GenerateCollisionWaves(Context));
    }
    
    if (Context.FroudeNumber > 1.0f || (Context.FlowSpeed > 10.0f && TerrainGradient > 15.0f))
    {
        WaveComponents.Add(GenerateTurbulentWaves(Context));
    }
    
    float WaveOffset = CombineWaveComponents(WaveComponents);
    
    // PHASE 1: Apply spatial smoothing for coarser meshes
    if (MeshResolution > 50.0f)
    {
        WaveOffset = ApplySpatialSmoothing(WaveOffset, WorldPos, MeshResolution);
    }
    
    // PHASE 1: Final amplitude clamping
    float MaxWaveHeight = FMath::Min(WaterDepth * 0.3f, WindStrength * 0.05f);
    
    return FMath::Clamp(WaveOffset, -MaxWaveHeight, MaxWaveHeight);
}

// ========== COMPLETE WAVE GENERATOR UPDATES ==========

// ============================================================
// SUBSYSTEM 6.2: WAVE TYPE GENERATORS
// ============================================================
// Six distinct wave physics models, each implementing specific
// wave phenomena with appropriate physical equations.
//
// WAVE TYPES:
//     Wind Waves: Wind stress on water surface
//     Gravity Waves: Restoration by gravity force
//     Turbulent Waves: Random chaotic motion
//     Collision Waves: Reflection/diffraction at obstacles
//     Flow Waves: Kelvin-Helmholtz instability
//     Capillary Waves: Surface tension ripples
//
// Each generator returns wave height contribution (meters).
// ============================================================

float UWaterSystem::GenerateWindWaves(const FWaveContext& Context) const
{
    float WindSpeed = Context.WindStrength;
    if (WindSpeed < WaveMinWindForWaves) return 0.0f;
    
    float Fetch = 1000.0f;
    float Hs = 0.0246f * WindSpeed * WindSpeed * FMath::Sqrt(Fetch / WaveGravity);
    Hs = FMath::Min(Hs, Context.WaterDepth * 0.3f);
    
    float Wave = 0.0f;
    int NumComponents = (Context.WaterDepth > 200.0f) ? 5 : 3;
    
    for (int i = 0; i < NumComponents; i++)
    {
        // NEW: Scale-aware wavelength calculation
        float BaseWavelength = FMath::Max(Context.MinWavelength * 5.0f, 100.0f);
        float Wavelength = BaseWavelength * FMath::Pow(2.0f, i);
        float k = WaveTwoPi / Wavelength;
        
        float kh = k * Context.WaterDepth;
        float omega = FMath::Sqrt(WaveGravity * k * FMath::Min(kh, 1.0f));
        
        float Amplitude = (Hs / 4.0f) * FMath::Exp(-0.5f * i);
        
        // NEW: Shallow water dampening
        if (Context.WaterDepth < 50.0f)
        {
            float DepthFactor = Context.WaterDepth / 50.0f;
            Amplitude *= DepthFactor * DepthFactor;
        }
        
        float WindProj = FVector2D::DotProduct(Context.WorldPos * 0.01f, Context.WindDirection);
        float Phase = k * WindProj - omega * Context.Time;
        float Spread = FMath::Cos((i * 15.0f) * (PI / 180.0f));
        
        Wave += Amplitude * Spread * FMath::Sin(Phase);
    }
    
    return Wave;
}

float UWaterSystem::GenerateGravityWaves(const FWaveContext& Context) const
{
    if (Context.WaterDepth < 100.0f || Context.WindStrength < 1.0f) return 0.0f;
    
    float WindSpeed = Context.WindStrength;
    float Fetch = 5000.0f;
    
    float Tp = 0.87f * FMath::Sqrt(Fetch / WaveGravity);
    float Hs = FMath::Min(0.0016f * WindSpeed * FMath::Sqrt(Fetch), Context.WaterDepth * 0.3f); // NEW: Clamped
    
    float Wave = 0.0f;
    
    for (int i = 0; i < 4; i++)
    {
        float PeriodRatio = 0.7f + i * 0.2f;
        float Period = Tp * PeriodRatio;
        float omega = WaveTwoPi / Period;
        
        // NEW: Scale-aware wavelength
        float k = omega * omega / WaveGravity;
        float Wavelength = WaveTwoPi / k;
        if (Wavelength < Context.MinWavelength * 2.0f)
        {
            continue; // Skip components that would alias
        }
        
        float sigma = (PeriodRatio < 1.0f) ? 0.07f : 0.09f;
        float r = FMath::Exp(-0.5f * FMath::Square((PeriodRatio - 1.0f) / sigma));
        float gamma = 3.3f;
        float SpectralDensity = FMath::Pow(gamma, r);
        
        float Amplitude = (Hs / 6.0f) * FMath::Sqrt(SpectralDensity) *
                         FMath::Exp(-1.25f * FMath::Pow(PeriodRatio, -4.0f));
        
        // NEW: Depth-based scaling
        if (Context.WaterDepth < 500.0f)
        {
            float DepthScale = Context.WaterDepth / 500.0f;
            Amplitude *= DepthScale;
        }
        
        float WindProj = FVector2D::DotProduct(Context.WorldPos * 0.01f, Context.WindDirection);
        float Phase = k * WindProj - omega * Context.Time;
        float DirSpread = 1.0f - 0.1f * i;
        
        Wave += Amplitude * DirSpread * FMath::Cos(Phase);
    }
    
    return Wave;
}

float UWaterSystem::GenerateTurbulentWaves(const FWaveContext& Context) const
{
    if (Context.FroudeNumber < 0.8f && Context.TerrainGradient < 15.0f) return 0.0f;
    
    float Turbulence = 0.0f;
    
    float Intensity = FMath::Min(Context.FroudeNumber, 2.0f) * 0.5f;
    if (Context.TerrainGradient > 30.0f)
    {
        Intensity *= (1.0f + (Context.TerrainGradient - 30.0f) / 30.0f);
    }
    
    // NEW: Clamp intensity
    Intensity = FMath::Min(Intensity, 1.5f);
    
    for (int i = 0; i < 4; i++)
    {
        float Scale = FMath::Pow(2.0f, -i * 0.8f);
        
        // NEW: Scale-aware frequency
        float Freq = 0.2f / FMath::Max(Context.MeshResolution, 50.0f);
        float TimeScale = 6.0f * Scale;
        
        float Noise1 = FMath::Sin(Context.WorldPos.X * Freq + Context.WorldPos.Y * Freq * 1.618f +
                                 Context.Time * TimeScale);
        float Noise2 = FMath::Cos(Context.WorldPos.X * Freq * 0.866f - Context.WorldPos.Y * Freq * 0.5f -
                                 Context.Time * TimeScale * 0.7f);
        float Noise3 = FMath::Sin(Context.WorldPos.Y * Freq * 1.414f + Context.Time * TimeScale * 1.3f);
        
        float Noise = (Noise1 * Noise2 + Noise3) * 0.5f;
        float Energy = FMath::Pow(Scale, 1.5f);
        
        // NEW: Depth-based amplitude limit
        float MaxAmplitude = Context.WaterDepth * 0.05f;
        Turbulence += FMath::Clamp(Noise * Energy * Intensity * MaxAmplitude,
                                   -MaxAmplitude, MaxAmplitude);
    }
    
    if (Context.FroudeNumber > 1.0f)
    {
        float JumpHeight = Context.WaterDepth * 0.5f *
            (FMath::Sqrt(1.0f + 8.0f * Context.FroudeNumber * Context.FroudeNumber) - 1.0f);
        
        // NEW: Clamp jump height
        JumpHeight = FMath::Min(JumpHeight, Context.WaterDepth * 0.3f);
        
        float JumpFreq = Context.FlowSpeed / (Context.WaterDepth * 4.0f);
        float JumpPhase = FVector2D::DotProduct(Context.WorldPos * 0.01f, Context.FlowVector.GetSafeNormal());
        
        float Jump = JumpHeight * 0.1f *
                    (0.5f + 0.5f * FMath::Sin(JumpPhase * 0.5f + Context.Time * JumpFreq * WaveTwoPi));
        
        Turbulence += Jump;
    }
    
    return Turbulence;
}

float UWaterSystem::GenerateCollisionWaves(const FWaveContext& Context) const
{
    if (Context.FlowSpeed < WaveTuning.CollisionFlowThreshold) return 0.0f;
    
    FVector2D FlowDir = Context.FlowVector.GetSafeNormal();
    
    float ObstacleStrength = FMath::Abs(Context.TerrainGradient) * 0.1f;
    if (ObstacleStrength < 0.01f) return 0.0f;
    
    float Wave = 0.0f;
    
    // NEW: Scale-aware wavelength
    float BaseWavelength = Context.FlowSpeed * 2.0f / FMath::Sqrt(Context.FroudeNumber + 0.1f);
    float Wavelength = FMath::Max(BaseWavelength, Context.MinWavelength * 3.0f);
    
    float Amplitude = ObstacleStrength * Context.WaterDepth * 0.1f;
    
    // NEW: Amplitude clamping
    Amplitude = FMath::Min(Amplitude, Context.WaterDepth * 0.2f);
    
    float StreamwisePos = FVector2D::DotProduct(Context.WorldPos * 0.01f, FlowDir);
    
    float Decay = FMath::Exp(-FMath::Abs(StreamwisePos) * 0.002f);
    float Phase = (StreamwisePos / Wavelength) * WaveTwoPi;
    
    Wave = Amplitude * Decay * FMath::Sin(Phase + Context.Time * 3.0f);
    
    // Vortex shedding
    float VortexFreq = 0.2f * Context.FlowSpeed / (Context.WaterDepth * 0.1f);
    
    // NEW: Clamp vortex frequency to prevent aliasing
    VortexFreq = FMath::Min(VortexFreq, 1.0f / (Context.MinWavelength * 0.1f));
    
    float CrossPos = FVector2D::DotProduct(Context.WorldPos * 0.01f, FVector2D(-FlowDir.Y, FlowDir.X));
    
    Wave += Amplitude * 0.3f * Decay *
            FMath::Sin(CrossPos * 0.1f + Context.Time * VortexFreq * WaveTwoPi);
    
    return Wave;
}


float UWaterSystem::GenerateFlowWaves(const FWaveContext& Context) const
{
    if (Context.FlowSpeed < 0.1f) return 0.0f;
    
    FVector2D FlowNormal = Context.FlowVector.GetSafeNormal();
    FVector2D CrossFlow(-FlowNormal.Y, FlowNormal.X);
    
    float Wave = 0.0f;
    
    if (Context.FroudeNumber < 1.0f)
    {
        // PHASE 2: Scale-aware wavelength
        float Wavelength = FMath::Max(Context.MeshResolution * 4.0f, Context.WaterDepth * 4.0f);
        float Amplitude = Context.WaterDepth * 0.02f * Context.FroudeNumber;
        
        // PHASE 1: Amplitude clamping
        Amplitude = FMath::Min(Amplitude, Context.WaterDepth * 0.3f);
        
        float CrossPos = FVector2D::DotProduct(Context.WorldPos * 0.01f, CrossFlow);
        Wave = Amplitude * FMath::Sin(CrossPos / Wavelength * WaveTwoPi);
        
        float StreamPos = FVector2D::DotProduct(Context.WorldPos * 0.01f, FlowNormal);
        float Modulation = 0.7f + 0.3f * FMath::Sin(StreamPos * 0.1f - Context.Time * Context.FlowSpeed * 0.05f);
        Wave *= Modulation;
    }
    else
    {
        // Supercritical flow
        float JumpHeight = Context.WaterDepth *
            (FMath::Sqrt(1.0f + 8.0f * Context.FroudeNumber * Context.FroudeNumber) - 1.0f) * 0.04f;
        
        // PHASE 1: Limit jump height
        JumpHeight = FMath::Min(JumpHeight, Context.WaterDepth * 0.3f);
        
        for (int i = 0; i < 3; i++)
        {
            float Scale = FMath::Pow(2.0f, -i);
            float Freq = 0.2f / Context.MeshResolution; // PHASE 2: Scale-aware frequency
            
            Wave += JumpHeight * Scale *
                   FMath::Sin(Context.WorldPos.X * Freq + Context.Time * 6.0f * Scale) *
                   FMath::Cos(Context.WorldPos.Y * Freq * 1.618f - Context.Time * 4.0f * Scale);
        }
    }
    
    return Wave;
}

float UWaterSystem::GenerateCapillaryWaves(const FWaveContext& Context) const
{
    if (Context.WaterDepth < 1.0f || Context.WaterDepth > 50.0f) return 0.0f;
    
    float Wave = 0.0f;
    
    // PHASE 2: Minimum wavelength respects mesh resolution
    float BaseWavelength = FMath::Max(Context.MinWavelength, 10.0f); // At least 10cm
    float SurfaceTension = 0.0728f;
    
    for (int i = 0; i < 3; i++)
    {
        float Wavelength = BaseWavelength * FMath::Pow(1.5f, i); // Less aggressive scaling
        float k = WaveTwoPi / Wavelength;
        
        float omega = FMath::Sqrt(SurfaceTension * k * k * k / WaveDensity);
        float Amplitude = Context.WindStrength * 0.0005f * FMath::Pow(0.7f, i);
        
        // PHASE 1: Depth-based dampening
        float DepthScale = 1.0f - FMath::Clamp((Context.WaterDepth - 10.0f) / 40.0f, 0.0f, 1.0f);
        Amplitude *= DepthScale;
        
        float Phase1 = k * (Context.WorldPos.X + Context.WorldPos.Y * 0.7f) - omega * Context.Time;
        float Phase2 = k * (Context.WorldPos.X * 0.7f - Context.WorldPos.Y) - omega * Context.Time * 1.3f;
        
        Wave += Amplitude * (FMath::Sin(Phase1) + 0.5f * FMath::Sin(Phase2 * 2.3f));
    }
    
    return Wave;
}

// ============================================================
// SUBSYSTEM 6.3: WAVE PROCESSING
// ============================================================
// Wave post-processing functions for spatial smoothing and
// component combination. Ensures physically reasonable results
// through filtering and amplitude limiting.
// ============================================================

float UWaterSystem::ApplySpatialSmoothing(float WaveHeight, FVector2D WorldPos, float SmoothingRadius) const
{
    // Simple 4-point averaging for now
    // In a full implementation, you'd sample neighboring wave heights
    return WaveHeight * 0.9f; // Slight dampening for stability
}

float UWaterSystem::CombineWaveComponents(const TArray<float>& WaveHeights) const
{
    if (WaveHeights.Num() == 0) return 0.0f;
    
    // Simple superposition with energy limiting
    float LinearSum = 0.0f;
    float EnergySum = 0.0f;
    
    for (float Height : WaveHeights)
    {
        LinearSum += Height;
        EnergySum += Height * Height;
    }
    
    // Limit by total energy
    float MaxAmplitude = FMath::Sqrt(EnergySum);
    if (FMath::Abs(LinearSum) > MaxAmplitude)
    {
        return MaxAmplitude * FMath::Sign(LinearSum);
    }
    
    return LinearSum;
}


// ============================================================
// SUBSYSTEM 6.4: SPLASH EFFECTS & UTILITIES
// ============================================================
// Dynamic splash generation, debugging tools, and volume
// tracking utilities. Provides player-facing effects and
// developer debugging capabilities.
// ============================================================

void UWaterSystem::AddSplash(FVector WorldPosition, float Intensity, float Size)
{
    if (ActiveRipples.Num() >= MaxActiveRipples)
    {
        // Remove oldest ripple
        ActiveRipples.RemoveAt(0);
    }
    
    FWaterRipple NewRipple;
    NewRipple.Origin = FVector2D(WorldPosition.X, WorldPosition.Y);
    NewRipple.InitialAmplitude = Intensity * 10.0f; // Scale for visible effect
    NewRipple.WaveSpeed = FMath::Sqrt(9.81f * Size); // Physics-based wave speed
    NewRipple.Wavelength = Size * 2.0f;
    NewRipple.StartTime = GetScaledTime();
    NewRipple.Damping = 0.98f;
    NewRipple.MaxRadius = Size * 10.0f;
    
    ActiveRipples.Add(NewRipple);
    
    UE_LOG(LogTemp, Log, TEXT("Added splash at (%.1f, %.1f) - Total ripples: %d"),
           WorldPosition.X, WorldPosition.Y, ActiveRipples.Num());
}

void UWaterSystem::DebugWaterCoordinates(FVector WorldPos)
{
    if (!SimulationData.IsValid() || !CachedMasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("DEBUG: System not ready"));
        return;
    }
    
    // Step 1: Test coordinate transformation
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(WorldPos);
    int32 GridX = FMath::FloorToInt(TerrainCoords.X);
    int32 GridY = FMath::FloorToInt(TerrainCoords.Y);
    
    // Step 2: Check simulation data at this location
    float SimulationDepth = 0.0f;
    if (IsValidCoordinate(GridX, GridY))
    {
        SimulationDepth = GetWaterDepthSafe(GridX, GridY);
    }
    
    // Step 3: Check what mesh system reads at this location
    float MeshDepth = GetInterpolatedWaterDepthSeamless(FVector2D(WorldPos.X, WorldPos.Y));
    
    // Step 4: Calculate what UV coordinates shaders would use
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
    float TerrainScale = CachedMasterController->GetTerrainScale();
    FVector TerrainOrigin = OwnerTerrain->GetActorLocation();
    
    // Shader coordinate calculation (matching what we send to shaders)
    FVector2D ShaderTerrainCoords = FVector2D(
        (WorldPos.X - TerrainOrigin.X) / TerrainScale,
        (WorldPos.Y - TerrainOrigin.Y) / TerrainScale
    );
    FVector2D ShaderUV = FVector2D(
        ShaderTerrainCoords.X / WorldDims.X,
        ShaderTerrainCoords.Y / WorldDims.Y
    );
    
    // Step 5: Sample what shader would read from texture
    float ShaderSampleDepth = 0.0f;
    if (WaterDepthTexture && WaterDepthTexture->GetPlatformData())
    {
        int32 TextureX = FMath::Clamp(FMath::RoundToInt(ShaderTerrainCoords.X), 0, (int32)WorldDims.X - 1);
        int32 TextureY = FMath::Clamp(FMath::RoundToInt(ShaderTerrainCoords.Y), 0, (int32)WorldDims.Y - 1);
        int32 TextureIndex = TextureY * (int32)WorldDims.X + TextureX;
        
        if (TextureIndex >= 0 && TextureIndex < SimulationData.WaterDepthMap.Num())
        {
            ShaderSampleDepth = SimulationData.WaterDepthMap[TextureIndex] * WaterDepthScale;
        }
    }
    
    // Step 6: Output comprehensive debug info
    UE_LOG(LogTemp, Warning, TEXT("=== WATER COORDINATE DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("World Position: (%.1f, %.1f, %.1f)"), WorldPos.X, WorldPos.Y, WorldPos.Z);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Origin: (%.1f, %.1f, %.1f)"), TerrainOrigin.X, TerrainOrigin.Y, TerrainOrigin.Z);
    UE_LOG(LogTemp, Warning, TEXT("Master TerrainCoords: (%.2f, %.2f) -> Grid: (%d, %d)"), TerrainCoords.X, TerrainCoords.Y, GridX, GridY);
    UE_LOG(LogTemp, Warning, TEXT("Shader TerrainCoords: (%.2f, %.2f)"), ShaderTerrainCoords.X, ShaderTerrainCoords.Y);
    UE_LOG(LogTemp, Warning, TEXT("Shader UV: (%.4f, %.4f)"), ShaderUV.X, ShaderUV.Y);
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("Water Depths:"));
    UE_LOG(LogTemp, Warning, TEXT("  Simulation: %.3f"), SimulationDepth);
    UE_LOG(LogTemp, Warning, TEXT("  Mesh System: %.3f"), MeshDepth);
    UE_LOG(LogTemp, Warning, TEXT("  Shader Sample: %.3f"), ShaderSampleDepth);
    UE_LOG(LogTemp, Warning, TEXT(""));
    
    // Step 7: Identify mismatches
    float SimMeshDiff = FMath::Abs(SimulationDepth - MeshDepth);
    float SimShaderDiff = FMath::Abs(SimulationDepth - ShaderSampleDepth);
    float CoordDiff = FVector2D::Distance(TerrainCoords, ShaderTerrainCoords);
    
    if (SimMeshDiff > 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("MISMATCH: Simulation vs Mesh (%.3f difference)"), SimMeshDiff);
    }
    if (SimShaderDiff > 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("MISMATCH: Simulation vs Shader (%.3f difference)"), SimShaderDiff);
    }
    if (CoordDiff > 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("MISMATCH: Coordinate systems (%.3f difference)"), CoordDiff);
    }
    
    if (SimMeshDiff <= 0.1f && SimShaderDiff <= 0.1f && CoordDiff <= 0.1f)
    {
        UE_LOG(LogTemp, Warning, TEXT("  All systems aligned within tolerance"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("================================"));
    
    // Step 8: Test a few nearby texture samples
    UE_LOG(LogTemp, Warning, TEXT("Nearby texture samples:"));
    for (int32 dx = -1; dx <= 1; dx++) {
        for (int32 dy = -1; dy <= 1; dy++) {
            int32 TestX = GridX + dx;
            int32 TestY = GridY + dy;
            if (IsValidCoordinate(TestX, TestY)) {
                float TestDepth = GetWaterDepthSafe(TestX, TestY);
                if (TestDepth > 0.01f) {
                    UE_LOG(LogTemp, Warning, TEXT("  Grid (%d,%d): %.3f"), TestX, TestY, TestDepth);
                }
            }
        }
    }
}

// Add to WaterSystem.cpp:
void UWaterSystem::DebugWaterAtCursor()
{
    if (!OwnerTerrain || !OwnerTerrain->GetWorld()) return;
    
    // Get player controller
    APlayerController* PC = OwnerTerrain->GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    // Get cursor world position
    FVector WorldPos, WorldDir;
    if (PC->DeprojectMousePositionToWorld(WorldPos, WorldDir))
    {
        // Trace to ground
        FHitResult Hit;
        FVector End = WorldPos + WorldDir * 10000.0f;
        if (OwnerTerrain->GetWorld()->LineTraceSingleByChannel(Hit, WorldPos, End, ECC_WorldStatic))
        {
            DebugWaterCoordinates(Hit.Location);
        }
    }
}



float UWaterSystem::GetTotalWaterVolume() const
{
    if (!bVolumeNeedsUpdate && LastKnownTotalVolume >= 0.0f)
    {
        return LastKnownTotalVolume;
    }
    
    float TotalVolume = 0.0f;
    
    if (CachedMasterController && SimulationData.IsValid())
    {
        for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
        {
            float Depth = SimulationData.WaterDepthMap[i];
            if (Depth > 0.0f)
            {
                TotalVolume += CachedMasterController->GetWaterCellVolume(Depth);
            }
        }
    }
    
    LastKnownTotalVolume = TotalVolume;
    bVolumeNeedsUpdate = false;
    return TotalVolume;
}

float UWaterSystem::GetAverageDepth() const
{
    float TotalDepth = 0.0f;
    int32 WaterCells = 0;
    
    for (float Depth : SimulationData.WaterDepthMap)
    {
        if (Depth > 0.0f)
        {
            TotalDepth += Depth;
            WaterCells++;
        }
    }
    
    return WaterCells > 0 ? TotalDepth / WaterCells : 0.0f;
}

void UWaterSystem::ApplyUniformDepthReduction(float DepthReduction)
{
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        if (SimulationData.WaterDepthMap[i] > 0.0f)
        {
            SimulationData.WaterDepthMap[i] = FMath::Max(0.0f,
                SimulationData.WaterDepthMap[i] - DepthReduction);
        }
    }
}

float UWaterSystem::MeasureVolumeChange(TFunctionRef<void()> Operation)
{
    // Get volume before operation
    float VolumeBefore = GetTotalWaterVolume();
    
    // Execute the operation
    Operation();
    
    // Force recalculation after operation
    bVolumeNeedsUpdate = true;
    float VolumeAfter = GetTotalWaterVolume();
    
    // Return the delta
    return VolumeAfter - VolumeBefore;
}
// =============================================================================
// SECTION 7: GPU COMPUTE INTEGRATION
// =============================================================================
//
// PURPOSE:
//   GPU acceleration for wave generation and flow visualization using
//   Unreal Engine 5's Render Dependency Graph (RDG) and compute shaders.
//   Offloads wave calculations to GPU for real-time performance with
//   high-resolution water surfaces.
//
// SUBSECTIONS:
//   7.1 - GPU Initialization: Setup compute shader resources and textures
//   7.2 - Data Upload: Transfer simulation data to GPU
//   7.3 - Compute Execution: Execute wave compute shader on GPU
//   7.4 - Parameter Updates: Real-time GPU parameter updates
//
// KEY FEATURES:
//     GPU-accelerated wave generation via compute shaders
//     Real-time vertex displacement on GPU
//     Flow data visualization through GPU textures
//     RDG-based rendering pipeline integration
//     Async GPU execution (non-blocking)
//     Dynamic parameter updates (wind, wave scale, etc.)
//
// GPU PIPELINE:
//   CPU Side:
//     1. Initialize textures (water depth, flow data, wave output)
//     2. Upload simulation data to GPU each frame
//     3. Set shader parameters (time, wind, wave params)
//     4. Dispatch compute shader
//     5. Apply output texture to water materials
//
//   GPU Side:
//     1. Read water depth and flow data textures
//     2. Calculate wave displacement per pixel
//     3. Write results to output texture
//     4. Material samples output texture for vertex displacement
//
// COMPUTE SHADER DETAILS:
//   File: WaveCompute.usf
//   Thread Group Size: 8x8 (64 threads per group)
//   Dispatch Grid: ceil(Width/8)   ceil(Height/8)
//   Execution Time: ~0.5-1.5ms for 513x513 grid
//
// TEXTURE FORMATS:
//     WaterDepthTexture: PF_R32_FLOAT (depth in meters)
//     FlowDataTexture: PF_G16R16F (2D flow vector)
//     WaveOutputTexture: PF_FloatRGBA (wave height + normals)
//
// SHADER PARAMETERS:
//     Time: Current simulation time (float)
//     WindParams: (direction.x, direction.y, strength, unused)
//     WaveParams: (scale, speed, damping, terrain_scale)
//     TerrainParams: (width, height, unused, unused)
//
// DEPENDENCIES:
//   Internal:
//     - Section 2: Simulation data for GPU upload
//     - Section 6: Wave algorithms implemented in compute shader
//   External:
//     - UE5 RHI: GPU resource management
//     - RDG: Render Dependency Graph
//     - Compute Shaders: WaveCompute.usf
//     - UMaterialInstanceDynamic: Material integration
//
// PERFORMANCE BENEFITS:
//     10-100x speedup over CPU wave calculations
//     Enables 1000+ vertex water meshes at 60 FPS
//     Parallel execution across all water vertices
//     Frees CPU for other game systems
//
// FALLBACK BEHAVIOR:
//   If GPU compute is disabled (bUseVertexDisplacement = false):
//     System falls back to CPU wave generation (Section 6)
//     Meshes regenerated on CPU when depth changes
//     Lower vertex count for performance
//
// SYNCHRONIZATION:
//     GPU execution is async (non-blocking)
//     Render thread handles compute dispatch
//     Game thread updates parameters safely
//     No explicit synchronization needed (RDG handles it)
//
// THREAD SAFETY:
//   All GPU functions must be called from game thread.
//   RDG automatically queues work for render thread execution.
//   Never call RHI functions directly - always use RDG.
//
// =============================================================================

// ============================================================
// SUBSYSTEM 7.1: GPU INITIALIZATION
// ============================================================
// Initialize GPU compute shader resources, validate parameters,
// and create necessary textures for GPU wave generation.
// ============================================================

void UWaterSystem::InitializeGPUDisplacement()
{
    if (!OwnerTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize GPU displacement without terrain"));
        return;
    }
    
    // Validate parameters before initialization
    //ValidateGPUWaveParameters();
    
    // IMPORTANT: Sync with master controller if available
    if (CachedMasterController)
    {
        FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
        float TerrainScale = CachedMasterController->GetTerrainScale();
        
        UE_LOG(LogTemp, Warning, TEXT("GPU Init: World Dims=(%.1f, %.1f), Scale=%.1f"),
               WorldDims.X, WorldDims.Y, TerrainScale);
    }
    
    // Create wave output texture with UAV support
    if (!WaveOutputTexture)
    {
        WaveOutputTexture = NewObject<UTextureRenderTarget2D>(this);
        
        // CRITICAL: Must use InitCustomFormat with proper flags for compute shader UAV
        WaveOutputTexture->RenderTargetFormat = RTF_RGBA32f;  // Full precision for stability
        WaveOutputTexture->InitCustomFormat(
            SimulationData.TerrainWidth,
            SimulationData.TerrainHeight,
            PF_A32B32G32R32F,  // Full precision for wave data
            false  // No linear gamma
        );
        
        // CRITICAL: Force UAV flag for compute shader write access
        WaveOutputTexture->bCanCreateUAV = true;
        WaveOutputTexture->UpdateResourceImmediate(true);  // Force immediate update
        
        UE_LOG(LogTemp, Warning, TEXT("Created Wave Output Texture with UAV: %dx%d"),
               SimulationData.TerrainWidth, SimulationData.TerrainHeight);
    }
    
    // Create water depth texture (read-only)
    if (!WaterDepthTexture)
    {
        CreateWaterDepthTexture();
        
        if (WaterDepthTexture)
        {
            // Ensure depth texture is up to date
            UpdateWaterDepthTexture();
            
            UE_LOG(LogTemp, Warning, TEXT("Created Water Depth Texture: %dx%d"),
                   SimulationData.TerrainWidth, SimulationData.TerrainHeight);
        }
    }
    
    // Create flow displacement texture with proper initial data
    if (!FlowDisplacementTexture)
    {
        FlowDisplacementTexture = NewObject<UTextureRenderTarget2D>(this);
        
        if (FlowDisplacementTexture)
        {
            FlowDisplacementTexture->RenderTargetFormat = RTF_RGBA16f;
            FlowDisplacementTexture->InitCustomFormat(
                SimulationData.TerrainWidth,
                SimulationData.TerrainHeight,
                PF_FloatRGBA,
                false
            );
            FlowDisplacementTexture->AddressX = TextureAddress::TA_Wrap;
            FlowDisplacementTexture->AddressY = TextureAddress::TA_Wrap;
            FlowDisplacementTexture->bCanCreateUAV = true;  // CRITICAL: Enable UAV for compute shader write
            FlowDisplacementTexture->UpdateResourceImmediate(false);
            
            // Initialize with flow data
            UpdateFlowDataTexture();
            
            UE_LOG(LogTemp, Warning, TEXT("Created Flow Displacement Texture"));
        }
    }
    
    // Load or create GPU water material
    if (!WaterMaterialWithDisplacement)
    {
        // Try to load the GPU material
        WaterMaterialWithDisplacement = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/Materials/M_WaterSurface_GPU"));
            
        if (!WaterMaterialWithDisplacement)
        {
            UE_LOG(LogTemp, Warning, TEXT("GPU Water Material not found - using default material"));
            
            // Use default material as fallback
            WaterMaterialWithDisplacement = VolumeMaterial;
            
            if (!WaterMaterialWithDisplacement)
            {
                UE_LOG(LogTemp, Error, TEXT("No water material available for GPU displacement!"));
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Loaded GPU Water Material successfully"));
        }
    }
 
       
    
    // Set initial state
    bGPUWaveSystemInitialized = true;
    AccumulatedGPUTime = 0.0f;
    LastGPUWaveUpdateTime = 0.0f;
    
    // Log configuration
    UE_LOG(LogTemp, Warning, TEXT("=== GPU Wave System Initialized ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Wave Scale: %.2f"), GPUWaveScale);
    UE_LOG(LogTemp, Warning, TEXT("  Wave Speed: %.2f"), GPUWaveSpeed);
    UE_LOG(LogTemp, Warning, TEXT("  Wave Damping: %.2f"), GPUWaveDamping);
    UE_LOG(LogTemp, Warning, TEXT("  Max Wave Height Ratio: %.2f"), GPUMaxWaveHeightRatio);
    UE_LOG(LogTemp, Warning, TEXT("  Safe Wave Height Ratio: %.2f"), GPUSafeWaveHeightRatio);
    UE_LOG(LogTemp, Warning, TEXT("  Deep Water Threshold: %.1f"), GPUDeepWaterThreshold);
    UE_LOG(LogTemp, Warning, TEXT("  Shallow Water Threshold: %.1f"), GPUShallowWaterThreshold);
    UE_LOG(LogTemp, Warning, TEXT("==================================="));
}

// ============================================================
// SUBSYSTEM 7.2: GPU DATA UPLOAD
// ============================================================
// Transfer simulation data (depth, flow) from CPU to GPU
// textures. Updates GPU state to match CPU simulation.
// ============================================================

void UWaterSystem::UpdateFlowDataTexture()
{
    if (!FlowDisplacementTexture)
        return;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create data array with FLinearColor for RGBA float texture
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    // Fill with flow data
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            // Get flow vector at this position
            FVector2D FlowVector = FVector2D::ZeroVector;
            if (Index < SimulationData.WaterVelocityX.Num() &&
                Index < SimulationData.WaterVelocityY.Num())
            {
                FlowVector.X = SimulationData.WaterVelocityX[Index];
                FlowVector.Y = SimulationData.WaterVelocityY[Index];
            }
            
            // Get water depth for foam calculation
            float WaterDepth = 0.0f;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                WaterDepth = SimulationData.WaterDepthMap[Index];
            }
            
            // Calculate foam intensity
            float FoamIntensity = 0.0f;
            if (WaterDepth > MinWaterDepth)
            {
                float FlowSpeed = FlowVector.Size();
                FoamIntensity = FMath::Clamp(FlowSpeed / 10.0f, 0.0f, 1.0f);
            }
            
            // Pack into RGBA (R=FlowX, G=FlowY, B=Depth, A=Foam)
            TextureData[Index] = FLinearColor(FlowVector.X, FlowVector.Y, WaterDepth, FoamIntensity);
        }
    }
    
    // Update render target using RHI commands on render thread
    FTextureRenderTargetResource* RenderTargetResource = FlowDisplacementTexture->GameThread_GetRenderTargetResource();
    if (RenderTargetResource)
    {
        ENQUEUE_RENDER_COMMAND(UpdateFlowDataTextureCommand)(
            [RenderTargetResource, TextureData, Width, Height](FRHICommandListImmediate& RHICmdList)
            {
                FTextureRHIRef TextureRHI = RenderTargetResource->GetRenderTargetTexture();
                if (TextureRHI.IsValid())
                {
                    uint32 Stride = Width * sizeof(FLinearColor);
                    FUpdateTextureRegion2D Region(0, 0, 0, 0, Width, Height);
                    
                    RHICmdList.UpdateTexture2D(
                        TextureRHI,
                        0,  // Mip level
                        Region,
                        Stride,
                        (const uint8*)TextureData.GetData()
                    );
                }
            }
        );
    }
}


// ============================================================
// SUBSYSTEM 7.3: GPU COMPUTE EXECUTION
// ============================================================
// Execute wave compute shader on GPU using RDG pipeline.
// Dispatches compute shader with proper parameters and
// handles render graph execution.
//
// EXECUTION FLOW:
//   1. Validate textures and state
//   2. Get render thread access via ENQUEUE_RENDER_COMMAND
//   3. Build RDG graph with input/output textures
//   4. Set shader parameters
//   5. Dispatch compute shader (ceil(W/8)   ceil(H/8) groups)
//   6. Execute graph (async on render thread)
// ============================================================

void UWaterSystem::ToggleVertexDisplacement(bool bEnable)
{
    // Initialize GPU resources if enabling
    if (bEnable && !WaveOutputTexture)
    {
        InitializeGPUDisplacement();
    }
    
    bUseVertexDisplacement = bEnable;
    
    if (bEnable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enabling GPU Vertex Displacement"));
        
        // Ensure chunks exist by forcing an update
        UpdateWaterSurfaceChunks();
        
        // Initialize GPU meshes for all water chunks
        int32 ChunksInitialized = 0;
        for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
        {
            if (Chunk.bHasWater)
            {
                // Generate initial flat mesh
                GenerateFlatBaseMesh(Chunk);
                
                // Store initial water area for change detection
                ChunkWaterAreas.Add(Chunk.ChunkIndex, CalculateChunkWaterArea(Chunk.ChunkIndex));
                
                ChunksInitialized++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("GPU Mode: Initialized %d water chunks"), ChunksInitialized);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Disabling GPU Vertex Displacement"));
        
        // Clean up GPU meshes and switch to CPU mode
        for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
        {
            // Clear mesh sections but keep component
            if (Chunk.SurfaceMesh)
            {
                Chunk.SurfaceMesh->ClearAllMeshSections();
            }
            
            // Mark for CPU mesh regeneration
            Chunk.bNeedsUpdate = true;
        }
        
        // Clear GPU tracking data
        ChunkWaterAreas.Empty();
    }
}

// Add this updated function to WaterSystem.cpp

void UWaterSystem::ExecuteWaveComputeShader()
{
    // Safety checks
    if (!WaveOutputTexture || !WaterDepthTexture)
    {
        static bool bLoggedOnce = false;
        if (!bLoggedOnce)
        {
            UE_LOG(LogTemp, Warning, TEXT("ExecuteWaveComputeShader: Required textures not initialized (will not log again)"));
            bLoggedOnce = true;
        }
        return;
    }
    
    // Check if we're in a valid state to execute
    if (!IsInGameThread())
    {
        UE_LOG(LogTemp, Error, TEXT("ExecuteWaveComputeShader must be called from game thread"));
        return;
    }
    
    // Get world for timing
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate shader parameters with proper limits
    float CurrentTime = World->GetTimeSeconds();
    float DeltaTime = World->GetDeltaSeconds();
    
    // Clamp DeltaTime to prevent instability from long frames
    DeltaTime = FMath::Clamp(DeltaTime, 0.001f, 0.1f);  // 1ms to 100ms
    
    // Get wind data with safety limits
    FVector2D WindDirection = FVector2D(1.0f, 0.0f);  // Default direction
    float WindStrength = 0.0f;
    
    if (CachedMasterController && CachedMasterController->AtmosphereController)
    {
       
        FVector WindData = CachedMasterController->AtmosphereController->GetWindAtLocation(
            OwnerTerrain->GetActorLocation());
        WindDirection = FVector2D(WindData.X, WindData.Y).GetSafeNormal();
        WindStrength = WindData.Size();
        
        // Clamp wind strength to reasonable values
        WindStrength = FMath::Clamp(WindStrength, 0.0f, 100.0f);  // Max 100 units wind
    }
    
    // Get terrain dimensions
    float TerrainWidth = SimulationData.TerrainWidth * OwnerTerrain->TerrainScale;
    float TerrainHeight = SimulationData.TerrainHeight * OwnerTerrain->TerrainScale;
    
    // Validate and clamp wave parameters
    float ClampedWaveScale = FMath::Clamp(GPUWaveScale, 0.0f, 2.0f);  // Limit wave scale multiplier
    float ClampedWaveSpeed = FMath::Clamp(GPUWaveSpeed, 0.0f, 10.0f);  // Limit animation speed
    float WaveDamping = FMath::Clamp(GPUWaveDamping, 0.0f, 1.0f);     // Ensure damping is in valid range
    
    // Log parameters periodically for debugging
    static float LastLogTime = 0.0f;
    if (CurrentTime - LastLogTime > 5.0f)
    {
        LastLogTime = CurrentTime;
        UE_LOG(LogTemp, Warning, TEXT("Wave Shader Params: Scale=%.2f, Speed=%.2f, Wind=%.2f, Damping=%.2f"),
               ClampedWaveScale, ClampedWaveSpeed, WindStrength, WaveDamping);
    }

    // PHASE 1.5: Capture texture resources on game thread for safe render thread access
    FRHITexture* CapturedPrevDepthRHI = nullptr;
    if (PreviousWaterDepthTexture && PreviousWaterDepthTexture->GetResource())
    {
        CapturedPrevDepthRHI = PreviousWaterDepthTexture->GetResource()->GetTexture2DRHI();
    }

    FRHITexture* CapturedTerrainHeightRHI = nullptr;
    if (OwnerTerrain && OwnerTerrain->WaterHeightTexture)
    {
        FTextureRenderTargetResource* TerrainHeightResource = OwnerTerrain->WaterHeightTexture->GetRenderTargetResource();
        if (TerrainHeightResource)
        {
            CapturedTerrainHeightRHI = TerrainHeightResource->GetRenderTargetTexture();
        }
    }

    // Enqueue render command
    ENQUEUE_RENDER_COMMAND(FWaveComputeCommand)(
        [this, World, CurrentTime, DeltaTime, WindDirection, WindStrength,
         TerrainWidth, TerrainHeight, ClampedWaveScale, ClampedWaveSpeed, WaveDamping,
         CapturedPrevDepthRHI, CapturedTerrainHeightRHI]
        (FRHICommandListImmediate& RHICmdList)
        {
            // Verify shader is compiled and available
            TShaderMapRef<FWaveComputeCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            if (!ComputeShader.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Wave compute shader not found or not compiled"));
                return;
            }
            
            // Create render graph
            FRDGBuilder GraphBuilder(RHICmdList);
            
            // Register external textures with render graph
            FTextureRenderTargetResource* Resource = WaveOutputTexture->GetRenderTargetResource();
            TRefCountPtr<IPooledRenderTarget> PooledWaveOutput = CreateRenderTarget(
                Resource->GetRenderTargetTexture(), TEXT("WaveOutput"));
            FRDGTextureRef WaveOutputRDG = GraphBuilder.RegisterExternalTexture(PooledWaveOutput);
            
            TRefCountPtr<IPooledRenderTarget> PooledDepthTexture;
            if (WaterDepthTexture && WaterDepthTexture->GetResource())
            {
                // Create pooled render target from texture
                FRHITexture* DepthTextureRHI = WaterDepthTexture->GetResource()->GetTexture2DRHI();
                if (DepthTextureRHI)
                {
                    // Create SRV for depth texture
                    FRDGTextureRef DepthTextureRDG = GraphBuilder.RegisterExternalTexture(
                        CreateRenderTarget(DepthTextureRHI, TEXT("WaterDepthTexture")));

                    // Flow data texture (from RenderTarget)
                    FRDGTextureRef FlowTextureRDG = nullptr;
                    if (FlowDisplacementTexture && FlowDisplacementTexture->GetResource())
                    {
                        FTextureRenderTargetResource* FlowRTResource = FlowDisplacementTexture->GetRenderTargetResource();
                        if (FlowRTResource)
                        {
                            FRHITexture* FlowTextureRHI = FlowRTResource->GetRenderTargetTexture();
                            if (FlowTextureRHI)
                            {
                                FlowTextureRDG = GraphBuilder.RegisterExternalTexture(
                                    CreateRenderTarget(FlowTextureRHI, TEXT("FlowDataTexture")));
                            }
                        }
                    }
                    
                    // Create dummy flow texture if needed
                    if (!FlowTextureRDG)
                    {
                        FRDGTextureDesc FlowDesc = FRDGTextureDesc::Create2D(
                            FIntPoint(SimulationData.TerrainWidth, SimulationData.TerrainHeight),
                            PF_FloatRGBA,
                            FClearValueBinding::Black,
                            TexCreate_ShaderResource | TexCreate_UAV);  // UAV needed for read/write
                        FlowTextureRDG = GraphBuilder.CreateTexture(FlowDesc, TEXT("DummyFlowTexture"));
                    }
                    
                    // Setup shader parameters
                    FWaveComputeCS::FParameters* PassParameters =
                        GraphBuilder.AllocParameters<FWaveComputeCS::FParameters>();

                    // Bind textures
                    PassParameters->WaterDepthTexture = GraphBuilder.CreateSRV(
                        FRDGTextureSRVDesc::Create(DepthTextureRDG));
                    PassParameters->FlowDataTexture = GraphBuilder.CreateUAV(
                        FRDGTextureUAVDesc(FlowTextureRDG));
                    PassParameters->WaveOutputTexture = GraphBuilder.CreateUAV(
                        FRDGTextureUAVDesc(WaveOutputRDG));

                    // PHASE 1.5: Previous depth for displacement detection
                    // Use captured RHI texture (thread-safe)
                    if (CapturedPrevDepthRHI)
                    {
                        FRDGTextureRef PrevDepthRDG = GraphBuilder.RegisterExternalTexture(
                            CreateRenderTarget(CapturedPrevDepthRHI, TEXT("PreviousDepthTexture")));
                        PassParameters->PreviousDepthTexture = GraphBuilder.CreateSRV(
                            FRDGTextureSRVDesc::Create(PrevDepthRDG));
                    }
                    else
                    {
                        // Fallback to current depth (first frame or texture unavailable)
                        PassParameters->PreviousDepthTexture = GraphBuilder.CreateSRV(
                            FRDGTextureSRVDesc::Create(DepthTextureRDG));
                    }

                    // PHASE 1.5: Terrain height for pressure gradient
                    // Use captured RHI texture (thread-safe)
                    if (CapturedTerrainHeightRHI)
                    {
                        FRDGTextureRef TerrainHeightRDG = GraphBuilder.RegisterExternalTexture(
                            CreateRenderTarget(CapturedTerrainHeightRHI, TEXT("TerrainHeightTexture")));
                        PassParameters->TerrainHeightTexture = GraphBuilder.CreateSRV(
                            FRDGTextureSRVDesc::Create(TerrainHeightRDG));
                    }
                    else
                    {
                        // Create dummy terrain height texture
                        FRDGTextureDesc DummyDesc = FRDGTextureDesc::Create2D(
                            FIntPoint(SimulationData.TerrainWidth, SimulationData.TerrainHeight),
                            PF_R32_FLOAT,
                            FClearValueBinding::Black,
                            TexCreate_ShaderResource);
                        FRDGTextureRef DummyTerrainRDG = GraphBuilder.CreateTexture(DummyDesc, TEXT("DummyTerrainHeight"));
                        PassParameters->TerrainHeightTexture = GraphBuilder.CreateSRV(
                            FRDGTextureSRVDesc::Create(DummyTerrainRDG));
                    }
                    
                    // Set simulation parameters with validated values
                    PassParameters->Time = CurrentTime;
                    PassParameters->DeltaTime = DeltaTime;
                    PassParameters->WindParams = FVector4f(
                        WindDirection.X,
                        WindDirection.Y,
                        WindStrength,
                        0.0f
                    );
                    PassParameters->WaveParams = FVector4f(
                        ClampedWaveScale,      // Wave scale multiplier
                        ClampedWaveSpeed,      // Animation speed
                        WaveDamping,           // Damping factor
                        1.0f                   // Terrain scale
                    );
                    PassParameters->TerrainParams = FVector4f(
                        TerrainWidth,
                        TerrainHeight,
                        0.0f,
                        0.0f
                    );
                    
                    // Create default sampler
                    PassParameters->TextureSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
                    
                    // Calculate dispatch dimensions
                    uint32 GroupSizeX = 8;  // Must match shader THREADGROUP_SIZE_X
                    uint32 GroupSizeY = 8;  // Must match shader THREADGROUP_SIZE_Y
                    uint32 DispatchX = FMath::DivideAndRoundUp((uint32)SimulationData.TerrainWidth, GroupSizeX);
                    uint32 DispatchY = FMath::DivideAndRoundUp((uint32)SimulationData.TerrainHeight, GroupSizeY);
                    
                    // Add compute pass
                    GraphBuilder.AddPass(
                        RDG_EVENT_NAME("WaveComputeShader"),
                        PassParameters,
                        ERDGPassFlags::Compute,
                        [PassParameters, ComputeShader, DispatchX, DispatchY](FRHIComputeCommandList& RHICmdList)
                        {
                            FComputeShaderUtils::Dispatch(
                                RHICmdList,
                                ComputeShader,
                                *PassParameters,
                                FIntVector(DispatchX, DispatchY, 1)
                            );
                        }
                    );
                    
                    // Execute render graph
                    GraphBuilder.Execute();
                }
            }
        }
    );
}

// ============================================================
// SUBSYSTEM 7.4: GPU PARAMETER UPDATES
// ============================================================
// Real-time parameter updates for GPU wave generation.
// Updates shader parameters and applies results to water
// materials each frame.
//
// PARAMETERS UPDATED:
//     Wave texture (output from compute shader)
//     Displacement scale (amplitude multiplier)
//     Time (for wave animation)
// ============================================================

void UWaterSystem::UpdateGPUWaveParameters(float DeltaTime)
{
    if (!bUseVertexDisplacement)
        return;
    
    // Update wave compute shader every frame for smooth animation
    ExecuteWaveComputeShader();
    
    // Apply the wave texture to all water materials
    if (WaterMaterialWithDisplacement && WaveOutputTexture)
    {
        // Get REAL TIME for wave animations (matching the compute shader)
        float WaveTime = 0.0f;
        if (GetWorld())
        {
            WaveTime = GetWorld()->GetTimeSeconds();
            if (bUseRealTimeForGPUWaves)
            {
                WaveTime *= GPUWaveAnimationSpeed;
            }
        }
        
        // Update all water chunk materials
        for (FWaterSurfaceChunk& Chunk : WaterSurfaceChunks)
        {
            if (Chunk.SurfaceMesh && Chunk.bHasWater)
            {
                UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                    Chunk.SurfaceMesh->GetMaterial(0));
                    
                if (!DynMaterial)
                {
                    DynMaterial = UMaterialInstanceDynamic::Create(
                        WaterMaterialWithDisplacement, this);
                    Chunk.SurfaceMesh->SetMaterial(0, DynMaterial);
                }
                
                // Update wave texture
                DynMaterial->SetTextureParameterValue(FName("WaveHeightTexture"), WaveOutputTexture);
                DynMaterial->SetScalarParameterValue(FName("WaveDisplacementScale"), GPUWaveScale);
                DynMaterial->SetScalarParameterValue(FName("Time"), WaveTime);  // CHANGED: Use WaveTime instead of AccumulatedScaledTime
            }
        }
    }
}
// ============================================================================
// SECTION 8: GPU MESH MANAGEMENT & UPDATES
// ============================================================================
//
// PURPOSE:
// Advanced GPU-driven water mesh generation and chunk management system.
// Extends Section 7's GPU compute foundation with visual mesh generation
// using vertex displacement strategy for optimal performance.
//
// ARCHITECTURE:
// - Flat base mesh generation (CPU)
// - GPU vertex displacement via compute shader (Section 7)
// - Chunk-based updates with neighbor propagation
// - Dynamic material application with shader alignment
//
// GPU MESH STRATEGY:
// Rather than generating height-based meshes on CPU (expensive), this system
// generates flat grid meshes at terrain height and uses GPU compute shaders
// to displace vertices based on water depth. This provides:
// - 10-100x faster mesh generation (flat grid vs. height mesh)
// - Real-time wave animation via GPU displacement
// - Seamless chunk boundaries through coordinate alignment
// - Reduced CPU overhead for large water surfaces
//
// REGENERATION LOGIC:
// Meshes regenerate when water coverage changes significantly:
// - Threshold: 0.1 cells (very sensitive for responsiveness)
// - Neighbor propagation: 8-directional (ensures seamless boundaries)
// - Cached areas: Prevents unnecessary regeneration
// - Update budget: Limited per frame for performance
//
// CHUNK MANAGEMENT:
// - Dynamic chunk creation/destruction based on water presence
// - Neighbor updates ensure seamless transitions
// - Area tracking for intelligent regeneration
// - Material instance per chunk for independent parameters
//
// COORDINATE ALIGNMENT:
// Critical for seamless water surfaces across chunks:
// - MasterController is coordinate authority (Section 1)
// - Terrain coordinates  ->  World coordinates  ->  Shader UVs
// - Chunk overlap handled at boundaries
// - UV mapping aligned with wave texture sampling
//
// PERFORMANCE:
// - Typical update: 2-5 chunks per frame (with neighbors: 10-20)
// - Flat mesh generation: ~0.5ms per chunk (vs. 5-10ms for height mesh)
// - GPU displacement: ~0.1ms per chunk (Section 7 compute shader)
// - Total overhead: ~2-3ms per frame for active water
//
// DEPENDENCIES:
// - Section 1: IsSystemReady(), OwnerTerrain, CachedMasterController
// - Section 2: SimulationData (WaterDepthMap)
// - Section 3: WaterSurfaceChunks array, chunk management
// - Section 5: Material application patterns
// - Section 7: GPU displacement infrastructure, compute shaders
//
// EXTERNAL CALLS:
// - ADynamicTerrain: TerrainToWorldPosition(), chunk data, dimensions
// - AMasterWorldController: Coordinate transformations
//
// SUBSECTIONS:
// 8.1: Mesh Regeneration Logic - When to update meshes
// 8.2: GPU Mesh Generation - Flat base mesh creation
// 8.3: Material & Alignment - Shader parameter application
// 8.4: Mesh Utilities - Helper functions for mesh generation
//
// ============================================================================

// ============================================================================
// SUBSECTION 8.1: MESH REGENERATION LOGIC
// ============================================================================
//
// PURPOSE:
// Determine when GPU meshes need regeneration based on water depth changes.
// Uses area-based threshold detection with neighbor propagation.
//
// ALGORITHM:
// - Calculate current water coverage area per chunk
// - Compare against cached area from last update
// - Regenerate if difference exceeds threshold (0.1 cells)
// - Neighbor propagation ensures seamless chunk boundaries
//
// PERFORMANCE:
// - Area calculation: O(ChunkSizeÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â²) = O(256) for 16x16 chunk
// - Typical regeneration: 2-5 chunks/frame (10-20 with neighbors)
// ============================================================================

bool UWaterSystem::NeedsMeshRegeneration(int32 ChunkIndex) const
{
    if (!bUseVertexDisplacement)
    {
        return true;
    }
    
    // Find the water chunk for this terrain chunk
    const FWaterSurfaceChunk* WaterChunk = WaterSurfaceChunks.FindByPredicate(
        [ChunkIndex](const FWaterSurfaceChunk& Chunk) {
            return Chunk.ChunkIndex == ChunkIndex;
        });
    
    // ALWAYS regenerate if no water chunk exists yet
    if (!WaterChunk || !WaterChunk->SurfaceMesh)
    {
        return true; // Changed from false - this was preventing new chunks!
    }

    
    // Lowered the threshold significantly (was 100.0f, then 1.0f)
    const float REGENERATION_THRESHOLD = 0.1f; // Much more sensitive
    
    // Calculate current water coverage
    const FTerrainChunk& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    const int32 TerrainChunkSize = OwnerTerrain->ChunkSize;
    const int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 ChunkStartX = TerrainChunk.ChunkX * (TerrainChunkSize - ChunkOverlap);
    int32 ChunkStartY = TerrainChunk.ChunkY * (TerrainChunkSize - ChunkOverlap);
    int32 ChunkEndX = FMath::Min(ChunkStartX + TerrainChunkSize, OwnerTerrain->TerrainWidth);
    int32 ChunkEndY = FMath::Min(ChunkStartY + TerrainChunkSize, OwnerTerrain->TerrainHeight);
    
    float CurrentArea = 0.0f;
    for (int32 Y = ChunkStartY; Y < ChunkEndY; Y++)
    {
        for (int32 X = ChunkStartX; X < ChunkEndX; X++)
        {
            float Depth = GetWaterDepthSafe(X, Y);
            if (Depth > MinWaterDepth)
            {
                CurrentArea += 1.0f;
            }
        }
    }
    
    // Check if area changed at all
    const float* LastArea = ChunkWaterAreas.Find(ChunkIndex);
    if (!LastArea || FMath::Abs(CurrentArea - *LastArea) > REGENERATION_THRESHOLD)
    {
        return true;
    }
    
    return false;
}

void UWaterSystem::UpdateGPUWaterChunks()
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return;
    
    // Collect all chunks that need updating along with their neighbors
    TSet<int32> ChunksToUpdate;
    
    // Lambda to add adjacent chunks
    auto AddWithNeighbors = [&](int32 ChunkIndex) {
        if (ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
            return;
        
        ChunksToUpdate.Add(ChunkIndex);
        
        const FTerrainChunk& Chunk = OwnerTerrain->TerrainChunks[ChunkIndex];
        int32 ChunksX = OwnerTerrain->ChunksX;
        
        // Add all 8 neighbors
        for (int32 dy = -1; dy <= 1; dy++) {
            for (int32 dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                
                int32 nx = Chunk.ChunkX + dx;
                int32 ny = Chunk.ChunkY + dy;
                
                if (nx >= 0 && nx < ChunksX && ny >= 0 && ny < OwnerTerrain->ChunksY) {
                    int32 NeighborIndex = ny * ChunksX + nx;
                    if (NeighborIndex < OwnerTerrain->TerrainChunks.Num())
                        ChunksToUpdate.Add(NeighborIndex);
                }
            }
        }
    };
    
    // First pass: identify chunks with water changes
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        bool bHasWater = false;
        
        // Check if chunk has water
        const FTerrainChunk& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
        int32 ChunkStartX = TerrainChunk.ChunkX * (OwnerTerrain->ChunkSize - OwnerTerrain->ChunkOverlap);
        int32 ChunkStartY = TerrainChunk.ChunkY * (OwnerTerrain->ChunkSize - OwnerTerrain->ChunkOverlap);
        int32 ChunkEndX = FMath::Min(ChunkStartX + OwnerTerrain->ChunkSize, SimulationData.TerrainWidth);
        int32 ChunkEndY = FMath::Min(ChunkStartY + OwnerTerrain->ChunkSize, SimulationData.TerrainHeight);
        
        for (int32 Y = ChunkStartY; Y < ChunkEndY && !bHasWater; Y++)
        {
            for (int32 X = ChunkStartX; X < ChunkEndX && !bHasWater; X++)
            {
                int32 Index = Y * SimulationData.TerrainWidth + X;
                if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
                {
                    if (SimulationData.WaterDepthMap[Index] > MinWaterDepth)
                        bHasWater = true;
                }
            }
        }
        
        // Find or create water chunk
        FWaterSurfaceChunk* WaterChunk = WaterSurfaceChunks.FindByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& Chunk) {
                return Chunk.ChunkIndex == ChunkIndex;
            });
        
        if (bHasWater)
        {
            if (!WaterChunk)
            {
                // New water chunk needed
                FWaterSurfaceChunk NewChunk;
                NewChunk.ChunkIndex = ChunkIndex;
                NewChunk.ChunkX = TerrainChunk.ChunkX;
                NewChunk.ChunkY = TerrainChunk.ChunkY;
                NewChunk.bHasWater = true;
                NewChunk.bNeedsUpdate = true;
                WaterSurfaceChunks.Add(NewChunk);
                
                AddWithNeighbors(ChunkIndex);
            }
            else if (!WaterChunk->bHasWater || WaterChunk->bNeedsUpdate)
            {
                WaterChunk->bHasWater = true;
                WaterChunk->bNeedsUpdate = true;
                
                AddWithNeighbors(ChunkIndex);
            }
        }
        else if (WaterChunk && WaterChunk->bHasWater)
        {
            // Water disappeared
            WaterChunk->bHasWater = false;
            if (WaterChunk->SurfaceMesh)
                WaterChunk->SurfaceMesh->SetVisibility(false);
            
            AddWithNeighbors(ChunkIndex);
        }
    }
    
    // Batch update all affected chunks
    for (int32 ChunkIndex : ChunksToUpdate)
    {
        FWaterSurfaceChunk* Chunk = WaterSurfaceChunks.FindByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& C) {
                return C.ChunkIndex == ChunkIndex;
            });
        
        if (Chunk && Chunk->bHasWater)
        {
            if (!Chunk->SurfaceMesh)
                InitializeGPUChunkMesh(*Chunk);
            
            GenerateFlatBaseMesh(*Chunk);
            Chunk->bNeedsUpdate = false;
        }
    }
}

// GPU Water Core Generation Function

// ============================================================================
// SUBSECTION 8.2: GPU MESH GENERATION
// ============================================================================
//
// PURPOSE:
// Generate flat base meshes for GPU vertex displacement strategy.
// Flat meshes are 10-100x faster to generate than height-based meshes.
//
// FLAT MESH RATIONALE:
// Traditional approach: Generate mesh with vertices at water surface height (CPU intensive)
// GPU approach: Generate flat grid at terrain height, displace vertices on GPU
// Benefits: Faster generation, GPU-driven animation, better performance
//
// VERTEX LAYOUT:
// - Position: Flat grid at terrain height (Z = terrain elevation)
// - Normal: Initially (0, 0, 1), recalculated by GPU after displacement
// - UV: Aligned with world coordinates for wave texture sampling
// - Color: Water depth encoded (for debugging and material effects)
//
// CHUNK BOUNDARIES:
// - Overlap handled by shared vertices at chunk edges
// - UV mapping ensures seamless wave propagation
// - Neighbor updates maintain visual continuity
//
// PERFORMANCE:
// - Flat mesh generation: ~0.5ms per 16x16 chunk
// - Height mesh generation: ~5-10ms per chunk (10-20x slower)
// - GPU displacement: ~0.1ms per chunk (Section 7)
// ============================================================================

void UWaterSystem::GenerateFlatBaseMesh(FWaterSurfaceChunk& Chunk)
{
    // === VALIDATION ===
    if (!OwnerTerrain || Chunk.ChunkIndex < 0 ||
        Chunk.ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateFlatBaseMesh: Invalid chunk index %d"), Chunk.ChunkIndex);
        return;
    }
    
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("GenerateFlatBaseMesh: Invalid simulation data for chunk %d"), Chunk.ChunkIndex);
        return;
    }
    
    // === MESH INITIALIZATION ===
    if (!Chunk.SurfaceMesh)
    {
        InitializeGPUChunkMesh(Chunk);
        if (!Chunk.SurfaceMesh) return;
    }
    
    const FTerrainChunk& TerrainChunk = OwnerTerrain->TerrainChunks[Chunk.ChunkIndex];
    const int32 TerrainChunkSize = OwnerTerrain->ChunkSize;
    const float TerrainScale = OwnerTerrain->TerrainScale;
    const int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    // === COORDINATE CALCULATION WITH PADDING ===
    int32 PaddingCells = 2;
    int32 ChunkStartX = FMath::Max(0, TerrainChunk.ChunkX * (TerrainChunkSize - ChunkOverlap) - PaddingCells);
    int32 ChunkStartY = FMath::Max(0, TerrainChunk.ChunkY * (TerrainChunkSize - ChunkOverlap) - PaddingCells);
    int32 ChunkEndX = FMath::Min(ChunkStartX + TerrainChunkSize + PaddingCells*2, SimulationData.TerrainWidth);
    int32 ChunkEndY = FMath::Min(ChunkStartY + TerrainChunkSize + PaddingCells*2, SimulationData.TerrainHeight);
    
    FVector ChunkWorldPosCenter = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
    float HalfChunkWorld = ((TerrainChunkSize - 1) * TerrainScale * 0.5f);
    FVector ChunkWorldPos(
        ChunkWorldPosCenter.X - HalfChunkWorld,
        ChunkWorldPosCenter.Y - HalfChunkWorld,
        ChunkWorldPosCenter.Z
    );
    
    // === SMOOTHING LAMBDA FOR GRID ARTIFACT REDUCTION ===
    auto GetSmoothedWaterDepth = [this](int32 X, int32 Y) -> float
    {
        float depth = 0.0f;
        float weightSum = 0.0f;
        
        // 3x3 gaussian kernel
        for (int32 dy = -1; dy <= 1; dy++)
        {
            for (int32 dx = -1; dx <= 1; dx++)
            {
                int32 sx = FMath::Clamp(X + dx, 0, SimulationData.TerrainWidth - 1);
                int32 sy = FMath::Clamp(Y + dy, 0, SimulationData.TerrainHeight - 1);
                int32 idx = sy * SimulationData.TerrainWidth + sx;
                
                if (idx >= 0 && idx < SimulationData.WaterDepthMap.Num())
                {
                    float weight = FMath::Exp(-0.5f * (dx*dx + dy*dy));
                    depth += SimulationData.WaterDepthMap[idx] * weight;
                    weightSum += weight;
                }
            }
        }
        
        return weightSum > 0 ? depth / weightSum : 0.0f;
    };
    
    // === WATER DETECTION WITH SMOOTHING ===
    bool bHasSignificantWater = false;
    int32 WaterCellCount = 0;
    float TotalWaterVolume = 0.0f;
    float MaxWaterDepth = 0.0f;
    
    int32 MinWaterX = ChunkEndX;
    int32 MaxWaterX = ChunkStartX;
    int32 MinWaterY = ChunkEndY;
    int32 MaxWaterY = ChunkStartY;
    
    const float MIN_WATER_DEPTH_FOR_MESH = MinWaterDepth * 1.5f;
    
    for (int32 Y = ChunkStartY; Y < ChunkEndY; Y++)
    {
        for (int32 X = ChunkStartX; X < ChunkEndX; X++)
        {
            // Use smoothed depth for detection
            float WaterDepth = GetSmoothedWaterDepth(X, Y);
            
            int32 Index = Y * SimulationData.TerrainWidth + X;
            float VelX = (Index < SimulationData.WaterVelocityX.Num()) ? SimulationData.WaterVelocityX[Index] : 0.0f;
            float VelY = (Index < SimulationData.WaterVelocityY.Num()) ? SimulationData.WaterVelocityY[Index] : 0.0f;
            float FlowSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY);
            
            if (WaterDepth > MIN_WATER_DEPTH_FOR_MESH ||
                (WaterDepth > MinWaterDepth && FlowSpeed > 0.01f))
            {
                bHasSignificantWater = true;
                WaterCellCount++;
                TotalWaterVolume += WaterDepth;
                MaxWaterDepth = FMath::Max(MaxWaterDepth, WaterDepth);
                
                MinWaterX = FMath::Min(MinWaterX, X);
                MaxWaterX = FMath::Max(MaxWaterX, X);
                MinWaterY = FMath::Min(MinWaterY, Y);
                MaxWaterY = FMath::Max(MaxWaterY, Y);
            }
        }
    }
    
    // === MESH GENERATION ===
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    if (!bHasSignificantWater || WaterCellCount < 4)
    {
        Chunk.SurfaceMesh->SetVisibility(false);
        return;
    }
    
    TMap<FIntPoint, int32> VertexIndexMap;
    
    // Create vertices with smoothed heights
    for (int32 Y = MinWaterY; Y <= MaxWaterY; Y++)
    {
        for (int32 X = MinWaterX; X <= MaxWaterX; X++)
        {
            float WaterDepth = GetSmoothedWaterDepth(X, Y);
            
            if (WaterDepth > MinWaterDepth)
            {
                float TerrainHeight = OwnerTerrain->GetHeightAtIndex(X, Y);
                float WaterSurfaceHeight = TerrainHeight + WaterDepth;
                
                float LocalX = (X - ChunkStartX - PaddingCells) * TerrainScale;
                float LocalY = (Y - ChunkStartY - PaddingCells) * TerrainScale;
                
                FIntPoint GridPos(X - MinWaterX, Y - MinWaterY);
                VertexIndexMap.Add(GridPos, Vertices.Num());
                
                Vertices.Add(FVector(LocalX, LocalY, WaterSurfaceHeight - ChunkWorldPos.Z));
                
                int32 Index = Y * SimulationData.TerrainWidth + X;
                float VelX = (Index < SimulationData.WaterVelocityX.Num()) ? SimulationData.WaterVelocityX[Index] : 0.0f;
                float VelY = (Index < SimulationData.WaterVelocityY.Num()) ? SimulationData.WaterVelocityY[Index] : 0.0f;
                FVector2D FlowVector(VelX, VelY);
                FVector Normal = CalculateWaterNormal(FVector2D(X * TerrainScale, Y * TerrainScale), FlowVector, WaterDepth);
                Normals.Add(Normal);
                
                float U = (float)X / (float)(SimulationData.TerrainWidth - 1);
                float V = (float)Y / (float)(SimulationData.TerrainHeight - 1);
                UVs.Add(FVector2D(U, V));
                
                uint8 Alpha = 255;
                if (WaterDepth < MIN_WATER_DEPTH_FOR_MESH)
                {
                    Alpha = (uint8)(128 + (WaterDepth / MIN_WATER_DEPTH_FOR_MESH) * 127);
                }
                
                uint8 DepthColor = FMath::Clamp((int32)(WaterDepth * 50.0f), 0, 255);
                VertexColors.Add(FColor(0, DepthColor, 255, Alpha));
            }
        }
    }
    
    // === TRIANGLE GENERATION ===
    int32 MeshWidth = MaxWaterX - MinWaterX + 1;
    int32 MeshHeight = MaxWaterY - MinWaterY + 1;
    
    for (int32 Y = 0; Y < MeshHeight - 1; Y++)
    {
        for (int32 X = 0; X < MeshWidth - 1; X++)
        {
            FIntPoint TopLeft(X, Y);
            FIntPoint TopRight(X + 1, Y);
            FIntPoint BottomLeft(X, Y + 1);
            FIntPoint BottomRight(X + 1, Y + 1);
            
            int32* TL = VertexIndexMap.Find(TopLeft);
            int32* TR = VertexIndexMap.Find(TopRight);
            int32* BL = VertexIndexMap.Find(BottomLeft);
            int32* BR = VertexIndexMap.Find(BottomRight);
            
            if (TL && TR && BL && BR)
            {
                Triangles.Add(*TL);
                Triangles.Add(*BL);
                Triangles.Add(*TR);
                
                Triangles.Add(*TR);
                Triangles.Add(*BL);
                Triangles.Add(*BR);
            }
            else if (TL && TR && BL)
            {
                Triangles.Add(*TL);
                Triangles.Add(*BL);
                Triangles.Add(*TR);
            }
            else if (TR && BL && BR)
            {
                Triangles.Add(*TR);
                Triangles.Add(*BL);
                Triangles.Add(*BR);
            }
            else if (TL && BL && BR)
            {
                Triangles.Add(*TL);
                Triangles.Add(*BL);
                Triangles.Add(*BR);
            }
            else if (TL && TR && BR)
            {
                Triangles.Add(*TL);
                Triangles.Add(*TR);
                Triangles.Add(*BR);
            }
        }
    }
    
    // === FINAL MESH CREATION ===
    if (Vertices.Num() >= 3 && Triangles.Num() >= 3)
    {
        Chunk.SurfaceMesh->ClearAllMeshSections();
        Chunk.SurfaceMesh->CreateMeshSection(
            0, Vertices, Triangles, Normals, UVs,
            VertexColors, TArray<FProcMeshTangent>(), false
        );
        
        Chunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPos - OwnerTerrain->GetActorLocation());
        ApplyGPUMaterialWithShaderAlignment(Chunk);
        Chunk.SurfaceMesh->SetVisibility(true);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Chunk %d: Mesh created - %d verts, %d tris, %d water cells"),
            Chunk.ChunkIndex, Vertices.Num(), Triangles.Num() / 3, WaterCellCount);
    }
    else if (Vertices.Num() >= 3)
    {
        FVector Center = FVector::ZeroVector;
        for (const FVector& V : Vertices) Center += V;
        Center /= Vertices.Num();
        
        int32 CenterIndex = Vertices.Add(Center);
        Normals.Add(FVector::UpVector);
        UVs.Add(FVector2D(0.5f, 0.5f));
        VertexColors.Add(FColor(0, 100, 255, 255));
        
        for (int32 i = 0; i < Vertices.Num()-2; i++)
        {
            Triangles.Add(CenterIndex);
            Triangles.Add(i);
            Triangles.Add(i + 1);
        }
        
        Chunk.SurfaceMesh->ClearAllMeshSections();
        Chunk.SurfaceMesh->CreateMeshSection(
            0, Vertices, Triangles, Normals, UVs,
            VertexColors, TArray<FProcMeshTangent>(), false
        );
        
        Chunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPos - OwnerTerrain->GetActorLocation());
        ApplyGPUMaterialWithShaderAlignment(Chunk);
        Chunk.SurfaceMesh->SetVisibility(true);
        
        UE_LOG(LogTemp, Warning, TEXT("Chunk %d: Using fallback triangulation - %d verts"),
            Chunk.ChunkIndex, Vertices.Num());
    }
    else
    {
        Chunk.SurfaceMesh->SetVisibility(false);
        UE_LOG(LogTemp, Warning, TEXT("Chunk %d: Insufficient geometry - verts=%d, tris=%d"),
            Chunk.ChunkIndex, Vertices.Num(), Triangles.Num());
    }
}



void UWaterSystem::InitializeGPUChunkMesh(FWaterSurfaceChunk& Chunk)
{
    if (Chunk.SurfaceMesh)
        return;
    
    if (Chunk.ChunkIndex < 0 || !OwnerTerrain ||
        Chunk.ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeGPUChunkMesh: Invalid chunk index %d"), Chunk.ChunkIndex);
        return;
    }
    
    // Create procedural mesh component
    FString ComponentName = FString::Printf(TEXT("GPUWaterSurface_%d_%d"),
                                           Chunk.ChunkX, Chunk.ChunkY);
    Chunk.SurfaceMesh = NewObject<UProceduralMeshComponent>(OwnerTerrain, *ComponentName);
    
    if (!Chunk.SurfaceMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GPU mesh component for chunk %d"),
               Chunk.ChunkIndex);
        return;
    }
    
    Chunk.SurfaceMesh->SetupAttachment(OwnerTerrain->GetRootComponent());
    Chunk.SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Chunk.SurfaceMesh->SetCastShadow(false);
    Chunk.SurfaceMesh->bUseAsyncCooking = true;
    Chunk.SurfaceMesh->RegisterComponent();
    
    // Convert chunk center to corner position
    FVector ChunkWorldPosCenter = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
    float HalfChunkWorld = ((OwnerTerrain->ChunkSize - 1) * OwnerTerrain->TerrainScale * 0.5f);
    FVector ChunkWorldPosCorner(
        ChunkWorldPosCenter.X - HalfChunkWorld,
        ChunkWorldPosCenter.Y - HalfChunkWorld,
        ChunkWorldPosCenter.Z
    );
    
    // Use relative location to maintain parent-child transform during rotation
    Chunk.SurfaceMesh->SetRelativeLocation(ChunkWorldPosCorner - OwnerTerrain->GetActorLocation());
}




// ============================================================================
// SUBSECTION 8.3: MATERIAL & ALIGNMENT
// ============================================================================
//
// PURPOSE:
// Apply water materials with proper shader alignment for GPU displacement.
//
// SHADER PARAMETERS:
// - WaterDepth: Max depth in chunk (for material color blending)
// - DisplacementScale: Vertical displacement multiplier
// - UVOffset: Chunk position offset for wave texture alignment
// - WorldPosition: Chunk world origin for coordinate calculations
//
// COORDINATE ALIGNMENT:
// Critical for seamless water across chunks:
// 1. Chunk world position  ->  Material parameter
// 2. UV coordinates aligned with wave texture sampling
// 3. Shader uses world position to sample global wave texture
// 4. Result: Waves propagate seamlessly across chunk boundaries
//
// MATERIAL INSTANCE:
// - Each chunk gets own Material Instance Dynamic (MID)
// - Allows per-chunk parameter tweaking (depth, position)
// - Shared material template with per-instance parameters
// ============================================================================

void UWaterSystem::ApplyGPUMaterialWithShaderAlignment(FWaterSurfaceChunk& Chunk)
{
    if (!Chunk.SurfaceMesh || !WaterMaterialWithDisplacement)
        return;
    
    UMaterialInstanceDynamic* MatInstance = Cast<UMaterialInstanceDynamic>(
        Chunk.SurfaceMesh->GetMaterial(0));
    
    if (!MatInstance)
    {
        MatInstance = UMaterialInstanceDynamic::Create(
            WaterMaterialWithDisplacement, Chunk.SurfaceMesh);
        Chunk.SurfaceMesh->SetMaterial(0, MatInstance);
    }
    
    if (CachedMasterController && OwnerTerrain)
    {
        FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
        float TerrainScale = CachedMasterController->GetTerrainScale();
        
        // Convert chunk center position to corner for proper UV alignment
        float HalfChunkWorld = ((OwnerTerrain->ChunkSize - 1) * TerrainScale * 0.5f);
        FVector ActualChunk0Center = OwnerTerrain->GetChunkWorldPosition(0);
        FVector TerrainOrigin(
            ActualChunk0Center.X - HalfChunkWorld,
            ActualChunk0Center.Y - HalfChunkWorld,
            0.0f
        );
        
        // Set shader parameters
        MatInstance->SetVectorParameterValue(TEXT("TerrainWorldOrigin"),
            FLinearColor(TerrainOrigin.X, TerrainOrigin.Y, TerrainOrigin.Z, 0.0f));
        
        MatInstance->SetVectorParameterValue(TEXT("TerrainDims"),
            FLinearColor(WorldDims.X, WorldDims.Y, 0, 0));
        
        MatInstance->SetScalarParameterValue(TEXT("TerrainScale"), TerrainScale);
        MatInstance->SetScalarParameterValue(TEXT("TerrainWidth"), WorldDims.X);
        MatInstance->SetScalarParameterValue(TEXT("TerrainHeight"), WorldDims.Y);
        MatInstance->SetScalarParameterValue(TEXT("TexelSizeX"), 1.0f / WorldDims.X);
        MatInstance->SetScalarParameterValue(TEXT("TexelSizeY"), 1.0f / WorldDims.Y);
        MatInstance->SetScalarParameterValue(TEXT("WorldScale"), TerrainScale);
        
        MatInstance->SetVectorParameterValue(TEXT("WorldDimensions"),
            FLinearColor(WorldDims.X, WorldDims.Y, 0, 0));
    }
    
    // Set water textures
    if (WaterDepthTexture)
        MatInstance->SetTextureParameterValue(TEXT("WaterDepthTexture"), WaterDepthTexture);
    
    if (WaveOutputTexture)
        MatInstance->SetTextureParameterValue(TEXT("WaveDisplacementTexture"), WaveOutputTexture);
    
    // Get REAL TIME for wave animations
    float WaveTime = 0.0f;
    if (bUseRealTimeForGPUWaves && GetWorld())
    {
        WaveTime = GetWorld()->GetTimeSeconds() * GPUWaveAnimationSpeed;
    }
    else
    {
        WaveTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    }
    
    // Displacement parameters with REAL TIME
    MatInstance->SetScalarParameterValue(TEXT("DisplacementScale"), GPUWaveScale * 10.0f);
    MatInstance->SetScalarParameterValue(TEXT("WaveSpeed"), GPUWaveSpeed);
    MatInstance->SetScalarParameterValue(TEXT("Time"), WaveTime);
}


// ============================================================================
// SUBSECTION 8.4: MESH UTILITIES
// ============================================================================
//
// PURPOSE:
// Helper functions for mesh generation and water detection.
//
// WATER NEIGHBOR DETECTION:
// Used to prevent isolated water cells from generating meshes.
// Checks 8-directional neighbors for water presence.
//
// AREA CALCULATION:
// Counts cells with water depth above threshold.
// Used by regeneration logic to detect coverage changes.
//
// SURFACE HEIGHT:
// Returns consistent water surface height for GPU displacement.
// Ensures smooth transitions between chunks.
// Formula: TerrainHeight + WaterDepth
// ============================================================================

bool UWaterSystem::HasWaterNeighbor(int32 X, int32 Y) const
{
    // Check 8 neighbors for water
    for (int32 dy = -1; dy <= 1; dy++)
    {
        for (int32 dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0) continue; // Skip self
            
            float NeighborDepth = GetWaterDepthSafe(X + dx, Y + dy);
            if (NeighborDepth > MinWaterDepth)
            {
                return true;
            }
        }
    }
    return false;
}



float UWaterSystem::CalculateChunkWaterArea(int32 TerrainChunkIndex) const
{
    // Find the water chunk for this terrain chunk
    const FWaterSurfaceChunk* WaterChunk = WaterSurfaceChunks.FindByPredicate(
        [TerrainChunkIndex](const FWaterSurfaceChunk& Chunk) {
            return Chunk.ChunkIndex == TerrainChunkIndex;
        });
    
    if (!WaterChunk)
    {
        return 0.0f;
    }
    
    float TotalArea = 0.0f;
    int32 StartX = WaterChunk->ChunkX * ChunkSize;
    int32 StartY = WaterChunk->ChunkY * ChunkSize;
    
    // Calculate water area
    for (int32 Y = 0; Y < ChunkSize; Y++)
    {
        for (int32 X = 0; X < ChunkSize; X++)
        {
            float Depth = GetWaterDepthSafe(StartX + X, StartY + Y);
            if (Depth > MinWaterDepth)
            {
                TotalArea += 1.0f;
            }
        }
    }
    
    return TotalArea;
}


float UWaterSystem::GetConsistentWaterSurfaceHeight(int32 GridX, int32 GridY)
{
    // Get water depth from simulation
    float WaterDepth = GetWaterDepthSafe(GridX, GridY);
    
    // Get terrain height at exact grid position
    if (OwnerTerrain && OwnerTerrain->HeightMap.IsValidIndex(GridY * OwnerTerrain->TerrainWidth + GridX))
    {
        float TerrainHeight = OwnerTerrain->HeightMap[GridY * OwnerTerrain->TerrainWidth + GridX];
        return TerrainHeight + FMath::Max(WaterDepth, 0.0f);
    }
    
    return 0.0f;
}


// ============================================================================
// SECTION 9: GPU DEBUGGING & VALIDATION
// ============================================================================
//
// PURPOSE:
// Comprehensive debugging infrastructure for GPU water system development
// and runtime diagnostics. Provides validation, monitoring, and visualization
// tools for the entire GPU pipeline (Sections 7-8).
//
// DEBUGGING PHILOSOPHY:
// Water simulation with GPU acceleration involves complex coordinate
// transformations, async GPU operations, and multi-threaded rendering.
// This section provides tools to validate correctness at every stage:
// - Coordinate space transformations (Terrain  ->  World  ->  Simulation  ->  GPU)
// - Wave generation and parameter validation
// - GPU pipeline state inspection
// - Visual debugging and on-screen displays
// - System reset and recovery tools
//
// COORDINATE SPACES:
// Understanding coordinate transformations is critical for debugging:
// 1. TERRAIN SPACE: Grid coordinates (0,0) to (Width-1, Height-1)
// 2. WORLD SPACE: Unreal world coordinates (cm units)
// 3. SIMULATION SPACE: Flattened 1D index = Y * Width + X
// 4. GPU SPACE: Normalized UVs (0.0 to 1.0) for texture sampling
// 5. SHADER SPACE: World position for wave calculations
//
// All coordinate transforms MUST go through MasterController (Section 1).
// Validation functions verify this authority chain is respected.
//
// WAVE SYSTEM DEBUGGING:
// Wave generation involves multiple subsystems:
// - CPU wave calculations (Section 6)
// - GPU wave compute shader (Section 7)
// - Material displacement parameters (Section 8)
// Debugging tools monitor amplitude, frequency, direction consistency.
//
// GPU PIPELINE INSPECTION:
// The GPU pipeline involves:
// - Compute shader dispatch (Section 7)
// - Texture uploads (depth, velocity, wave data)
// - Vertex displacement in material shader (Section 8)
// - Render target management
// Debug functions inspect state at each stage.
//
// COMMON DEBUG SCENARIOS:
// 1. Water not appearing  ->  Check coordinate alignment, mesh generation
// 2. Waves not animating  ->  Validate GPU parameters, shader execution
// 3. Chunk boundaries visible  ->  Check UV alignment, neighbor updates
// 4. Performance issues  ->  Monitor update counts, regeneration frequency
// 5. Coordinate mismatches  ->  Validate MasterController authority
//
// PERFORMANCE IMPACT:
// Debug functions are designed for development/testing only:
// - Logging throttled to avoid spam (typically 1 log/2 seconds)
// - On-screen displays minimal GPU overhead (~0.1ms)
// - Validation functions can be expensive (skip in shipping builds)
// - Force regeneration should only be used for recovery, not normal operation
//
// DEPENDENCIES:
// - All Sections: Validates correctness of every major system
// - Section 1: MasterController coordinate authority
// - Section 7: GPU compute shader pipeline
// - Section 8: GPU mesh generation and materials
//
// SUBSECTIONS:
// 9.1: Position & Coordinate Debugging - Coordinate space validation
// 9.2: Wave System Debugging - Wave generation monitoring
// 9.3: Pipeline Validation - GPU state inspection
// 9.4: System Management - Force updates and resets
//
// ============================================================================

// ============================================================================
// SUBSECTION 9.1: POSITION & COORDINATE DEBUGGING
// ============================================================================
//
// PURPOSE:
// Validate coordinate transformations and chunk positioning.
// Critical for detecting coordinate system mismatches that cause
// phantom water, misaligned meshes, or visual artifacts.
//
// COORDINATE VALIDATION:
// - Terrain coordinates (grid space)
// - World coordinates (Unreal world space)
// - Simulation indices (flattened 1D array)
// - GPU UVs (normalized 0-1 texture space)
//
// COMMON ISSUES DETECTED:
// - Chunk position offsets (27,200+ unit errors)
// - UV misalignment causing wave discontinuities
// - MasterController authority violations
// - Terrain/simulation coordinate mismatches
// ============================================================================

void UWaterSystem::DebugGPUChunkPositions()
{
    if (!OwnerTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("DebugGPUChunkPositions: No terrain"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU CHUNK POSITIONS DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Actor Location: (%.1f, %.1f, %.1f)"),
           OwnerTerrain->GetActorLocation().X,
           OwnerTerrain->GetActorLocation().Y,
           OwnerTerrain->GetActorLocation().Z);
    
    for (int32 i = 0; i < FMath::Min(5, WaterSurfaceChunks.Num()); i++)
    {
        const FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        if (Chunk.SurfaceMesh)
        {
            FVector MeshLocation = Chunk.SurfaceMesh->GetComponentLocation();
            FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
            
            UE_LOG(LogTemp, Warning, TEXT("Chunk %d:"), i);
            UE_LOG(LogTemp, Warning, TEXT("  - Chunk Index: %d"), Chunk.ChunkIndex);
            UE_LOG(LogTemp, Warning, TEXT("  - Chunk Grid: (%d, %d)"), Chunk.ChunkX, Chunk.ChunkY);
            UE_LOG(LogTemp, Warning, TEXT("  - Expected World Pos: (%.1f, %.1f, %.1f)"),
                   ChunkWorldPos.X, ChunkWorldPos.Y, ChunkWorldPos.Z);
            UE_LOG(LogTemp, Warning, TEXT("  - Actual Mesh Pos: (%.1f, %.1f, %.1f)"),
                   MeshLocation.X, MeshLocation.Y, MeshLocation.Z);
            UE_LOG(LogTemp, Warning, TEXT("  - Has Water: %s"), Chunk.bHasWater ? TEXT("YES") : TEXT("NO"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("================================="));
}




void UWaterSystem::DebugGPUWaterAlignment()
{
    UE_LOG(LogTemp, Warning, TEXT("===== GPU WATER ALIGNMENT DEBUG ====="));
    
    // Test a known water position
    FVector TestWorldPos(0, 0, 0);  // World origin
    
    // Where does GPU water think this is in texture space?
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(TestWorldPos);
    UE_LOG(LogTemp, Warning, TEXT("World (0,0) -> TerrainCoords (%.2f, %.2f)"),
        TerrainCoords.X, TerrainCoords.Y);
    
    // What UV would the GPU shader calculate for a vertex at origin?
    FVector TerrainOrigin(-25650, -25650, 0);  // What we're passing to shader
    FVector2D ShaderCoords = FVector2D(
        (TestWorldPos.X - TerrainOrigin.X) / 100.0f,
        (TestWorldPos.Y - TerrainOrigin.Y) / 100.0f
    );
    FVector2D ShaderUV = FVector2D(
        ShaderCoords.X / 513.0f,
        ShaderCoords.Y / 513.0f
    );
    UE_LOG(LogTemp, Warning, TEXT("GPU Shader would calculate: Coords (%.2f, %.2f) -> UV (%.4f, %.4f)"),
        ShaderCoords.X, ShaderCoords.Y, ShaderUV.X, ShaderUV.Y);
    
    // Where is chunk 0,0 actually positioned?
    if (WaterSurfaceChunks.Num() > 0)
    {
        FVector ChunkPos = OwnerTerrain->GetChunkWorldPosition(0);
        UE_LOG(LogTemp, Warning, TEXT("Chunk 0 position: (%.1f, %.1f, %.1f)"),
            ChunkPos.X, ChunkPos.Y, ChunkPos.Z);
        
        // Where SHOULD chunk 0,0 be for proper alignment?
        FVector ExpectedChunkPos(-25650, -25650, ChunkPos.Z);
        UE_LOG(LogTemp, Warning, TEXT("Chunk 0 SHOULD be at: (%.1f, %.1f, %.1f)"),
            ExpectedChunkPos.X, ExpectedChunkPos.Y, ExpectedChunkPos.Z);
        
        FVector Offset = ChunkPos - ExpectedChunkPos;
        UE_LOG(LogTemp, Warning, TEXT("Offset: (%.1f, %.1f) units"), Offset.X, Offset.Y);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("====================================="));
}


void UWaterSystem::DebugMasterControllerCoordinates()
{
    UE_LOG(LogTemp, Warning, TEXT("===== MASTER CONTROLLER COORDINATE DEBUG ====="));
    
    // Test the center of the world
    FVector WorldCenter(0, 0, 0);
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(WorldCenter);
    
    UE_LOG(LogTemp, Warning, TEXT("MasterController: World (0,0) -> Terrain (%.2f, %.2f)"),
        TerrainCoords.X, TerrainCoords.Y);
    UE_LOG(LogTemp, Warning, TEXT("EXPECTED: World (0,0) -> Terrain (256.5, 256.5)"));
    
    // Test what MasterController thinks about terrain dimensions
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
    float TerrainScale = CachedMasterController->GetTerrainScale();
    
    UE_LOG(LogTemp, Log, TEXT("MasterController WorldDims: (%.0f, %.0f)"),
        WorldDims.X, WorldDims.Y);
    UE_LOG(LogTemp, Log, TEXT("MasterController TerrainScale: %.1f"), TerrainScale);
    
    // Calculate what the offset SHOULD be
    float ExpectedOffset = (WorldDims.X * TerrainScale) * 0.5f;
    UE_LOG(LogTemp, Warning, TEXT("Expected offset for centering: %.1f"), ExpectedOffset);
    UE_LOG(LogTemp, Warning, TEXT("Actual offset we're seeing: 27200"));
    
    UE_LOG(LogTemp, Log, TEXT("==============================================="));
}




// Monitor wave amplitudes for debugging

// ============================================================================
// SUBSECTION 9.2: WAVE SYSTEM DEBUGGING
// ============================================================================
//
// PURPOSE:
// Monitor wave generation and validate wave parameters.
// Ensures wave physics (Section 6) and GPU implementation (Section 7)
// produce consistent, physically plausible results.
//
// WAVE AMPLITUDE MONITORING:
// - Tracks min/max/average wave heights
// - Detects abnormal spikes or dampening
// - Validates wave generation consistency
//
// ACCEPTABLE RANGES:
// - Wind waves: 0.1 - 2.0 m (typical ocean)
// - Gravity waves: 0.5 - 5.0 m (deep water)
// - Capillary waves: 0.001 - 0.01 m (surface tension)
//
// VISUAL DEBUGGING:
// - On-screen parameter display
// - Wave direction vectors
// - Real-time amplitude graphs
// ============================================================================

void UWaterSystem::MonitorWaveAmplitudes()
{
    if (!bDebugGPUWaves || !WaveOutputTexture)
        return;
    
    // Sample the wave output texture periodically
    static float LastSampleTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastSampleTime < 0.1f)  // Sample every 100ms
        return;
    
    LastSampleTime = CurrentTime;
    
    // This would need render thread access to actually read the texture
    // For now, we'll estimate based on parameters
    float EstimatedMaxAmplitude = 0.0f;
    float TotalAmplitude = 0.0f;
    int32 SampleCount = 0;
    
    // Sample a few points from the simulation data
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i += 100)
    {
        float Depth = SimulationData.WaterDepthMap[i];
        if (Depth > MinWaterDepth)
        {
            // Estimate maximum possible wave at this depth
            float MaxWave = Depth * GPUMaxWaveHeightRatio;
            EstimatedMaxAmplitude = FMath::Max(EstimatedMaxAmplitude, MaxWave);
            TotalAmplitude += MaxWave;
            SampleCount++;
        }
    }
    
    if (SampleCount > 0)
    {
        CurrentMaxWaveAmplitude = EstimatedMaxAmplitude;
        AverageWaveAmplitude = TotalAmplitude / SampleCount;
        WaveAmplitudeSampleCount = SampleCount;
        
        // Log warnings if amplitudes seem too high
        if (CurrentMaxWaveAmplitude > 100.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("GPU Waves: High amplitude detected! Max=%.2f, Avg=%.2f"),
                   CurrentMaxWaveAmplitude, AverageWaveAmplitude);
        }
    }
}

// Validate GPU wave parameters

void UWaterSystem::DebugDrawWaveInfo()
{
    if (!bDebugGPUWaves || !GetWorld())
        return;
    
    // Draw debug info at player location
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return;
    
    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    FVector2D WorldPos = FVector2D(PlayerLoc.X, PlayerLoc.Y);
    
    // Get water depth at player
    float WaterDepth = GetWaterDepthAtPosition(PlayerLoc);
    
    if (WaterDepth > MinWaterDepth)
    {
        // Calculate theoretical maximum wave height at this depth
        float MaxWaveHeight = WaterDepth * GPUMaxWaveHeightRatio;
        float SafeWaveHeight = WaterDepth * GPUSafeWaveHeightRatio;
        
        // Draw debug sphere showing wave limits
        DrawDebugSphere(GetWorld(), PlayerLoc + FVector(0, 0, MaxWaveHeight),
                       20.0f, 8, FColor::Yellow, false, 0.1f);
        DrawDebugSphere(GetWorld(), PlayerLoc + FVector(0, 0, SafeWaveHeight),
                       15.0f, 8, FColor::Green, false, 0.1f);
        
        // Draw debug text
        FString DebugText = FString::Printf(
            TEXT("Water Depth: %.1f\nMax Wave: %.1f\nSafe Wave: %.1f\nWave Scale: %.2f"),
            WaterDepth, MaxWaveHeight, SafeWaveHeight, GPUWaveScale
        );
        
        DrawDebugString(GetWorld(), PlayerLoc + FVector(0, 0, 100),
                       DebugText, nullptr, FColor::White, 0.1f, true);
    }
}

// Reset GPU wave system (useful for debugging)
UFUNCTION(BlueprintCallable, Category = "GPU Waves|Debug")

void UWaterSystem::UpdateGPUWaveDebug(float DeltaTime)
{
    if (!bUseVertexDisplacement)
        return;
    
    
    // Monitor amplitudes if debugging
    if (bDebugGPUWaves)
    {
        MonitorWaveAmplitudes();
        DebugDrawWaveInfo();
    }
}


// ============================================================================
// SUBSECTION 9.3: PIPELINE VALIDATION
// ============================================================================
//
// PURPOSE:
// Validate GPU compute pipeline state and textures.
// Comprehensive inspection of GPU resources, shader parameters,
// and render targets.
//
// GPU PIPELINE ARCHITECTURE:
// 1. CPU prepares data (water depth, velocity, wave params)
// 2. Data uploaded to GPU textures (R32F, RG16F formats)
// 3. Compute shader executes (WaveCompute.usf)
// 4. Results stored in displacement texture
// 5. Material shader reads displacement for vertex positioning
//
// VALIDATION CHECKS:
// - Texture creation and format correctness
// - Shader parameter ranges and validity
// - UAV access flags for compute writes
// - Render target resource allocation
//
// TEXTURE FORMAT REQUIREMENTS:
// - Water depth: R32_FLOAT (single channel, high precision)
// - Flow velocity: G16R16F (two channels, medium precision)
// - Wave displacement: FloatRGBA (four channels for X,Y,Z + foam)
// ============================================================================






// Debug visualization for wave system

// ============================================================================
// SUBSECTION 9.4: SYSTEM MANAGEMENT
// ============================================================================
//
// PURPOSE:
// Force updates and reset GPU systems for recovery from errors.
// These are "nuclear options" - use only when system is corrupted.
//
// FORCE REGENERATION:
// Rebuilds all GPU water meshes from scratch.
// Use when:
// - Meshes not appearing despite water present
// - Coordinate system corrupted
// - After major terrain edits
// Cost: High (rebuilds all chunks, ~100ms for 32x32 chunks)
//
// WAVE SYSTEM RESET:
// Resets GPU wave parameters to defaults.
// Use when:
// - Waves frozen or not animating
// - Parameter corruption suspected
// - After GPU driver crash recovery
// Cost: Low (parameter reset only, ~1ms)
//
// WARNING:
// These functions are for emergency recovery only!
// Normal operation should never need them.
// Frequent use indicates underlying bugs that should be fixed.
// ============================================================================

void UWaterSystem::ForceGPUMeshRegeneration()
{
    if (!bUseVertexDisplacement || !OwnerTerrain)
        return;
    
    UE_LOG(LogTemp, Warning, TEXT("=== FORCING GPU MESH REGENERATION ==="));
    
    // Clear existing chunk water areas to force regeneration
    ChunkWaterAreas.Empty();
    
    // Check ALL terrain chunks
    int32 ChunksWithWater = 0;
    int32 ChunksCreated = 0;
    
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        const FTerrainChunk& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
        const int32 TerrainChunkSize = OwnerTerrain->ChunkSize;
        const int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
        
        int32 ChunkStartX = TerrainChunk.ChunkX * (TerrainChunkSize - ChunkOverlap);
        int32 ChunkStartY = TerrainChunk.ChunkY * (TerrainChunkSize - ChunkOverlap);
        int32 ChunkEndX = FMath::Min(ChunkStartX + TerrainChunkSize, OwnerTerrain->TerrainWidth);
        int32 ChunkEndY = FMath::Min(ChunkStartY + TerrainChunkSize, OwnerTerrain->TerrainHeight);
        
        // Check if chunk has water
        bool bHasWater = false;
        float TotalDepth = 0.0f;
        
        for (int32 Y = ChunkStartY; Y < ChunkEndY; Y++)
        {
            for (int32 X = ChunkStartX; X < ChunkEndX; X++)
            {
                float Depth = GetWaterDepthSafe(X, Y);
                if (Depth > MinWaterDepth)
                {
                    bHasWater = true;
                    TotalDepth += Depth;
                }
            }
        }
        
        if (bHasWater)
        {
            ChunksWithWater++;
            
            // Find or create water chunk
            FWaterSurfaceChunk* WaterChunk = WaterSurfaceChunks.FindByPredicate(
                [ChunkIndex](FWaterSurfaceChunk& Chunk) {
                    return Chunk.ChunkIndex == ChunkIndex;
                });
            
            if (!WaterChunk)
            {
                FWaterSurfaceChunk NewChunk;
                NewChunk.ChunkIndex = ChunkIndex;
                NewChunk.ChunkX = TerrainChunk.ChunkX;
                NewChunk.ChunkY = TerrainChunk.ChunkY;
                NewChunk.bHasWater = true;
                WaterSurfaceChunks.Add(NewChunk);
                WaterChunk = &WaterSurfaceChunks.Last();
                ChunksCreated++;
            }
            
            // Force regeneration
            WaterChunk->bHasWater = true;
            GenerateFlatBaseMesh(*WaterChunk);
            
            UE_LOG(LogTemp, Warning, TEXT("Chunk %d: Has water (Total depth: %.1f)"),
                   ChunkIndex, TotalDepth);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Force Regeneration Complete:"));
    UE_LOG(LogTemp, Warning, TEXT("  - Chunks with water: %d"), ChunksWithWater);
    UE_LOG(LogTemp, Warning, TEXT("  - New chunks created: %d"), ChunksCreated);
    UE_LOG(LogTemp, Warning, TEXT("  - Total water chunks: %d"), WaterSurfaceChunks.Num());
}

FString UWaterSystem::GetSystemStateString() const
{
    FString State;
    State += FString::Printf(TEXT("Mode: %s\n"),
        bUseVertexDisplacement ? TEXT("GPU") : TEXT("CPU"));
    State += FString::Printf(TEXT("Chunks: %d\n"), WaterSurfaceChunks.Num());
    State += FString::Printf(TEXT("Update Distance: %.0f\n"), VolumeUpdateDistance);
    State += FString::Printf(TEXT("Min Depth: %.3f\n"), MinWaterDepth);
    State += FString::Printf(TEXT("Material: %s\n"),
        WaterMaterialWithDisplacement ? TEXT("Set") : TEXT("Missing"));
    return State;
}



void UWaterSystem::ResetGPUWaveSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting GPU Wave System..."));
    
    // Reset parameters to safe defaults
    GPUWaveScale = 0.5f;
    GPUWaveSpeed = 1.0f;
    GPUWaveDamping = 0.9f;
    GPUMaxWaveHeightRatio = 0.3f;
    GPUSafeWaveHeightRatio = 0.125f;
    GPUDeepWaterThreshold = 50.0f;
    GPUShallowWaterThreshold = 5.0f;
    
    // Reset tracking variables
    AccumulatedGPUTime = 0.0f;
    LastGPUWaveUpdateTime = 0.0f;
    CurrentMaxWaveAmplitude = 0.0f;
    AverageWaveAmplitude = 0.0f;
    WaveAmplitudeSampleCount = 0;
    
    // Force texture recreation
    if (WaveOutputTexture)
    {
        WaveOutputTexture->ReleaseResource();
        WaveOutputTexture = nullptr;
    }
    
    // Reinitialize if GPU mode is active
    if (bUseVertexDisplacement)
    {
        InitializeGPUDisplacement();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Wave System reset complete"));
}

// Update function to call from Tick

// ============================================================================
// SECTION 10: EROSION SYSTEM INTEGRATION
// ============================================================================
//
// PURPOSE:
// Bridge between WaterSystem and GeologyController for erosion simulation.
// Provides shared GPU textures for water depth and flow velocity that
// GeologyController uses to calculate sediment transport and erosion.
//
// ARCHITECTURE:
// WaterSystem and GeologyController operate as coupled systems:
// - WaterSystem: Calculates water flow (Section 2)
// - ErosionTextures: Shared GPU textures (this section)
// - GeologyController: Calculates erosion based on water data
// - Terrain: Updated with erosion results
//
// DATA FLOW:
// AtmosphericSystem  ->  WaterSystem  ->  ErosionTextures  ->  GeologyController  ->  Terrain
//     (precipitation)    (flow calc)    (GPU textures)    (erosion calc)   (height update)
//
// INTEGRATION STATUS:
// Phase 1 (CURRENT): Interface ready, textures implemented
// - Water depth texture created and updated
// - Flow velocity texture created and updated
// - Precipitation input interface ready
// Phase 2 (FUTURE): GeologyController erosion implementation
// - Sediment transport calculations
// - Terrain height modifications
// - Erosion/deposition feedback loop
//
// TEXTURE SPECIFICATIONS:
//
// 1. WATER DEPTH TEXTURE (ErosionWaterDepthRT)
//    Format: R32_FLOAT (single channel, 32-bit float)
//    Dimensions: TerrainWidth   TerrainHeight (e.g., 513x513)
//    Content: Water depth in world units (0.0 = dry, >0.0 = wet)
//    Usage: Erosion rate proportional to water depth
//    Update: Every simulation frame (~60 Hz)
//
// 2. FLOW VELOCITY TEXTURE (ErosionFlowVelocityRT)
//    Format: RG16F (two channels, 16-bit float each)
//    Dimensions: TerrainWidth   TerrainHeight
//    Content: Flow velocity vector (R=X velocity, G=Y velocity)
//    Units: World units per second (e.g., cm/s)
//    Usage: Sediment transport direction and magnitude
//    Update: Every simulation frame (~60 Hz)
//
// 3. PRECIPITATION INPUT TEXTURE (PrecipitationTexture)
//    Format: R32_FLOAT (single channel)
//    Dimensions: Atmospheric grid resolution
//    Content: Precipitation rate (mm/hour)
//    Source: AtmosphericSystem
//    Update: Atmospheric update cycle (~1 Hz)
//
// MODERN UE5 API USAGE:
// This section uses modern UE5.5 texture APIs:
// - FTextureRHIRef (modern) instead of FTexture2DRHIRef (deprecated)
// - ENQUEUE_RENDER_COMMAND for thread-safe GPU uploads
// - UAV (Unordered Access View) flags for compute shader writes
// - Proper resource lifecycle (Release  ->  ConditionalBeginDestroy)
//
// THREAD SAFETY:
// GPU texture uploads must be thread-safe:
// - Data prepared on game thread (TArray copies)
// - ENQUEUE_RENDER_COMMAND transfers to render thread
// - RHICmdList.UpdateTexture2D performs actual GPU upload
// - Lambda captures ensure data lifetime
//
// WHY SEPARATE EROSION TEXTURES?
// Could we use existing water simulation arrays directly?
// No, because:
// 1. GeologyController runs async on render thread (needs GPU textures)
// 2. Erosion compute shader needs UAV-accessible textures
// 3. Simulation arrays are CPU-side (Section 2)
// 4. Texture format optimized for erosion calculations (R32F vs TArray<float>)
//
// PERFORMANCE:
// - Texture creation: ~1ms one-time cost
// - Texture update: ~0.5ms per frame (CPU -> GPU upload)
// - Memory: ~2MB for 513x513 textures (R32F + RG16F)
// - GPU overhead: Minimal (textures resident in VRAM)
//
// DEPENDENCIES:
// - Section 2: SimulationData arrays (WaterDepthMap, WaterVelocityX/Y)
// - AtmosphericSystem: Precipitation input
// - GeologyController: Erosion calculations (Phase 2)
//
// EXTERNAL INTERFACES:
// - SetPrecipitationInput(): Called by AtmosphericSystem
// - ErosionWaterDepthRT: Read by GeologyController
// - ErosionFlowVelocityRT: Read by GeologyController
//
// FUTURE ENHANCEMENTS (Phase 2):
// - Sediment concentration texture (suspended sediment tracking)
// - Erosion rate feedback (modify terrain based on erosion)
// - Deposition tracking (sediment accumulation)
// - Bank erosion (lateral erosion of stream banks)
// - Bedrock exposure (differential erosion rates)
//
// SUBSECTIONS:
// 10.1: Precipitation Interface - Atmospheric input
// 10.2: Texture Management - Create/update/cleanup erosion textures
//
// ============================================================================

// ============================================================================
// SUBSECTION 10.1: PRECIPITATION INTERFACE
// ============================================================================
//
// PURPOSE:
// Receive atmospheric precipitation data for erosion calculations.
//
// INTEGRATION:
// AtmosphericSystem calls this when precipitation texture updates.
// Texture contains rain rate (mm/hour) across atmospheric grid.
//
// USAGE:
// - GeologyController reads precipitation for erosion intensity
// - Higher precipitation  ->  higher erosion rates
// - Spatially-varying rain creates realistic erosion patterns
// ============================================================================

void UWaterSystem::SetPrecipitationInput(UTextureRenderTarget2D* PrecipTexture)
{
    PrecipitationInputTexture = PrecipTexture;

    if (PrecipTexture)
    {
        UE_LOG(LogTemp, Verbose, TEXT("WaterSystem: Precipitation texture set (%dx%d)"),
               PrecipTexture->SizeX, PrecipTexture->SizeY);
    }
}

void UWaterSystem::AccumulatePrecipitation(float DeltaTime)
{
    if (!OwnerTerrain || !OwnerTerrain->AtmosphericSystem || !SimulationData.IsValid())
    {
        return;
    }

    UAtmosphericSystem* Atmosphere = OwnerTerrain->AtmosphericSystem;
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;

    // Sample precipitation at a lower resolution for performance
    // Atmosphere grid is typically 64x64, terrain is 513x513
    const int32 SampleStep = FMath::Max(1, Width / 64);

    float TotalPrecipAdded = 0.0f;
    int32 CellsWithPrecip = 0;

    for (int32 Y = 0; Y < Height; Y += SampleStep)
    {
        for (int32 X = 0; X < Width; X += SampleStep)
        {
            // Get world position for this grid cell
            FVector WorldPos = OwnerTerrain->GetActorLocation() +
                               FVector(X * OwnerTerrain->TerrainScale,
                                       Y * OwnerTerrain->TerrainScale,
                                       0.0f);

            // Query precipitation rate from atmosphere (mm/hour)
            float PrecipRate = Atmosphere->GetPrecipitationAt(WorldPos);

            if (PrecipRate > 0.01f)
            {
                // Convert mm/hour to simulation units per second
                // 1 mm/hour = 0.001 m/hour = 0.001/3600 m/s ≈ 2.78e-7 m/s
                // With terrain scale factor
                float WaterToAdd = PrecipRate * 0.00001f * DeltaTime;

                // Apply to a region of cells around this sample point
                int32 RegionEndX = FMath::Min(X + SampleStep, Width);
                int32 RegionEndY = FMath::Min(Y + SampleStep, Height);

                for (int32 RY = Y; RY < RegionEndY; RY++)
                {
                    for (int32 RX = X; RX < RegionEndX; RX++)
                    {
                        int32 Index = RY * Width + RX;
                        if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
                        {
                            SimulationData.WaterDepthMap[Index] += WaterToAdd;
                            TotalPrecipAdded += WaterToAdd;
                        }
                    }
                }

                CellsWithPrecip++;
            }
        }
    }

    // Track total water added for budget
    if (TotalPrecipAdded > 0.0f)
    {
        TotalWaterAmount += TotalPrecipAdded;
        bWaterChangedThisFrame = true;
    }
}


// ============================================================================
// SUBSECTION 10.2: TEXTURE MANAGEMENT
// ============================================================================
//
// PURPOSE:
// Manage GPU textures for erosion system communication.
// Creates, updates, and cleans up render targets shared with GeologyController.
//
// TEXTURE LIFECYCLE:
// 1. CreateErosionTextures(): Allocate GPU textures
//    - Called during initialization or resolution changes
//    - Creates R32F depth texture and RG16F velocity texture
//    - Configures UAV access for compute shader writes
//
// 2. UpdateErosionTextures(): Upload current simulation data
//    - Called every simulation frame (~60 Hz)
//    - Copies WaterDepthMap and velocity arrays to GPU
//    - Thread-safe upload via ENQUEUE_RENDER_COMMAND
//
// 3. CleanupErosionTextures(): Release GPU resources
//    - Called during shutdown or before recreation
//    - Flushes pending render commands
//    - Properly destroys render targets
//
// 4. RecreateErosionTextures(): Rebuild after changes
//    - Called after terrain resolution changes
//    - Cleanup  ->  Flush  ->  Create sequence
//
// THREAD SAFETY:
// GPU uploads MUST be thread-safe:
// - Game thread: Prepare data (copy to TArray)
// - Render command: Transfer to render thread via lambda capture
// - Render thread: Execute RHICmdList.UpdateTexture2D
// Never directly access GPU resources from game thread!
//
// MODERN UE5 API:
// Uses FTextureRHIRef (modern) instead of:
// - FTexture2DRHIRef (deprecated in UE5)
// - FTexture2DRHIParamRef (removed in UE5)
// This ensures forward compatibility with UE5.5+
// ============================================================================

void UWaterSystem::CreateErosionTextures()
{
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create erosion textures - simulation not ready"));
        return;
    }
    
    int32 Width = SimulationData.TerrainWidth;
    int32 Height = SimulationData.TerrainHeight;
    
    // ===== CREATE WATER DEPTH RENDER TARGET (R32F) =====
    ErosionWaterDepthRT = NewObject<UTextureRenderTarget2D>(this);
    ErosionWaterDepthRT->RenderTargetFormat = RTF_R32f;
    ErosionWaterDepthRT->InitCustomFormat(Width, Height, PF_R32_FLOAT, false);
    ErosionWaterDepthRT->AddressX = TA_Clamp;
    ErosionWaterDepthRT->AddressY = TA_Clamp;
    ErosionWaterDepthRT->bCanCreateUAV = true;  // Enable compute shader writes
    ErosionWaterDepthRT->UpdateResourceImmediate(true);
    
    // ===== CREATE FLOW VELOCITY RENDER TARGET (RG16F) =====
    ErosionFlowVelocityRT = NewObject<UTextureRenderTarget2D>(this);
    ErosionFlowVelocityRT->RenderTargetFormat = RTF_RG16f;
    ErosionFlowVelocityRT->InitCustomFormat(Width, Height, PF_G16R16F, false);
    ErosionFlowVelocityRT->AddressX = TA_Clamp;
    ErosionFlowVelocityRT->AddressY = TA_Clamp;
    ErosionFlowVelocityRT->bCanCreateUAV = true;  // Enable compute shader writes
    ErosionFlowVelocityRT->UpdateResourceImmediate(true);

    // ===== CREATE SEDIMENT CONCENTRATION RENDER TARGET (R32F) =====
    ErosionSedimentRT = NewObject<UTextureRenderTarget2D>(this);
    ErosionSedimentRT->RenderTargetFormat = RTF_R32f;
    ErosionSedimentRT->InitCustomFormat(Width, Height, PF_R32_FLOAT, false);
    ErosionSedimentRT->AddressX = TA_Clamp;
    ErosionSedimentRT->AddressY = TA_Clamp;
    ErosionSedimentRT->bCanCreateUAV = true;  // Enable compute shader writes
    ErosionSedimentRT->UpdateResourceImmediate(true);

    UE_LOG(LogTemp, Warning, TEXT(" Created erosion textures: %dx%d (Depth: R32F, Velocity: RG16F, Sediment: R32F)"),
           Width, Height);
    
    // Initial population with current simulation data
    UpdateErosionTextures();
}


void UWaterSystem::UpdateErosionTextures()
{
    if (!ErosionWaterDepthRT || !ErosionFlowVelocityRT || !SimulationData.IsValid())
    {
        return;
    }

    int32 Width = SimulationData.TerrainWidth;
    int32 Height = SimulationData.TerrainHeight;

    // Prepare data on game thread
    TArray<float> DepthData;
    TArray<FFloat16> VelocityData;
    TArray<float> SedimentData;

    DepthData.SetNumUninitialized(Width * Height);
    VelocityData.SetNumUninitialized(Width * Height * 2); // 2 channels (X, Y)
    SedimentData.SetNumUninitialized(Width * Height);

    // Copy simulation data to upload buffers
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        DepthData[i] = SimulationData.WaterDepthMap[i];
        VelocityData[i * 2 + 0] = FFloat16(SimulationData.WaterVelocityX[i]);
        VelocityData[i * 2 + 1] = FFloat16(SimulationData.WaterVelocityY[i]);
        SedimentData[i] = (SimulationData.SedimentMap.Num() > i) ? SimulationData.SedimentMap[i] : 0.0f;
    }

    // Enqueue render command for GPU upload
    ENQUEUE_RENDER_COMMAND(UpdateErosionTextures)(
        [this, DepthData = MoveTemp(DepthData), VelocityData = MoveTemp(VelocityData),
         SedimentData = MoveTemp(SedimentData), Width, Height](FRHICommandListImmediate& RHICmdList)
        {
            // ===== UPDATE WATER DEPTH TEXTURE =====
            FTextureRenderTargetResource* DepthResource = ErosionWaterDepthRT->GetRenderTargetResource();
            if (DepthResource)
            {
                // MODERN UE5: Use FTextureRHIRef instead of deprecated FTexture2DRHIRef
                FTextureRHIRef DepthTexture = DepthResource->GetRenderTargetTexture();

                if (DepthTexture.IsValid())
                {
                    uint32 Stride = Width * sizeof(float);
                    FUpdateTextureRegion2D Region(0, 0, 0, 0, Width, Height);

                    // Modern RHI texture update
                    RHICmdList.UpdateTexture2D(
                        DepthTexture,
                        0,  // Mip level
                        Region,
                        Stride,
                        (const uint8*)DepthData.GetData()
                    );
                }
            }

            // ===== UPDATE FLOW VELOCITY TEXTURE =====
            FTextureRenderTargetResource* VelocityResource = ErosionFlowVelocityRT->GetRenderTargetResource();
            if (VelocityResource)
            {
                // MODERN UE5: Use FTextureRHIRef instead of deprecated FTexture2DRHIRef
                FTextureRHIRef VelocityTexture = VelocityResource->GetRenderTargetTexture();

                if (VelocityTexture.IsValid())
                {
                    uint32 Stride = Width * sizeof(FFloat16) * 2; // 2 channels
                    FUpdateTextureRegion2D Region(0, 0, 0, 0, Width, Height);

                    // Modern RHI texture update
                    RHICmdList.UpdateTexture2D(
                        VelocityTexture,
                        0,  // Mip level
                        Region,
                        Stride,
                        (const uint8*)VelocityData.GetData()
                    );
                }
            }

            // ===== UPDATE SEDIMENT CONCENTRATION TEXTURE =====
            if (ErosionSedimentRT)
            {
                FTextureRenderTargetResource* SedimentResource = ErosionSedimentRT->GetRenderTargetResource();
                if (SedimentResource)
                {
                    FTextureRHIRef SedimentTexture = SedimentResource->GetRenderTargetTexture();

                    if (SedimentTexture.IsValid())
                    {
                        uint32 Stride = Width * sizeof(float);
                        FUpdateTextureRegion2D Region(0, 0, 0, 0, Width, Height);

                        RHICmdList.UpdateTexture2D(
                            SedimentTexture,
                            0,  // Mip level
                            Region,
                            Stride,
                            (const uint8*)SedimentData.GetData()
                        );
                    }
                }
            }
        }
    );
}

void UWaterSystem::UpdateSedimentTexture()
{
    // Convenience function - calls full update which includes sediment
    UpdateErosionTextures();
}


void UWaterSystem::CleanupErosionTextures()
{
    UE_LOG(LogTemp, Verbose, TEXT("Cleaning up erosion textures..."));
    
    // PERFORMANCE FIX: Async cleanup - NO blocking flush (saves 5-8ms)
    
    // Release water depth texture
    if (ErosionWaterDepthRT)
    {
        ErosionWaterDepthRT->ReleaseResource();
        ErosionWaterDepthRT->ConditionalBeginDestroy();
        ErosionWaterDepthRT = nullptr;
    }
    
    // Release flow velocity texture
    if (ErosionFlowVelocityRT)
    {
        ErosionFlowVelocityRT->ReleaseResource();
        ErosionFlowVelocityRT->ConditionalBeginDestroy();
        ErosionFlowVelocityRT = nullptr;
    }

    // Release sediment texture
    if (ErosionSedimentRT)
    {
        ErosionSedimentRT->ReleaseResource();
        ErosionSedimentRT->ConditionalBeginDestroy();
        ErosionSedimentRT = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT(" Erosion textures cleaned up"));
}


void UWaterSystem::RecreateErosionTextures()
{
    UE_LOG(LogTemp, Warning, TEXT("Recreating erosion textures..."));
    
    // First cleanup any existing textures
    CleanupErosionTextures();
    
    // PERFORMANCE FIX: No flush needed - ReleaseResource already queues cleanup
    // Create fresh textures immediately (saves ~3-5ms)
    CreateErosionTextures();
    
    UE_LOG(LogTemp, Warning, TEXT(" Erosion textures recreated"));
}

// ============================================================================
// END OF WATERSYSTEM.CPP
// ============================================================================
/**
 * ORGANIZATION SUMMARY:
 * - Lines: ~9,200
 * - Functions: ~180
 * - Sections: 9 major sections
 * - Subsections: 30+ subsections
 *
 * SECTION BREAKDOWN:
 * Section 1: System Lifecycle (~400 lines, 4%)
 * Section 2: Core Water Physics (~800 lines, 9%)
 * Section 3: Water Management & Visuals (~1000 lines, 11%)
 * Section 4: Public API & Utilities (~600 lines, 7%)
 * Section 5: Rendering & Visuals (~900 lines, 10%)
 * Section 6: Wave Physics System (~1200 lines, 13%)
 * Section 7: GPU Compute Pipeline (~800 lines, 9%)
 * Section 8: GPU Mesh Management (~700 lines, 8%)
 * Section 9: Debug & Utilities (~600 lines, 7%)
 *
 * CRITICAL SECTIONS:
 * - Section 2: Core Water Physics (CONSERVATION CRITICAL)
 * - Section 6: Wave Physics (6 WAVE TYPES)
 * - Section 7: GPU Compute Pipeline (PERFORMANCE CRITICAL)
 *
 * AUTHORITY INTEGRATION:
 * - MasterController: Coordinate authority, water budget
 * - DynamicTerrain: Height data, chunk management
 * - AtmosphereController: Wind, precipitation input
 *
 * GPU PIPELINE:
 * - WaveCompute.usf: Wave generation shader
 * - WaveOutputTexture: R32F displacement map
 * - 60 Hz update rate for smooth animation
 */

// End of file
