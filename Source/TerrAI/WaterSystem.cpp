// WaterSystem.cpp - Simplified Water Physics Implementation
#include "WaterSystem.h"
#include "DynamicTerrain.h"
#include "Engine/Engine.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "ProceduralMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceTexture.h"

// Debug optimization static variables
float UWaterSystem::LastDebugLogTime = 0.0f;
int32 UWaterSystem::DebugLogCounter = 0;
const float UWaterSystem::DEBUG_LOG_INTERVAL = 2.0f;

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
    
    // Weather settings
    bIsRaining = false;
    RainIntensity = 1.0f;
    WeatherChangeInterval = 60.0f;
    bAutoWeather = false;
    WeatherTimer = 0.0f;
    
    // Debug settings
    bShowWaterStats = true;
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
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Created with surface water and Niagara FX support"));
}

// ===== INITIALIZATION =====

void UWaterSystem::Initialize(ADynamicTerrain* InTerrain)
{
    if (!InTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterSystem: Cannot initialize with null terrain"));
        return;
    }
    
    OwnerTerrain = InTerrain;
    
    // Initialize simulation data
    SimulationData.Initialize(OwnerTerrain->TerrainWidth, OwnerTerrain->TerrainHeight);
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Initialized with terrain %dx%d (%d water cells)"),
           SimulationData.TerrainWidth, SimulationData.TerrainHeight, SimulationData.WaterDepthMap.Num());
    
    // Initialize shader system if enabled
    if (bUseShaderWater)
    {
        UE_LOG(LogTemp, Log, TEXT("INITIALIZING SHADER WATER SYSTEM"));
        CreateWaterDepthTexture();
        CreateAdvancedWaterTexture();
        // CreateFlowDisplacementTexture();  // DISABLED - causes threading crashes
        UE_LOG(LogTemp, Log, TEXT("WaterSystem: Shader system initialized (displacement disabled)"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Shader water system DISABLED - bUseShaderWater=false"));
    }
     
     UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Initialized with terrain %dx%d (%d water cells)"),
            SimulationData.TerrainWidth, SimulationData.TerrainHeight, SimulationData.WaterDepthMap.Num());

}

bool UWaterSystem::IsSystemReady() const
{
    return OwnerTerrain != nullptr && SimulationData.IsValid();
}

// ===== WATER PHYSICS SIMULATION =====

void UWaterSystem::UpdateWaterSimulation(float DeltaTime)
{
    if (!IsSystemReady() || !bEnableWaterSimulation)
    {
        return;
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
    
    // Step 6: Process erosion if enabled
    if (bEnableErosion)
    {
        UpdateErosion(DeltaTime);
    }
    
    // Step 7: Update surface-based water if enabled
    if (bEnableWaterVolumes)
    {
        UpdateWaterSurfaceChunks();
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated water surface chunks"));
    }
    
    // Step 8: Update Niagara FX if enabled (PHASE 4)
    if (bEnableNiagaraFX)
    {
        UpdateNiagaraFX(DeltaTime);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated Niagara FX"));
    }
    
    // Step 9: Update shader system with volumetric water + displacement
    if (bUseShaderWater)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updating shader system - bUseShaderWater=true"));
        
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
    else
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Shader system disabled - bUseShaderWater=false"));
    }
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
    if (!OwnerTerrain || !SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[SURFACE DEBUG] UpdateWaterSurfaceChunks FAILED - OwnerTerrain:%s SimulationData:%s"),
               OwnerTerrain ? TEXT("OK") : TEXT("NULL"),
               SimulationData.IsValid() ? TEXT("OK") : TEXT("INVALID"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SURFACE DEBUG] Starting surface update - bEnableWaterVolumes:%s"),
           bEnableWaterVolumes ? TEXT("TRUE") : TEXT("FALSE"));
    
    if (!bEnableWaterVolumes)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SURFACE DEBUG] Surface water DISABLED - skipping update"));
        return;
    }
    
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
    
    // Remove surface chunks where water disappeared
    for (int32 i = WaterSurfaceChunks.Num() - 1; i >= 0; i--)
    {
        if (!ShouldGenerateSurfaceForChunk(WaterSurfaceChunks[i].ChunkIndex))
        {
            if (WaterSurfaceChunks[i].SurfaceMesh)
                WaterSurfaceChunks[i].SurfaceMesh->DestroyComponent();
            if (WaterSurfaceChunks[i].UndersideMesh)
                WaterSurfaceChunks[i].UndersideMesh->DestroyComponent();
            WaterSurfaceChunks.RemoveAt(i);
        }
    }
    
    // Update existing chunks and create new ones
    for (int32 ChunkIndex = 0; ChunkIndex < OwnerTerrain->TerrainChunks.Num(); ChunkIndex++)
    {
        if (!ShouldGenerateSurfaceForChunk(ChunkIndex))
            continue;
            
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
            
            // Calculate LOD based on distance
            FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(ChunkIndex);
            float Distance = FVector::Dist(CameraLocation, ChunkWorldPos);
            
            if (Distance > 4000.0f) NewSurfaceChunk.CurrentLOD = 3;
            else if (Distance > 2000.0f) NewSurfaceChunk.CurrentLOD = 2;
            else if (Distance > 1000.0f) NewSurfaceChunk.CurrentLOD = 1;
            else NewSurfaceChunk.CurrentLOD = 0;
            
            WaterSurfaceChunks.Add(NewSurfaceChunk);
            SurfaceChunk = &WaterSurfaceChunks.Last();
            
            UE_LOG(LogTemp, Warning, TEXT("[SURFACE DEBUG] CREATED surface chunk for terrain chunk %d"), ChunkIndex);
        }
        
        if (SurfaceChunk)
        {
            // PHASE 1-2: SYNCHRONIZE CHUNK WITH SIMULATION AUTHORITY
            SynchronizeChunkWithSimulation(*SurfaceChunk);
            
            // Update LOD and mesh if needed (driven by simulation state)
            if (SurfaceChunk->bNeedsUpdate || bWaterChangedThisFrame)
            {
                CreateWaterSurfaceMesh(*SurfaceChunk);
                SurfaceChunk->bNeedsUpdate = false;
            }
        }
    }
}

// ===== PHASE 1-2: SIMULATION AUTHORITY IMPLEMENTATION =====

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
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[PHASE 1-2] Chunk %d sync: MaxDepth=%.2f, Flow=%.2f, Foam=%s"),
           SurfaceChunk.ChunkIndex, SurfaceChunk.MaxDepth, SurfaceChunk.FlowSpeed, 
           SurfaceChunk.bHasFoam ? TEXT("YES") : TEXT("NO"));
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

/**
 * Determines if a terrain chunk should have a water volume generated
 * Checks for sufficient water depth and cell count to justify volume creation
 *
 * @param ChunkIndex - Index into terrain chunks array
 * @return true if volume should be generated, false otherwise
 */
bool UWaterSystem::ShouldGenerateSurfaceForChunk(int32 ChunkIndex) const
{
    if (!OwnerTerrain || !SimulationData.IsValid() ||
        ChunkIndex < 0 || ChunkIndex >= OwnerTerrain->TerrainChunks.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("[VOLUMETRIC DEBUG] ShouldGenerate FAILED for chunk %d - validation failed"), ChunkIndex);
        return false;
    }
    
    // Get chunk bounds in terrain coordinates
    const auto& TerrainChunk = OwnerTerrain->TerrainChunks[ChunkIndex];
    int32 ChunkSize = OwnerTerrain->ChunkSize;
    int32 ChunkOverlap = OwnerTerrain->ChunkOverlap;
    
    int32 StartX = TerrainChunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = TerrainChunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, OwnerTerrain->TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, OwnerTerrain->TerrainHeight);
    
    // Check if chunk has significant water
    float MaxDepth = 0.0f;
    int32 WaterCells = 0;
    int32 TotalCells = (EndX - StartX) * (EndY - StartY);
    
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            float Depth = GetWaterDepthSafe(X, Y);
            if (Depth > MinVolumeDepth)
            {
                WaterCells++;
                MaxDepth = FMath::Max(MaxDepth, Depth);
            }
        }
    }
    
    bool ShouldGenerate = WaterCells > 10 && MaxDepth > MinVolumeDepth;
    
    UE_LOG(LogTemp, Warning, TEXT("[VOLUMETRIC DEBUG] Chunk %d analysis - WaterCells:%d/%d MaxDepth:%.2f MinRequired:%.2f ShouldGenerate:%s"),
           ChunkIndex, WaterCells, TotalCells, MaxDepth, MinVolumeDepth, ShouldGenerate ? TEXT("YES") : TEXT("NO"));
    
    // Generate volume if enough water exists (at least 10 cells with significant depth)
    return ShouldGenerate;
}

/**
 * Creates surface-based water mesh for realistic water visualization
 * Generates smooth water surfaces with optical depth encoding
 * Applies surface + underside materials for complete underwater viewing
 *
 * @param SurfaceChunk - Reference to surface chunk to generate mesh for
 */
void UWaterSystem::CreateWaterSurfaceMesh(FWaterSurfaceChunk& SurfaceChunk)
{
    if (!OwnerTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("[SURFACE DEBUG] CreateWaterSurfaceMesh FAILED - No OwnerTerrain"));
        return;
    }
    
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
    // Create surface mesh component if needed
    if (!SurfaceChunk.SurfaceMesh)
    {
        FString ComponentName = FString::Printf(TEXT("WaterSurface_%d"), SurfaceChunk.ChunkIndex);
        SurfaceChunk.SurfaceMesh = NewObject<UProceduralMeshComponent>(OwnerTerrain, *ComponentName);
        SurfaceChunk.SurfaceMesh->SetupAttachment(OwnerTerrain->GetRootComponent());
        SurfaceChunk.SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SurfaceChunk.SurfaceMesh->RegisterComponent();
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
    
    UE_LOG(LogTemp, Warning, TEXT("[SURFACE DEBUG] SURFACE MESH COMPLETE - Chunk:%d Vertices:%d MaxDepth:%.2f"),
           SurfaceChunk.ChunkIndex, Vertices.Num(), SurfaceChunk.MaxDepth);
    
    UE_LOG(LogTemp, Warning, TEXT("[SURFACE DEBUG] MESH GENERATION COMPLETE - Chunk:%d Vertices:%d Material:%s Visible:%s"),
           SurfaceChunk.ChunkIndex, Vertices.Num(),
           SurfaceChunk.SurfaceMesh->GetMaterial(0) ? TEXT("ASSIGNED") : TEXT("NONE"),
           SurfaceChunk.SurfaceMesh->IsVisible() ? TEXT("YES") : TEXT("NO"));
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
    
    // Calculate resolution based on LOD
    int32 Resolution = FMath::Max(8, BaseSurfaceResolution >> SurfaceChunk.CurrentLOD);
    // Fix: Calculate proper cell size based on chunk coverage
    float CellSize = (OwnerTerrain->TerrainScale * (OwnerTerrain->ChunkSize - 1)) / (float)(Resolution - 1);
    
    // Get chunk position
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    
    // Reserve arrays
    int32 VertexCount = Resolution * Resolution;
    Vertices.Reserve(VertexCount);
    Normals.Reserve(VertexCount);
    UVs.Reserve(VertexCount);
    VertexColors.Reserve(VertexCount);
    
    // Generate smooth surface vertices
    SurfaceChunk.MaxDepth = 0.0f;
    SurfaceChunk.AverageDepth = 0.0f;
    int32 WaterVertices = 0;
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float LocalX = X * CellSize;
            float LocalY = Y * CellSize;
            FVector2D SamplePos(ChunkWorldPos.X + LocalX, ChunkWorldPos.Y + LocalY);
            
            // Get interpolated water depth for smooth surfaces
            float WaterDepth = GetInterpolatedWaterDepth(SamplePos);
            float TerrainHeight = OwnerTerrain->GetHeightSafe(SamplePos.X / OwnerTerrain->TerrainScale, SamplePos.Y / OwnerTerrain->TerrainScale);
            
            if (WaterDepth > MinVolumeDepth)
            {
                WaterVertices++;
                SurfaceChunk.MaxDepth = FMath::Max(SurfaceChunk.MaxDepth, WaterDepth);
                SurfaceChunk.AverageDepth += WaterDepth;
                
                // Fix: Calculate proper water surface height without artificial offset
                float WaterSurfaceHeight = TerrainHeight + WaterDepth;
                if (OwnerTerrain->GetWorld())
                {
                    float Time = OwnerTerrain->GetWorld()->GetTimeSeconds();
                    float WaveOffset = FMath::Sin((SamplePos.X * 0.01f) + (SamplePos.Y * 0.01f) + (Time * 2.0f)) * 1.0f;
                    WaterSurfaceHeight += WaveOffset;
                }
                
                // Use world height directly (relative to world origin, not chunk)
                Vertices.Add(FVector(LocalX, LocalY, WaterSurfaceHeight - ChunkWorldPos.Z));
            }
            else
            {
                Vertices.Add(FVector(LocalX, LocalY, TerrainHeight));
            }
            
            // Surface normal calculation
            Normals.Add(FVector::UpVector);
            
            // UV coordinates
            UVs.Add(FVector2D((float)X / (Resolution - 1), (float)Y / (Resolution - 1)));
            
            // Vertex color encodes depth
            uint8 DepthValue = FMath::Clamp(FMath::RoundToInt(WaterDepth * 10.0f), 0, 255);
            VertexColors.Add(FColor(DepthValue, 0, 0, 255));
        }
    }
    
    // Calculate average depth
    if (WaterVertices > 0)
    {
        SurfaceChunk.AverageDepth /= WaterVertices;
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
            
            // Only create triangles where there's significant water (fixes rectangular chunk issue)
            float DepthThreshold = FMath::RoundToInt(MinDepthThreshold * 10.0f);
            bool QuadHasWater = (VertexColors[TopLeft].R >= DepthThreshold) ||
                               (VertexColors[TopRight].R >= DepthThreshold) ||
                               (VertexColors[BottomLeft].R >= DepthThreshold) ||
                               (VertexColors[BottomRight].R >= DepthThreshold);
            
            if (QuadHasWater)
            {
                Triangles.Add(TopLeft);
                Triangles.Add(BottomLeft);
                Triangles.Add(TopRight);
                
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
float UWaterSystem::GetInterpolatedWaterDepth(FVector2D WorldPosition) const
{
    if (!OwnerTerrain || !SimulationData.IsValid())
        return 0.0f;
    
    // Fix: Ensure proper coordinate conversion
    FVector WorldPos3D(WorldPosition.X, WorldPosition.Y, 0);
    FVector2D TerrainCoords = OwnerTerrain->WorldToTerrainCoordinates(WorldPos3D);
    
    // Get integer coordinates
    int32 X0 = FMath::FloorToInt(TerrainCoords.X);
    int32 Y0 = FMath::FloorToInt(TerrainCoords.Y);
    int32 X1 = X0 + 1;
    int32 Y1 = Y0 + 1;
    
    // Get fractional parts
    float FracX = TerrainCoords.X - X0;
    float FracY = TerrainCoords.Y - Y0;
    
    // Sample four corners
    float Depth00 = GetWaterDepthSafe(X0, Y0);
    float Depth10 = GetWaterDepthSafe(X1, Y0);
    float Depth01 = GetWaterDepthSafe(X0, Y1);
    float Depth11 = GetWaterDepthSafe(X1, Y1);
    
    // Bilinear interpolation
    float DepthTop = FMath::Lerp(Depth00, Depth10, FracX);
    float DepthBottom = FMath::Lerp(Depth01, Depth11, FracX);
    return FMath::Lerp(DepthTop, DepthBottom, FracY);
}

/**
 * Updates UV mapping for seamless chunk transitions
 * Ensures water textures tile correctly across chunk boundaries
 *
 * @param SurfaceChunk - Surface chunk to update UV mapping for
 */
void UWaterSystem::UpdateSurfaceUVMapping(FWaterSurfaceChunk& SurfaceChunk)
{
    if (!SurfaceChunk.SurfaceMesh || !OwnerTerrain)
        return;
    
    // Calculate chunk UV offset
    FVector ChunkWorldPos = OwnerTerrain->GetChunkWorldPosition(SurfaceChunk.ChunkIndex);
    float TotalWorldSize = OwnerTerrain->TerrainScale * OwnerTerrain->TerrainWidth;
    
    FVector2D ChunkUVOffset(
        ChunkWorldPos.X / TotalWorldSize,
        ChunkWorldPos.Y / TotalWorldSize
    );
    
    // Update material with UV offset
    UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(
        SurfaceChunk.SurfaceMesh->GetMaterial(0));
    
    if (MaterialInstance)
    {
        MaterialInstance->SetVectorParameterValue(FName("ChunkUVOffset"),
            FLinearColor(ChunkUVOffset.X, ChunkUVOffset.Y, 0, 0));
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
            
            // Calculate water surface height (terrain + water depth)
            float TerrainHeight = GetTerrainHeightSafe(X, Y);
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
        // Simple weather state changes
        bIsRaining = FMath::RandBool();
        RainIntensity = FMath::RandRange(0.1f, 2.0f);
        WeatherTimer = 0.0f;
        
        FString RainText = bIsRaining ? TEXT("Yes") : TEXT("No");
        UE_LOG(LogTemp, Log, TEXT("WaterSystem: Weather changed - Rain=%s, Intensity=%.2f"),
               *RainText, RainIntensity);
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
    if (!IsSystemReady())
    {
        return;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    // Use radius-aware water placement (matches terrain brush behavior)
    float TerrainRadius = 5.0f; // Default radius in terrain coordinates
    if (OwnerTerrain)
    {
        TerrainRadius = OwnerTerrain->BrushRadius / OwnerTerrain->TerrainScale;
    }
    
    AddWaterInRadius(X, Y, TerrainRadius, Amount);
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
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
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
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
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

float UWaterSystem::GetTerrainHeightSafe(int32 X, int32 Y) const
{
    if (!OwnerTerrain)
    {
        return 0.0f;
    }
    
    return OwnerTerrain->GetHeightSafe(X, Y);
}

FVector2D UWaterSystem::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    if (!OwnerTerrain)
    {
        return FVector2D::ZeroVector;
    }
    
    return OwnerTerrain->WorldToTerrainCoordinates(WorldPosition);
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
    if (!Material || !WaterDepthTexture)
    {
        if (!Material)
        {
            UE_LOG(LogTemp, Error, TEXT("Material is NULL"));
        }
        if (!WaterDepthTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("WaterDepthTexture is NULL - forcing creation"));
            CreateWaterDepthTexture();
        }
        return;
    }
    
    Material->SetTextureParameterValue(FName("WaterDepthTexture"), WaterDepthTexture);
    ApplyWaterMaterialParams(Material, MaterialParams);
    
    // Apply displacement system (Phase 1 & 2) - only if system is ready
    if (FlowDisplacementTexture && bUseShaderWater)
    {
        UpdateWaterShaderForDisplacement(Material);
    }
}

    
    // Apply water texture and scaling parameters


void UWaterSystem::UpdateWaterShaderParameters()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("UPDATING VOLUMETRIC WATER SHADER PARAMETERS"));
    
    if (!WaterParameterCollection)
    {
        UE_LOG(LogTemp, Warning, TEXT("No WaterParameterCollection assigned"));
        return;
    }
    
    if (!OwnerTerrain || !OwnerTerrain->GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No world context for parameter collection"));
        return;
    }
    
    UWorld* World = OwnerTerrain->GetWorld();
    UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(WaterParameterCollection);
    
    if (Instance)
    {
        // Core time-based parameters
        float GameTime = World->GetTimeSeconds();
        bool bTimeSet = Instance->SetScalarParameterValue(FName("WaterTime"), GameTime);
        
        // Flow and physics parameters
        float MaxFlow = GetMaxFlowSpeed();
        bool bFlowSet = Instance->SetScalarParameterValue(FName("WaterFlowStrength"), MaxFlow);
        
        // Weather responsiveness
        float RainValue = bIsRaining ? RainIntensity : 0.0f;
        bool bRainSet = Instance->SetScalarParameterValue(FName("RainIntensity"), RainValue);
        
        // Volumetric water optical parameters
        bool bAbsorptionSet = Instance->SetScalarParameterValue(FName("AbsorptionCoeff"), MaterialParams.Absorption);
        bool bCausticSet = Instance->SetScalarParameterValue(FName("CausticStrength"), MaterialParams.CausticStrength);
        bool bClaritySet = Instance->SetScalarParameterValue(FName("WaterClarityFactor"), MaterialParams.Clarity);
        bool bWaveSet = Instance->SetScalarParameterValue(FName("WaveStrength"), MaterialParams.WaveStrength);
        bool bTurbiditySet = Instance->SetScalarParameterValue(FName("WaterTurbidity"), GetDynamicTurbidity());
        
        // Volumetric color parameters
        bool bDeepColorSet = Instance->SetVectorParameterValue(FName("DeepWaterColor"), MaterialParams.DeepColor);
        bool bShallowColorSet = Instance->SetVectorParameterValue(FName("ShallowWaterColor"), MaterialParams.ShallowColor);
        bool bWeatherColorSet = Instance->SetVectorParameterValue(FName("WeatherAdjustedWaterColor"), GetWeatherAdjustedWaterColor());
        
        // Atmospheric integration
        if (OwnerTerrain && OwnerTerrain->AtmosphericSystem)
        {
            // Use default sun direction if AtmosphericSystem doesn't have GetSunDirection
            FVector SunDirection = FVector(0.7f, 0.7f, 0.3f); // Default sun direction
            Instance->SetVectorParameterValue(FName("SunDirection"),
                FLinearColor(SunDirection.X, SunDirection.Y, SunDirection.Z, 1.0f));
        }
        
        FString TimeResult = bTimeSet ? TEXT("SUCCESS") : TEXT("FAILED");
        FString FlowResult = bFlowSet ? TEXT("SUCCESS") : TEXT("FAILED");
        FString RainResult = bRainSet ? TEXT("SUCCESS") : TEXT("FAILED");
        FString VolumetricResult = (bAbsorptionSet && bCausticSet && bClaritySet) ? TEXT("SUCCESS") : TEXT("FAILED");
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Volumetric parameters updated - Time:%s Flow:%s Rain:%s Volumetric:%s"),
               *TimeResult, *FlowResult, *RainResult, *VolumetricResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get parameter collection instance"));
    }
}

void UWaterSystem::CreateWaterDepthTexture()
{
    UE_LOG(LogTemp, Log, TEXT("CREATING WATER DEPTH TEXTURE"));
    
    if (!IsSystemReady())
    {
        UE_LOG(LogTemp, Error, TEXT("System not ready - TerrainWidth=%d, TerrainHeight=%d"),
               SimulationData.TerrainWidth, SimulationData.TerrainHeight);
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    UE_LOG(LogTemp, Log, TEXT("Creating water depth texture: %dx%d"), Width, Height);
    
    // Check if we're on the game thread
    if (!IsInGameThread())
    {
        UE_LOG(LogTemp, Error, TEXT("NOT ON GAME THREAD - deferring texture creation"));
        // Defer to game thread
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            CreateWaterDepthTexture();
        });
        return;
    }
    
    // Destroy existing texture if it exists
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Destroying existing texture"));
        WaterDepthTexture = nullptr;
    }
    
    // Create texture with explicit settings
    WaterDepthTexture = UTexture2D::CreateTransient(Width, Height, PF_R8);
    
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Log, TEXT("Water texture created successfully"));
        
        // Configure texture settings
        WaterDepthTexture->Filter = TextureFilter::TF_Nearest;
        WaterDepthTexture->AddressX = TextureAddress::TA_Clamp;
        WaterDepthTexture->AddressY = TextureAddress::TA_Clamp;
        WaterDepthTexture->SRGB = false;
        WaterDepthTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
        
        // Force update
        WaterDepthTexture->UpdateResource();
        
        UE_LOG(LogTemp, Log, TEXT("Water texture configured and updated"));
        
        // Immediately populate with test data
        UpdateWaterDepthTexture();
        
        UE_LOG(LogTemp, Log, TEXT("Initial texture data populated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TEXTURE CREATION FAILED - CreateTransient returned NULL"));
        
        // Try alternative creation method
        WaterDepthTexture = NewObject<UTexture2D>();
        if (WaterDepthTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("Trying NewObject approach..."));
            // This would need additional setup - but let's see if CreateTransient is the issue
        }
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
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Applied volumetric water parameters - Clarity: %.2f, Absorption: %.2f"),
           Params.Clarity, Params.Absorption);
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

void UWaterSystem::ApplyVolumetricWaterToMaterial(UMaterialInstanceDynamic* Material)
{
    if (!Material) return;
    
    // Apply complete volumetric water material setup
    ApplyWaterTextureToMaterial(Material);
    
    // Additional volumetric-specific parameters
    Material->SetScalarParameterValue(FName("VolumetricEnabled"), 1.0f);
    Material->SetScalarParameterValue(FName("OpticalDepthScale"), MaterialParams.Absorption);
    
    UE_LOG(LogTemp, Log, TEXT("Applied volumetric water configuration to material"));
}

/**
 * Hydraulic erosion simulation with sediment transport
 *
 * Algorithm: Velocity-based erosion with deposition
 * References:
 * - Musgrave, F.K. (1989). "The synthesis and rendering of eroded fractal terrains"
 * - Benes, B. & Forsbach, R. (2001). "Layered data representation for visual simulation of terrain erosion"
 *
 * Process:
 * 1. Check flow velocity against erosion threshold
 * 2. Fast water erodes terrain and picks up sediment
 * 3. Slow water deposits sediment
 * 4. Update terrain height and mark chunks for visual update
 */

void UWaterSystem::UpdateErosion(float DeltaTime)
{
    if (!SimulationData.IsValid() || !OwnerTerrain)
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Process erosion for cells with significant water
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (SimulationData.WaterDepthMap[Index] > MinWaterDepth)
            {
                // Calculate flow speed
                float FlowSpeed = FMath::Sqrt(
                    SimulationData.WaterVelocityX[Index] * SimulationData.WaterVelocityX[Index] +
                    SimulationData.WaterVelocityY[Index] * SimulationData.WaterVelocityY[Index]
                );
                
                if (FlowSpeed > MinErosionVelocity) // Fast water erodes
                {
                    float ErosionAmount = ErosionRate * FlowSpeed * DeltaTime;
                    
                    // Lower terrain height
                    float CurrentHeight = OwnerTerrain->GetHeightSafe(X, Y);
                    OwnerTerrain->SetHeightSafe(X, Y, CurrentHeight - ErosionAmount);
                    
                    // Store sediment in water
                    SimulationData.SedimentMap[Index] += ErosionAmount;
                    
                    // Mark chunk for visual update
                    MarkChunkForUpdate(X, Y);
                    
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Erosion at (%d,%d): %.3f, Flow: %.1f"),
                           X, Y, ErosionAmount, FlowSpeed);
                }
                else if (SimulationData.SedimentMap[Index] > 0.0f) // Slow water deposits sediment
                {
                    float DepositionAmount = SimulationData.SedimentMap[Index] * DepositionRate * DeltaTime;
                    
                    // Raise terrain height
                    float CurrentHeight = OwnerTerrain->GetHeightSafe(X, Y);
                    OwnerTerrain->SetHeightSafe(X, Y, CurrentHeight + DepositionAmount);
                    
                    // Remove sediment from water
                    SimulationData.SedimentMap[Index] -= DepositionAmount;
                    
                    // Mark chunk for visual update
                    MarkChunkForUpdate(X, Y);
                    
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Deposition at (%d,%d): %.3f"),
                           X, Y, DepositionAmount);
                }
            }
        }
    }
}
