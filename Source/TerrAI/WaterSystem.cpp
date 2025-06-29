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

UWaterSystem::UWaterSystem()
{
    // Initialize default settings
    bEnableWaterSimulation = true;
    WaterFlowSpeed = 50.0f;
    WaterEvaporationRate = 0.01f;
    WaterAbsorptionRate = 0.02f;
    WaterDamping = 0.95f;
    MaxWaterVelocity = 100.0f;
    MinWaterDepth = 0.01f;
    
    // Erosion settings removed - now handled by GeologyController
    
    // Weather settings
    bIsRaining = false;
    RainIntensity = 1.0f;
    WeatherChangeInterval = 60.0f;
    bAutoWeather = false;
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
    MinVolumeDepth = 0.5f;           // Minimum depth to create surfaces
    VolumeUpdateDistance = 2000.0f;  // LOD distance for surface updates
    MaxVolumeChunks = 64;            // Performance limit
    
    // Initialize edge drainage settings
    bEnableEdgeDrainage = false;
    EdgeDrainageStrength = 0.2f;
    bEnhancedWaterfallEffect = false;
    WaterfallDrainageMultiplier = 0.3f;
    MinDepthThreshold = 0.1f;
    
    // Initialize Niagara FX settings
    bEnableNiagaraFX = true;
    NiagaraUpdateRate = 0.1f;
    MaxNiagaraDistance = 3000.0f;
    MaxActiveNiagaraComponents = 32;
    
    // Initialize Water Authority & Quality Settings
    MinMeshDepth = 0.3f;
    MinContiguousWaterCells = 9;
    bAlwaysShowWaterMeshes = true;
    WaterMeshQuality = 64;
    
    // Initialize water quality presets
    InitializeWaterQualityDefaults();
    
    // Call WaterParameterCollection
    WaterParameterCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/MPC_WaterParameters.MPC_WaterParameters"));
    
    
    if (bEnableVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Created with surface water and Niagara FX support"));
    }
}

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
        UE_LOG(LogTemp, Error, TEXT("[PHASE 1] Cannot initialize WaterSystem with null terrain"));
        return;
    }
    
    if (!InMasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("[PHASE 1] Cannot initialize WaterSystem without MasterController - coordinate authority required"));
        return;
    }
    
    OwnerTerrain = InTerrain;
    CachedMasterController = InMasterController;
    
    // PHASE 1: Initialize simulation data with authority validation
    SimulationData.Initialize(OwnerTerrain->TerrainWidth, OwnerTerrain->TerrainHeight);
    
    // PHASE 1: Immediate texture creation for consistent startup (no on-demand creation)
    if (bUseShaderWater)
    {
        CreateWaterDepthTexture();
        CreateAdvancedWaterTexture();
        
        if (WaterDepthTexture && WaterDataTexture)
        {
            UE_LOG(LogTemp, Log, TEXT("[PHASE 1] Shader textures created successfully during initialization"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[PHASE 1] Failed to create shader textures - system may not be ready"));
        }
    }
    
    // PHASE 2: Validate material system
    if (bEnableWaterVolumes && !VolumeMaterial && OwnerTerrain && !OwnerTerrain->CurrentActiveMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] No water material available - mesh rendering may fail"));
    }
    
    // PHASE 1: Test coordinate authority
    FVector2D TestCoords = CachedMasterController->WorldToTerrainCoordinates(FVector::ZeroVector);
    UE_LOG(LogTemp, Log, TEXT("[PHASE 1] Coordinate authority test: Origin maps to (%.2f, %.2f)"), TestCoords.X, TestCoords.Y);
    
    // UE5.4 Initialization
    InitializeForUE54();
    
    // Final validation
    bool bIsReady = IsSystemReady();
    UE_LOG(LogTemp, Warning, TEXT("[PHASE 1-2] WaterSystem initialized %dx%d (%d cells) - Ready: %s"),
           SimulationData.TerrainWidth, SimulationData.TerrainHeight, 
           SimulationData.WaterDepthMap.Num(), bIsReady ? TEXT("YES") : TEXT("NO"));
           
    if (!bIsReady)
    {
        UE_LOG(LogTemp, Error, TEXT("[PHASE 1-2] WaterSystem failed readiness check - review initialization requirements"));
    }
}



/**
 * PHASE 1-2: Event-driven system readiness check
 * Eliminates timing-dependent initialization for more reliable startup
 * Uses actual system state validation instead of arbitrary delays
 */
bool UWaterSystem::IsSystemReady() const
{
    // PHASE 1: Basic system components validation
    bool bBasicReady = (OwnerTerrain != nullptr && SimulationData.IsValid());
    bool bControllerReady = (CachedMasterController != nullptr);
    
    // PHASE 1: Shader system validation (if enabled)
    bool bTexturesReady = true;
    if (bUseShaderWater)
    {
        bTexturesReady = (WaterDepthTexture != nullptr);
    }
    
    // PHASE 2: Material system validation
    bool bMaterialReady = true;
    if (bEnableWaterVolumes)
    {
        bMaterialReady = (VolumeMaterial != nullptr || 
                         (OwnerTerrain && OwnerTerrain->CurrentActiveMaterial != nullptr));
    }
    
    // PHASE 1-2: Coordinate authority validation
    bool bCoordinateReady = true;
    if (CachedMasterController)
    {
        // Verify coordinate transforms are working
        FVector2D TestCoords = CachedMasterController->WorldToTerrainCoordinates(FVector::ZeroVector);
        bCoordinateReady = !TestCoords.IsZero() || true; // Zero is valid for origin
    }
    
    bool bReady = bBasicReady && bControllerReady && bTexturesReady && bMaterialReady && bCoordinateReady;
    
    // Enhanced logging for component validation
    static bool bFirstReadyCheck = true;
    if (bFirstReadyCheck && !bReady)
    {
        if (!bBasicReady) UE_LOG(LogTemp, Warning, TEXT("[PHASE 1] SimulationData or OwnerTerrain invalid"));
        if (!bControllerReady) UE_LOG(LogTemp, Warning, TEXT("[PHASE 1] MasterController missing - coordinate authority required"));
        if (!bTexturesReady) UE_LOG(LogTemp, Warning, TEXT("[PHASE 1] Shader textures not created"));
        if (!bMaterialReady) UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] Material system not ready"));
        if (!bCoordinateReady) UE_LOG(LogTemp, Warning, TEXT("[PHASE 1] Coordinate authority validation failed"));
        bFirstReadyCheck = false;
    }
    
    return bReady;
}

// ===== WATER PHYSICS SIMULATION =====

void UWaterSystem::UpdateWaterSimulation(float DeltaTime)
{
    if (!IsSystemReady() || !bEnableWaterSimulation)
    {
        return;
    }
    
    // ✅ ENHANCED TERRAIN SYNCHRONIZATION - FORCE TERRAIN HEIGHT REFRESH
    if (bTerrainChanged || bForceTerrainResync)
    {
        RefreshTerrainHeightCache();
        ForceWaterReflow();  // Make water respond to new terrain immediately
        bTerrainChanged = false;
        bForceTerrainResync = false;
        
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: FORCED terrain sync - water recalculated"));
    }
    
    // Step 1: Update weather system
    if (bAutoWeather)
    {
        UpdateWeatherSystem(DeltaTime);
    }
    
    // Step 2: Apply rain if active
    if (bIsRaining)
    {
        ApplyRain(DeltaTime);
    }
    
    // Step 3: Calculate water flow forces
    CalculateWaterFlow(DeltaTime);
    
    // Step 4: Move water based on flow
    ApplyWaterFlow(DeltaTime);
    
    // Step 5: Handle evaporation and absorption
    ProcessWaterEvaporation(DeltaTime);
    
    // Step 6: Delegate erosion to GeologyController
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        // Find GeologyController and trigger erosion events based on water flow
        AGeologyController* GeologyController = Cast<AGeologyController>(
            UGameplayStatics::GetActorOfClass(OwnerTerrain->GetWorld(), AGeologyController::StaticClass()));
        if (GeologyController)
        {
            // Trigger erosion events where water flow exceeds threshold
            for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
            {
                float VelX = SimulationData.WaterVelocityX[i];
                float VelY = SimulationData.WaterVelocityY[i];
                float FlowSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY);
                if (FlowSpeed > 15.0f) // Minimum erosion velocity
                {
                    int32 X = i % SimulationData.TerrainWidth;
                    int32 Y = i / SimulationData.TerrainWidth;
                    FVector WorldPos = OwnerTerrain->TerrainToWorldPosition(X, Y);
                    float ErosionAmount = FlowSpeed * 0.01f * DeltaTime;
                    GeologyController->OnErosionOccurred(WorldPos, ErosionAmount);
                }
            }
        }
    }
    
    // Step 7: Update surface-based water if enabled (ALWAYS VISIBLE VERSION)
    if (bEnableWaterVolumes)
    {
        // Use new localized mesh system instead of chunk-based
        GenerateLocalizedWaterMeshes();
        
        if (bEnableVerboseLogging)
        {
            static float LastSurfaceLogTime = 0.0f;
            float CurrentTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
            if (CurrentTime - LastSurfaceLogTime >= 2.0f)
            {
                UE_LOG(LogTemp, Log, TEXT("💧 Water surface chunks updated (%d active)"), WaterSurfaceChunks.Num());
                LastSurfaceLogTime = CurrentTime;
            }
        }
    }
    
    // Step 8: Update Niagara FX if enabled (PHASE 4)
    if (bEnableNiagaraFX)
    {
        UpdateNiagaraFX(DeltaTime);
        // Niagara FX updated (verbose logging controlled by bEnableVerboseLogging)
    }
    
    // Step 9: Update shader system with volumetric water + displacement
    if (bUseShaderWater)
    {
        // Shader system updating (reduced logging for performance)
        
        // Calculate foam data for advanced rendering
        CalculateFoamData();
        
        // Use new unified visual update system
        UpdateAllWaterVisuals(DeltaTime);
        
        // Update flow displacement system (Phase 1 & 2) - DISABLED due to threading crashes
        // Displacement system causes LowLevelTasks conflicts in UE5.4
        /*
        static float DisplacementTimer = 0.0f;
        static bool bStartupComplete = false;
        
        // Wait 3 seconds after PIE start before enabling displacement
        if (!bStartupComplete)
        {
            static float StartupTimer = 0.0f;
            StartupTimer += DeltaTime;
            if (StartupTimer > 3.0f)
            {
                bStartupComplete = true;
                UE_LOG(LogTemp, Warning, TEXT("Displacement system startup complete"));
            }
        }
        
        if (bStartupComplete)
        {
            DisplacementTimer += DeltaTime;
            if (DisplacementTimer >= FlowDisplacementUpdateInterval)
            {
                UpdateFlowDisplacementTexture();
                DisplacementTimer = 0.0f;
            }
        }
        */
        
        // Also update parameters every frame
        UpdateWaterShaderParameters();
    }
    // Shader system disabled (logging reduced for performance)
}

// ===== VOLUMETRIC WATER IMPLEMENTATION =====

/**
 * PHASE 1-2: SYNCHRONIZED WATER SURFACE CHUNKS
 * Updates water surface chunks based EXCLUSIVELY on simulation data authority
 * 
 * NEW DATA PIPELINE: [FWaterSimulationData] → [Mesh Generation] → [Procedural Surfaces]
 * 
 * Algorithm:
 * 1. Query simulation state for each chunk (WaterDepthMap as ground truth)
 * 2. Calculate flow properties from velocity arrays (VelocityX/Y → FlowDirection/Speed)
 * 3. Generate visual effects based on FoamMap and simulation state
 * 4. Create surface meshes with simulation-driven parameters
 * 5. Apply materials with physically-based optical properties
 */
void UWaterSystem::UpdateWaterSurfaceChunks()
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !bEnableWaterVolumes)
    {
        return;
    }
    
    // 🔥 PHANTOM CLEANUP: Clean up phantom chunks first
    static float LastPhantomCleanup = 0.0f;
    float CurrentTime = OwnerTerrain->GetWorld() ? OwnerTerrain->GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPhantomCleanup > 2.0f) // Every 2 seconds
    {
        ValidateAndCleanupPhantomChunks();
        LastPhantomCleanup = CurrentTime;
    }
    
    // Reset frame counters
    MeshUpdatesThisFrame = 0;
    
    // Get player camera position for LOD calculations
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
    
    // PERFORMANCE OPTIMIZATION: Spatial culling and throttling
    OptimizeWaterMeshUpdates(CameraLocation, 0.0f);
    
    // ENHANCED CLEANUP: Remove surface chunks where water disappeared OR too far
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(Chunk.ChunkIndex);
        float Distance = FVector::Dist(CameraLocation, ChunkWorldPos);
        
        // Remove if no water (no distance check - always visible)
        float SimDepth = GetChunkMaxDepthFromSimulation(Chunk.ChunkIndex);
        
        if (SimDepth < MinWaterDepth)
        {
            // Return component to pool before removal
            if (Chunk.SurfaceMesh && bEnableComponentPooling)
            {
                ReturnComponentToValidatedPool(Chunk.SurfaceMesh);
                Chunk.SurfaceMesh = nullptr;
            }
            else if (Chunk.SurfaceMesh)
            {
                Chunk.SurfaceMesh->ClearAllMeshSections();
                Chunk.SurfaceMesh->DestroyComponent();
            }
            
            if (Chunk.UndersideMesh)
            {
                Chunk.UndersideMesh->ClearAllMeshSections();
                Chunk.UndersideMesh->DestroyComponent();
            }
            
            WaterSurfaceChunks.RemoveAt(i);
        }
    }
    
    // Update existing chunks and create new ones (with distance/update limits)
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        // Skip chunks without water (no distance limit)
        if (!ShouldGenerateSurfaceForChunk(ChunkIndex))
        {
            continue;
        }
        
        // Find existing surface chunk
        FWaterSurfaceChunk* SurfaceChunk = WaterSurfaceChunks.FindByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& Chunk) { return Chunk.ChunkIndex == ChunkIndex; }
        );
        
        if (!SurfaceChunk && WaterSurfaceChunks.Num() < MaxVolumeChunks)
        {
            // Create new surface chunk
            FWaterSurfaceChunk NewSurfaceChunk;
            NewSurfaceChunk.ChunkIndex = ChunkIndex;
            NewSurfaceChunk.bNeedsUpdate = true;
            NewSurfaceChunk.CurrentLOD = 0; // Always highest quality
            
            WaterSurfaceChunks.Add(NewSurfaceChunk);
            SurfaceChunk = &WaterSurfaceChunks.Last();
        }
        
        if (SurfaceChunk)
        {
            // Synchronize with simulation authority
            SynchronizeChunkWithSimulation(*SurfaceChunk);
            
            // Update mesh if needed (no LOD, just simulation changes)
            if (SurfaceChunk->bNeedsUpdate || bWaterChangedThisFrame)
            {
                CreateWaterSurfaceMesh(*SurfaceChunk);
                SurfaceChunk->bNeedsUpdate = false;
                MeshUpdatesThisFrame++;
            }
        }
    }
    
    // Debug logging (throttled)
    static float LastDebugLogTime = 0.0f;
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        float CurrentTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDebugLogTime >= 5.0f) // Every 5 seconds
        {
            UE_LOG(LogTemp, Log, TEXT("[WATER MESH] Active chunks: %d, Updates this frame: %d, Pool size: %d"), 
            WaterSurfaceChunks.Num(), MeshUpdatesThisFrame, ValidatedMeshPool.Num());
            LastDebugLogTime = CurrentTime;
        }
    }
}

// ===== REMOVED LOD FUNCTIONS =====
// These functions have been removed as part of the LOD system elimination:
// - CalculateWaterMeshLOD()
// - ShouldUpdateWaterMesh()
// - OptimizeWaterMeshUpdates()
// Water meshes now use authority-only generation with configurable quality

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

// ===== REMOVED OLD LOD FUNCTIONS =====
// These functions are no longer needed with the always-visible system:
// bool UWaterSystem::ShouldUpdateWaterMesh() - removed distance checking
// void UWaterSystem::OptimizeWaterMeshUpdates() - removed distance optimization
// Water meshes now appear wherever simulation has water, regardless of distance

// ===== ENHANCED MESH GENERATION WITH LOD =====

/**
 * PHASE 1-2: Synchronizes surface chunk with simulation data authority
 * Replaces manual property setting with simulation-driven data extraction
 * 
 * Implementation follows refactor strategy data pipeline:
 * [FWaterSimulationData] → [WaterSurfaceChunk Properties] → [Mesh Generation]
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
    
    // Only log detailed chunk info occasionally
    static TMap<int32, float> LastLoggedTime;
    float CurrentTime = OwnerTerrain && OwnerTerrain->GetWorld() ? OwnerTerrain->GetWorld()->GetTimeSeconds() : 0.0f;
    float* LastTime = LastLoggedTime.Find(SurfaceChunk.ChunkIndex);
    if (!LastTime || (CurrentTime - *LastTime) > 30.0f) // Only log every 30 seconds per chunk
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 1-2] Chunk %d sync: MaxDepth=%.2f, Flow=%.2f, Foam=%s"),
               SurfaceChunk.ChunkIndex, SurfaceChunk.MaxDepth, SurfaceChunk.FlowSpeed, 
               SurfaceChunk.bHasFoam ? TEXT("YES") : TEXT("NO"));
        LastLoggedTime.Add(SurfaceChunk.ChunkIndex, CurrentTime);
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
 * Provides normalized flow vector for surface animation and Niagara FX
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
 * Provides physically-based foam generation for materials and Niagara FX
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

// ===== PHASE 4: NIAGARA FX INTEGRATION IMPLEMENTATION =====

/**
 * PHASE 4: Main Niagara FX update function
 * Called every frame to manage particle systems based on simulation state
 * Integrates with Phase 1-2 synchronized chunk data for accurate FX placement
 */
void UWaterSystem::UpdateNiagaraFX(float DeltaTime)
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !bEnableNiagaraFX)
    {
        return;
    }
    
    static float NiagaraUpdateTimer = 0.0f;
    NiagaraUpdateTimer += DeltaTime;
    
    // Only update Niagara FX at specified rate (performance optimization)
    if (NiagaraUpdateTimer < NiagaraUpdateRate)
    {
        return;
    }
    NiagaraUpdateTimer = 0.0f;
    
    // Get camera location for distance culling
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
    
    // Clean up distant FX first
    CleanupDistantNiagaraFX(CameraLocation);
    
    // Update FX for active water surface chunks
    for (const FWaterSurfaceChunk& SurfaceChunk : WaterSurfaceChunks)
    {
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
        float DistanceToCamera = FVector::Dist(CameraLocation, ChunkWorldPos);
        
        // Only spawn FX within max distance
        if (DistanceToCamera <= MaxNiagaraDistance)
        {
            SpawnNiagaraFXForChunk(SurfaceChunk.ChunkIndex);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[NIAGARA] Updated FX - Active components: %d"), 
           ActiveNiagaraComponents.Num());
}

/**
 * PHASE 4: Spawns appropriate Niagara FX for a specific chunk
 * Uses synchronized simulation data to determine which effects to spawn
 */
void UWaterSystem::SpawnNiagaraFXForChunk(int32 ChunkIndex)
{
    if (!OwnerTerrain || ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        return;
    }
    
    // Skip if already has active FX for this chunk
    if (ActiveNiagaraComponents.Contains(ChunkIndex))
    {
        // Update existing component parameters
        UNiagaraComponent* ExistingComp = ActiveNiagaraComponents[ChunkIndex];
        if (ExistingComp && IsValid(ExistingComp))
        {
            // Find the corresponding surface chunk for parameters
            const FWaterSurfaceChunk* SurfaceChunk = WaterSurfaceChunks.FindByPredicate(
                [ChunkIndex](const FWaterSurfaceChunk& Chunk) { return Chunk.ChunkIndex == ChunkIndex; }
            );
            
            if (SurfaceChunk)
            {
                UpdateNiagaraParameters(ExistingComp, *SurfaceChunk);
            }
        }
        return;
    }
    
    // Limit number of active components
    if (ActiveNiagaraComponents.Num() >= MaxActiveNiagaraComponents)
    {
        return;
    }
    
    // Get chunk data from simulation (authoritative)
    float MaxDepth = GetChunkMaxDepthFromSimulation(ChunkIndex);
    float FlowSpeed = GetChunkFlowSpeedFromSimulation(ChunkIndex);
    bool bHasFoam = GetChunkFoamStateFromSimulation(ChunkIndex);
    
    // Determine which FX to spawn based on simulation state
    UNiagaraSystem* SystemToSpawn = nullptr;
    
    if (bIsRaining && RainImpactEmitterTemplate)
    {
        SystemToSpawn = RainImpactEmitterTemplate;
    }
    else if (bHasFoam && FoamEmitterTemplate)
    {
        SystemToSpawn = FoamEmitterTemplate;
    }
    else if (FlowSpeed > 10.0f && RiverFlowEmitterTemplate)
    {
        SystemToSpawn = RiverFlowEmitterTemplate;
    }
    else if (MaxDepth > 5.0f && FlowSpeed < 2.0f && LakeMistEmitterTemplate)
    {
        SystemToSpawn = LakeMistEmitterTemplate;
    }
    
    if (SystemToSpawn)
    {
        // Get pooled component
        UNiagaraComponent* NiagaraComp = GetPooledNiagaraComponent(SystemToSpawn);
        if (NiagaraComp)
        {
            // Position at chunk location
            FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(ChunkIndex);
            NiagaraComp->SetWorldLocation(ChunkWorldPos);
            
            // Set parameters based on simulation data
            const FWaterSurfaceChunk* SurfaceChunk = WaterSurfaceChunks.FindByPredicate(
                [ChunkIndex](const FWaterSurfaceChunk& Chunk) { return Chunk.ChunkIndex == ChunkIndex; }
            );
            
            if (SurfaceChunk)
            {
                UpdateNiagaraParameters(NiagaraComp, *SurfaceChunk);
            }
            
            // Activate the system
            NiagaraComp->Activate();
            
            // Track active component
            ActiveNiagaraComponents.Add(ChunkIndex, NiagaraComp);
            
            UE_LOG(LogTemp, Log, TEXT("[NIAGARA] Spawned FX for chunk %d - Depth:%.2f, Flow:%.2f, Foam:%s"),
                   ChunkIndex, MaxDepth, FlowSpeed, bHasFoam ? TEXT("YES") : TEXT("NO"));
        }
    }
}

/**
 * PHASE 4: Updates Niagara component parameters with simulation data
 * Provides real-time parameter updates for dynamic water effects
 */
void UWaterSystem::UpdateNiagaraParameters(UNiagaraComponent* NiagaraComp, const FWaterSurfaceChunk& SurfaceChunk)
{
    if (!NiagaraComp || !IsValid(NiagaraComp))
    {
        return;
    }
    
    // Core water parameters (from Phase 1-2 synchronized data)
    NiagaraComp->SetVectorParameter(FName("FlowVector"), 
        FVector(SurfaceChunk.FlowDirection.X, SurfaceChunk.FlowDirection.Y, 0.0f));
    
    NiagaraComp->SetFloatParameter(FName("WaterDepth"), SurfaceChunk.AverageDepth);
    NiagaraComp->SetFloatParameter(FName("FlowSpeed"), SurfaceChunk.FlowSpeed);
    NiagaraComp->SetFloatParameter(FName("MaxDepth"), SurfaceChunk.MaxDepth);
    
    // Foam and visual effects
    float FoamIntensity = SurfaceChunk.bHasFoam ? 1.0f : 0.0f;
    NiagaraComp->SetFloatParameter(FName("FoamIntensity"), FoamIntensity);
    
    // Environmental parameters
    float RainValue = bIsRaining ? RainIntensity : 0.0f;
    NiagaraComp->SetFloatParameter(FName("RainIntensity"), RainValue);
    
    // Time-based animation
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        float GameTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
        NiagaraComp->SetFloatParameter(FName("GameTime"), GameTime);
        NiagaraComp->SetFloatParameter(FName("WavePhase"), SurfaceChunk.WavePhase);
    }
    
    // Texture data interfaces (if available)
    if (WaterDepthTexture)
    {
        NiagaraComp->SetVariableObject(FName("WaterDepthTexture"), WaterDepthTexture);
    }
    
    if (WaterDataTexture)
    {
        NiagaraComp->SetVariableObject(FName("WaterDataTexture"), WaterDataTexture);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[NIAGARA] Updated parameters - Flow:%.2f, Depth:%.2f, Foam:%.2f"),
           SurfaceChunk.FlowSpeed, SurfaceChunk.AverageDepth, FoamIntensity);
}

/**
 * PHASE 4: Component pooling for performance optimization
 * Reuses Niagara components to avoid constant allocation/deallocation
 */
UNiagaraComponent* UWaterSystem::GetPooledNiagaraComponent(UNiagaraSystem* SystemTemplate)
{
    if (!SystemTemplate || !OwnerTerrain)
    {
        return nullptr;
    }
    
    // Try to find a pooled component with matching system
    for (int32 i = NiagaraComponentPool.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* PooledComp = NiagaraComponentPool[i];
        if (PooledComp && IsValid(PooledComp) && PooledComp->GetAsset() == SystemTemplate)
        {
            NiagaraComponentPool.RemoveAt(i);
            return PooledComp;
        }
    }
    
    // Create new component if none available
    UNiagaraComponent* NewComp = NewObject<UNiagaraComponent>(OwnerTerrain);
    if (NewComp)
    {
        NewComp->SetAsset(SystemTemplate);
        NewComp->SetupAttachment(OwnerTerrain->GetRootComponent());
        NewComp->RegisterComponent();
        
        UE_LOG(LogTemp, Log, TEXT("[NIAGARA] Created new component for system: %s"), 
               SystemTemplate ? *SystemTemplate->GetName() : TEXT("NULL"));
    }
    
    return NewComp;
}

/**
 * PHASE 4: Returns component to pool for reuse
 */
void UWaterSystem::ReturnNiagaraComponentToPool(UNiagaraComponent* Component)
{
    if (!Component || !IsValid(Component))
    {
        return;
    }
    
    // Deactivate and reset
    Component->Deactivate();
    Component->SetWorldLocation(FVector::ZeroVector);
    
    // Add to pool if not too many
    if (NiagaraComponentPool.Num() < MaxActiveNiagaraComponents)
    {
        NiagaraComponentPool.Add(Component);
    }
    else
    {
        // Destroy if pool is full
        Component->DestroyComponent();
    }
}

/**
 * PHASE 4: Cleans up distant Niagara FX for performance
 */
void UWaterSystem::CleanupDistantNiagaraFX(FVector CameraLocation)
{
    TArray<int32> ChunksToRemove;
    
    for (auto& Pair : ActiveNiagaraComponents)
    {
        int32 ChunkIndex = Pair.Key;
        UNiagaraComponent* NiagaraComp = Pair.Value;
        
        if (!NiagaraComp || !IsValid(NiagaraComp))
        {
            ChunksToRemove.Add(ChunkIndex);
            continue;
        }
        
        // Check distance
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(ChunkIndex);
        float Distance = FVector::Dist(CameraLocation, ChunkWorldPos);
        
        if (Distance > MaxNiagaraDistance * 1.2f) // Hysteresis to prevent flickering
        {
            ReturnNiagaraComponentToPool(NiagaraComp);
            ChunksToRemove.Add(ChunkIndex);
        }
    }
    
    // Remove cleaned up components
    for (int32 ChunkIndex : ChunksToRemove)
    {
        ActiveNiagaraComponents.Remove(ChunkIndex);
    }
    
    if (ChunksToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[NIAGARA] Cleaned up %d distant FX components"), ChunksToRemove.Num());
    }
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
    
    // Create RGBA texture for flow displacement
    // R: Flow displacement X, G: Flow displacement Y
    // B: Wave phase offset, A: Flow intensity
    FlowDisplacementTexture = UTexture2D::CreateTransient(Width, Height, PF_FloatRGBA);
    
    if (FlowDisplacementTexture)
    {
        FlowDisplacementTexture->Filter = TextureFilter::TF_Bilinear;
        FlowDisplacementTexture->AddressX = TextureAddress::TA_Wrap;
        FlowDisplacementTexture->AddressY = TextureAddress::TA_Wrap;
        FlowDisplacementTexture->SRGB = false;
        FlowDisplacementTexture->UpdateResource();
        
        UE_LOG(LogTemp, Warning, TEXT("Flow displacement texture created: %dx%d RGBA"), Width, Height);
        
        // DON'T populate immediately - wait for first update cycle
        // UpdateFlowDisplacementTexture(); // REMOVED - causes crash
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create flow displacement texture"));
    }
}

void UWaterSystem::UpdateFlowDisplacementTexture()
{
    if (!FlowDisplacementTexture || !SimulationData.IsValid())
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create displacement data
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        float VelX = SimulationData.WaterVelocityX[i];
        float VelY = SimulationData.WaterVelocityY[i];
        float FlowIntensity = FMath::Sqrt(VelX * VelX + VelY * VelY);
        
        // Convert to displacement vectors
        FLinearColor DisplacementData(
            VelX * DisplacementScale,                    // Flow X
            VelY * DisplacementScale,                    // Flow Y
            GenerateWavePhase(i),                        // Wave offset
            FMath::Clamp(FlowIntensity / 50.0f, 0, 1)   // Intensity
        );
        
        TextureData[i] = DisplacementData;
    }
    
    // Upload to GPU
    if (FlowDisplacementTexture->GetPlatformData() && FlowDisplacementTexture->GetPlatformData()->Mips.Num() > 0)
    {
        FTexture2DMipMap& Mip = FlowDisplacementTexture->GetPlatformData()->Mips[0];
        void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
        if (Data)
        {
            FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num() * sizeof(FLinearColor));
            Mip.BulkData.Unlock();
            FlowDisplacementTexture->UpdateResource();
        }
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
    if (!SimulationData.IsValid() || Index >= SimulationData.WaterDepthMap.Num())
    {
        return 0.0f;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    int32 X = Index % Width;
    int32 Y = Index / Width;
    
    // Generate wave phase based on position and flow
    float BasePhase = (X * 0.1f + Y * 0.15f) * 2.0f * UE_PI;
    float FlowModulation = SimulationData.WaterVelocityX[Index] * 0.05f + SimulationData.WaterVelocityY[Index] * 0.05f;
    
    return BasePhase + FlowModulation;
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

/**
 * PHASE 2: Enhanced phantom chunk cleanup with simulation authority validation
 * Actively removes meshes that don't match simulation reality
 * Prevents water chunks from persisting without simulation authorization
 */
void UWaterSystem::ValidateAndCleanupPhantomChunks()
{
    if (!OwnerTerrain || !SimulationData.IsValid())
    {
        return;
    }
    
    int32 RemovedChunks = 0;
    int32 ValidatedChunks = 0;
    
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        
        // PHASE 2: Validate against simulation authority
        float SimDepth = GetChunkMaxDepthFromSimulation(Chunk.ChunkIndex);
        bool bSimulationHasWater = (SimDepth >= MinVolumeDepth);
        bool bChunkQualifies = ShouldGenerateSurfaceForChunk(Chunk.ChunkIndex);
        
        // PHASE 2: Remove if simulation doesn't authorize this chunk
        if (!bSimulationHasWater || !bChunkQualifies)
        {
            UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] Removing phantom chunk %d - SimDepth:%.3f SimHasWater:%s Qualifies:%s"),
                   Chunk.ChunkIndex, SimDepth, bSimulationHasWater ? TEXT("YES") : TEXT("NO"), 
                   bChunkQualifies ? TEXT("YES") : TEXT("NO"));
            
            // Clean up mesh components
            if (Chunk.SurfaceMesh)
            {
                if (bEnableComponentPooling)
                {
                    ReturnComponentToValidatedPool(Chunk.SurfaceMesh);
                }
                else
                {
                    Chunk.SurfaceMesh->ClearAllMeshSections();
                    Chunk.SurfaceMesh->DestroyComponent();
                }
                Chunk.SurfaceMesh = nullptr;
            }
            
            if (Chunk.UndersideMesh)
            {
                Chunk.UndersideMesh->ClearAllMeshSections();
                Chunk.UndersideMesh->DestroyComponent();
                Chunk.UndersideMesh = nullptr;
            }
            
            WaterSurfaceChunks.RemoveAt(i);
            RemovedChunks++;
        }
        else
        {
            ValidatedChunks++;
        }
    }
    
    if (RemovedChunks > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 2] Phantom cleanup complete - Removed:%d Validated:%d Remaining:%d"), 
               RemovedChunks, ValidatedChunks, WaterSurfaceChunks.Num());
    }
}

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
/*
bool UWaterSystem::IsSystemScaled() const
{
    return bIsScaledByMaster && CachedMasterController != nullptr;
}
*/
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
    if (!bIsScaledByMaster)
    {
        return TEXT("WaterSystem: Not scaled by master");
    }
    
    return FString::Printf(TEXT("WaterSystem: OK - Simulation:%dx%d, WaterCells:%d, MaxDepth:%.2f, FlowSpeed:%.2f"),
                          CurrentWorldConfig.WaterConfig.SimulationArrayWidth,
                          CurrentWorldConfig.WaterConfig.SimulationArrayHeight,
                          CurrentWorldConfig.WaterConfig.WaterCellScale,
                          CurrentWorldConfig.WaterConfig.CoordinateScale);
}

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
    if (SimMaxDepth < MinMeshDepth || SimAvgDepth < (MinWaterDepth * 1.5f))
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

void UWaterSystem::UpdateWaterSurfaceChunks_AlwaysVisible()
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !bEnableWaterVolumes)
    {
        return;
    }
    
    // Reset frame counters
    MeshUpdatesThisFrame = 0;
    
    // CLEANUP: Remove chunks where simulation no longer has water
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        FWaterSurfaceChunk& Chunk = WaterSurfaceChunks[i];
        
        // Check if simulation still authorizes this chunk
        if (!ShouldGenerateSurfaceForChunk_AuthorityOnly(Chunk.ChunkIndex))
        {
            UE_LOG(LogTemp, Log, TEXT("[AUTHORITY] Removing water chunk %d - simulation no longer has water"), 
                   Chunk.ChunkIndex);
            
            // Clean up mesh components
            if (Chunk.SurfaceMesh)
            {
                if (bEnableComponentPooling)
                {
                    ReturnComponentToValidatedPool(Chunk.SurfaceMesh);
                }
                else
                {
                    Chunk.SurfaceMesh->ClearAllMeshSections();
                    Chunk.SurfaceMesh->DestroyComponent();
                }
                Chunk.SurfaceMesh = nullptr;
            }
            
            WaterSurfaceChunks.RemoveAt(i);
        }
    }
    
    // CREATE/UPDATE: Process all terrain chunks
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        // Check if simulation authorizes water mesh for this chunk
        bool bShouldHaveWater = ShouldGenerateSurfaceForChunk_AuthorityOnly(ChunkIndex);
        
        // Find existing surface chunk
        FWaterSurfaceChunk* SurfaceChunk = WaterSurfaceChunks.FindByPredicate(
            [ChunkIndex](const FWaterSurfaceChunk& Chunk) { return Chunk.ChunkIndex == ChunkIndex; }
        );
        
        if (bShouldHaveWater)
        {
            if (!SurfaceChunk && WaterSurfaceChunks.Num() < MaxVolumeChunks)
            {
                // Create new surface chunk - simulation authorizes it
                FWaterSurfaceChunk NewSurfaceChunk;
                NewSurfaceChunk.ChunkIndex = ChunkIndex;
                NewSurfaceChunk.bNeedsUpdate = true;
                NewSurfaceChunk.CurrentLOD = 0; // Always highest quality
                
                WaterSurfaceChunks.Add(NewSurfaceChunk);
                SurfaceChunk = &WaterSurfaceChunks.Last();
                
                UE_LOG(LogTemp, Log, TEXT("[AUTHORITY] Created water chunk %d - simulation has water"), ChunkIndex);
            }
            
            if (SurfaceChunk)
            {
                // Synchronize with simulation data
                SynchronizeChunkWithSimulation(*SurfaceChunk);
                
                // Update mesh if needed (no LOD changes, just simulation changes)
                if (SurfaceChunk->bNeedsUpdate || bWaterChangedThisFrame)
                {
                    CreateWaterSurfaceMesh_AlwaysVisible(*SurfaceChunk);
                    SurfaceChunk->bNeedsUpdate = false;
                    MeshUpdatesThisFrame++;
                }
            }
        }
    }
    
    // Debug logging
    static float LastDebugLogTime = 0.0f;
    if (OwnerTerrain && OwnerTerrain->GetWorld())
    {
        float CurrentTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDebugLogTime >= 5.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("[ALWAYS VISIBLE] Active water chunks: %d, Updates this frame: %d"), 
                   WaterSurfaceChunks.Num(), MeshUpdatesThisFrame);
            LastDebugLogTime = CurrentTime;
        }
    }
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
        
        // Position mesh at chunk location
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
        SurfaceChunk.SurfaceMesh->SetWorldLocation(ChunkWorldPos);
    }
    
    // Generate mesh data
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    // Use configurable quality setting
    GenerateSmoothWaterSurface_HighQuality(SurfaceChunk, Vertices, Triangles, Normals, UVs, VertexColors);
    
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

void UWaterSystem::GenerateSmoothWaterSurface_HighQuality(FWaterSurfaceChunk& SurfaceChunk,
                                                         TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                                         TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                                         TArray<FColor>& VertexColors)
{
    // Use configurable quality setting
    int32 Resolution = FMath::Clamp(WaterMeshQuality, 8, 256);
    
    float ChunkSize = OwnerTerrain->ChunkSize - 1;
    float CellSize = (OwnerTerrain->TerrainScale * ChunkSize) / (float)(Resolution - 1);
    
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
    // Reserve arrays for performance
    int32 VertexCount = Resolution * Resolution;
    Vertices.Reserve(VertexCount);
    Normals.Reserve(VertexCount);
    UVs.Reserve(VertexCount);
    VertexColors.Reserve(VertexCount);
    
    SurfaceChunk.MaxDepth = 0.0f;
    SurfaceChunk.AverageDepth = 0.0f;
    float TotalDepth = 0.0f;
    int32 WaterVertices = 0;
    
    // Generate vertices using simulation authority
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D WorldSamplePos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            // AUTHORITY: Get water depth from simulation (single source of truth)
            float WaterDepth = GetSimulationDepth(WorldSamplePos);
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldSamplePos.X, WorldSamplePos.Y, 0));
            
            if (WaterDepth > MinWaterDepth)
            {
                // Create water vertex
                FVector VertexPos(LocalX, LocalY, TerrainHeight + WaterDepth);
                Vertices.Add(VertexPos);
                
                // Calculate normal from simulation flow
                FVector2D FlowVector = GetSimulationVelocity(WorldSamplePos);
                FVector Normal = CalculateWaterNormal(WorldSamplePos, FlowVector, WaterDepth);
                Normals.Add(Normal);
                
                // UV coordinates
                float U = (float)X / (Resolution - 1);
                float V = (float)Y / (Resolution - 1);
                UVs.Add(FVector2D(U, V));
                
                // Color based on depth and flow
                float FlowSpeed = FlowVector.Size();
                FColor VertexColor = FColor(
                    FMath::Clamp(WaterDepth * 25.0f, 0.0f, 255.0f),     // Red: Depth
                    FMath::Clamp(FlowSpeed * 5.0f, 0.0f, 255.0f),       // Green: Flow
                    128,                                                  // Blue: Constant
                    255                                                   // Alpha: Full
                );
                VertexColors.Add(VertexColor);
                
                // Track depth statistics
                SurfaceChunk.MaxDepth = FMath::Max(SurfaceChunk.MaxDepth, WaterDepth);
                TotalDepth += WaterDepth;
                WaterVertices++;
            }
            else
            {
                // No water - create terrain-level vertex for seamless edges
                FVector VertexPos(LocalX, LocalY, TerrainHeight);
                Vertices.Add(VertexPos);
                Normals.Add(FVector::UpVector);
                
                float U = (float)X / (Resolution - 1);
                float V = (float)Y / (Resolution - 1);
                UVs.Add(FVector2D(U, V));
                
                VertexColors.Add(FColor(0, 0, 128, 0)); // Transparent for no water
            }
        }
    }
    
    SurfaceChunk.AverageDepth = WaterVertices > 0 ? (TotalDepth / WaterVertices) : 0.0f;
    
    // Generate triangles
    GenerateWaterSurfaceTriangles(Resolution, Triangles);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[CONFIGURABLE QUALITY] Generated %d vertices at resolution %d, max depth %.2f"), 
           Vertices.Num(), Resolution, SurfaceChunk.MaxDepth);
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
    UE_LOG(LogTemp, Warning, TEXT("Terrain height cache refreshed (%d cells)"), TotalCells);
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
    
    UE_LOG(LogTemp, Warning, TEXT("Water velocities reset - forcing reflow"));
}

void UWaterSystem::ForceTerrainSync()
{
    RefreshTerrainHeightCache();
    ForceWaterReflow();
    
    UE_LOG(LogTemp, Warning, TEXT("Manual terrain sync completed"));
}

// Enhanced GetTerrainHeightSafe using cache
float UWaterSystem::GetTerrainHeightSafe(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y))
        return 0.0f;
    
    int32 Index = Y * SimulationData.TerrainWidth + X;
    
    // Use cached heights if available
    if (CachedTerrainHeights.IsValidIndex(Index))
    {
        return CachedTerrainHeights[Index];
    }
    
    // Fallback to direct lookup (slower)
    if (OwnerTerrain)
    {
        FVector WorldPos = OwnerTerrain->TerrainToWorldPosition(X, Y);
        return OwnerTerrain->GetHeightAtPosition(WorldPos);
    }
    
    return 0.0f;
}

FVector2D UWaterSystem::GetFlowVectorAtWorld(FVector2D WorldPos) const
{
    return GetSimulationVelocity(WorldPos);
}

FVector UWaterSystem::CalculateWaterNormal(FVector2D WorldPos, FVector2D FlowVector, float WaterDepth) const
{
    FVector BaseNormal = FVector(0, 0, 1);
    
    if (WaterDepth > MinWaterDepth && FlowVector.Size() > 0.1f)
    {
        float TiltAmount = FMath::Clamp(FlowVector.Size() / 20.0f, 0.0f, 0.1f);
        BaseNormal.X += FlowVector.X * TiltAmount;
        BaseNormal.Y += FlowVector.Y * TiltAmount;
        BaseNormal = BaseNormal.GetSafeNormal();
    }
    
    return BaseNormal;
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

// ===== LOCALIZED MESH GENERATION =====

void UWaterSystem::GenerateLocalizedWaterMeshes()
{
    if (!OwnerTerrain || !SimulationData.IsValid() || !CachedMasterController)
    {
        return;
    }
    
    // Sample simulation data every 4 terrain units to find water regions
    TArray<FWaterMeshRegion> NewRegions;
    float SampleDistance = OwnerTerrain->TerrainScale * 4.0f;
    
    for (int32 Y = 0; Y < SimulationData.TerrainHeight; Y += 4)
    {
        for (int32 X = 0; X < SimulationData.TerrainWidth; X += 4)
        {
            int32 Index = Y * SimulationData.TerrainWidth + X;
            if (Index < SimulationData.WaterDepthMap.Num() && 
                SimulationData.WaterDepthMap[Index] > MinWaterDepth)
            {
                FVector TerrainPos = OwnerTerrain->TerrainToWorldPosition(X, Y);
                FVector2D WorldPos(TerrainPos.X, TerrainPos.Y);
                
                if (ShouldGenerateLocalizedMesh(WorldPos, SampleDistance))
                {
                    FWaterMeshRegion Region;
                    Region.CenterPosition = WorldPos;
                    Region.MeshRadius = SampleDistance * 2.0f;
                    Region.bNeedsUpdate = true;
                    NewRegions.Add(Region);
                }
            }
        }
    }
    
    // Update regions (simplified - replace existing system)
    for (FWaterMeshRegion& Region : WaterMeshRegions)
    {
        if (Region.MeshComponent)
        {
            Region.MeshComponent->DestroyComponent();
        }
    }
    WaterMeshRegions = NewRegions;
    
    // Create new meshes
    for (FWaterMeshRegion& Region : WaterMeshRegions)
    {
        CreateRegionMesh(Region);
    }
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

void UWaterSystem::CreateRegionMesh(FWaterMeshRegion& Region)
{
    // Validate simulation authorizes this region
    float CenterDepth = GetSimulationDepthAuthority(Region.CenterPosition);
    if (CenterDepth < MinVolumeDepth)
    {
        return;
    }
    
    // Create mesh component
    if (!Region.MeshComponent)
    {
        Region.MeshComponent = NewObject<UProceduralMeshComponent>(OwnerTerrain);
        Region.MeshComponent->SetupAttachment(OwnerTerrain->GetRootComponent());
        Region.MeshComponent->RegisterComponent();
    }
    
    // Generate localized mesh data
    int32 Resolution = WaterMeshQuality;
    float CellSize = (Region.MeshRadius * 2.0f) / (Resolution - 1);
    FVector2D MeshMin = Region.CenterPosition - FVector2D(Region.MeshRadius, Region.MeshRadius);
    
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    
    // Generate vertices only where water exists
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            FVector2D LocalPos(X * CellSize, Y * CellSize);
            FVector2D WorldPos = MeshMin + LocalPos;
            
            float WaterDepth = GetSimulationDepthAuthority(WorldPos);
            if (WaterDepth > MinWaterDepth)
            {
                float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldPos.X, WorldPos.Y, 0));
                FVector VertexPos(WorldPos.X, WorldPos.Y, TerrainHeight + WaterDepth); // Use world coordinates
                Vertices.Add(VertexPos);
                
                Normals.Add(FVector::UpVector);
                UVs.Add(FVector2D((float)X / (Resolution - 1), (float)Y / (Resolution - 1)));
                VertexColors.Add(FColor(0, 100, 255, 255));
            }
        }
    }
    
    // Simple triangle generation (for demonstration)
    if (Vertices.Num() >= 3)
    {
        // Throttled logging - only log every 10th mesh creation
        static int32 MeshCount = 0;
        MeshCount++;
        
        for (int32 i = 0; i < Vertices.Num() - 2; i += 3)
        {
            Triangles.Add(i);
            Triangles.Add(i + 1);
            Triangles.Add(i + 2);
        }
        
        Region.MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors,
                                               TArray<FProcMeshTangent>(), true);
        
        // Apply material
        if (VolumeMaterial)
        {
            Region.MeshComponent->SetMaterial(0, VolumeMaterial);
        }
        
        Region.MeshComponent->SetWorldLocation(FVector::ZeroVector);
        Region.MeshComponent->SetRelativeLocation(FVector::ZeroVector);
        
        // Only log every 10th mesh to reduce spam
        if (MeshCount % 10 == 1)
        {
            UE_LOG(LogTemp, Log, TEXT("[LOCALIZED] Created %d meshes, latest at %.0f,%.0f with %d vertices"), 
                   MeshCount, Region.CenterPosition.X, Region.CenterPosition.Y, Vertices.Num());
        }
    }
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
    
    // Simulation authorizes mesh creation - proceed
    UE_LOG(LogTemp, Log, TEXT("[PHASE 2] ✓ Simulation authorizes mesh creation for chunk %d (depth: %.2f)"), 
           SurfaceChunk.ChunkIndex, SimDepth);
    
    UE_LOG(LogTemp, Warning, TEXT("[MESH DEBUG] ✓ Proceeding with mesh creation for chunk %d"), SurfaceChunk.ChunkIndex);
    
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
    
    // Generate smooth surface geometry
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
    
    // Set mesh position to chunk world location
    SurfaceChunk.SurfaceMesh->SetWorldLocation(ChunkWorldPos);
    
    SurfaceChunk.LastUpdateTime = OwnerTerrain->GetWorld()->GetTimeSeconds();
    
    
    /* Commented Out for Overloading Log, Maintain Code For Later Optimizations
    // Only log successful mesh creation occasionally
    static int32 MeshCreationCount = 0;
    MeshCreationCount++;
    if (MeshCreationCount % 20 == 1) // Only log every 20th mesh creation
    {
        UE_LOG(LogTemp, Log, TEXT("[SURFACE] Mesh generation: %d meshes created, Chunk:%d Vertices:%d MaxDepth:%.2f"),
               MeshCreationCount, SurfaceChunk.ChunkIndex, Vertices.Num(), SurfaceChunk.MaxDepth);
    }
     */
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

void UWaterSystem::GenerateSmoothWaterSurface(FWaterSurfaceChunk& SurfaceChunk,
                                             TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                             TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                             TArray<FColor>& VertexColors)
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return;
    
    int32 Resolution = FMath::Max(8, BaseSurfaceResolution >> SurfaceChunk.CurrentLOD);
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WATER MESH] Generating chunk %d with LOD %d (Resolution: %dx%d)"), 
           SurfaceChunk.ChunkIndex, SurfaceChunk.CurrentLOD, Resolution, Resolution);
    
    // SIMPLIFIED APPROACH: Always create a complete grid, set water height = terrain height for dry areas
    float ChunkSize = OwnerTerrain->ChunkSize - 1;
    float CellSize = (OwnerTerrain->TerrainScale * ChunkSize) / (float)(Resolution - 1);
    
    // Pre-check for any water in chunk
    bool bHasAnyWater = false;
    for (int32 TestY = 0; TestY < Resolution && !bHasAnyWater; TestY += 4)
    {
        for (int32 TestX = 0; TestX < Resolution && !bHasAnyWater; TestX += 4)
        {
            float LocalX = TestX * CellSize;
            float LocalY = TestY * CellSize;
            FVector2D WorldPos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            if (GetInterpolatedWaterDepth(WorldPos) > MinWaterDepth)
            {
                bHasAnyWater = true;
            }
        }
    }
    
    if (!bHasAnyWater)
    {
        // Clear mesh completely
        Vertices.Empty();
        Triangles.Empty();
        Normals.Empty();
        UVs.Empty();
        VertexColors.Empty();
        return;
    }
    
    // CREATE COMPLETE GRID - no missing vertices
    SurfaceChunk.MaxDepth = 0.0f;
    SurfaceChunk.AverageDepth = 0.0f;
    int32 WaterVertexCount = 0;
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D WorldSamplePos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            float WaterDepth = GetInterpolatedWaterDepth(WorldSamplePos);
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(FVector(WorldSamplePos.X, WorldSamplePos.Y, 0));
            
            // For areas without water, use terrain height (creates flat surface)
            float SurfaceHeight = TerrainHeight;
            if (WaterDepth > MinWaterDepth)
            {
                SurfaceHeight = TerrainHeight + WaterDepth;
                SurfaceChunk.MaxDepth = FMath::Max(SurfaceChunk.MaxDepth, WaterDepth);
                SurfaceChunk.AverageDepth += WaterDepth;
                WaterVertexCount++;
                
                // Add subtle waves for water areas
                if (OwnerTerrain->GetWorld())
                {
                    float Time = OwnerTerrain->GetWorld()->GetTimeSeconds();
                    float WaveOffset = FMath::Sin(LocalX * 0.02f + Time * 2.0f) * 
                                      FMath::Sin(LocalY * 0.02f + Time * 1.5f) * 
                                      FMath::Min(WaterDepth * 0.1f, 2.0f);
                    SurfaceHeight += WaveOffset;
                }
            }
            
            // Create vertex (always exists - no gaps)
            FVector VertexPos(LocalX, LocalY, SurfaceHeight - ChunkWorldPos.Z);
            Vertices.Add(VertexPos);
            
            // Normal calculation
            FVector2D FlowVector = GetFlowVectorAtWorld(WorldSamplePos);
            FVector Normal = CalculateWaterNormal(WorldSamplePos, FlowVector, WaterDepth);
            Normals.Add(Normal);
            
            // UV mapping
            float U = (float)X / (Resolution - 1);
            float V = (float)Y / (Resolution - 1);
            UVs.Add(FVector2D(U, V));
            
            // Color: Blue for water, transparent for dry areas
            uint8 Alpha = (WaterDepth > MinWaterDepth) ? 
                         FMath::Clamp(WaterDepth / 3.0f * 255, 50, 255) : 0;
            VertexColors.Add(FColor(0, 100, 255, Alpha));
        }
    }
    
    // SIMPLE GRID TRIANGULATION - no missing vertices to worry about
    Triangles.Empty();
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            int32 TopLeft = Y * Resolution + X;
            int32 TopRight = Y * Resolution + (X + 1);
            int32 BottomLeft = (Y + 1) * Resolution + X;
            int32 BottomRight = (Y + 1) * Resolution + (X + 1);
            
            // First triangle
            Triangles.Add(TopLeft);
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);
            
            // Second triangle
            Triangles.Add(TopRight);
            Triangles.Add(BottomLeft);
            Triangles.Add(BottomRight);
        }
    }
    
    // Finalize statistics
    if (WaterVertexCount > 0)
    {
        SurfaceChunk.AverageDepth /= WaterVertexCount;
    }
}

void UWaterSystem::GenerateWaterSurfaceTriangles(int32 Resolution, TArray<int32>& Triangles)
{
    // SIMPLIFIED: Standard grid triangulation
    Triangles.Empty();
    
    for (int32 Y = 0; Y < Resolution - 1; Y++)
    {
        for (int32 X = 0; X < Resolution - 1; X++)
        {
            int32 TopLeft = Y * Resolution + X;
            int32 TopRight = Y * Resolution + (X + 1);
            int32 BottomLeft = (Y + 1) * Resolution + X;
            int32 BottomRight = (Y + 1) * Resolution + (X + 1);
            
            // Two triangles per quad - no vertex existence checking needed
            Triangles.Add(TopLeft);
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);
            
            Triangles.Add(TopRight);
            Triangles.Add(BottomLeft);
            Triangles.Add(BottomRight);
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

int32 UWaterSystem::CalculateWaterMeshLODStable(float Distance) const
{
    // Always return 0 for highest quality in always-visible system
    // This function is kept for compatibility but LOD is not used
    return 0;
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
        
        // Position mesh
        FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
        SurfaceChunk.SurfaceMesh->SetWorldLocation(ChunkWorldPos);
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
            
            float WaterDepth = GetInterpolatedWaterDepthFixed(WorldPos);
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

// ===== MISSING SCALABLE LOD FUNCTIONS =====

float UWaterSystem::GetDynamicLODDistance(int32 LODLevel) const
{
    // LOD system has been removed - return large distance for compatibility
    // In always-visible system, distance doesn't affect water mesh creation
    return 10000.0f; // Always allow mesh creation
}

void UWaterSystem::SetLODScaling(float ScaleMultiplier, float LOD0Factor, float LOD1Factor, float LOD2Factor)
{
    // LOD system has been removed - this function is kept for compatibility
    UE_LOG(LogTemp, Warning, TEXT("[LOD] LOD scaling disabled - using always-visible water system"));
}

FString UWaterSystem::GetLODDebugInfo() const
{
    return TEXT("WaterSystem LOD: Always-visible system active (LOD disabled)");
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
    if (!SurfaceChunk.SurfaceMesh || !CachedMasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PHASE 1] Cannot update UV mapping - missing authority or mesh"));
        return;
    }
    
    // SINGLE AUTHORITY: Always use MasterController for live dimensions
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();  // ✅ Live dimensions
    float TerrainScale = CachedMasterController->GetTerrainScale();      // ✅ Live scale
    
    // Calculate UV offset using current world bounds (no caching)
    FVector2D ChunkUVOffset(
        ChunkWorldPos.X / (TerrainScale * WorldDims.X),
        ChunkWorldPos.Y / (TerrainScale * WorldDims.Y)
    );
    
    // Apply to material with validation
    UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(
        SurfaceChunk.SurfaceMesh->GetMaterial(0));
    
    if (MaterialInstance)
    {
        MaterialInstance->SetVectorParameterValue(FName("ChunkUVOffset"),
            FLinearColor(ChunkUVOffset.X, ChunkUVOffset.Y, 0, 0));
        
        // Also update world scale parameters for shader consistency
        MaterialInstance->SetScalarParameterValue(FName("WorldScale"), TerrainScale);
        MaterialInstance->SetVectorParameterValue(FName("WorldDimensions"),
            FLinearColor(WorldDims.X, WorldDims.Y, 0, 0));
            
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 1] Updated UV mapping - Chunk %d, Offset(%.3f,%.3f)"),
               SurfaceChunk.ChunkIndex, ChunkUVOffset.X, ChunkUVOffset.Y);
    }
}


void UWaterSystem::ResetWaterSystem()
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    // Reset all water data
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        SimulationData.WaterDepthMap[i] = 0.0f;
        SimulationData.WaterVelocityX[i] = 0.0f;
        SimulationData.WaterVelocityY[i] = 0.0f;
        SimulationData.SedimentMap[i] = 0.0f;
    }
    
    // Reset weather
    bIsRaining = false;
    WeatherTimer = 0.0f;
    
    // Clear tracking data
    ChunksWithWater.Empty();
    TotalWaterAmount = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Reset complete"));
}

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
 */
void UWaterSystem::CalculateWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    // Create temporary arrays for new velocities
    TArray<float> NewVelocityX = SimulationData.WaterVelocityX;
    TArray<float> NewVelocityY = SimulationData.WaterVelocityY;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // STEP 1: Process each cell for pressure-based flow
    // Note: Includes edges for realistic waterfall drainage
    for (int32 Y = 0; Y < Height; Y++)
    {
    for (int32 X = 0; X < Width; X++)
    {
    int32 Index = Y * Width + X;
    
    // Skip cells with minimal water (performance optimization)
    if (SimulationData.WaterDepthMap[Index] <= MinWaterDepth)
    {
        continue;
                }
            
            // ✅ FIXED: Use MasterController authority for terrain height
            FVector WorldPos = CachedMasterController ? 
                CachedMasterController->TerrainToWorldPosition(FVector2D(X, Y)) :
                OwnerTerrain->TerrainToWorldPosition(X, Y);
            
            float TerrainHeight = OwnerTerrain->GetHeightAtPosition(WorldPos);
            
            // ✅ CRITICAL: Validate height data is current
            if (CachedMasterController)
            {
                // Force fresh terrain height read after dramatic changes
                TerrainHeight = OwnerTerrain->GetHeightAtPosition(WorldPos);
            }
            float WaterSurfaceHeight = TerrainHeight + SimulationData.WaterDepthMap[Index];
            
            // Get neighbor indices (with edge handling)
            int32 LeftIdx = (X > 0) ? Y * Width + (X - 1) : -1;
            int32 RightIdx = (X < Width - 1) ? Y * Width + (X + 1) : -1;
            int32 UpIdx = (Y > 0) ? (Y - 1) * Width + X : -1;
            int32 DownIdx = (Y < Height - 1) ? (Y + 1) * Width + X : -1;
            
            // Calculate pressure gradients with proper neighbor terrain heights
            float TerrainScale = OwnerTerrain ? OwnerTerrain->TerrainScale : 100.0f;
            float ForceX = 0.0f;
            float ForceY = 0.0f;
            
            // STEP 2: Calculate pressure gradients with edge handling
            
            // X-DIRECTION: Handle left boundary (configurable edge drainage)
            if (X == 0) // Left edge - configurable drainage for waterfall effect
            {
                if (RightIdx != -1)
                {
                    float RightTerrainHeight = GetTerrainHeightSafe(X + 1, Y);
                    float RightWaterHeight = RightTerrainHeight + SimulationData.WaterDepthMap[RightIdx];
                    ForceX = (WaterSurfaceHeight - RightWaterHeight) / TerrainScale;
                    
                    if (bEnableEdgeDrainage)
                    {
                        float EdgeBonus = bEnhancedWaterfallEffect ?
                            WaterSurfaceHeight * EdgeDrainageStrength : 0.0f;
                        ForceX += EdgeBonus;
                    }
                }
                else
                {
                    ForceX = bEnableEdgeDrainage ? WaterSurfaceHeight * WaterfallDrainageMultiplier : 0.0f;
                }
            }
            else if (X == Width - 1) // Right edge - configurable drainage
            {
                if (LeftIdx != -1)
                {
                    float LeftTerrainHeight = GetTerrainHeightSafe(X - 1, Y);
                    float LeftWaterHeight = LeftTerrainHeight + SimulationData.WaterDepthMap[LeftIdx];
                    ForceX = (LeftWaterHeight - WaterSurfaceHeight) / TerrainScale;
                    
                    if (bEnableEdgeDrainage)
                    {
                        float EdgeBonus = bEnhancedWaterfallEffect ?
                            WaterSurfaceHeight * EdgeDrainageStrength : 0.0f;
                        ForceX += EdgeBonus;
                    }
                }
                else
                {
                    ForceX = bEnableEdgeDrainage ? WaterSurfaceHeight * (EdgeDrainageStrength * 0.5f) : 0.0f;
                }
            }
            else // Interior - normal gradient with proper neighbor terrain heights
            {
                float LeftTerrainHeight = GetTerrainHeightSafe(X - 1, Y);
                float RightTerrainHeight = GetTerrainHeightSafe(X + 1, Y);
                float LeftWaterHeight = LeftTerrainHeight + SimulationData.WaterDepthMap[LeftIdx];
                float RightWaterHeight = RightTerrainHeight + SimulationData.WaterDepthMap[RightIdx];
                ForceX = (LeftWaterHeight - RightWaterHeight) / (2.0f * TerrainScale);
            }
            
            // Y-direction force calculation with configurable edge drainage
            if (Y == 0) // Top edge - configurable drainage
            {
                if (DownIdx != -1)
                {
                    float DownTerrainHeight = GetTerrainHeightSafe(X, Y + 1);
                    float DownWaterHeight = DownTerrainHeight + SimulationData.WaterDepthMap[DownIdx];
                    ForceY = (WaterSurfaceHeight - DownWaterHeight) / TerrainScale;
                    
                    if (bEnableEdgeDrainage)
                    {
                        float EdgeBonus = bEnhancedWaterfallEffect ?
                            WaterSurfaceHeight * EdgeDrainageStrength : 0.0f;
                        ForceY += EdgeBonus;
                    }
                }
                else
                {
                    ForceY = bEnableEdgeDrainage ? -WaterSurfaceHeight * WaterfallDrainageMultiplier : 0.0f;
                }
            }
            else if (Y == Height - 1) // Bottom edge - configurable drainage
            {
                if (UpIdx != -1)
                {
                    float UpTerrainHeight = GetTerrainHeightSafe(X, Y - 1);
                    float UpWaterHeight = UpTerrainHeight + SimulationData.WaterDepthMap[UpIdx];
                    ForceY = (UpWaterHeight - WaterSurfaceHeight) / TerrainScale;
                    
                    if (bEnableEdgeDrainage)
                    {
                        float EdgeBonus = bEnhancedWaterfallEffect ?
                            WaterSurfaceHeight * EdgeDrainageStrength : 0.0f;
                        ForceY += EdgeBonus;
                    }
                }
                else
                {
                    ForceY = bEnableEdgeDrainage ? WaterSurfaceHeight * WaterfallDrainageMultiplier : 0.0f;
                }
            }
            else // Interior - normal gradient with proper neighbor terrain heights
            {
                float UpTerrainHeight = GetTerrainHeightSafe(X, Y - 1);
                float DownTerrainHeight = GetTerrainHeightSafe(X, Y + 1);
                float UpWaterHeight = UpTerrainHeight + SimulationData.WaterDepthMap[UpIdx];
                float DownWaterHeight = DownTerrainHeight + SimulationData.WaterDepthMap[DownIdx];
                ForceY = (UpWaterHeight - DownWaterHeight) / (2.0f * TerrainScale);
            }
            
            // Apply forces to velocity with damping to prevent oscillation
            NewVelocityX[Index] = (SimulationData.WaterVelocityX[Index] + ForceX * WaterFlowSpeed * DeltaTime) * WaterDamping;
            NewVelocityY[Index] = (SimulationData.WaterVelocityY[Index] + ForceY * WaterFlowSpeed * DeltaTime) * WaterDamping;
            
            // Limit velocity to prevent instability
            NewVelocityX[Index] = FMath::Clamp(NewVelocityX[Index], -MaxWaterVelocity, MaxWaterVelocity);
            NewVelocityY[Index] = FMath::Clamp(NewVelocityY[Index], -MaxWaterVelocity, MaxWaterVelocity);
        }
    }
    
    // Update velocity arrays
    SimulationData.WaterVelocityX = NewVelocityX;
    SimulationData.WaterVelocityY = NewVelocityY;
    
    // Mark that water has changed for volumetric updates
    bWaterChangedThisFrame = true;
}

void UWaterSystem::ApplyWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    TArray<float> NewWaterDepth = SimulationData.WaterDepthMap;
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
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
            
            // Get current velocities
            float VelX = SimulationData.WaterVelocityX[Index];
            float VelY = SimulationData.WaterVelocityY[Index];
            
            // Calculate flow amounts based on velocity and available water
            float FlowRate = SimulationData.WaterDepthMap[Index] * DeltaTime * 0.1f;  // Scale factor for stability
            float FlowRight = FMath::Max(0.0f, VelX * FlowRate);
            float FlowLeft = FMath::Max(0.0f, -VelX * FlowRate);
            float FlowDown = FMath::Max(0.0f, VelY * FlowRate);
            float FlowUp = FMath::Max(0.0f, -VelY * FlowRate);
            
            // Ensure total outflow doesn't exceed available water
            float TotalOutflow = FlowRight + FlowLeft + FlowDown + FlowUp;
            if (TotalOutflow > SimulationData.WaterDepthMap[Index])
            {
                float Scale = SimulationData.WaterDepthMap[Index] / TotalOutflow;
                FlowRight *= Scale;
                FlowLeft *= Scale;
                FlowDown *= Scale;
                FlowUp *= Scale;
            }
            
            // Remove water from current cell
            NewWaterDepth[Index] -= TotalOutflow;
            
            // Add water to neighbor cells OR let flow off-world (waterfall effect)
            if (X < Width - 1)
                NewWaterDepth[Y * Width + (X + 1)] += FlowRight;
            // else: FlowRight disappears off right edge as waterfall
            
            if (X > 0)
                NewWaterDepth[Y * Width + (X - 1)] += FlowLeft;
            // else: FlowLeft disappears off left edge as waterfall
            
            if (Y < Height - 1)
                NewWaterDepth[(Y + 1) * Width + X] += FlowDown;
            // else: FlowDown disappears off bottom edge as waterfall
            
            if (Y > 0)
                NewWaterDepth[(Y - 1) * Width + X] += FlowUp;
            // else: FlowUp disappears off top edge as waterfall
        }
    }
    
    // Update water depths, ensuring no negative values
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        SimulationData.WaterDepthMap[i] = FMath::Max(0.0f, NewWaterDepth[i]);
    }
    
    // Mark that water has changed for volumetric updates
    bWaterChangedThisFrame = true;
}



void UWaterSystem::ProcessWaterEvaporation(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        if (SimulationData.WaterDepthMap[i] > MinWaterDepth)
        {
            // Evaporation (faster in shallow areas)
            float EvaporationAmount = WaterEvaporationRate * DeltaTime;
            SimulationData.WaterDepthMap[i] -= EvaporationAmount;
            
            // Absorption into terrain
            float AbsorptionAmount = WaterAbsorptionRate * DeltaTime;
            SimulationData.WaterDepthMap[i] -= AbsorptionAmount;
            
            // Ensure water depth doesn't go negative
            SimulationData.WaterDepthMap[i] = FMath::Max(0.0f, SimulationData.WaterDepthMap[i]);
        }
    }
    
    // Mark that water has changed for volumetric updates if any evaporation occurred
    bWaterChangedThisFrame = true;
}

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
    if (!WaterDataTexture || !SimulationData.IsValid())
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create RGBA texture data
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                float Depth = SimulationData.WaterDepthMap[Index];
                float VelX = SimulationData.WaterVelocityX[Index];
                float VelY = SimulationData.WaterVelocityY[Index];
                
                // Calculate flow speed and normalize direction
                float FlowSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY);
                float NormVelX = FlowSpeed > 0.01f ? VelX / FlowSpeed : 0.0f;
                float NormVelY = FlowSpeed > 0.01f ? VelY / FlowSpeed : 0.0f;
                
                // Pack data into RGBA channels
                TextureData[Index] = FLinearColor(
                    Depth * WaterDepthScale,                           // R: Depth
                    FMath::Clamp(FlowSpeed / 50.0f, 0.0f, 1.0f),      // G: Speed normalized
                    (NormVelX + 1.0f) * 0.5f,                         // B: Direction X [-1,1] → [0,1]
                    (NormVelY + 1.0f) * 0.5f                          // A: Direction Y [-1,1] → [0,1]
                );
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
}

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

void UWaterSystem::UpdateWeatherSystem(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    
    if (WeatherTimer >= WeatherChangeInterval)
    {
        // Disable debug weather changes - keep current state
        WeatherTimer = 0.0f;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("WaterSystem: Weather timer reset (auto-changes disabled)"));
    }
}


void UWaterSystem::ApplyRain(float DeltaTime)
    {
        if (!SimulationData.IsValid())
        {
            return;
        }
        
        float WaterPerCell = RainIntensity * DeltaTime * 0.1f;
        
        // Add water to random coordinates (not indices)
        int32 RainCells = FMath::RandRange(100, 1000);
        for (int32 i = 0; i < RainCells; i++)
        {
            int32 RandomX = FMath::RandRange(0, SimulationData.TerrainWidth - 1);
            int32 RandomY = FMath::RandRange(0, SimulationData.TerrainHeight - 1);
            int32 Index = RandomY * SimulationData.TerrainWidth + RandomX;
            
            SimulationData.WaterDepthMap[Index] += WaterPerCell;
        }
    }


// ===== PLAYER INTERACTION =====

void UWaterSystem::AddWater(FVector WorldPosition, float Amount)
{
    SCOPE_CYCLE_COUNTER(STAT_WaterAddition); // UE5.4 profiling
    
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[WATER ADD] SimulationData invalid"));
        return;
    }
    
    // Coordinate transformation with fallback
    // ✅ FIX: FORCE MasterController dependency - NO FALLBACKS
    if (!CachedMasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VIOLATION] Water placement failed - no MasterController"));
        return; // FAIL rather than use wrong coordinates
    }

    FVector2D TerrainCoords = MasterController->WorldToTerrainCoordinates(WorldPosition);

    // Validate coordinates are within bounds
    if (!MasterController->ValidateWorldPosition(WorldPosition))
    {
        UE_LOG(LogTemp, Error, TEXT("[WATER ADD] Position outside world bounds"));
        return;
    }
    
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    

    
    if (IsValidCoordinate(X, Y))
    {
        int32 Index = Y * SimulationData.TerrainWidth + X;
        float OldDepth = SimulationData.WaterDepthMap[Index];
        SimulationData.WaterDepthMap[Index] += Amount;
        float NewDepth = SimulationData.WaterDepthMap[Index];
        bWaterChangedThisFrame = true;
        
    
        
        // UE5.4 Rendering Thread Safety - DISABLED FOR PERFORMANCE
        /*
        ENQUEUE_RENDER_COMMAND(UpdateWaterTexture)(
            [this](FRHICommandListImmediate& RHICmdList)
            {
                if (WaterDepthTexture && IsInRenderingThread())
                {
                    UpdateWaterDepthTextureRenderThread();
                }
            });
        */
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[WATER ADD] Invalid coordinates for terrain"));
    }
}

void UWaterSystem::AddWaterInRadius(int32 CenterX, int32 CenterY, float Radius, float Amount)
{
    if (!IsValidCoordinate(CenterX, CenterY) || !SimulationData.IsValid())
    {
        return;
    }
    
    int32 IntRadius = FMath::CeilToInt(Radius);
    TSet<int32> AffectedChunks;
    
    // Add water in circular pattern with quadratic falloff (matches terrain brush)
    for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
    {
        for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
        {
            int32 CurrentX = CenterX + OffsetX;
            int32 CurrentY = CenterY + OffsetY;
            
            if (IsValidCoordinate(CurrentX, CurrentY))
            {
                float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
                if (Distance <= Radius)
                {
                    // Quadratic falloff: more water at center, smooth edges
                    float Falloff = FMath::Pow(1.0f - (Distance / Radius), 2.0f);
                    float WaterAmount = Amount * Falloff;
                    
                    int32 Index = GetTerrainIndex(CurrentX, CurrentY);
                    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
                    {
                        SimulationData.WaterDepthMap[Index] += WaterAmount;
                        
                        // CRITICAL: Reduce update frequency during water placement
                        static float LastWaterModTime = 0.0f;
                        float CurrentTime = OwnerTerrain ? OwnerTerrain->GetCachedFrameTime() : 0.0f;
                        
                        if (CurrentTime - LastWaterModTime < 0.1f) // 10 Hz max
                        {
                            continue; // Skip this cell
                        }
                        LastWaterModTime = CurrentTime;
                        
                        // Track affected chunks for visual updates
                        if (OwnerTerrain)
                        {
                            int32 ChunkIndex = OwnerTerrain->GetChunkIndexFromCoordinates(CurrentX, CurrentY);
                            if (ChunkIndex >= 0)
                            {
                                AffectedChunks.Add(ChunkIndex);
                            }
                        }
                        
                        // Mark water as changed for volumetric updates
                        bWaterChangedThisFrame = true;
                    }
                }
            }
        }
    }
    
    // Mark affected chunks for visual update
    if (OwnerTerrain)
    {
        for (int32 ChunkIndex : AffectedChunks)
        {
            OwnerTerrain->MarkChunkForUpdate(ChunkIndex);
        }
    }
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
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("WaterSystem: Added %.2f water at (%d,%d), depth now %.2f"),
                   Amount, X, Y, SimulationData.WaterDepthMap[Index]);
    }
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
            SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, SimulationData.WaterDepthMap[Index] - Amount);
            
            // Mark chunk for visual update
            MarkChunkForUpdate(X, Y);
            
            // Mark water as changed for volumetric updates
            bWaterChangedThisFrame = true;
        }
    }
}

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

void UWaterSystem::StartRain(float Intensity)
{
    bIsRaining = true;
    RainIntensity = Intensity;
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Rain started with intensity %.2f"), Intensity);
}

void UWaterSystem::StopRain()
{
    bIsRaining = false;
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Rain stopped"));
}

void UWaterSystem::SetAutoWeather(bool bEnable)
{
    bAutoWeather = bEnable;
    if (bEnable)
    {
        WeatherTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Auto weather enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Auto weather disabled"));
    }
}

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

void UWaterSystem::DrawDebugInfo() const
{
    if (!GEngine || !bShowWaterStats)
    {
        return;
    }
    
    int32 WaterCells = GetWaterCellCount();
    float TotalWater = GetTotalWaterInSystem();
    float MaxFlow = GetMaxFlowSpeed();
    
    GEngine->AddOnScreenDebugMessage(20, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Water Cells: %d"), WaterCells));
    GEngine->AddOnScreenDebugMessage(21, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Total Water: %.1f"), TotalWater));
    GEngine->AddOnScreenDebugMessage(22, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Max Flow Speed: %.1f"), MaxFlow));
    
    FString RainStatus = bIsRaining ? TEXT("ON") : TEXT("OFF");
    GEngine->AddOnScreenDebugMessage(23, 0.5f, FColor::Cyan,
        FString::Printf(TEXT("Rain: %s (%.1f)"), *RainStatus, RainIntensity));
    
    FString SystemStatus = bEnableWaterSimulation ? TEXT("ACTIVE") : TEXT("DISABLED");
    GEngine->AddOnScreenDebugMessage(24, 0.5f, FColor::Cyan,
        FString::Printf(TEXT("Water System: %s"), *SystemStatus));
}

// ===== HELPER FUNCTIONS =====

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
    return X >= 0 && X < SimulationData.TerrainWidth &&
           Y >= 0 && Y < SimulationData.TerrainHeight;
}


FVector2D UWaterSystem::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    // ✅ FIX: FORCE MasterController dependency - remove fallback completely
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VIOLATION] WorldToTerrainCoordinates called without MasterController"));
        return FVector2D::ZeroVector; // Return zero rather than wrong coordinates
    }
    
    return CachedMasterController->WorldToTerrainCoordinates(WorldPosition);
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

// ===== UE5.4 ENHANCED INPUT SYSTEM INTEGRATION =====
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

// ===== UE5.4 NIAGARA 5.0 INTEGRATION =====
void UWaterSystem::UpdateNiagaraFX_UE54(float DeltaTime)
{
    if (!bEnableNiagaraFX || !OwnerTerrain)
    {
        return;
    }
    
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer < NiagaraUpdateRate)
    {
        return;
    }
    UpdateTimer = 0.0f;
    
    for (auto& NiagaraPair : ActiveNiagaraComponents)
    {
        UNiagaraComponent* NiagaraComp = NiagaraPair.Value;
        if (NiagaraComp && IsValid(NiagaraComp))
        {
            if (WaterDepthTexture)
            {
                NiagaraComp->SetVariableTexture(FName("WaterDepthTexture"), WaterDepthTexture);
            }
            
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
            
            NiagaraComp->SetVectorParameter(FName("ViewerLocation"), CameraLocation);
            NiagaraComp->SetFloatParameter(FName("WorldTime"), GetWorld()->GetTimeSeconds());
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

void UWaterSystem::FixChunkUVMapping()
{
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("[UV FIX] No MasterController authority"));
        return;
    }
    
    FVector2D WorldDims = MasterController->GetWorldDimensions();
    float TerrainScale = MasterController->GetTerrainScale();
    
    if (WaterParameterCollection && OwnerTerrain && OwnerTerrain->GetWorld())
    {
        UMaterialParameterCollectionInstance* Instance = 
            OwnerTerrain->GetWorld()->GetParameterCollectionInstance(WaterParameterCollection);
        
        if (Instance)
        {
            Instance->SetScalarParameterValue(FName("WaterTextureWidth"), WorldDims.X);
            Instance->SetScalarParameterValue(FName("WaterTextureHeight"), WorldDims.Y);
            Instance->SetScalarParameterValue(FName("WaterTexelScale"), 1.0f / WorldDims.X);
            Instance->SetScalarParameterValue(FName("TerrainScale"), TerrainScale);
            
            UE_LOG(LogTemp, Warning, TEXT("[UV FIX] Updated shader parameters - %dx%d @ %.2f scale"), 
                   (int32)WorldDims.X, (int32)WorldDims.Y, TerrainScale);
        }
    }
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
        MPC->SetScalarParameterValue(FName("RainIntensity"), bIsRaining ? RainIntensity : 0.0f);
        
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
    UE_LOG(LogTemp, VeryVerbose, TEXT("UPDATING WATER DEPTH TEXTURE"));
    
    if (!WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update - WaterDepthTexture is NULL"));
        return;
    }
    
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update - SimulationData invalid"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create texture data array
    TArray<uint8> TextureData;
    TextureData.SetNum(Width * Height);
    
    // Track water statistics for debugging
    int32 NonZeroPixels = 0;
    float MaxWaterDepth = 0.0f;
    float TotalWater = 0.0f;
    
    // Convert water depth to texture values
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                float WaterDepth = SimulationData.WaterDepthMap[Index];
                
                if (WaterDepth > 0.01f)
                {
                    NonZeroPixels++;
                    TotalWater += WaterDepth;
                    MaxWaterDepth = FMath::Max(MaxWaterDepth, WaterDepth);
                }
                
                // Convert depth to 0-255 range
                uint8 TextureValue = 0;  // Default to no water
                if (WaterDepth > MinWaterDepth)  // Only set value if significant water
                {
                    TextureValue = FMath::Clamp(
                        FMath::RoundToInt(WaterDepth * WaterDepthScale),
                        1, 255  // Minimum 1 if water exists
                    );
                }
                TextureData[Index] = TextureValue;
            }
        }
    }
        
        // Only log debug info every 2 seconds, and only if there's actually water
        float CurrentTime = GetWorld()->GetTimeSeconds();
        bool bShouldLog = (CurrentTime - LastDebugLogTime >= DEBUG_LOG_INTERVAL);
        
        if (bShouldLog && (NonZeroPixels > 0 || TotalWater > 0.1f))
        {
        UE_LOG(LogTemp, Log, TEXT("Volumetric Water Update #%d: %d pixels, Max depth: %.2f, Total: %.2f"),
           ++DebugLogCounter, NonZeroPixels, MaxWaterDepth, TotalWater);
        
        // Sample values only if we have significant water
        if (TotalWater > 10.0f)
        {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Optical depth samples: [0]=%d [mid]=%d [end]=%d"),
               TextureData[0], TextureData[TextureData.Num()/2], TextureData[TextureData.Num()-1]);
        }
        
        LastDebugLogTime = CurrentTime;
    }
        
        // Upload to GPU with error checking
        if (WaterDepthTexture->GetPlatformData() && WaterDepthTexture->GetPlatformData()->Mips.Num() > 0)
        {
            FTexture2DMipMap& Mip = WaterDepthTexture->GetPlatformData()->Mips[0];
            
            void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
            if (Data)
            {
                FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num());
                Mip.BulkData.Unlock();
                
                // Force GPU update
                WaterDepthTexture->UpdateResource();
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Texture data uploaded to GPU"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to lock texture mip data"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Texture platform data or mips invalid"));
        }
    }




void UWaterSystem::UpdateWaterDepthTextureRenderThread()
{
    if (!WaterDepthTexture || !IsInRenderingThread())
    {
        return;
    }
    
    // UE5.4 texture update pattern - use FTextureRHIRef instead of deprecated type
    FTextureRHIRef TextureRHI = WaterDepthTexture->GetResource()->GetTextureRHI();
    if (!TextureRHI)
    {
        return;
    }
    
    // Lock texture for update
    uint32 DestStride;
    uint8* DestData = (uint8*)RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false);
    
    if (DestData)
    {
        // Copy simulation data to texture
        const int32 Width = SimulationData.TerrainWidth;
        const int32 Height = SimulationData.TerrainHeight;
        
        for (int32 Y = 0; Y < Height; Y++)
        {
            for (int32 X = 0; X < Width; X++)
            {
                int32 Index = Y * Width + X;
                float WaterDepth = SimulationData.WaterDepthMap[Index];
                uint8 TextureValue = FMath::Clamp(
                    FMath::RoundToInt(WaterDepth * WaterDepthScale), 0, 255);
                
                DestData[Y * DestStride + X] = TextureValue;
            }
        }
        
        RHIUnlockTexture2D(TextureRHI, 0, false);
    }
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
    Material->SetScalarParameterValue(FName("WaterTurbidity"), GetDynamicTurbidity());
    Material->SetVectorParameterValue(FName("WeatherAdjustedDeepColor"), GetWeatherAdjustedWaterColor());
    Material->SetScalarParameterValue(FName("WaveStrength"), Params.WaveStrength);
    
    // Water depth scaling for Beer's law
    Material->SetScalarParameterValue(FName("WaterDepthScale"), 1.0f / WaterDepthScale);
    
   // UE_LOG(LogTemp, VeryVerbose, TEXT("Applied volumetric water parameters - Clarity: %.2f, Absorption: %.2f"),
       //    Params.Clarity, Params.Absorption);
}

float UWaterSystem::GetDynamicTurbidity() const
{
    // Environment-responsive water clarity
    if (bIsRaining) return 0.3f;  // Rain makes water muddy
    if (GetMaxFlowSpeed() > 10.0f) return 0.2f;  // Fast flow picks up sediment
    return MaterialParams.Turbidity;  // Base clear water
}

FLinearColor UWaterSystem::GetWeatherAdjustedWaterColor() const
{
    // Storms darken water color
    float StormFactor = bIsRaining ? 0.7f : 1.0f;
    return MaterialParams.DeepColor * StormFactor;
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

float UWaterSystem::GetTotalWaterVolume() const
{
    if (!SimulationData.IsValid()) return 0.0f;
    
    float TotalVolume = 0.0f;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        TotalVolume += WaterDepth;
    }
    return TotalVolume;
}

int32 UWaterSystem::GetActiveVolumeChunkCount() const
{
    return WaterSurfaceChunks.Num();
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



