/**
 * ============================================
 * DRIFT DYNAMIC TERRAIN SYSTEM - REORGANIZED
 * ============================================
 * Reorganized: November 2025
 * Original: 4,062 lines | Reorganized: ~4,622 lines | Functions: 98
 * All function logic preserved exactly - zero changes to implementation
 * Added comprehensive documentation (~560 lines, 12% overhead)
 */

#include "DynamicTerrain.h"
#include "Drift.h"  // Include for validation macros and constants
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "Async/ParallelFor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterUtils.h"
#include "Kismet/GameplayStatics.h"
#include "MasterController.h"
#include "WaterController.h"  // CRITICAL: Add WaterController include
#include "GeologyController.h"
#include "TemporalManager.h"

using namespace DriftConstants;  // Use named constants



// ============================================================================
// SECTION 1: SYSTEM LIFECYCLE & INITIALIZATION (~650 lines, 7%)
// ============================================================================
/**
 * PURPOSE:
 * Core system initialization, frame management, subsystem integration.
 *
 * SUBSECTIONS:
 * 1.1 Constructor
 * 1.2 Frame Caching
 * 1.3 BeginPlay
 * 1.4 Tick
 * 1.5 Water System
 * 1.6 Atmospheric System
 * 1.7 MasterController
 * 1.8 Terrain Data Init
 * 1.9 Chunk Init
 */

// 1.1 CONSTRUCTOR

ADynamicTerrain::ADynamicTerrain()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component for chunk organization
    TerrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TerrainRoot"));
    RootComponent = TerrainRoot;
    
    // Set default material to prevent teal flash
    DefaultTerrainMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    
    // If that fails, will use wireframe as fallback in InitializeChunks
    if (!DefaultTerrainMaterial) {
        UE_LOG(LogTemp, Log, TEXT("Could not load BasicShapeMaterial, will use wireframe fallback"));
    }
    
    // Initialize scalable world configuration FIRST
    CurrentWorldSize = ETerrainWorldSize::Medium;
    WorldConfig = GetWorldConfigForSize(CurrentWorldSize);
    
    // Apply configuration to legacy properties
    TerrainWidth = WorldConfig.TerrainWidth;
    TerrainHeight = WorldConfig.TerrainHeight;
    ChunkSize = WorldConfig.ChunkSize;
    TerrainScale = 100.0f;
    MaxTerrainHeight = 8000.0f;
    MinTerrainHeight = -25000.0f;
    
    // Create water system AFTER terrain dimensions are set
    // WaterSystem = NewObject<UWaterSystem>(this, UWaterSystem::StaticClass(), TEXT("WaterSystem"));
    
    // Create atmospheric system AFTER terrain dimensions are set
    // AtmosphericSystem = NewObject<UAtmosphericSystem>(this, UAtmosphericSystem::StaticClass(), TEXT("AtmosphericSystem"));
    ChunkOverlap = 2; // Enhanced overlap for authority integration
    MaxUpdatesPerFrame = 2;
    
    // AUTHORITY: Brush settings now come from MasterController
    // BrushRadius and BrushStrength removed - get from Master
    
    // Performance optimization variables
    ModificationCooldown = MODIFICATION_COOLDOWN; // 20 modifications per second max
    LastModificationTime = 0.0f;
    TotalChunkUpdatesThisFrame = 0;
    StatUpdateTimer = 0.0f;
    
    // Performance settings
    bShowPerformanceStats = true;
    bShowChunkBounds = false;
    
    // Frustum culling system
    bEnableFrustumCulling = true;
    CullingUpdateTimer = 0.0f;
    CullingUpdateRate = FRUSTUM_CULLING_UPDATE_RATE; // Update visibility 10 times per second
}

// 1.2 FRAME TIME CACHING

// ===== FRAME TIME CACHING SYSTEM =====
// Eliminates 20+ GetWorld()->GetTimeSeconds() calls per frame

float ADynamicTerrain::GetCachedFrameTime() const
{
    // Get current frame number from engine
    int32 CurrentFrameNumber = GFrameCounter;
    
    // Update cache if this is a new frame
    if (CurrentFrameNumber != CachedFrameNumber)
    {
        CachedFrameTime = GetWorld()->GetTimeSeconds();
        CachedFrameNumber = CurrentFrameNumber;
    }
    
    return CachedFrameTime;
}

// 1.3 BEGINPLAY

/**
 * Terrain initialization sequence with dependency management
 *
 * CRITICAL INITIALIZATION ORDER:
 * Phase 1: Basic data structures (no dependencies)
 * Phase 2: MasterController integration (authority-based configuration)
 * Phase 3: System initialization (requires configured dimensions)
 *
 * Dependencies:
 * - Called automatically by UE5 ActorSystem
 * - Must complete before any terrain modification calls
 * - MasterController may not be available initially (graceful degradation)
 *
 * Error Handling:
 * - Falls back to standalone mode if MasterController unavailable
 * - All systems designed to work with or without authority
 */
void ADynamicTerrain::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Verbose, TEXT("DynamicTerrain: Beginning initialization"));
    
    // PHASE 1: Initialize basic data structures ONLY
    InitializeTerrainData();
    
    // CRITICAL FIX: DO NOT initialize GPU here!
    // GPU initialization must happen AFTER terrain generation
    if (bUseGPUTerrain)
    {
        UE_LOG(LogTemp, Verbose, TEXT("GPU Terrain will be initialized AFTER terrain generation"));
        // Set a flag to initialize GPU later
        bPendingGPUInit = true;
    }
}

// 1.4 TICK

void ADynamicTerrain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Reset frame counters
    TotalChunkUpdatesThisFrame = 0;
    
    // Update frustum culling (needed for both modes)
    if (bEnableFrustumCulling)
    {
        UpdateFrustumCulling(DeltaTime);
    }
    
    // Process based on compute mode
    switch (CurrentComputeMode)
    {
        case ETerrainComputeMode::CPU:
            // Normal CPU processing
            ProcessPendingChunkUpdates();
            break;
            
        case ETerrainComputeMode::GPU:
            // Execute GPU compute shader for terrain modifications
            if (bHasPendingBrush || bEnableGPUErosion)
            {
                ExecuteTerrainComputeShader(DeltaTime);
            }

            // Update height buffer interpolation for smooth terrain transitions
            UpdateHeightBufferInterpolation(DeltaTime);

            // Update water height texture EVERY FRAME (no interpolation)
            // This decouples water visuals from the terrain double-buffer system
            UpdateWaterHeightTexture();

            // CRITICAL: Still process water chunk updates in GPU mode
            ProcessPendingWaterChunkUpdates();

            // FAST SYNC: Lightweight height-only sync for water (30Hz)
            // This lets water see terrain changes smoothly without expensive mesh rebuilds
            if (bEnableGPUErosion && bGPUDataDirty)
            {
                WaterHeightSyncTimer += DeltaTime;
                if (WaterHeightSyncTimer >= WaterHeightSyncInterval)
                {
                    SyncGPUHeightsForWater();
                    WaterHeightSyncTimer = 0.0f;
                }
            }

            // SLOW SYNC: Full sync with validation and mesh updates (0.5s)
            // This handles mesh rebuilds and is expensive - keep it slow
            if (bGPUDataDirty)
            {
                GPUSyncTimer += DeltaTime;
                float SyncInterval = 0.5f;  // Full sync always at 0.5s
                if (GPUSyncTimer >= SyncInterval)
                {
                    SyncGPUToCPU();
                    SyncGPUChunkVisuals();
                    GPUSyncTimer = 0.0f;
                }
            }

            // Process any chunks that need mesh regeneration (queued by SyncGPUToCPU)
            if (PendingChunkUpdates.Num() > 0)
            {
                ProcessPendingChunkUpdates();
            }
            break;
            
        case ETerrainComputeMode::Switching:
            // Wait for transition to complete
            break;
    }
    
    // CRITICAL: Update water system regardless of terrain compute mode
    if (WaterSystem && WaterSystem->IsSystemReady())
    {
        WaterSystem->UpdateWaterSimulation(DeltaTime);
        
        // Process any water-specific chunk updates
        if (PendingWaterChunkUpdates.Num() > 0)
        {
            ProcessPendingWaterChunkUpdates();
        }
    }
    
    // Update atmospheric system (if not handled by TemporalManager)
    /*if (AtmosphericSystem)
    {
        AtmosphericSystem->UpdateAtmosphericSimulation(DeltaTime);
    }*/
    
    // Update performance statistics
    if (bShowPerformanceStats)
    {
        UpdatePerformanceStats(DeltaTime);
    }
}

// 1.5 WATER SYSTEM INTEGRATION

void ADynamicTerrain::InitializeWaterSystem()
{
    // Prevent Duplicate Water System Creation
    if (WaterSystem && WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Verbose, TEXT("WaterSystem already initialized"));
            return;
        }
    // Ensure terrain dimensions are finalized FIRST
    if (TerrainWidth <= 0 || TerrainHeight <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize water system - invalid terrain dimensions"));
        return;
    }
    
    if (!WaterSystem)
    {
        WaterSystem = NewObject<UWaterSystem>(this, UWaterSystem::StaticClass(), TEXT("WaterSystem"));
        UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: WaterSystem created"));
    }
    
    if (WaterSystem)
    {
        // Register with MasterController FIRST
        if (CachedMasterController)
        {
            WaterSystem->RegisterWithMasterController(CachedMasterController);
            UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: WaterSystem registered with MasterController"));
        }
        
        // Pass validated dimensions explicitly
        WaterSystem->Initialize(this, CachedMasterController);
        
        // Validate initialization completed successfully
        if (WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: WaterSystem fully initialized and ready"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: WaterSystem initialized but not ready yet"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DynamicTerrain: Failed to create water system"));
    }
}



bool ADynamicTerrain::IsWaterSystemReady() const
{
    return WaterSystem && WaterSystem->IsSystemReady();
}

void ADynamicTerrain::ValidateGPUUpload()
{
    // Check for NaN or extreme values
    int32 InvalidCount = 0;
    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;
    
    for (int32 i = 0; i < HeightMap.Num(); i++)
    {
        float Height = HeightMap[i];
        
        if (FMath::IsNaN(Height) || !FMath::IsFinite(Height))
        {
            HeightMap[i] = 0.0f;  // Fix invalid values
            InvalidCount++;
        }
        else
        {
            MinHeight = FMath::Min(MinHeight, Height);
            MaxHeight = FMath::Max(MaxHeight, Height);
        }
    }
    
    if (InvalidCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Fixed %d invalid height values!"), InvalidCount);
    }
    
    // Clamp extreme values
    const float MaxAllowedHeight = 10000.0f;
    if (MaxHeight > MaxAllowedHeight || MinHeight < -MaxAllowedHeight)
    {
        UE_LOG(LogTemp, Log, TEXT("Clamping extreme heights: Min=%.2f, Max=%.2f"),
               MinHeight, MaxHeight);
        
        for (int32 i = 0; i < HeightMap.Num(); i++)
        {
            HeightMap[i] = FMath::Clamp(HeightMap[i], -MaxAllowedHeight, MaxAllowedHeight);
        }
    }
}

// 1.6 ATMOSPHERIC SYSTEM INTEGRATION

void ADynamicTerrain::InitializeAtmosphericSystem()
{
    // Create atmospheric system with proper timing
    if (!AtmosphericSystem)
    {
        AtmosphericSystem = NewObject<UAtmosphericSystem>(this, UAtmosphericSystem::StaticClass(), TEXT("AtmosphericSystem"));
    }
    
    if (AtmosphericSystem)
    {
        // CRITICAL: Pass both terrain AND water system references
        AtmosphericSystem->Initialize(this, WaterSystem);
        
        // CRITICAL INTEGRATION TEST: Verify atmospheric-water connection
        bool bWaterSystemConnected = (AtmosphericSystem->WaterSystem != nullptr);
        bool bTerrainSystemConnected = (AtmosphericSystem->TerrainSystem != nullptr);
        
        UE_LOG(LogTemp, Log, TEXT("[ATMOSPHERIC INTEGRATION] System initialization:"));
        UE_LOG(LogTemp, Log, TEXT("  - WaterSystem connection: %s"), bWaterSystemConnected ? TEXT(" CONNECTED") : TEXT(" FAILED"));
        UE_LOG(LogTemp, Log, TEXT("  - TerrainSystem connection: %s"), bTerrainSystemConnected ? TEXT(" CONNECTED") : TEXT(" FAILED"));
        
        if (bWaterSystemConnected && bTerrainSystemConnected)
        {
            UE_LOG(LogTemp, Log, TEXT("[ATMOSPHERIC INTEGRATION]  READY FOR PRECIPITATION TRANSFER"));
            
            // AUTHORITY FIX: Use CachedMasterController for terrain scale
            float AuthScale = CachedMasterController ? CachedMasterController->GetTerrainScale() : TerrainScale;
            FVector2D TerrainCenter(TerrainWidth * AuthScale * 0.5f, TerrainHeight * AuthScale * 0.5f);

            UE_LOG(LogTemp, Log, TEXT("[ATMOSPHERIC INTEGRATION]  Created initial weather patterns"));
            UE_LOG(LogTemp, Verbose, TEXT("[ATMOSPHERIC INTEGRATION]  Precipitation should begin within 30-60 seconds"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[ATMOSPHERIC INTEGRATION]  CONNECTION FAILED - Precipitation will not work"));
        }
    }
}

// 1.7 MASTERCONTROLLER INTEGRATION

void ADynamicTerrain::InitializeWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeWithMasterController called with null Master"));
        return;
    }
    
    // Prevent duplicate initialization
    if (CachedMasterController == Master && HeightMap.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: Already initialized with this MasterController"));
        return;
    }
    
    CachedMasterController = Master;
    
    // Apply authoritative values
    FVector2D AuthoritativeDims = Master->GetWorldDimensions();
    int32 AuthoritativeChunkSize = Master->GetOptimalChunkSize();
    float AuthoritativeScale = Master->GetTerrainScale();
    FVector2D ChunkDims = Master->GetChunkDimensions();
    
    TerrainWidth = AuthoritativeDims.X;
    TerrainHeight = AuthoritativeDims.Y;
    ChunkSize = AuthoritativeChunkSize;
    TerrainScale = AuthoritativeScale;
    ChunksX = ChunkDims.X;
    ChunksY = ChunkDims.Y;
    
    // Initialize HeightMap with proper size
    int32 TotalSize = TerrainWidth * TerrainHeight;
    HeightMap.SetNumZeroed(TotalSize);
    
    UE_LOG(LogTemp, Verbose, TEXT("DynamicTerrain: Authority established - %dx%d terrain, %dx%d chunks"),
           TerrainWidth, TerrainHeight, ChunksX, ChunksY);
    
    // CRITICAL: Generate terrain BEFORE GPU initialization
    GenerateProceduralTerrain();
    InitializeChunks();
    
    // NOW initialize GPU with actual terrain data
    if (bPendingGPUInit)
    {
        UE_LOG(LogTemp, Log, TEXT("Initializing GPU Terrain with generated data..."));
        InitializeGPUTerrainWithData();
        bPendingGPUInit = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: Terrain generation complete, awaiting system initialization"));
}

bool ADynamicTerrain::ValidateMasterControllerAuthority() const
{
    bool bValid = true;
    
    // Test 1: Terrain authority
    if (!CachedMasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("AUTHORITY FAILURE: No MasterController"));
        bValid = false;
    }
    
    // Test 2: Coordinate roundtrip
    if (CachedMasterController)
    {
        FVector TestPos(1000, 1000, 0);
        FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(TestPos);
        FVector BackToWorld = TerrainToWorldPosition(TerrainCoords.X, TerrainCoords.Y);
        if (FVector::Dist(TestPos, BackToWorld) > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("AUTHORITY FAILURE: Coordinates"));
            bValid = false;
        }
    }
    
    return bValid;
}

// 1.8 TERRAIN DATA INITIALIZATION

void ADynamicTerrain::InitializeTerrainData()
{
    UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: Initializing basic terrain data"));
    
    // Initialize using current world config (will be overridden by Master if available)
    TerrainWidth = WorldConfig.TerrainWidth;
    TerrainHeight = WorldConfig.TerrainHeight;
    ChunkSize = WorldConfig.ChunkSize;
    ChunksX = WorldConfig.ChunksX;
    ChunksY = WorldConfig.ChunksY;
    
    UE_LOG(LogTemp, Log, TEXT("DynamicTerrain: Basic initialization complete - %dx%d terrain"), TerrainWidth, TerrainHeight);
}

// 1.9 CHUNK SYSTEM INITIALIZATION

void ADynamicTerrain::InitializeChunks()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing chunk system with %dx%d chunks"), ChunksX, ChunksY);
    
    // Clear existing chunks
    TerrainChunks.Empty();
    
    // Track statistics for batched logging
    int32 FallbackMaterialCount = 0;
    int32 FailedChunkCount = 0;
    
    // Create chunks
    for (int32 ChunkY = 0; ChunkY < ChunksY; ChunkY++)
    {
        for (int32 ChunkX = 0; ChunkX < ChunksX; ChunkX++)
        {
            FTerrainChunk NewChunk;
            NewChunk.ChunkX = ChunkX;
            NewChunk.ChunkY = ChunkY;
            
            // PHASE 4: Use pooled mesh component
            NewChunk.MeshComponent = bEnableChunkPooling ? GetPooledMeshComponent() : nullptr;
            
            if (!NewChunk.MeshComponent)
            {
                // Create new component if pool empty
                CachedChunkNameBuffer = FString::Printf(TEXT("Chunk_%d_%d"), ChunkX, ChunkY);
                NewChunk.MeshComponent = NewObject<UProceduralMeshComponent>(this, UProceduralMeshComponent::StaticClass(), *CachedChunkNameBuffer);
            }
            
            if (NewChunk.MeshComponent)
            {
                NewChunk.MeshComponent->SetupAttachment(TerrainRoot);
                NewChunk.MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                NewChunk.MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
                
                // Position chunk in world space
                FVector2D ChunkWorldPos = GetChunkWorldPosition(ChunkX, ChunkY);
                NewChunk.MeshComponent->SetRelativeLocation(FVector(ChunkWorldPos.X, ChunkWorldPos.Y, 0.0f));
                
                // Apply material BEFORE registering component
                UMaterialInterface* MaterialToUse = CurrentActiveMaterial;
                if (!MaterialToUse) {
                    MaterialToUse = DefaultTerrainMaterial;
                }
                if (!MaterialToUse && GEngine) {
                    MaterialToUse = GEngine->WireframeMaterial;
                    FallbackMaterialCount++;
                }
                
                if (MaterialToUse) {
                    NewChunk.MeshComponent->SetMaterial(0, MaterialToUse);
                }
                
                // Register component with the world
                NewChunk.MeshComponent->RegisterComponent();
                
                TerrainChunks.Add(NewChunk);
                
                // Generate mesh with material already applied
                GenerateChunkMesh(ChunkX, ChunkY);
            }
            else
            {
                FailedChunkCount++;
            }
        }
    }
    
    // Batch logging
    if (FallbackMaterialCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Using fallback wireframe material for %d/%d chunks"),
               FallbackMaterialCount, TerrainChunks.Num());
    }
    
    if (FailedChunkCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create %d chunks"), FailedChunkCount);
    }
    
    UE_LOG(LogTemp, Log, TEXT(" Created %d chunks with validated materials"), TerrainChunks.Num());
}


// ============================================================================
// SECTION 2: TERRAIN GENERATION (~450 lines, 10%)
// ============================================================================
/**
 * PURPOSE:
 * Procedural and scripted terrain generation with multiple algorithm support.
 *
 * SUBSECTIONS:
 * 2.1 Procedural Generation
 * 2.2 Simple Generation
 * 2.3 Advanced Settings
 * 2.4 Reset & Clean Generation
 * 2.5 Map Definitions
 * 2.6 Runtime Parameters
 * 2.7 DEM Integration
 */

// 2.1 PROCEDURAL GENERATION

void ADynamicTerrain::GenerateProceduralTerrain()
{
    // CRITICAL FIX: Check if we have map definition with specific parameters
    if (bHasMapDefinition)
    {
        UE_LOG(LogTemp, Verbose, TEXT("=== USING MAP DEFINITION FOR TERRAIN GENERATION ==="));
        UE_LOG(LogTemp, Log, TEXT("Map: %s"), *CurrentMapDefinition.DisplayName.ToString());
        UE_LOG(LogTemp, Log, TEXT("Seed: %d"), CurrentMapDefinition.ProceduralSeed);
        UE_LOG(LogTemp, Log, TEXT("Height Variation: %.1f"), CurrentMapDefinition.HeightVariation);
        UE_LOG(LogTemp, Log, TEXT("Noise Scale: %.4f"), CurrentMapDefinition.NoiseScale);
        UE_LOG(LogTemp, Log, TEXT("Octaves: %d"), CurrentMapDefinition.NoiseOctaves);
        
        // Use the map definition parameters
        GenerateProceduralTerrainWithSettings(
            CurrentMapDefinition.ProceduralSeed,
            CurrentMapDefinition.HeightVariation,
            CurrentMapDefinition.NoiseScale,
            CurrentMapDefinition.NoiseOctaves
        );
        return;
    }
    
    // FALLBACK: Default procedural generation (no map definition)
    UE_LOG(LogTemp, Log, TEXT("Generating default sinusoidal terrain (no map definition)..."));
    
    // Create sinusoidal terrain with multiple wave frequencies for interesting water flow
    for (int32 Y = 0; Y < TerrainHeight; Y++)
    {
        for (int32 X = 0; X < TerrainWidth; X++)
        {
            // Normalize coordinates to 0-1 range
            float NormX = (float)X / (TerrainWidth - 1);
            float NormY = (float)Y / (TerrainHeight - 1);
            
            // Multiple sine waves for complex terrain
            float Wave1 = FMath::Sin(NormX * 2.0f * PI) * FMath::Sin(NormY * 2.0f * PI);
            float Wave2 = FMath::Sin(NormX * 4.0f * PI) * FMath::Sin(NormY * 4.0f * PI) * 0.5f;
            float Wave3 = FMath::Sin(NormX * 8.0f * PI) * FMath::Sin(NormY * 8.0f * PI) * 0.25f;
            
            // Diagonal ridge pattern for interesting flow
            float Ridge = FMath::Sin((NormX + NormY) * 3.0f * PI) * 0.3f;
            
            // Combine waves with different amplitudes
            float CombinedHeight = (Wave1 + Wave2 + Wave3 + Ridge) * MaxTerrainHeight * 0.3f;
            
            SetHeightSafe(X, Y, CombinedHeight);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Sinusoidal terrain generation complete - ready for water flow"));
}

// 2.2 SIMPLE GENERATION

void ADynamicTerrain::GenerateSimpleTerrain()
{
    // Fill height map with simple sine wave pattern
    for (int32 Y = 0; Y < TerrainHeight; Y++)
    {
        for (int32 X = 0; X < TerrainWidth; X++)
        {
            float Height = FMath::Sin(X * 0.05f) * FMath::Cos(Y * 0.05f) * 300.0f;
            SetHeightSafe(X, Y, Height);
        }
    }
    
    // FIXED: Don't mark ALL chunks - they get updated automatically during generation
    UE_LOG(LogTemp, Log, TEXT("Simple terrain generated - chunks will update as needed"));
}

// 2.3 ADVANCED PROCEDURAL

void ADynamicTerrain::GenerateProceduralTerrainWithSettings(int32 Seed, float HeightVar, float NoiseScl, int32 Octaves)
{
    UE_LOG(LogTemp, Verbose, TEXT("=== GENERATING TERRAIN WITH SETTINGS ==="));
    UE_LOG(LogTemp, Log, TEXT("Seed=%d, Height=%.1f, Noise=%.4f, Octaves=%d"),
           Seed, HeightVar, NoiseScl, Octaves);
    UE_LOG(LogTemp, Log, TEXT("HeightMultiplier=%.2f"), HeightMultiplier);
    
    // Use Seed if >= 0, otherwise random
    FRandomStream RandomStream;
    if (Seed >= 0)
    {
        RandomStream.Initialize(Seed);
        UE_LOG(LogTemp, Log, TEXT("Using FIXED SEED %d for reproducible terrain"), Seed);
    }
    else
    {
        Seed = FMath::Rand();
        RandomStream.Initialize(Seed);
        UE_LOG(LogTemp, Log, TEXT("Generated RANDOM seed %d"), Seed);
    }
    
    // Generate with parameters
    for (int32 Y = 0; Y < TerrainHeight; Y++)
    {
        for (int32 X = 0; X < TerrainWidth; X++)
        {
            float NormX = (float)X / (TerrainWidth - 1);
            float NormY = (float)Y / (TerrainHeight - 1);
            
            float Height = 0.0f;
            float Amplitude = HeightVar * HeightMultiplier; // USE EXPOSED PROPERTY
            float Frequency = 1.0f;
            
            // Multi-octave generation with proper seed variation
            for (int32 Octave = 0; Octave < Octaves; Octave++)
            {
                // Add seed-based phase offset for variation
                float PhaseX = (float)(Seed % 1000) * 0.001f;
                float PhaseY = (float)((Seed / 1000) % 1000) * 0.001f;
                
                // NoiseScl controls the BASE frequency
                float ScaledFreq = Frequency * (NoiseScl * 100.0f);
                
                Height += FMath::Sin((NormX + PhaseX) * ScaledFreq * 2.0f * PI) *
                         FMath::Sin((NormY + PhaseY) * ScaledFreq * 2.0f * PI) * Amplitude;
                
                Amplitude *= 0.5f;
                Frequency *= 2.0f;
            }
            
            SetHeightSafe(X, Y, Height);
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("=== TERRAIN GENERATION COMPLETE ==="));
    UE_LOG(LogTemp, Verbose, TEXT("Effective max height: ~%.0f meters"), HeightVar * HeightMultiplier * 2.0f);
}

// 2.4 RESET & CLEAN GENERATION

void ADynamicTerrain::ResetTerrainFully()
{
    UE_LOG(LogTemp, Log, TEXT("Performing full terrain reset..."));

    // CRITICAL: Clear ALL queues first to prevent zombie chunks
    PendingChunkUpdates.Empty();
    PendingWaterChunkUpdates.Empty();
    PriorityChunkQueue.Empty();
    
    // Reset frame counter
    TotalChunkUpdatesThisFrame = 0;
    
    // Stop any active timers that might trigger chunk updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    // Destroy existing chunks cleanly
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent)
        {
            Chunk.MeshComponent->ClearAllMeshSections();
            Chunk.MeshComponent->DestroyComponent();
        }
    }
    TerrainChunks.Empty();
    
    // Reset heightmap
    HeightMap.Empty();
    int32 TotalSize = TerrainWidth * TerrainHeight;
    HeightMap.SetNumZeroed(TotalSize);

    // Generate terrain data ONLY
    GenerateProceduralTerrain();

    // Initialize chunk system (creates meshes automatically)
    InitializeChunks();

    // ===== CRITICAL: UPLOAD NEW TERRAIN TO GPU =====
    // If in GPU mode, the new procedural terrain must be uploaded to GPU textures
    if (CurrentComputeMode == ETerrainComputeMode::GPU && bGPUInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Uploading new terrain to GPU after reset..."));
        TransferHeightmapToGPU();
        bGPUDataDirty = true;  // Mark dirty so sync system picks it up

        // Reset sync timers so water sees fresh data immediately
        GPUSyncTimer = 0.0f;
        WaterHeightSyncTimer = 0.0f;
    }

    // ===== CRITICAL FIX: PROPER WATER SYSTEM RESET SEQUENCE =====
    
    if (WaterSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Resetting water system with erosion texture cleanup..."));
        WaterSystem->ResetWaterSystem();  // This now includes erosion texture recreation
        
        // Re-register with MasterController
        if (CachedMasterController)
        {
            WaterSystem->RegisterWithMasterController(CachedMasterController);
            CachedMasterController->ResetWaterBudget();
        }
        
        // Force texture recreation and update
        WaterSystem->CreateWaterDepthTexture();
        WaterSystem->CreateAdvancedWaterTexture();
        WaterSystem->UpdateWaterDepthTexture();
        
        // CRITICAL: Reconnect GPU terrain to ensure erosion textures are linked
        if (CurrentComputeMode == ETerrainComputeMode::GPU && bEnableGPUErosion)
        {
            ConnectToGPUWaterSystem(WaterSystem);
        }
    }
        
        // Step 4: Enhanced ready check with timeout
        float WaitTime = 0.0f;
        const float MaxWaitTime = 3.0f; // Max 3 seconds wait
        const float CheckInterval = 0.1f; // 100ms intervals
        
        while (!WaterSystem->IsSystemReady() && WaitTime < MaxWaitTime)
        {
            // Use frame-based waiting instead of sleep to avoid blocking
            if (GetWorld())
            {
                // Create a timer handle for this operation
                FTimerHandle WaitTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(
                    WaitTimerHandle,
                    [](){}, // Empty lambda, just for timing
                    CheckInterval,
                    false
                );
            }
            
            // Force a tick update to help system initialization
            if (WaterSystem)
            {
                WaterSystem->UpdateWaterSimulation(CheckInterval);
            }
            
            WaitTime += CheckInterval;
            if (WaitTime >= MaxWaitTime) break;
        }
        
        if (WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Log, TEXT("Water system ready after %.1f seconds"), WaitTime);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Water system taking longer than expected (%.1f seconds) - continuing with initialization"), WaitTime);
        }
    
    
    // Step 5: Reconnect WaterController to the reset water system
    if (CachedMasterController && CachedMasterController->WaterController)
    {
        AWaterController* WaterController = CachedMasterController->WaterController;
        
        // Force WaterController to re-initialize with the reset water system
        WaterController->Initialize(this);
        
        // Validate the connection with a small delay to allow initialization
        if (GetWorld())
        {
            // Create a timer to validate the connection after initialization
            FTimerHandle ValidationTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                [this, WaterController]()
                {
                    // Check if WaterController has proper references
                    bool bTerrainConnected = (WaterController->GetTargetTerrain() == this);
                    bool bWaterSystemConnected = (WaterController->GetWaterSystem() == WaterSystem);
                    
                    if (bTerrainConnected && bWaterSystemConnected)
                    {
                        UE_LOG(LogTemp, Verbose, TEXT("WaterController successfully reconnected after reset"));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("WaterController reconnection FAILED after reset - Terrain: %s, WaterSystem: %s"),
                               bTerrainConnected ? TEXT("OK") : TEXT("FAILED"),
                               bWaterSystemConnected ? TEXT("OK") : TEXT("FAILED"));
                        
                        // Attempt one more reconnection
                        WaterController->Initialize(this);
                    }
                },
                0.1f, // 100ms delay
                false
            );
        }
    }
    
    // Step 6: RESET atmospheric system AFTER water is confirmed ready
    if (CachedMasterController && CachedMasterController->AtmosphereController)
    {
        AAtmosphereController* AtmoController = CachedMasterController->AtmosphereController;
        
        // Reset the atmosphere system completely
        AtmoController->ResetAtmosphereSystem();
        
        AtmoController->InitializeWithAuthority(CachedMasterController, this);
        
        // Reinitialize with proper connections
        if (AtmosphericSystem)
        {
            AtmosphericSystem->Initialize(this, WaterSystem);
            UE_LOG(LogTemp, Verbose, TEXT("Atmospheric system reset and reinitialized"));
        }
    }
    else if (AtmosphericSystem)
    {
        // Fallback if no MasterController
        AtmosphericSystem->Initialize(this, WaterSystem);
        UE_LOG(LogTemp, Log, TEXT("Atmospheric system reinitialized (no controller reset)"));
    }
    
    // Step 6.5: RESET GEOLOGY SYSTEM 
    if (CachedMasterController && CachedMasterController->GeologyController)
    {
        AGeologyController* GeoController = CachedMasterController->GeologyController;
        
        // Reset geology system completely
        GeoController->ResetGeologySystem();
        
        // Reinitialize with fresh terrain and water system
        GeoController->Initialize(this, WaterSystem);
        
        UE_LOG(LogTemp, Warning, TEXT("Geology system reset and reinitialized"));
    }
    
    // Step 7: Force material refresh to ensure water shaders are properly connected
    if (CurrentActiveMaterial)
    {
        // Simple approach: Just refresh materials immediately after reset
        // The systems should be ready by now, and if not, the next tick will retry
        SetActiveMaterial(CurrentActiveMaterial);
        UE_LOG(LogTemp, Warning, TEXT("Materials refreshed after reset"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Full terrain reset complete - %d chunks created"), TerrainChunks.Num());
}

void ADynamicTerrain::PerformCleanGeneration(bool bInitializeSystems)
{
    UE_LOG(LogTemp, Warning, TEXT("Performing clean terrain generation..."));

    // Initialize height map from scratch
    HeightMap.Empty();
    int32 TotalSize = TerrainWidth * TerrainHeight;
    HeightMap.SetNumZeroed(TotalSize);

    // Clear any existing chunks
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent)
        {
            Chunk.MeshComponent->ClearAllMeshSections();
            Chunk.MeshComponent->DestroyComponent();
        }
    }
    TerrainChunks.Empty();
    
    // Clear ALL pending update queues to prevent leftover chunk processing
    PendingChunkUpdates.Empty();
    PendingWaterChunkUpdates.Empty();
    PriorityChunkQueue.Empty();
    TotalChunkUpdatesThisFrame = 0;

    // ROUTER: Check if we have map definition
    if (bHasMapDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("Using map definition: %s"), *CurrentMapDefinition.DisplayName.ToString());
        ApplyMapDefinitionGeneration();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No map definition - using default procedural"));
        GenerateProceduralTerrain();
    }

    // Initialize chunk system
    InitializeChunks();

    if (bInitializeSystems)
       {
           InitializeWaterSystem();
           InitializeAtmosphericSystem();
       }

    UE_LOG(LogTemp, Warning, TEXT("Clean terrain generation complete."));
}

// 2.5 MAP DEFINITION SYSTEM

void ADynamicTerrain::SetMapDefinition(const FTerrainMapDefinition& MapDef)
{
    CurrentMapDefinition = MapDef;
    bHasMapDefinition = true;
    
    // Apply terrain scale from map definition
    TerrainScale = MapDef.TerrainScale;
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Map definition set - Scale=%.1f, Mode=%s"),
           TerrainScale, *MapDef.GetGenerationModeName());
}

void ADynamicTerrain::ApplyMapDefinitionGeneration()
{
    switch (CurrentMapDefinition.GenerationMode)
    {
        case ETerrainGenerationMode::Procedural:
        case ETerrainGenerationMode::ProceduralSeed:
            GenerateProceduralTerrainWithSettings(
                CurrentMapDefinition.ProceduralSeed,
                CurrentMapDefinition.HeightVariation,
                CurrentMapDefinition.NoiseScale,
                CurrentMapDefinition.NoiseOctaves
            );
            break;
            
        case ETerrainGenerationMode::DEM:
            UE_LOG(LogTemp, Warning, TEXT("DEM loading not yet implemented - using fallback procedural"));
            GenerateProceduralTerrain(); // Fallback
            break;
            
        case ETerrainGenerationMode::Preset:
            UE_LOG(LogTemp, Warning, TEXT("Preset loading not yet implemented - using fallback procedural"));
            GenerateProceduralTerrain(); // Fallback
            break;
    }
}

// 2.6 RUNTIME PARAMETER UPDATES

void ADynamicTerrain::SetHeightMultiplier(float NewMultiplier)
{
    NewMultiplier = FMath::Clamp(NewMultiplier, 0.1f, 10.0f);
    
    if (FMath::IsNearlyEqual(HeightMultiplier, NewMultiplier, 0.01f))
    {
        return; // No significant change
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Updating HeightMultiplier: %.2f   %.2f"),
           HeightMultiplier, NewMultiplier);
    
    HeightMultiplier = NewMultiplier;
    
    // Regenerate terrain with new multiplier
    if (bHasMapDefinition)
    {
        GenerateProceduralTerrainWithSettings(
            CurrentMapDefinition.ProceduralSeed,
            CurrentMapDefinition.HeightVariation,
            CurrentMapDefinition.NoiseScale,
            CurrentMapDefinition.NoiseOctaves
        );
        
        // Mark all chunks for update
        for (FTerrainChunk& Chunk : TerrainChunks)
        {
            Chunk.bNeedsUpdate = true;
        }
    }
}

// 2.7 DEM INTEGRATION

bool ADynamicTerrain::ApplyHeightData(const TArray<float>& HeightData,
                                      bool bNormalize, bool bUpdateChunks)
{
    UE_LOG(LogTemp, Warning, TEXT("=== APPLYING EXTERNAL HEIGHT DATA ==="));
    
    // ===== VALIDATION =====
    
    if (!ValidateHeightData(HeightData))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid height data - cannot apply!"));
        return false;
    }
    
    if (HeightData.Num() != TerrainWidth * TerrainHeight)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data size mismatch: %d vs expected %d"),
               HeightData.Num(), TerrainWidth * TerrainHeight);
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Height data validated: %d values"), HeightData.Num());
    
    // ===== FIX INVALID VALUES =====
    
    // Make mutable copy for fixing
    TArray<float> CleanedData = HeightData;
    int32 FixedCount = FixInvalidHeights(CleanedData);
    
    if (FixedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fixed %d invalid height values"), FixedCount);
    }
    
    // ===== CALCULATE HEIGHT RANGE =====
    
    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;
    
    for (float Height : CleanedData)
    {
        MinHeight = FMath::Min(MinHeight, Height);
        MaxHeight = FMath::Max(MaxHeight, Height);
    }
    
    float HeightRange = MaxHeight - MinHeight;
    
    UE_LOG(LogTemp, Log, TEXT("Source height range: %.1fm to %.1fm (range: %.1fm)"),
           MinHeight, MaxHeight, HeightRange);
    
    // ===== NORMALIZE IF REQUESTED =====
    
    if (bNormalize && HeightRange > 0.001f)
    {
        UE_LOG(LogTemp, Log, TEXT("Normalizing to terrain range: %.1fm to %.1fm"),
               MinTerrainHeight, MaxTerrainHeight);
        
        float TargetRange = MaxTerrainHeight - MinTerrainHeight;
        
        for (int32 i = 0; i < CleanedData.Num(); i++)
        {
            // Normalize: [MinHeight, MaxHeight] → [MinTerrainHeight, MaxTerrainHeight]
            float Normalized = (CleanedData[i] - MinHeight) / HeightRange;
            CleanedData[i] = MinTerrainHeight + (Normalized * TargetRange);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Height normalization complete"));
    }
    
    // ===== APPLY TO HEIGHTMAP =====
    
    if (HeightMap.Num() != CleanedData.Num())
    {
        HeightMap.SetNum(CleanedData.Num());
        UE_LOG(LogTemp, Warning, TEXT("Resized HeightMap array to %d"), HeightMap.Num());
    }
    
    // Direct copy to heightmap
    FMemory::Memcpy(HeightMap.GetData(), CleanedData.GetData(),
                    CleanedData.Num() * sizeof(float));
    
    UE_LOG(LogTemp, Log, TEXT("Height data copied to terrain heightmap"));
    
    // ===== SYNC GPU TEXTURE =====
    
    if (HeightRenderTexture)
    {
        // Update GPU texture with new heights
        FTextureRenderTargetResource* Resource = HeightRenderTexture->GetRenderTargetResource();
        if (Resource)
        {
            ENQUEUE_RENDER_COMMAND(UpdateHeightTexture)
            ([Resource, this](FRHICommandListImmediate& RHICmdList)
            {
                // Upload heightmap to GPU
                FUpdateTextureRegion2D UpdateRegion(0, 0, 0, 0, TerrainWidth, TerrainHeight);
                
                RHICmdList.UpdateTexture2D(
                    Resource->GetRenderTargetTexture(),
                    0, // Mip level
                    UpdateRegion,
                    TerrainWidth * sizeof(float),
                    (uint8*)HeightMap.GetData()
                );
            });
            
            UE_LOG(LogTemp, Log, TEXT("GPU height texture updated"));
        }
    }
    
    // ===== UPDATE CHUNK MESHES =====
    
    if (bUpdateChunks)
    {
        // Mark all chunks for update
        for (FTerrainChunk& Chunk : TerrainChunks)
        {
            Chunk.bNeedsUpdate = true;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Marked %d chunks for mesh update"), TerrainChunks.Num());
    }
    
    // ===== NOTIFY CONNECTED SYSTEMS =====
    
    if (CachedMasterController)
    {
        // Water system may need to redistribute based on new terrain
        UE_LOG(LogTemp, Log, TEXT("Notifying systems of terrain change..."));
        
        // NOTE: Add system notification hooks here if needed
        // Example: CachedMasterController->OnTerrainModified();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== HEIGHT DATA APPLICATION COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Final range: %.1fm to %.1fm"),
           MinHeight, MaxHeight);
    
    return true;
}

bool ADynamicTerrain::ValidateHeightData(const TArray<float>& HeightData) const
{
    if (HeightData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data is empty!"));
        return false;
    }
    
    if (HeightData.Num() != TerrainWidth * TerrainHeight)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data size mismatch: %d vs %d"),
               HeightData.Num(), TerrainWidth * TerrainHeight);
        return false;
    }
    
    // Count invalid values
    int32 NaNCount = 0;
    int32 InfCount = 0;
    int32 ExtremeCount = 0;
    
    for (float Height : HeightData)
    {
        if (FMath::IsNaN(Height))
            NaNCount++;
        else if (!FMath::IsFinite(Height))
            InfCount++;
        else if (FMath::Abs(Height) > 50000.0f) // Beyond Everest + Marianas
            ExtremeCount++;
    }
    
    // Allow up to 1% corrupted data (will be fixed)
    float CorruptionRate = (NaNCount + InfCount) / (float)HeightData.Num();
    
    if (CorruptionRate > 0.01f)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data too corrupted: %.2f%% (NaN=%d, Inf=%d)"),
               CorruptionRate * 100.0f, NaNCount, InfCount);
        return false;
    }
    
    if (ExtremeCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Height data has %d extreme values (>50km)"),
               ExtremeCount);
    }
    
    return true;
}

int32 ADynamicTerrain::FixInvalidHeights(TArray<float>& HeightData)
{
    if (HeightData.Num() != TerrainWidth * TerrainHeight)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot fix - size mismatch"));
        return 0;
    }
    
    int32 FixedCount = 0;
    
    // First pass: Replace NaN/Inf with zero
    for (float& Height : HeightData)
    {
        if (!FMath::IsFinite(Height))
        {
            Height = 0.0f;
            FixedCount++;
        }
    }
    
    // Second pass: Smooth extreme outliers using neighbor average
    for (int32 Y = 1; Y < TerrainHeight - 1; Y++)
    {
        for (int32 X = 1; X < TerrainWidth - 1; X++)
        {
            int32 Index = Y * TerrainWidth + X;
            float& Height = HeightData[Index];
            
            // Check if this is an extreme outlier
            if (FMath::Abs(Height) > 50000.0f)
            {
                // Average of 8 neighbors
                float Sum = 0.0f;
                int32 Count = 0;
                
                for (int32 DY = -1; DY <= 1; DY++)
                {
                    for (int32 DX = -1; DX <= 1; DX++)
                    {
                        if (DX == 0 && DY == 0) continue;
                        
                        int32 NX = X + DX;
                        int32 NY = Y + DY;
                        int32 NIndex = NY * TerrainWidth + NX;
                        
                        float NHeight = HeightData[NIndex];
                        if (FMath::IsFinite(NHeight) && FMath::Abs(NHeight) < 50000.0f)
                        {
                            Sum += NHeight;
                            Count++;
                        }
                    }
                }
                
                if (Count > 0)
                {
                    Height = Sum / Count;
                    FixedCount++;
                }
            }
        }
    }
    
    return FixedCount;
}


// ============================================================================
// SECTION 3: TERRAIN MODIFICATION (~200 lines, 5%)
// ============================================================================
/**
 * Real-time terrain height modification with circular brush patterns.
 */

// 3.1 WORLD POSITION MODIFICATION

void ADynamicTerrain::ModifyTerrain(FVector WorldPosition, float Radius, float Strength, bool bRaise)
{
    static int32 ModifyTerrainCallCount = 0;
    ModifyTerrainCallCount++;
    if (ModifyTerrainCallCount <= 5)
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> ModifyTerrain CALLED #%d: Pos=%s Radius=%.1f Strength=%.1f Raise=%d"),
               ModifyTerrainCallCount, *WorldPosition.ToString(), Radius, Strength, bRaise ? 1 : 0);
    }

    /*if (CurrentComputeMode == ETerrainComputeMode::GPU) {
        UpdateGPUBrush(WorldPosition, Radius, Strength, bRaise);
        return; // Skip CPU path
    }
     */
    // PERFORMANCE: Throttle to 20 modifications/second (50ms cooldown)
    float CurrentTime = GetCachedFrameTime();
    if (CurrentTime - LastModificationTime < ModificationCooldown)
    {
        return;  // Throttled - too frequent
    }
    LastModificationTime = CurrentTime;
    
    // PHASE 2: Direct MasterController authority - no fallbacks
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    // Use MasterController scaling authority
    float ScaledRadius = CachedMasterController->GetBrushScaleMultiplier() * (Radius / TerrainScale);
    
    // Sample height BEFORE modification
    int32 CenterIndex = Y * TerrainWidth + X;
    float HeightBefore = (CenterIndex >= 0 && CenterIndex < HeightMap.Num()) ? HeightMap[CenterIndex] : 0.0f;

    ModifyTerrainAtIndex(X, Y, ScaledRadius, Strength, bRaise);

    // Sample height AFTER modification
    float HeightAfter = (CenterIndex >= 0 && CenterIndex < HeightMap.Num()) ? HeightMap[CenterIndex] : 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("BRUSH EDIT: Center[%d,%d] Before=%.2f After=%.2f Delta=%.2f"),
           X, Y, HeightBefore, HeightAfter, HeightAfter - HeightBefore);

    if (bUseGPUTerrain)
    {
        // Upload our edit to GPU and WAIT for it to complete
        SyncCPUToGPU();
        FlushRenderingCommands();

        // Mark the time of this edit - used to protect against sync overwrites
        LastUserEditTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

        UE_LOG(LogTemp, Warning, TEXT("BRUSH EDIT: Uploaded to GPU, edit protected until %.2f"), LastUserEditTime + 0.5f);
    }
}

// 3.2 INDEX-BASED MODIFICATION

void ADynamicTerrain::ModifyTerrainAtIndex(int32 X, int32 Y, float Radius, float Strength, bool bRaise)
{
    if (!HeightMap.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("HeightMap not initialized!"));
        return;
    }
    
    X = FMath::Clamp(X, 0, TerrainWidth - 1);
    Y = FMath::Clamp(Y, 0, TerrainHeight - 1);
    
    int32 IntRadius = FMath::CeilToInt(Radius);
    TSet<int32> AffectedChunks;
    
    // Modify terrain in circular pattern
    for (int32 OffsetY = -IntRadius; OffsetY <= IntRadius; OffsetY++)
    {
        for (int32 OffsetX = -IntRadius; OffsetX <= IntRadius; OffsetX++)
        {
            int32 CurrentX = X + OffsetX;
            int32 CurrentY = Y + OffsetY;
            
            if (CurrentX >= 0 && CurrentX < TerrainWidth &&
                CurrentY >= 0 && CurrentY < TerrainHeight)
            {
                float Distance = FMath::Sqrt((float)(OffsetX * OffsetX + OffsetY * OffsetY));
                if (Distance <= Radius)
                {
                    int32 Index = CurrentY * TerrainWidth + CurrentX;
                    if (Index >= 0 && Index < HeightMap.Num())
                    {
                        float Falloff = FMath::Pow(1.0f - (Distance / Radius), 2.0f);
                        float HeightChange = Strength * Falloff * (bRaise ? 1.0f : -1.0f);
                        
                        HeightMap[Index] = FMath::Clamp(
                            HeightMap[Index] + HeightChange,
                            MinTerrainHeight, MaxTerrainHeight
                        );
                        
                        int32 ChunkIndex = GetChunkIndexFromCoordinates(CurrentX, CurrentY);
                        if (ChunkIndex >= 0)
                        {
                            AffectedChunks.Add(ChunkIndex);
                        }
                    }
                }
            }
        }
    }
    
    // ===== CRITICAL: Include neighbors for boundary stitching =====
    TSet<int32> ChunksToUpdate = AffectedChunks;
    
    for (int32 ChunkIndex : AffectedChunks)
    {
        TArray<int32> Neighbors = GetNeighboringChunks(ChunkIndex, false);
        for (int32 NeighborIndex : Neighbors)
        {
            ChunksToUpdate.Add(NeighborIndex);
        }
    }
    
    TArray<int32> ChunkArray = ChunksToUpdate.Array();
    
    // ===== KEY FIX: Atomic vs Queued based on operation size =====
    if (ChunksToUpdate.Num() <= 25)
    {
        // SMALL TO MEDIUM: Update ALL atomically in single frame
        // Prevents any tears by ensuring all boundaries update together
        UE_LOG(LogTemp, Verbose, TEXT("CPU: Atomic update of %d chunks (incl. neighbors)"),
               ChunksToUpdate.Num());
        
        UpdateChunkGroupAtomic(ChunkArray);
    }
    else if (ChunksToUpdate.Num() <= 60)
    {
        // LARGE: Hybrid approach - immediate core + fast queuing
        UE_LOG(LogTemp, Warning, TEXT("CPU: Large brush - %d chunks, hybrid update"),
               ChunksToUpdate.Num());
        
        // Update core 20 chunks atomically
        int32 CoreSize = 20;
        TArray<int32> CoreChunks;
        TArray<int32> RemainingChunks;
        
        for (int32 i = 0; i < ChunkArray.Num(); i++)
        {
            if (i < CoreSize)
                CoreChunks.Add(ChunkArray[i]);
            else
                RemainingChunks.Add(ChunkArray[i]);
        }
        
        UpdateChunkGroupAtomic(CoreChunks);
        
        // Queue remaining with VERY high priority (process next frame)
        for (int32 ChunkIndex : RemainingChunks)
        {
            FChunkUpdateRequest Request;
            Request.ChunkIndex = ChunkIndex;
            Request.Priority = 95.0f; // Very high
            Request.RequestTime = GetCachedFrameTime();
            PriorityChunkQueue.Add(Request);
        }
    }
    else
    {
        // MASSIVE: Update first 30 atomically, queue rest
        UE_LOG(LogTemp, Warning, TEXT("CPU: Massive brush - %d chunks, staged update"),
               ChunksToUpdate.Num());
        
        int32 ImmediateSize = 30;
        
        for (int32 i = 0; i < FMath::Min(ImmediateSize, ChunkArray.Num()); i++)
        {
            UpdateChunk(ChunkArray[i]);
            PendingChunkUpdates.Remove(ChunkArray[i]);
        }
        
        // Queue remaining
        for (int32 i = ImmediateSize; i < ChunkArray.Num(); i++)
        {
            FChunkUpdateRequest Request;
            Request.ChunkIndex = ChunkArray[i];
            Request.Priority = 90.0f - (i * 0.02f);
            Request.RequestTime = GetCachedFrameTime();
            PriorityChunkQueue.Add(Request);
        }
    }
    
    if (bUseGPUTerrain)
    {
        SyncCPUToGPU();
    }
}


// ============================================================================
// SECTION 4: CHUNK MANAGEMENT SYSTEM (~1100 lines, 24%)
// ============================================================================
/**
 * Chunk initialization, mesh generation, update pipeline, and water integration.
 */

// 4.1 CHUNK MESH GENERATION

void ADynamicTerrain::GenerateChunkMesh(int32 ChunkX, int32 ChunkY)
{
    int32 ChunkIndex = ChunkY * ChunksX + ChunkX;
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid chunk index: %d"), ChunkIndex);
        return;
    }
    
    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    if (!Chunk.MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Chunk mesh component is null for chunk %d,%d"), ChunkX, ChunkY);
        return;
    }
    
    // CRITICAL FIX: Use consistent ChunkSize calculation with enhanced overlap
    int32 StartX = ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, TerrainHeight);
    
    // BOUNDARY VALIDATION: Ensure chunks actually overlap with neighbors
    if (ChunkX > 0)
    {
        int32 PrevChunkEndX = (ChunkX - 1) * (ChunkSize - ChunkOverlap) + ChunkSize;
        if (StartX >= PrevChunkEndX)
        {
            UE_LOG(LogTemp, Warning, TEXT("CHUNK BOUNDARY: Forcing X overlap between chunks %d and %d"), ChunkX-1, ChunkX);
            StartX = PrevChunkEndX - ChunkOverlap;
        }
    }
    
    if (ChunkY > 0)
    {
        int32 PrevChunkEndY = (ChunkY - 1) * (ChunkSize - ChunkOverlap) + ChunkSize;
        if (StartY >= PrevChunkEndY)
        {
            UE_LOG(LogTemp, Warning, TEXT("CHUNK BOUNDARY: Forcing Y overlap between chunks %d and %d"), ChunkY-1, ChunkY);
            StartY = PrevChunkEndY - ChunkOverlap;
        }
    }
    
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> VertexColors;
    
    // Generate vertices for this chunk
    for (int32 Y = StartY; Y < EndY; Y++)
    {
        for (int32 X = StartX; X < EndX; X++)
        {
            float Height;

            // GPU Rendering Mode: Generate FLAT mesh, material WPO handles height
            // This eliminates expensive mesh regeneration when erosion changes heights
            if (bUseGPUHeightmapRendering && bGPUInitialized)
            {
                Height = 0.0f;  // Flat mesh - WPO will displace
            }
            else
            {
                Height = GetHeightSafe(X, Y);  // Traditional CPU-baked heights
            }

            // Local position within chunk (relative to chunk origin)
            FVector LocalPosition = FVector(
                (X - StartX) * TerrainScale,
                (Y - StartY) * TerrainScale,
                Height
            );

            Vertices.Add(LocalPosition);

            // Calculate normal (for GPU mode, material will recalculate from heightmap)
            FVector Normal = bUseGPUHeightmapRendering ? FVector::UpVector : CalculateVertexNormal(X, Y);
            Normals.Add(Normal);

            // UV0: Local UVs for chunk texturing
            float U = (float)(X - StartX) / (EndX - StartX - 1);
            float V = (float)(Y - StartY) / (EndY - StartY - 1);
            UVs.Add(FVector2D(U, V));

            // Height-based vertex colors (for GPU mode, use 0 as placeholder)
            float NormalizedHeight = bUseGPUHeightmapRendering ? 0.0f : Height / MaxTerrainHeight;
            FLinearColor VertexColor = GetHeightBasedColor(NormalizedHeight);
            VertexColors.Add(VertexColor);
        }
    }
    
    // Generate triangles for this chunk
    int32 ChunkWidth = EndX - StartX;
    int32 ChunkHeight = EndY - StartY;
    
    for (int32 Y = 0; Y < ChunkHeight - 1; Y++)
    {
        for (int32 X = 0; X < ChunkWidth - 1; X++)
        {
            int32 BottomLeft = Y * ChunkWidth + X;
            int32 BottomRight = Y * ChunkWidth + (X + 1);
            int32 TopLeft = (Y + 1) * ChunkWidth + X;
            int32 TopRight = (Y + 1) * ChunkWidth + (X + 1);

            // First triangle
            Triangles.Add(BottomLeft);
            Triangles.Add(TopLeft);
            Triangles.Add(BottomRight);

            // Second triangle
            Triangles.Add(BottomRight);
            Triangles.Add(TopLeft);
            Triangles.Add(TopRight);
        }
    }
    
    // Create the mesh section
    Chunk.MeshComponent->CreateMeshSection_LinearColor(
        0, Vertices, Triangles, Normals, UVs, VertexColors,
        TArray<FProcMeshTangent>(), true
    );

    // CRITICAL: Expand bounds for GPU heightmap rendering mode
    // When using WPO, mesh vertices are at Z=0 but displaced by the shader
    // Without expanded bounds, frustum culling incorrectly clips chunks
    if (bUseGPUHeightmapRendering && bGPUInitialized)
    {
        // Use a large bounds scale to ensure WPO-displaced geometry is never culled
        // The flat mesh will be expanded to include the full terrain height range
        Chunk.MeshComponent->SetBoundsScale(100.0f);
    }

    // ===== CRITICAL FIX: MATERIAL APPLICATION ORDER =====
    
    // CRITICAL FIX: Ensure material is ready BEFORE mesh generation
    UMaterialInterface* MaterialToUse = nullptr;
    
    if (CurrentActiveMaterial)
    {
        MaterialToUse = CurrentActiveMaterial;
    }
    else if (GEngine && GEngine->WireframeMaterial)
    {
        // Use wireframe instead of default material to prevent teal flash
        MaterialToUse = GEngine->WireframeMaterial;
        UE_LOG(LogTemp, Warning, TEXT("Using fallback wireframe material for chunk %d,%d"), ChunkX, ChunkY);
    }
    
    // Step 1: Apply base material immediately (prevents teal flash)
    if (MaterialToUse)
    {
        Chunk.MeshComponent->SetMaterial(0, MaterialToUse);
    }
    
    // Step 2: Create dynamic material ONLY if water shader is ready
    if (CurrentActiveMaterial && WaterSystem && WaterSystem->IsSystemReady() && WaterSystem->bUseShaderWater)
    {
        UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
        if (DynMaterial)
        {
            // Apply chunk-specific parameters
            ApplyMaterialParameters(DynMaterial, ChunkX, ChunkY);
            
            // Apply water shader parameters (only when ready)
            WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
            
            // Replace base material with enhanced version
            Chunk.MeshComponent->SetMaterial(0, DynMaterial);
            
           /* // Throttled logging to prevent spam
            static float LastWaterShaderLogTime = 0.0f;
            float CurrentTime = GetCachedFrameTime();
            if (CurrentTime - LastWaterShaderLogTime >= 2.0f)
            {
                UE_LOG(LogTemp, VeryVerbose, TEXT("Applied water shader to chunk materials"));
                LastWaterShaderLogTime = CurrentTime;
            }*/
        }
    }
    
    // Update chunk status
    Chunk.bNeedsUpdate = false;
    Chunk.LastUpdateTime = GetCachedFrameTime();
}


// 4.2 CHUNK UPDATE PIPELINE

void ADynamicTerrain::UpdateChunk(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }

    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    GenerateChunkMesh(Chunk.ChunkX, Chunk.ChunkY);

    // CRITICAL: Update material parameters for GPU rendering mode
    // Without this, the flat mesh won't have heightmap texture binding or UV transforms
    if (bUseGPUHeightmapRendering && bGPUInitialized && HeightRenderTexture)
    {
        UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
            Chunk.MeshComponent->GetMaterial(0)
        );
        if (DynMaterial)
        {
            UpdateGPUChunkMaterialParams(DynMaterial, Chunk);
        }
    }

    TotalChunkUpdatesThisFrame++;
}


void ADynamicTerrain::MarkChunkForUpdate(int32 ChunkIndex)
{
    if (TerrainChunks.IsValidIndex(ChunkIndex))
    {
        PendingChunkUpdates.Add(ChunkIndex);
        TerrainChunks[ChunkIndex].bNeedsUpdate = true;
    }
}


void ADynamicTerrain::RequestPriorityChunkUpdate(int32 ChunkIndex, float Priority)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex)) return;
    
    // CRITICAL FIX: Get neighboring chunks to prevent tears
    TArray<int32> ChunkGroup = GetNeighboringChunks(ChunkIndex, false); // Only direct neighbors
    ChunkGroup.Insert(ChunkIndex, 0); // Main chunk gets highest priority
    
    // Remove all chunks from old pending system
    for (int32 GroupChunkIndex : ChunkGroup)
    {
        PendingChunkUpdates.Remove(GroupChunkIndex);
    }
    
    // Add entire group to priority queue with slight priority offset
    for (int32 i = 0; i < ChunkGroup.Num(); i++)
    {
        FChunkUpdateRequest Request;
        Request.ChunkIndex = ChunkGroup[i];
        Request.Priority = Priority - (i * 0.1f); // Main chunk gets highest priority
        Request.RequestTime = GetCachedFrameTime();
        
        PriorityChunkQueue.Add(Request);
        if (TerrainChunks.IsValidIndex(ChunkGroup[i]))
        {
            TerrainChunks[ChunkGroup[i]].bNeedsUpdate = true;
        }
    }
}


float ADynamicTerrain::CalculateChunkDistance(int32 ChunkIndex, FVector FromPosition) const
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex)) return 99999.0f;
    
    FVector ChunkWorldPos = GetChunkWorldPosition(ChunkIndex);
    return FVector::Dist(FromPosition, ChunkWorldPos);
}


// 4.3 WATER-SPECIFIC CHUNK UPDATES

void ADynamicTerrain::MarkChunkForWaterUpdate(int32 ChunkIndex)
{
    if (TerrainChunks.IsValidIndex(ChunkIndex))
    {
        PendingWaterChunkUpdates.Add(ChunkIndex);
    }
}


void ADynamicTerrain::BatchUpdateWaterChunks(const TArray<int32>& ChunkIndices)
{
    if (!bEnablePrecipitationOptimization)
    {
        // Fallback to regular chunk updates
        for (int32 ChunkIndex : ChunkIndices)
        {
            MarkChunkForUpdate(ChunkIndex);
        }
        return;
    }
    
    // Special water-only update path for precipitation
    int32 ProcessedThisFrame = 0;
    
    for (int32 ChunkIndex : ChunkIndices)
    {
        if (ProcessedThisFrame >= MaxWaterUpdatesPerFrame)
        {
            // Queue remaining for next frame
            PendingWaterChunkUpdates.Add(ChunkIndex);
            continue;
        }
        
        // Water-only update (faster than full chunk regeneration)
        UpdateChunkWaterOnly(ChunkIndex);
        ProcessedThisFrame++;
    }
}


void ADynamicTerrain::UpdateChunkWaterOnly(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex)) return;
    
    // Only update water shader parameters, skip mesh regeneration
    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    
    if (Chunk.MeshComponent && WaterSystem && WaterSystem->bUseShaderWater)
    {
        // Update water shader with new precipitation data
        UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Chunk.MeshComponent->GetMaterial(0));
        if (DynMaterial)
        {
            WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
        }
    }
}


void ADynamicTerrain::ProcessPendingWaterChunkUpdates()
{
    if (PendingWaterChunkUpdates.Num() == 0) return;
    
    // Process water updates with higher throughput
    int32 MaxWaterUpdatesThisFrame = MaxWaterUpdatesPerFrame;
    int32 ProcessedThisFrame = 0;
    
    // Copy indices to array
    TArray<int32> WaterChunksToUpdate;
    for (int32 ChunkIndex : PendingWaterChunkUpdates)
    {
        WaterChunksToUpdate.Add(ChunkIndex);
        if (WaterChunksToUpdate.Num() >= MaxWaterUpdatesThisFrame)
        {
            break;
        }
    }
    
    // Clear processed chunks from pending set
    for (int32 ChunkIndex : WaterChunksToUpdate)
    {
        PendingWaterChunkUpdates.Remove(ChunkIndex);
    }
    
    // Update water-only
    for (int32 ChunkIndex : WaterChunksToUpdate)
    {
        UpdateChunkWaterOnly(ChunkIndex);
        ProcessedThisFrame++;
    }
    
    // Performance monitoring
    if (PendingWaterChunkUpdates.Num() > 30)
    {
        UE_LOG(LogTemp, Warning, TEXT("Drift: Water update queue pressure, pending: %d"), PendingWaterChunkUpdates.Num());
    }
}

/**
 * Priority-based chunk update processing with adaptive throttling
 *
 * Algorithm:
 * 1. Process high-priority chunks first (editing areas, near camera)
 * 2. Adaptive throttling based on queue pressure (2-12 updates/frame)
 * 3. Separate water-only update pipeline for precipitation
 *
 * Performance:
 * - Normal: 2 chunks/frame
 * - High pressure: 6-8 chunks/frame
 * - Emergency: 12 chunks/frame
 *
 * Dependencies:
 * - Called from Tick() after frustum culling
 * - Requires PriorityChunkQueue and PendingChunkUpdates to be populated
 */


// 4.4 CHUNK UPDATE PROCESSING

void ADynamicTerrain::ProcessPendingChunkUpdates()
{
    int32 TotalUpdatesNeeded = PriorityChunkQueue.Num() + PendingChunkUpdates.Num();
    
    int32 UpdatesThisFrame;
    if (TotalUpdatesNeeded > 50)
        UpdatesThisFrame = EMERGENCY_CHUNK_UPDATES;
    else if (TotalUpdatesNeeded > 20)
        UpdatesThisFrame = 8;
    else if (TotalUpdatesNeeded > 10)
        UpdatesThisFrame = 6;
    else
        UpdatesThisFrame = MaxUpdatesPerFrame;
    
    int32 ProcessedThisFrame = 0;
    
    if (PriorityChunkQueue.Num() > 0)
    {
        PriorityChunkQueue.Sort();
        
        // ===== NEW: Group chunks by proximity =====
        TSet<int32> ProcessedChunks;
        
        while (ProcessedThisFrame < UpdatesThisFrame && PriorityChunkQueue.Num() > 0)
        {
            int32 ChunkIndex = PriorityChunkQueue[0].ChunkIndex;
            
            if (ProcessedChunks.Contains(ChunkIndex))
            {
                PriorityChunkQueue.RemoveAt(0);
                continue;
            }
            
            // Get this chunk + its neighbors
            TArray<int32> ChunkGroup;
            ChunkGroup.Add(ChunkIndex);
            
            // Check if neighbors are in queue with similar priority
            float BasePriority = PriorityChunkQueue[0].Priority;
            TArray<int32> Neighbors = GetNeighboringChunks(ChunkIndex, false);
            
            for (int32 NeighborIndex : Neighbors)
            {
                // Find neighbor in priority queue
                for (int32 i = 1; i < FMath::Min(8, PriorityChunkQueue.Num()); i++)
                {
                    if (PriorityChunkQueue[i].ChunkIndex == NeighborIndex)
                    {
                        float PriorityDiff = FMath::Abs(BasePriority - PriorityChunkQueue[i].Priority);
                        
                        // If neighbor has similar priority (within 1.0), group it
                        if (PriorityDiff < 1.0f)
                        {
                            ChunkGroup.Add(NeighborIndex);
                        }
                        break;
                    }
                }
            }
            
            // Update entire group atomically
            if (ChunkGroup.Num() <= 5 && (ProcessedThisFrame + ChunkGroup.Num()) <= UpdatesThisFrame)
            {
                // Can fit entire group this frame
                UpdateChunkGroupAtomic(ChunkGroup);
                ProcessedThisFrame += ChunkGroup.Num();
                
                // Mark as processed
                for (int32 Idx : ChunkGroup)
                {
                    ProcessedChunks.Add(Idx);
                }
                
                // Remove from queue
                PriorityChunkQueue.RemoveAll([&ProcessedChunks](const FChunkUpdateRequest& Req) {
                    return ProcessedChunks.Contains(Req.ChunkIndex);
                });
            }
            else
            {
                // Just update the main chunk
                UpdateChunk(ChunkIndex);
                ProcessedThisFrame++;
                ProcessedChunks.Add(ChunkIndex);
                PriorityChunkQueue.RemoveAt(0);
            }
        }
    }
    
    // Standard pending updates
    int32 RemainingCapacity = UpdatesThisFrame - ProcessedThisFrame;
    if (RemainingCapacity > 0 && PendingChunkUpdates.Num() > 0)
    {
        TArray<int32> ChunksToUpdate;
        for (int32 ChunkIndex : PendingChunkUpdates)
        {
            ChunksToUpdate.Add(ChunkIndex);
            if (ChunksToUpdate.Num() >= RemainingCapacity)
                break;
        }
        
        for (int32 ChunkIndex : ChunksToUpdate)
        {
            PendingChunkUpdates.Remove(ChunkIndex);
            UpdateChunk(ChunkIndex);
            ProcessedThisFrame++;
        }
    }
    
    ProcessPendingWaterChunkUpdates();
}

// ===== UTILITY FUNCTIONS =====


// 4.5 CHUNK POSITION UTILITIES

FVector2D ADynamicTerrain::GetChunkWorldPosition(int32 ChunkX, int32 ChunkY) const
{
    // AUTHORITY FIX: Use CachedMasterController
    float AuthScale = CachedMasterController ? CachedMasterController->GetTerrainScale() : TerrainScale;
    float WorldX = ChunkX * (WorldConfig.ChunkSize - ChunkOverlap) * AuthScale;
    float WorldY = ChunkY * (WorldConfig.ChunkSize - ChunkOverlap) * AuthScale;
    return FVector2D(WorldX, WorldY);
}

/**
 * Gets world position for volumetric water chunk by chunk index
 * Converts chunk index to grid coordinates then to world position
 *
 * @param ChunkIndex - Linear index into TerrainChunks array
 * @return World position vector of chunk center
 */

FVector ADynamicTerrain::GetChunkWorldPosition(int32 ChunkIndex) const
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return FVector::ZeroVector;
    }
    
    const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    FVector2D ChunkWorldPos2D = GetChunkWorldPosition(Chunk.ChunkX, Chunk.ChunkY);
    
    // AUTHORITY FIX: Use CachedMasterController for terrain scale
    float AuthScale = CachedMasterController ? CachedMasterController->GetTerrainScale() : TerrainScale;
    float ChunkCenterX = ChunkWorldPos2D.X + ((ChunkSize - 1) * AuthScale * 0.5f);
    float ChunkCenterY = ChunkWorldPos2D.Y + ((ChunkSize - 1) * AuthScale * 0.5f);
    
    // Use average terrain height for Z position
    float AverageHeight = 0.0f;
    int32 SampleCount = 0;
    
    int32 StartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, TerrainHeight);
    
    for (int32 Y = StartY; Y < EndY; Y += 4) // Sample every 4th point for performance
    {
        for (int32 X = StartX; X < EndX; X += 4)
        {
            AverageHeight += GetHeightSafe(X, Y);
            SampleCount++;
        }
    }
    
    if (SampleCount > 0)
    {
        AverageHeight /= SampleCount;
    }
    
    return GetActorTransform().TransformPosition(FVector(ChunkCenterX, ChunkCenterY, AverageHeight));
}


int32 ADynamicTerrain::GetChunkIndexFromCoordinates(int32 X, int32 Y) const
{
    if (X < 0 || X >= TerrainWidth || Y < 0 || Y >= TerrainHeight)
    {
        return -1;
    }
    
    // CRITICAL FIX: Use consistent ChunkSize calculation
    int32 ChunkX = X / (ChunkSize - ChunkOverlap);
    int32 ChunkY = Y / (ChunkSize - ChunkOverlap);
    
    // Clamp to valid chunk range
    ChunkX = FMath::Clamp(ChunkX, 0, ChunksX - 1);
    ChunkY = FMath::Clamp(ChunkY, 0, ChunksY - 1);
    
    return ChunkY * ChunksX + ChunkX;
}



// ============================================================================
// SECTION 5: HEIGHT QUERIES & UTILITIES (~350 lines, 8%)
// ============================================================================
/**
 * Coordinate transformations, height queries, normal calculation, and validation.
 */

// 5.1 COORDINATE TRANSFORMS

FVector ADynamicTerrain::TerrainToWorldPosition(int32 X, int32 Y) const
{
    if (!CachedMasterController) return FVector::ZeroVector;
    return CachedMasterController->TerrainToWorldPosition(FVector2D(X, Y));
}

// 5.2 HEIGHT QUERIES

float ADynamicTerrain::GetHeightAtPosition(FVector WorldPosition) const
{
    // PHASE 2: Simplified authority delegation
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(TerrainCoords.X);
    int32 Y = FMath::FloorToInt(TerrainCoords.Y);
    
    if (X < 0 || X >= TerrainWidth - 1 || Y < 0 || Y >= TerrainHeight - 1)
    {
        return 0.0f;
    }
    
    // Bilinear interpolation
    float FracX = TerrainCoords.X - X;
    float FracY = TerrainCoords.Y - Y;
    
    float Height00 = GetHeightSafe(X, Y);
    float Height10 = GetHeightSafe(X + 1, Y);
    float Height01 = GetHeightSafe(X, Y + 1);
    float Height11 = GetHeightSafe(X + 1, Y + 1);
    
    float HeightX0 = FMath::Lerp(Height00, Height10, FracX);
    float HeightX1 = FMath::Lerp(Height01, Height11, FracX);
    
    return FMath::Lerp(HeightX0, HeightX1, FracY);
}

float ADynamicTerrain::GetHeightAtIndex(int32 X, int32 Y) const
{
    return GetHeightSafe(X, Y);
}

float ADynamicTerrain::GetHeightSafe(int32 X, int32 Y) const
{
    // Critical bounds checking to prevent crash
    if (HeightMap.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("HeightMap not initialized yet - returning 0"));
        return 0.0f;
    }
    
    if (X >= 0 && X < TerrainWidth && Y >= 0 && Y < TerrainHeight)
    {
        int32 Index = Y * TerrainWidth + X;
        if (Index >= 0 && Index < HeightMap.Num())
        {
            return HeightMap[Index];
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HeightMap index %d out of bounds (array size: %d)"), Index, HeightMap.Num());
        }
    }
    
    // NEW: Instead of returning 0, sample valid neighbors to prevent drainage holes
    float NeighborSum = 0.0f;
    int32 ValidNeighbors = 0;
    
    for (int32 dy = -1; dy <= 1; dy++)
    {
        for (int32 dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0) continue; // Skip center
            
            int32 NX = X + dx;
            int32 NY = Y + dy;
            
            // Check if neighbor is valid
            if (NX >= 0 && NX < TerrainWidth && NY >= 0 && NY < TerrainHeight)
            {
                int32 NIndex = NY * TerrainWidth + NX;
                if (NIndex >= 0 && NIndex < HeightMap.Num())
                {
                    NeighborSum += HeightMap[NIndex];
                    ValidNeighbors++;
                }
            }
        }
    }
    
    if (ValidNeighbors > 0)
    {
        return NeighborSum / ValidNeighbors; // Return average of valid neighbors
    }
    
    // Only return 0 if absolutely no valid neighbors exist
    return 0.0f;
}

void ADynamicTerrain::SetHeightSafe(int32 X, int32 Y, float Height)
{
    if (X >= 0 && X < TerrainWidth && Y >= 0 && Y < TerrainHeight && HeightMap.Num() > 0)
    {
        int32 Index = Y * TerrainWidth + X;
        if (Index >= 0 && Index < HeightMap.Num())
        {
            HeightMap[Index] = Height;
        }
    }
}

// 5.3 NORMAL CALCULATION & COLORS

FVector ADynamicTerrain::CalculateVertexNormal(int32 X, int32 Y) const
{
    // Sample neighboring heights for normal calculation
    float HeightL = GetHeightSafe(X - 1, Y);     // Left
    float HeightR = GetHeightSafe(X + 1, Y);     // Right
    float HeightD = GetHeightSafe(X, Y - 1);     // Down
    float HeightU = GetHeightSafe(X, Y + 1);     // Up
    
    // Calculate gradient vectors
    FVector TangentX = FVector(2.0f * TerrainScale, 0.0f, HeightR - HeightL);
    FVector TangentY = FVector(0.0f, 2.0f * TerrainScale, HeightU - HeightD);
    
    // Cross product to get normal
    FVector Normal = FVector::CrossProduct(TangentX, TangentY);
    Normal.Normalize();
    
    return Normal;
}

FLinearColor ADynamicTerrain::GetHeightBasedColor(float NormalizedHeight) const
{
    // Height-based color blending for terrain materials
    if (NormalizedHeight < 0.2f)
    {
        return FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Sand/Beach
    }
    else if (NormalizedHeight < 0.5f)
    {
        return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // Grass
    }
    else if (NormalizedHeight < 0.8f)
    {
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Rock
    }
    else
    {
        return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // Snow
    }
}

// 5.4 HEIGHT VALIDATION

float ADynamicTerrain::GetSafeNeighborAverage(int32 X, int32 Y) const
{
    float Sum = 0.0f;
    int32 Count = 0;
    
    // Sample 3x3 neighborhood in CURRENT heightmap
    for (int32 dy = -1; dy <= 1; dy++)
    {
        for (int32 dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0) continue;
            
            int32 NX = X + dx;
            int32 NY = Y + dy;
            
            if (NX >= 0 && NX < TerrainWidth && NY >= 0 && NY < TerrainHeight)
            {
                int32 Index = NY * TerrainWidth + NX;
                if (Index >= 0 && Index < HeightMap.Num())
                {
                    float Height = HeightMap[Index];
                    if (FMath::IsFinite(Height))
                    {
                        Sum += Height;
                        Count++;
                    }
                }
            }
        }
    }
    
    return (Count > 0) ? (Sum / Count) : 0.0f;
}


FVector2D ADynamicTerrain::GetHeightRange() const
{
    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;
    
    for (float Height : HeightMap)
    {
        MinHeight = FMath::Min(MinHeight, Height);
        MaxHeight = FMath::Max(MaxHeight, Height);
    }
    
    return FVector2D(MinHeight, MaxHeight);
}

// ===== WORLD SIZE MANAGEMENT IMPLEMENTATION =====



// ============================================================================
// SECTION 6: MATERIAL & VISUAL SYSTEMS (~200 lines, 4%)
// ============================================================================
/**
 * Material assignment, dynamic material instances, and material refresh.
 */

// 6.1 MATERIAL ASSIGNMENT

void ADynamicTerrain::SetActiveMaterial(UMaterialInterface* Material)
{
    if (!Material)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Attempted to set NULL material"));
        return;
    }
    
    CurrentActiveMaterial = Material;
    
    int32 MaterialsApplied = 0;
    int32 WaterShadersApplied = 0;
    
    // Apply to all existing chunks
    for (int32 i = 0; i < TerrainChunks.Num(); i++)
    {
        FTerrainChunk& Chunk = TerrainChunks[i];
        if (Chunk.MeshComponent)
        {
            // Step 1: Apply base material immediately (prevents flash)
            Chunk.MeshComponent->SetMaterial(0, Material);
            MaterialsApplied++;
            
            // Step 2: Enhance with water shader only if system is ready
            if (WaterSystem && WaterSystem->IsSystemReady() && WaterSystem->bUseShaderWater)
            {
                UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
                if (DynMaterial)
                {
                    ApplyMaterialParameters(DynMaterial, Chunk.ChunkX, Chunk.ChunkY);
                    WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
                    
                    // Replace with enhanced version
                    Chunk.MeshComponent->SetMaterial(0, DynMaterial);
                    WaterShadersApplied++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT(" Material applied to %d chunks, %d enhanced with water shaders"),
           MaterialsApplied, WaterShadersApplied);
}

void ADynamicTerrain::SetWaterVolumeMaterial(UMaterialInterface* Material)
{
    if (WaterSystem)
    {
        WaterSystem->VolumeMaterial = Material;
        UE_LOG(LogTemp, Warning, TEXT("Water Volume Material set: %s"),
               Material ? *Material->GetName() : TEXT("NULL"));
    }
}


// 6.2 MATERIAL PARAMETERS

void ADynamicTerrain::ApplyMaterialParameters(UMaterialInstanceDynamic* Material, int32 ChunkX, int32 ChunkY)
{
    if (!Material) return;
    
    // PHASE 2: Direct parameter application - authority established
    FLinearColor ChunkGridPosition(ChunkX, ChunkY, 0, 0);
    Material->SetVectorParameterValue(FName("ChunkGridPosition"), ChunkGridPosition);
    
    // AUTHORITY FIX: Use CachedMasterController for terrain scale
    float AuthScale = CachedMasterController ? CachedMasterController->GetTerrainScale() : TerrainScale;
    FLinearColor TerrainInfo(TerrainWidth, TerrainHeight, ChunkSize, AuthScale);
    Material->SetVectorParameterValue(FName("TerrainInfo"), TerrainInfo);
    
    // Set UV scaling parameters for water shader alignment
    float UScale = 1.0f / (float)TerrainWidth;
    float VScale = 1.0f / (float)TerrainHeight;
    Material->SetScalarParameterValue(FName("UVScale"), FMath::Max(UScale, VScale));
    Material->SetVectorParameterValue(FName("WorldSize"), FLinearColor(TerrainWidth * AuthScale, TerrainHeight * AuthScale, 0, 0));
}

// ===== SINGLE SOURCE OF TRUTH IMPLEMENTATION =====


// 6.3 MATERIAL REFRESH

void ADynamicTerrain::RefreshAllChunkMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Refreshing all chunk materials with updated scale %.2f"), TerrainScale);
    
    int32 UpdatedCount = 0;
    
    // Iterate through all chunks and reapply material parameters
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent)
        {
            UMaterialInstanceDynamic* ChunkMaterial = Cast<UMaterialInstanceDynamic>(
                Chunk.MeshComponent->GetMaterial(0));
            
            if (ChunkMaterial)
            {
                ApplyMaterialParameters(ChunkMaterial, Chunk.ChunkX, Chunk.ChunkY);
                UpdatedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Refreshed %d chunk materials"), UpdatedCount);
}


// =====================================================
// HELPER: Get Safe Neighbor Average
// =====================================================


// ============================================================================
// SECTION 7: ATMOSPHERIC INTEGRATION (~150 lines, 3%)
// ============================================================================
/**
 * Weather queries and precipitation interface for atmospheric system integration.
 */

// 7.1 WEATHER QUERIES

float ADynamicTerrain::GetTemperatureAt(FVector WorldPosition) const
{
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetTemperatureAt(WorldPosition);
    }
    return 288.15f; // 15 C default
}


float ADynamicTerrain::GetPrecipitationAt(FVector WorldPosition) const
{
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetPrecipitationAt(WorldPosition);
    }
    return 0.0f;
}


FVector ADynamicTerrain::GetWindAt(FVector WorldPosition) const
{
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetWindAt(WorldPosition);
    }
    return FVector::ZeroVector;
}

// 7.2 PRECIPITATION INTERFACE

void ADynamicTerrain::SetPrecipitationTexture(UTextureRenderTarget2D* PrecipitationTex)
{
    // Store precipitation texture for erosion calculations
    CachedPrecipitationTexture = PrecipitationTex;
    
    if (CachedPrecipitationTexture)
    {
        UE_LOG(LogTemp, Verbose, TEXT("DynamicTerrain: Precipitation texture set for enhanced erosion"));
        
        // Could trigger enhanced erosion here if needed
        // For now, just cache it for the next erosion compute pass
    }
}


// ============================================================================
// SECTION 8: FRUSTUM CULLING SYSTEM (~200 lines, 4%)
// ============================================================================
/**
 * Visibility optimization with frustum culling for 30-40% rendering performance gain.
 */

// 8.1 VISIBILITY UPDATES

void ADynamicTerrain::UpdateFrustumCulling(float DeltaTime)
{
    CullingUpdateTimer += DeltaTime;
    
    if (CullingUpdateTimer >= CullingUpdateRate)
    {
        CullingUpdateTimer = 0.0f;
        
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
            {
                FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
                    nullptr, GetWorld()->Scene, FEngineShowFlags(ESFIM_Game))
                    .SetRealtimeUpdate(true));
                
                FVector ViewLocation;
                FRotator ViewRotation;
                PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
                
                FSceneView* View = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, nullptr);
                if (View)
                {
                    int32 VisibleChunks = 0;
                    for (int32 i = 0; i < TerrainChunks.Num(); i++)
                    {
                        FTerrainChunk& Chunk = TerrainChunks[i];
                        bool bWasVisible = Chunk.bIsVisible;
                        Chunk.bIsVisible = IsChunkVisible(Chunk, View);
                        
                        if (Chunk.bIsVisible)
                        {
                        VisibleChunks++;
                            
                        // PHASE 4: Advanced LOD calculation for visible chunks
                        if (bEnableAdvancedLOD)
                        {
                            int32 NewLOD = CalculateChunkLOD(i, ViewLocation);
                            UpdateChunkLOD(i, NewLOD);
                        }
                    }
                        
                        // Update mesh visibility if changed
                        if (bWasVisible != Chunk.bIsVisible && Chunk.MeshComponent)
                        {
                            Chunk.MeshComponent->SetVisibility(Chunk.bIsVisible);
                        }
                    }
                    
                    // Store for performance stats
                    CurrentVisibleChunks = VisibleChunks;
                }
            }
        }
    }
}

// 8.2 CHUNK BOUNDS CALCULATION

bool ADynamicTerrain::IsChunkVisible(const FTerrainChunk& Chunk, const FSceneView* View) const
{
    if (!View || !Chunk.MeshComponent)
    {
        return true; // Default to visible if we can't determine
    }
    
    // Get chunk world bounds
    FBoxSphereBounds ChunkBounds = GetChunkWorldBounds(Chunk);
    
    // Test against view frustum
    return View->ViewFrustum.IntersectBox(ChunkBounds.Origin, ChunkBounds.BoxExtent);
}

FBoxSphereBounds ADynamicTerrain::GetChunkWorldBounds(const FTerrainChunk& Chunk) const
{
    // Calculate chunk dimensions in world space
    float ChunkWorldSizeX = (ChunkSize - 1) * TerrainScale;
    float ChunkWorldSizeY = (ChunkSize - 1) * TerrainScale;
    
    // Get chunk world position
    FVector2D ChunkWorldPos = GetChunkWorldPosition(Chunk.ChunkX, Chunk.ChunkY);
    
    // Create bounds box
    FVector BoxOrigin = GetActorTransform().TransformPosition(
        FVector(ChunkWorldPos.X + ChunkWorldSizeX * 0.5f,
                ChunkWorldPos.Y + ChunkWorldSizeY * 0.5f,
                MaxTerrainHeight * 0.5f));
    
    FVector BoxExtent = FVector(ChunkWorldSizeX * 0.5f, ChunkWorldSizeY * 0.5f, MaxTerrainHeight * 0.5f);
    
    return FBoxSphereBounds(BoxOrigin, BoxExtent, FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z));
}

// 8.3 PERFORMANCE STATS

void ADynamicTerrain::UpdatePerformanceStats(float DeltaTime)
{
    StatUpdateTimer += DeltaTime;
    
    if (StatUpdateTimer >= STATS_UPDATE_INTERVAL) // Update 4 times per second
    {
        if (GEngine)
        {
            float CurrentFPS = 1.0f / DeltaTime;
            int32 PendingUpdates = PendingChunkUpdates.Num();
            
            // Pre-allocate debug strings to reduce GC pressure
            CachedDebugStringBuffer = FString::Printf(TEXT("FPS: %.1f"), CurrentFPS);
            GEngine->AddOnScreenDebugMessage(10, 0.5f, FColor::Green, CachedDebugStringBuffer);
            
            //CachedDebugStringBuffer = FString::Printf(TEXT("Terrain: %dx%d (%d chunks)"), TerrainWidth, TerrainHeight, TerrainChunks.Num());
            //GEngine->AddOnScreenDebugMessage(11, 0.5f, FColor::Yellow, CachedDebugStringBuffer);
            
            CachedDebugStringBuffer = FString::Printf(TEXT("Updated %d chunks this frame, water pending: %d"),
                TotalChunkUpdatesThisFrame, PendingWaterChunkUpdates.Num());
            GEngine->AddOnScreenDebugMessage(12, 0.5f, FColor::Cyan, CachedDebugStringBuffer);
            
            //CachedDebugStringBuffer = FString::Printf(TEXT("Frustum Culling: %d/%d chunks visible"), CurrentVisibleChunks, TerrainChunks.Num());
           // GEngine->AddOnScreenDebugMessage(14, 0.5f, FColor::Magenta, CachedDebugStringBuffer);
            
           // CachedDebugStringBuffer = FString::Printf(TEXT("Chunk Size: %dx%d, Scale: %.1f"), ChunkSize, ChunkSize, TerrainScale);
           // GEngine->AddOnScreenDebugMessage(13, 0.5f, FColor::Orange, CachedDebugStringBuffer);
    
        }
        StatUpdateTimer = 0.0f;
    }
}

/**
 * ============================================
 * ATMOSPHERIC SYSTEM INTEGRATION
 * ============================================
 * Weather simulation based on atmospheric pressure dynamics
 *
 * References:
 * - Holton, J.R. (2012). "An Introduction to Dynamic Meteorology" 5th Ed.
 * - Dobashi, Y. et al. (2000). "A simple, efficient method for realistic animation of clouds"
 * - Miyazaki, R. et al. (2002). "Visual simulation of clouds and atmospheric phenomena"
 */



// ============================================================================
// SECTION 9: WORLD CONFIGURATION SYSTEM (~250 lines, 5%)
// ============================================================================
/**
 * Scalable world sizes from Small (257x257) to Massive (2049x2049).
 */

// 9.1 WORLD SIZE PRESETS

FWorldSizeConfig ADynamicTerrain::GetWorldConfigForSize(ETerrainWorldSize Size) const
{
    FWorldSizeConfig Config;
    
    switch(Size)
    {
    case ETerrainWorldSize::Small:
        Config.TerrainWidth = 257;
        Config.TerrainHeight = 257;
        Config.ChunkSize = 32;
        Config.ChunksX = 8;
        Config.ChunksY = 8;
        Config.EditingScale = 1.0f;
        Config.LODBias = 1.0f;
        break;
        
    case ETerrainWorldSize::Medium:
        Config.TerrainWidth = 513;
        Config.TerrainHeight = 513;
        Config.ChunkSize = 32;
        Config.ChunksX = 16;
        Config.ChunksY = 16;
        Config.EditingScale = 1.0f;
        Config.LODBias = 0.8f;
        break;
        
    case ETerrainWorldSize::Large:
        Config.TerrainWidth = 1025;
        Config.TerrainHeight = 1025;
        Config.ChunkSize = 32;
        Config.ChunksX = 32;
        Config.ChunksY = 32;
        Config.EditingScale = 0.25f;  // Much smaller brush for large world
        Config.LODBias = 0.6f;
        break;
        
    case ETerrainWorldSize::Massive:
        Config.TerrainWidth = 2049;
        Config.TerrainHeight = 2049;
        Config.ChunkSize = 32;
        Config.ChunksX = 64;
        Config.ChunksY = 64;
        Config.EditingScale = 0.1f;  // Very small brush for massive world
        Config.LODBias = 0.4f;
        break;
    }
    
    return Config;
}



// ============================================
// CHUNK BOUNDARY INTEGRITY SYSTEM
// ============================================
// Production-ready boundary management for seamless terrain


// 9.2 RUNTIME CONFIGURATION

void ADynamicTerrain::SetWorldSize(ETerrainWorldSize NewSize)
{
    if (NewSize == CurrentWorldSize)
    {
        return; // No change needed
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Changing world size from %d to %d"), (int32)CurrentWorldSize, (int32)NewSize);
    
    CurrentWorldSize = NewSize;
    FWorldSizeConfig NewConfig = GetWorldConfigForSize(NewSize);
    ApplyWorldConfiguration(NewConfig);
}


void ADynamicTerrain::ApplyWorldConfiguration(const FWorldSizeConfig& Config)
{
    WorldConfig = Config;
    
    // Update legacy properties
    TerrainWidth = Config.TerrainWidth;
    TerrainHeight = Config.TerrainHeight;
    ChunkSize = Config.ChunkSize;
    ChunksX = Config.ChunksX;
    ChunksY = Config.ChunksY;
    
    UE_LOG(LogTemp, Warning, TEXT("Applied world config: %dx%d terrain, %dx%d chunks"),
           TerrainWidth, TerrainHeight, ChunksX, ChunksY);
    
    // Trigger full reset with new configuration
    ResetTerrainFully();
}


// 9.3 CUSTOM WORLD SIZES

void ADynamicTerrain::MigrateToScalableSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Migrating to scalable chunk system..."));
    
    // Apply current world size configuration
    ApplyWorldConfiguration(WorldConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Migration complete: %d chunks active"), TerrainChunks.Num());
}


void ADynamicTerrain::SetCustomWorldSize(int32 Width, int32 Height, int32 CustomChunkSize)
{
    FWorldSizeConfig CustomConfig = CreateCustomWorldConfig(Width, Height, CustomChunkSize);
    ApplyWorldConfiguration(CustomConfig);
    UE_LOG(LogTemp, Warning, TEXT("Applied custom world size: %dx%d with %dx%d chunks"),
           Width, Height, CustomConfig.ChunksX, CustomConfig.ChunksY);
}


FWorldSizeConfig ADynamicTerrain::CreateCustomWorldConfig(int32 Width, int32 Height, int32 CustomChunkSize) const
{
    FWorldSizeConfig Config;
    Config.TerrainWidth = Width;
    Config.TerrainHeight = Height;
    Config.ChunkSize = CustomChunkSize;
    
    // Calculate optimal chunk count
    Config.ChunksX = FMath::CeilToInt((float)Width / (CustomChunkSize - 1));
    Config.ChunksY = FMath::CeilToInt((float)Height / (CustomChunkSize - 1));
    
    // Auto-scale editing based on terrain density
    float TerrainDensity = (float)(Width * Height) / (513.0f * 513.0f); // Relative to medium
    Config.EditingScale = 1.0f / FMath::Sqrt(TerrainDensity);
    Config.EditingScale = FMath::Clamp(Config.EditingScale, 0.05f, 2.0f);
    
    // Auto-adjust LOD bias for performance
    int32 TotalChunks = Config.ChunksX * Config.ChunksY;
    if (TotalChunks > 2000) Config.LODBias = 0.3f;
    else if (TotalChunks > 1000) Config.LODBias = 0.5f;
    else if (TotalChunks > 500) Config.LODBias = 0.7f;
    else Config.LODBias = 1.0f;
    
    return Config;
}

// GPU Terrain System



// ============================================================================
// SECTION 10: CHUNK BOUNDARY & VALIDATION (~400 lines, 9%)
// ============================================================================
/**
 * Neighbor management, boundary validation, tear detection and repair.
 */

// 10.1 NEIGHBOR MANAGEMENT

TArray<int32> ADynamicTerrain::GetNeighboringChunks(int32 ChunkIndex, bool bIncludeDiagonals) const
{
    TArray<int32> Neighbors;
    
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return Neighbors;
    }
    
    const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    int32 ChunkX = Chunk.ChunkX;
    int32 ChunkY = Chunk.ChunkY;
    
    // Direct neighbors (4-connected)
    TArray<FVector2D> NeighborOffsets = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}  // Left, Right, Down, Up
    };
    
    // Add diagonal neighbors if requested (8-connected)
    if (bIncludeDiagonals)
    {
        NeighborOffsets.Append({
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonals
        });
    }
    
    for (const FVector2D& Offset : NeighborOffsets)
    {
        int32 NeighborX = ChunkX + Offset.X;
        int32 NeighborY = ChunkY + Offset.Y;
        
        if (NeighborX >= 0 && NeighborX < ChunksX &&
            NeighborY >= 0 && NeighborY < ChunksY)
        {
            int32 NeighborIndex = NeighborY * ChunksX + NeighborX;
            if (TerrainChunks.IsValidIndex(NeighborIndex))
            {
                Neighbors.Add(NeighborIndex);
            }
        }
    }
    
    return Neighbors;
}


// 10.2 BOUNDARY VALIDATION & REPAIR

void ADynamicTerrain::ValidateAndRepairChunkBoundaries()
{
    int32 TearCount = 0;
    int32 RepairCount = 0;
    
    // Validate horizontal boundaries
    for (int32 ChunkY = 0; ChunkY < ChunksY; ChunkY++)
    {
        for (int32 ChunkX = 0; ChunkX < ChunksX - 1; ChunkX++)
        {
            int32 LeftIndex = ChunkY * ChunksX + ChunkX;
            int32 RightIndex = ChunkY * ChunksX + (ChunkX + 1);
            
            if (TerrainChunks.IsValidIndex(LeftIndex) && TerrainChunks.IsValidIndex(RightIndex))
            {
                if (!ValidateChunkBoundaryIntegrity(LeftIndex))
                {
                    TearCount++;
                    RepairBoundaryTear(LeftIndex, RightIndex, false); // Vertical boundary
                    RepairCount++;
                }
            }
        }
    }
    
    // Validate vertical boundaries
    for (int32 ChunkY = 0; ChunkY < ChunksY - 1; ChunkY++)
    {
        for (int32 ChunkX = 0; ChunkX < ChunksX; ChunkX++)
        {
            int32 TopIndex = ChunkY * ChunksX + ChunkX;
            int32 BottomIndex = (ChunkY + 1) * ChunksX + ChunkX;
            
            if (TerrainChunks.IsValidIndex(TopIndex) && TerrainChunks.IsValidIndex(BottomIndex))
            {
                if (!ValidateChunkBoundaryIntegrity(TopIndex))
                {
                    TearCount++;
                    RepairBoundaryTear(TopIndex, BottomIndex, true); // Horizontal boundary
                    RepairCount++;
                }
            }
        }
    }
    
    if (TearCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BOUNDARY VALIDATION: Found %d tears, repaired %d"), TearCount, RepairCount);
    }
}


bool ADynamicTerrain::ValidateChunkBoundaryIntegrity(int32 ChunkIndex) const
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return false;
    }

    const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    int32 ChunkX = Chunk.ChunkX;
    int32 ChunkY = Chunk.ChunkY;

    // Check right boundary
    if (ChunkX < ChunksX - 1)
    {
        int32 BoundaryX = (ChunkX + 1) * (ChunkSize - ChunkOverlap);
        if (BoundaryX < TerrainWidth)
        {
            for (int32 Y = ChunkY * (ChunkSize - ChunkOverlap);
                 Y < FMath::Min((ChunkY + 1) * (ChunkSize - ChunkOverlap) + ChunkOverlap, TerrainHeight); Y++)
            {
                float LeftHeight = GetHeightSafe(BoundaryX - 1, Y);
                float RightHeight = GetHeightSafe(BoundaryX, Y);

                if (FMath::Abs(LeftHeight - RightHeight) > BOUNDARY_HEIGHT_TOLERANCE)
                {
                    // DIAGNOSTIC: Log first few boundary failures
                    static int32 BoundaryFailLog = 0;
                    if (BoundaryFailLog < 10)
                    {
                        BoundaryFailLog++;
                        UE_LOG(LogTemp, Warning, TEXT("BOUNDARY FAIL #%d: Chunk(%d,%d) BoundaryX=%d Y=%d | Left[%d]=%.2f Right[%d]=%.2f Diff=%.2f"),
                               BoundaryFailLog, ChunkX, ChunkY, BoundaryX, Y,
                               BoundaryX - 1, LeftHeight, BoundaryX, RightHeight, FMath::Abs(LeftHeight - RightHeight));
                    }
                    return false; // Significant height difference detected
                }
            }
        }
    }
    
    // Check bottom boundary
    if (ChunkY < ChunksY - 1)
    {
        int32 BoundaryY = (ChunkY + 1) * (ChunkSize - ChunkOverlap);
        if (BoundaryY < TerrainHeight)
        {
            for (int32 X = ChunkX * (ChunkSize - ChunkOverlap);
                 X < FMath::Min((ChunkX + 1) * (ChunkSize - ChunkOverlap) + ChunkOverlap, TerrainWidth); X++)
            {
                float TopHeight = GetHeightSafe(X, BoundaryY - 1);
                float BottomHeight = GetHeightSafe(X, BoundaryY);
                
                if (FMath::Abs(TopHeight - BottomHeight) > BOUNDARY_HEIGHT_TOLERANCE)
                {
                    return false; // Significant height difference detected
                }
            }
        }
    }
    
    return true; // All boundaries pass validation
}


void ADynamicTerrain::RepairBoundaryTear(int32 ChunkA, int32 ChunkB, bool bIsVerticalBoundary)
{
    if (!TerrainChunks.IsValidIndex(ChunkA) || !TerrainChunks.IsValidIndex(ChunkB))
    {
        return;
    }
    
    // Force immediate update of both chunks to repair tear
    TArray<int32> RepairGroup = {ChunkA, ChunkB};
    ForceUpdateChunkGroup(RepairGroup);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("BOUNDARY REPAIR: Fixed tear between chunks %d and %d (%s boundary)"),
           ChunkA, ChunkB, bIsVerticalBoundary ? TEXT("vertical") : TEXT("horizontal"));
}


void ADynamicTerrain::ValidateChunkBoundary(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex)) return;
    
    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    int32 ChunkX = Chunk.ChunkX;
    int32 ChunkY = Chunk.ChunkY;
    
    // Check boundaries with adjacent chunks
    if (ChunkX > 0) // Left boundary
    {
        int32 LeftChunkIndex = ChunkY * ChunksX + (ChunkX - 1);
        if (TerrainChunks.IsValidIndex(LeftChunkIndex))
        {
            // Force material update on boundary vertices
            UpdateGPUChunkMaterial(TerrainChunks[LeftChunkIndex]);
        }
    }
    
    if (ChunkY > 0) // Top boundary
    {
        int32 TopChunkIndex = (ChunkY - 1) * ChunksX + ChunkX;
        if (TerrainChunks.IsValidIndex(TopChunkIndex))
        {
            UpdateGPUChunkMaterial(TerrainChunks[TopChunkIndex]);
        }
    }
}

// 10.3 ATOMIC GROUP UPDATES

void ADynamicTerrain::UpdateChunkGroupAtomic(const TArray<int32>& ChunkIndices)
{
    // Atomic update: process all chunks in single frame to prevent tears
    for (int32 ChunkIndex : ChunkIndices)
    {
        if (TerrainChunks.IsValidIndex(ChunkIndex))
        {
            UpdateChunk(ChunkIndex);
            PendingChunkUpdates.Remove(ChunkIndex);
            TerrainChunks[ChunkIndex].bNeedsUpdate = false;
        }
    }
    
    // Log atomic update for debugging
    if (ChunkIndices.Num() > 1)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("ATOMIC UPDATE: Processed %d chunks together"), ChunkIndices.Num());
    }
}


void ADynamicTerrain::ForceUpdateChunkGroup(const TArray<int32>& ChunkIndices)
{
    for (int32 ChunkIndex : ChunkIndices)
    {
        if (TerrainChunks.IsValidIndex(ChunkIndex))
        {
            FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
            
            // Regenerate mesh geometry
            GenerateChunkMesh(Chunk.ChunkX, Chunk.ChunkY);
            
            // CRITICAL: Also update GPU material binding for GPU mode
            if (CurrentComputeMode == ETerrainComputeMode::GPU && bGPUInitialized)
            {
                UpdateGPUChunkMaterial(Chunk);
            }
        }
    }
}


bool ADynamicTerrain::ValidateChunkCoordinateConsistency() const
{
    bool bConsistent = true;
    
    // Test coordinate roundtrip consistency
    for (int32 TestY = 0; TestY < TerrainHeight; TestY += ChunkSize)
    {
        for (int32 TestX = 0; TestX < TerrainWidth; TestX += ChunkSize)
        {
            int32 ChunkIndex = GetChunkIndexFromCoordinates(TestX, TestY);
            if (ChunkIndex >= 0 && TerrainChunks.IsValidIndex(ChunkIndex))
            {
                const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
                int32 BackX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
                int32 BackY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
                
                if (FMath::Abs(TestX - BackX) > ChunkSize || FMath::Abs(TestY - BackY) > ChunkSize)
                {
                    UE_LOG(LogTemp, Error, TEXT("COORDINATE INCONSISTENCY: (%d,%d) -> Chunk %d -> (%d,%d)"),
                           TestX, TestY, ChunkIndex, BackX, BackY);
                    bConsistent = false;
                }
            }
        }
    }
    
    return bConsistent;
}


TArray<FVector> ADynamicTerrain::GetChunkBoundaryVertices(int32 ChunkIndex, int32 BoundaryEdge) const
{
    TArray<FVector> BoundaryVertices;
    
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return BoundaryVertices;
    }
    
    const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    int32 StartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 StartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
    int32 EndX = FMath::Min(StartX + ChunkSize, TerrainWidth);
    int32 EndY = FMath::Min(StartY + ChunkSize, TerrainHeight);
    
    // Extract boundary vertices based on edge (0=left, 1=right, 2=top, 3=bottom)
    switch (BoundaryEdge)
    {
    case 0: // Left edge
        for (int32 Y = StartY; Y < EndY; Y++)
        {
            FVector Vertex = TerrainToWorldPosition(StartX, Y);
            BoundaryVertices.Add(Vertex);
        }
        break;
    case 1: // Right edge
        for (int32 Y = StartY; Y < EndY; Y++)
        {
            FVector Vertex = TerrainToWorldPosition(EndX - 1, Y);
            BoundaryVertices.Add(Vertex);
        }
        break;
    case 2: // Top edge
        for (int32 X = StartX; X < EndX; X++)
        {
            FVector Vertex = TerrainToWorldPosition(X, StartY);
            BoundaryVertices.Add(Vertex);
        }
        break;
    case 3: // Bottom edge
        for (int32 X = StartX; X < EndX; X++)
        {
            FVector Vertex = TerrainToWorldPosition(X, EndY - 1);
            BoundaryVertices.Add(Vertex);
        }
        break;
    }
    
    return BoundaryVertices;
}

// ============================================
// PHASE 4: CHUNK POOLING & ADVANCED LOD SYSTEM
// ============================================



// ============================================================================
// SECTION 11: CHUNK POOLING & LOD (~200 lines, 4%)
// ============================================================================
/**
 * Memory optimization through mesh component pooling and LOD management.
 */

// 11.1 MESH COMPONENT POOLING

UProceduralMeshComponent* ADynamicTerrain::GetPooledMeshComponent()
{
    if (!bEnableChunkPooling || ChunkMeshPool.Num() == 0)
    {
        return nullptr;
    }
    
    // Get component from pool
    UProceduralMeshComponent* PooledComponent = ChunkMeshPool.Pop();
    
    // Reset component state
    if (PooledComponent)
    {
        PooledComponent->ClearAllMeshSections();
        PooledComponent->SetVisibility(true);
    }
    
    return PooledComponent;
}


void ADynamicTerrain::ReturnMeshComponentToPool(UProceduralMeshComponent* MeshComponent)
{
    if (!bEnableChunkPooling || !MeshComponent)
    {
        return;
    }
    
    // Don't exceed pool size
    if (ChunkMeshPool.Num() >= ChunkPoolSize)
    {
        MeshComponent->DestroyComponent();
        return;
    }
    
    // Clean and return to pool
    MeshComponent->ClearAllMeshSections();
    MeshComponent->SetVisibility(false);
    MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    
    ChunkMeshPool.Add(MeshComponent);
}


// 11.2 LOD CALCULATION & UPDATES

int32 ADynamicTerrain::CalculateChunkLOD(int32 ChunkIndex, FVector CameraLocation) const
{
    if (!bEnableAdvancedLOD || !TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return 0;
    }
    
    FVector ChunkWorldPos = GetChunkWorldPosition(ChunkIndex);
    float Distance = FVector::Dist(CameraLocation, ChunkWorldPos) * LODDistanceMultiplier;
    
    // Advanced LOD calculation with multiple factors
    int32 BaseLOD = 0;
    if (Distance > 6000.0f) BaseLOD = 3;
    else if (Distance > 3000.0f) BaseLOD = 2;
    else if (Distance > 1500.0f) BaseLOD = 1;
    
    // Factor in chunk importance (water, recent edits)
    if (WaterSystem && WaterSystem->GetWaterDepthAtIndex(
        TerrainChunks[ChunkIndex].ChunkX * ChunkSize,
        TerrainChunks[ChunkIndex].ChunkY * ChunkSize) > 0.1f)
    {
        BaseLOD = FMath::Max(0, BaseLOD - 1); // Higher detail for water
    }
    
    return FMath::Clamp(BaseLOD, 0, 3);
}


void ADynamicTerrain::UpdateChunkLOD(int32 ChunkIndex, int32 NewLOD)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }
    
    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    
    // Only update if LOD changed significantly
    static TMap<int32, int32> LastKnownLODs;
    int32* LastLOD = LastKnownLODs.Find(ChunkIndex);
    if (LastLOD && *LastLOD == NewLOD)
    {
        return;
    }
    
    LastKnownLODs.Add(ChunkIndex, NewLOD);
    
    // Update chunk with new LOD
    GenerateChunkMesh(Chunk.ChunkX, Chunk.ChunkY);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Updated chunk %d LOD to %d"), ChunkIndex, NewLOD);
}

// ============================================
// DRIFT PHASE 2: AUTHORITY SIMPLIFICATION COMPLETE
// ============================================
// Eliminated fallback patterns, validation overhead, and retry logic
// Authority is established once and trusted throughout execution




// ============================================================================
// SECTION 12: GPU TERRAIN - LIFECYCLE (~450 lines, 10%)
// ============================================================================
/**
 * GPU compute mode management, resource creation/release, CPU-GPU authority handoff.
 */

// 12.1 COMPUTE MODE MANAGEMENT

void ADynamicTerrain::SetComputeMode(ETerrainComputeMode NewMode)
{
    if (NewMode == CurrentComputeMode) return;
    
    if (NewMode == ETerrainComputeMode::GPU)
    {
        if (!bGPUInitialized)
        {
            InitializeGPUTerrain();
        }
        TransferHeightmapToGPU();
        CurrentComputeMode = ETerrainComputeMode::GPU;
        UE_LOG(LogTemp, Verbose, TEXT("Switched to GPU mode"));
    }
    else
    {
        if (bGPUInitialized)
        {
            TransferHeightmapFromGPU();
        }
        CurrentComputeMode = ETerrainComputeMode::CPU;
        
        for (int32 i = 0; i < TerrainChunks.Num(); i++)
        {
            PendingChunkUpdates.Add(i);
        }
        UE_LOG(LogTemp, Warning, TEXT("Switched to CPU mode"));
    }
}


// 12.2 GPU RESOURCE CREATION & RELEASE

void ADynamicTerrain::CreateGPUResources()
{
    // Release any existing resources
    ReleaseGPUResources();

    // ========== DOUBLE BUFFERED HEIGHT TEXTURES ==========
    // Double buffering eliminates visual pulsing during GPU sync:
    // - Materials always read from the stable "read" buffer
    // - Compute shader writes to the "write" buffer
    // - After compute completes, buffers are swapped
    // This ensures materials never sample partially-updated data

    // Create height buffer A
    HeightRenderTexture_A = NewObject<UTextureRenderTarget2D>(this);
    HeightRenderTexture_A->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_FloatRGBA,
        false
    );
    HeightRenderTexture_A->AddressX = TA_Clamp;
    HeightRenderTexture_A->AddressY = TA_Clamp;
    HeightRenderTexture_A->bCanCreateUAV = true;
    HeightRenderTexture_A->UpdateResourceImmediate();

    // Create height buffer B
    HeightRenderTexture_B = NewObject<UTextureRenderTarget2D>(this);
    HeightRenderTexture_B->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_FloatRGBA,
        false
    );
    HeightRenderTexture_B->AddressX = TA_Clamp;
    HeightRenderTexture_B->AddressY = TA_Clamp;
    HeightRenderTexture_B->bCanCreateUAV = true;
    HeightRenderTexture_B->UpdateResourceImmediate();

    // Initialize buffer pointers - A starts as read (stable), B starts as write
    CurrentHeightBufferIndex = 0;
    HeightReadTexture = HeightRenderTexture_A;
    HeightWriteTexture = HeightRenderTexture_B;
    bFirstComputeExecution = true;  // Reset for new GPU session

    // Legacy compatibility - point to read texture so existing code works
    HeightRenderTexture = HeightReadTexture;

    // Create water-specific height texture (NO double buffering - updates every frame)
    // This decouples water flow visuals from the terrain interpolation system
    WaterHeightTexture = NewObject<UTextureRenderTarget2D>(this);
    WaterHeightTexture->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_FloatRGBA,
        false
    );
    WaterHeightTexture->AddressX = TA_Clamp;
    WaterHeightTexture->AddressY = TA_Clamp;
    WaterHeightTexture->bCanCreateUAV = false;  // Read-only for water, no compute writes
    WaterHeightTexture->UpdateResourceImmediate();

    // Create erosion render texture with UAV support
    ErosionRenderTexture = NewObject<UTextureRenderTarget2D>(this);
    ErosionRenderTexture->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_R32_FLOAT,
        false
    );
    ErosionRenderTexture->AddressX = TA_Clamp;
    ErosionRenderTexture->AddressY = TA_Clamp;
    ErosionRenderTexture->bCanCreateUAV = true;  // Enable UAV
    ErosionRenderTexture->UpdateResourceImmediate();
    
    // Create hardness render texture with UAV support
    HardnessRenderTexture = NewObject<UTextureRenderTarget2D>(this);
    HardnessRenderTexture->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_R8G8B8A8,
        false
    );
    HardnessRenderTexture->AddressX = TA_Clamp;
    HardnessRenderTexture->AddressY = TA_Clamp;
    HardnessRenderTexture->bCanCreateUAV = true;  // Enable UAV
    HardnessRenderTexture->UpdateResourceImmediate();
    
    // Create normal render texture with UAV support
    NormalRenderTexture = NewObject<UTextureRenderTarget2D>(this);
    NormalRenderTexture->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_FloatRGBA,
        false
    );
    NormalRenderTexture->AddressX = TA_Clamp;
    NormalRenderTexture->AddressY = TA_Clamp;
    NormalRenderTexture->bCanCreateUAV = true;  // Enable UAV
    NormalRenderTexture->UpdateResourceImmediate();
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Resources created with UAV support: %dx%d textures"),
           GPUTextureWidth, GPUTextureHeight);
}


void ADynamicTerrain::ReleaseGPUResources()
{
    // Release double-buffered height textures
    if (HeightRenderTexture_A)
    {
        HeightRenderTexture_A->ReleaseResource();
        HeightRenderTexture_A = nullptr;
    }

    if (HeightRenderTexture_B)
    {
        HeightRenderTexture_B->ReleaseResource();
        HeightRenderTexture_B = nullptr;
    }

    // Clear buffer pointers
    HeightReadTexture = nullptr;
    HeightWriteTexture = nullptr;
    HeightRenderTexture = nullptr;
    CurrentHeightBufferIndex = 0;
    bFirstComputeExecution = true;

    if (WaterHeightTexture)
    {
        WaterHeightTexture->ReleaseResource();
        WaterHeightTexture = nullptr;
    }

    if (ErosionRenderTexture)
    {
        ErosionRenderTexture->ReleaseResource();
        ErosionRenderTexture = nullptr;
    }

    if (HardnessRenderTexture)
    {
        HardnessRenderTexture->ReleaseResource();
        HardnessRenderTexture = nullptr;
    }

    if (NormalRenderTexture)
    {
        NormalRenderTexture->ReleaseResource();
        NormalRenderTexture = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("GPU Resources released"));
}


void ADynamicTerrain::SwapHeightBuffers()
{
    // NOTE: This function NO LONGER SWAPS buffer pointers.
    // We now use a copy-after-compute approach instead:
    // - HeightReadTexture (A) = stable buffer, material always samples this
    // - HeightWriteTexture (B) = compute writes here, then copied to A
    // This eliminates visual vibration caused by material alternating between textures.

    // Just update materials to use HeightReadTexture (always A)
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent)
        {
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                Chunk.MeshComponent->GetMaterial(0));
            if (DynMaterial)
            {
                DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightReadTexture);
                DynMaterial->SetScalarParameterValue(FName("HeightBlendFactor"), 1.0f);
            }
        }
    }
}

void ADynamicTerrain::UpdateHeightBufferInterpolation(float DeltaTime)
{
    // NOTE: Material-side interpolation is disabled because with only 2 buffers,
    // the "previous" buffer gets overwritten during compute execution.
    // This function is kept as a stub in case we add a 3rd buffer for true interpolation.
    // For now, double buffering alone provides stable compute shader execution.
}

void ADynamicTerrain::UpdateWaterHeightTexture()
{
    // Copy current height data to water texture EVERY FRAME
    // This ensures water always sees the latest terrain without interpolation artifacts
    if (!bGPUInitialized || !WaterHeightTexture || !HeightReadTexture) return;

    ENQUEUE_RENDER_COMMAND(UpdateWaterHeightTexture)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            if (!WaterHeightTexture || !HeightReadTexture) return;

            FTextureRenderTargetResource* SrcResource = HeightReadTexture->GetRenderTargetResource();
            FTextureRenderTargetResource* DstResource = WaterHeightTexture->GetRenderTargetResource();

            if (SrcResource && DstResource)
            {
                FRHITexture* SrcTexture = SrcResource->GetRenderTargetTexture();
                FRHITexture* DstTexture = DstResource->GetRenderTargetTexture();

                if (SrcTexture && DstTexture)
                {
                    // Direct GPU copy - very fast
                    FRHICopyTextureInfo CopyInfo;
                    RHICmdList.CopyTexture(SrcTexture, DstTexture, CopyInfo);
                }
            }
        });
}



// 12.3 INITIALIZATION & TOGGLE

void ADynamicTerrain::InitializeGPUTerrain()
{
    if (bGPUInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain already initialized"));
        return;
    }

    // CRITICAL: Require FULL terrain data before GPU init
    // This prevents corruption from partial/empty HeightMap
    int32 ExpectedSize = TerrainWidth * TerrainHeight;
    if (HeightMap.Num() < ExpectedSize || ExpectedSize <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain init deferred - HeightMap has %d elements, need %d"),
               HeightMap.Num(), ExpectedSize);
        bPendingGPUInit = true;
        return;
    }

    // Validate HeightMap has actual terrain data (not all zeros)
    float MinH = TNumericLimits<float>::Max();
    float MaxH = TNumericLimits<float>::Lowest();
    for (int32 i = 0; i < FMath::Min(1000, HeightMap.Num()); i++)
    {
        MinH = FMath::Min(MinH, HeightMap[i]);
        MaxH = FMath::Max(MaxH, HeightMap[i]);
    }
    float Range = MaxH - MinH;

    if (Range < 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain init deferred - HeightMap appears flat (range: %.2f)"), Range);
        bPendingGPUInit = true;
        return;
    }

    // We have valid data - initialize with it
    UE_LOG(LogTemp, Log, TEXT("GPU Terrain init proceeding - HeightMap has %d elements, range: %.2f"),
           HeightMap.Num(), Range);
    InitializeGPUTerrainWithData();
}


void ADynamicTerrain::InitializeGPUTerrainWithData()
{
    if (bGPUInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU Terrain already initialized"));
        return;
    }

    // CRITICAL: Require FULL and VALID terrain data
    int32 ExpectedSize = TerrainWidth * TerrainHeight;
    if (HeightMap.Num() < ExpectedSize || ExpectedSize <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize GPU terrain - HeightMap has %d elements, need %d!"),
               HeightMap.Num(), ExpectedSize);
        return;
    }

    // Validate HeightMap has actual terrain data (not all zeros)
    float MinH = TNumericLimits<float>::Max();
    float MaxH = TNumericLimits<float>::Lowest();
    for (int32 i = 0; i < FMath::Min(1000, HeightMap.Num()); i++)
    {
        MinH = FMath::Min(MinH, HeightMap[i]);
        MaxH = FMath::Max(MaxH, HeightMap[i]);
    }

    if ((MaxH - MinH) < 1.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize GPU terrain - HeightMap appears flat (range: %.2f)!"),
               MaxH - MinH);
        return;
    }

    // Use exact dimensions
    GPUTextureWidth = TerrainWidth;
    GPUTextureHeight = TerrainHeight;
    
    UE_LOG(LogTemp, Log, TEXT("Initializing GPU Terrain with existing data: %dx%d"),
           GPUTextureWidth, GPUTextureHeight);
    
    // Create GPU resources (includes double-buffered height textures)
    CreateGPUResources();

    if (!HeightRenderTexture_A || !HeightRenderTexture_B)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GPU resources! A=%s B=%s"),
               HeightRenderTexture_A ? TEXT("Valid") : TEXT("NULL"),
               HeightRenderTexture_B ? TEXT("Valid") : TEXT("NULL"));
        bGPUInitialized = false;
        return;
    }
    
    // CRITICAL: Upload existing CPU terrain to GPU
    UE_LOG(LogTemp, Warning, TEXT("Uploading CPU terrain data to GPU..."));
    TransferHeightmapToGPU();

    // Upload hardness data from GeologyController
    UE_LOG(LogTemp, Warning, TEXT("Uploading hardness data to GPU..."));
    SyncHardnessToGPU();

    // Wait for uploads to complete
    UE_LOG(LogTemp, Warning, TEXT(">>> Waiting for GPU uploads to complete (FlushRenderingCommands)..."));
    FlushRenderingCommands();
    UE_LOG(LogTemp, Warning, TEXT(">>> GPU upload flush completed, verifying..."));

    // Verify upload by reading back a few samples
    VerifyGPUUpload();

    UE_LOG(LogTemp, Warning, TEXT(">>> GPU verification completed"));
    bGPUInitialized = true;

    // Set compute mode but DON'T sync back from GPU
    if (bUseGPUTerrain)
    {
        CurrentComputeMode = ETerrainComputeMode::GPU;
        UE_LOG(LogTemp, Log, TEXT("  GPU Terrain initialized with CPU data as authority"));
    }

    // CRITICAL: If using GPU heightmap rendering, REGENERATE all chunks as FLAT meshes
    // Initial generation used CPU heights because bGPUInitialized was false at that time
    // We must regenerate as flat meshes so WPO can sample heights from GPU texture
    if (bUseGPUHeightmapRendering)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU Rendering Mode: Regenerating all %d chunks as FLAT meshes..."),
               TerrainChunks.Num());

        for (FTerrainChunk& Chunk : TerrainChunks)
        {
            if (Chunk.MeshComponent)
            {
                // Regenerate mesh as FLAT (Z=0) - now bGPUInitialized is true so it will be flat
                GenerateChunkMesh(Chunk.ChunkX, Chunk.ChunkY);

                // Ensure we have a dynamic material instance
                UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                    Chunk.MeshComponent->GetMaterial(0)
                );

                if (!DynMaterial && CurrentActiveMaterial)
                {
                    DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
                    Chunk.MeshComponent->SetMaterial(0, DynMaterial);
                }

                if (DynMaterial)
                {
                    UpdateGPUChunkMaterialParams(DynMaterial, Chunk);
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("GPU Rendering Mode: All chunks regenerated with flat geometry + heightmap binding"));
    }
}


void ADynamicTerrain::ToggleGPUTerrain(bool bEnable)
{
    if (bEnable && !bGPUInitialized)
    {
        InitializeGPUTerrain();
    }
    
    bUseGPUTerrain = bEnable && bGPUInitialized;
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Terrain %s"), bUseGPUTerrain ? TEXT("Enabled") : TEXT("Disabled"));
}



// ============================================================================
// SECTION 13: GPU TERRAIN - DATA SYNCHRONIZATION (~400 lines, 9%)
// ============================================================================
/**
 * CPU to GPU transfer, GPU to CPU readback, bidirectional sync with render fencing.
 */

// 13.1 CPU â†’ GPU TRANSFER

void ADynamicTerrain::TransferHeightmapToGPU()
{
    UE_LOG(LogTemp, Warning, TEXT("*** TransferHeightmapToGPU ENTERED ***"));

    // Validate double buffers exist
    if (!HeightRenderTexture_A || !HeightRenderTexture_B)
    {
        UE_LOG(LogTemp, Error, TEXT("*** TransferHeightmapToGPU: Height buffers not initialized! A=%s B=%s ***"),
               HeightRenderTexture_A ? TEXT("Valid") : TEXT("NULL"),
               HeightRenderTexture_B ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    // CRITICAL: Require valid HeightMap data before uploading
    int32 ExpectedSize = TerrainWidth * TerrainHeight;
    if (HeightMap.Num() < ExpectedSize || ExpectedSize <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: Skipping - HeightMap has %d elements, need %d"),
               HeightMap.Num(), ExpectedSize);
        return;
    }

    // NOTE: PF_FloatRGBA is actually 16-bit HALF-floats (FFloat16Color = 8 bytes per pixel)
    // NOT 32-bit floats! Must use FFloat16Color for correct upload.
    TArray<FFloat16Color> GPUData;
    GPUData.SetNum(GPUTextureWidth * GPUTextureHeight);

    // Initialize to zero height (prevents garbage)
    for (int32 i = 0; i < GPUData.Num(); i++)
    {
        GPUData[i] = FFloat16Color(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
    }

    // Copy height values into R channel with proper bounds checking
    for (int32 Y = 0; Y < FMath::Min(TerrainHeight, GPUTextureHeight); Y++)
    {
        for (int32 X = 0; X < FMath::Min(TerrainWidth, GPUTextureWidth); X++)
        {
            int32 CPUIndex = Y * TerrainWidth + X;
            int32 GPUIndex = Y * GPUTextureWidth + X;

            if (CPUIndex < HeightMap.Num() && GPUIndex < GPUData.Num())
            {
                // Store height in R channel only (convert to half-float)
                GPUData[GPUIndex] = FFloat16Color(FLinearColor(HeightMap[CPUIndex], 0.0f, 0.0f, 1.0f));
            }
        }
    }

    // DEBUG: Log sample values being uploaded
    UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: Uploading %d pixels as FFloat16Color, first 5 values:"), GPUData.Num());
    for (int32 i = 0; i < FMath::Min(5, GPUData.Num()); i++)
    {
        UE_LOG(LogTemp, Warning, TEXT("  [%d] Height=%.2f"), i, GPUData[i].R.GetFloat());
    }

    // DOUBLE BUFFER: Upload to BOTH buffers so they start with identical data
    ENQUEUE_RENDER_COMMAND(UploadHeightmapToGPU)(
        [this, GPUData](FRHICommandListImmediate& RHICmdList)
        {
            // Stride for RGBA half-float texture (4 half-floats = 8 bytes per pixel)
            uint32 Stride = GPUTextureWidth * sizeof(FFloat16Color);
            FUpdateTextureRegion2D Region(0, 0, 0, 0, GPUTextureWidth, GPUTextureHeight);

            // Upload to Buffer A
            if (HeightRenderTexture_A)
            {
                FTextureRHIRef TextureRHI_A = HeightRenderTexture_A->GetRenderTargetResource()->GetRenderTargetTexture();
                if (TextureRHI_A)
                {
                    RHICmdList.UpdateTexture2D(TextureRHI_A, 0, Region, Stride, (const uint8*)GPUData.GetData());
                    UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: Uploaded to Buffer A"));
                }
            }

            // Upload to Buffer B
            if (HeightRenderTexture_B)
            {
                FTextureRHIRef TextureRHI_B = HeightRenderTexture_B->GetRenderTargetResource()->GetRenderTargetTexture();
                if (TextureRHI_B)
                {
                    RHICmdList.UpdateTexture2D(TextureRHI_B, 0, Region, Stride, (const uint8*)GPUData.GetData());
                    UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: Uploaded to Buffer B"));
                }
            }

            // Upload to WaterHeightTexture (continuous water texture - no double buffering)
            if (WaterHeightTexture)
            {
                FTextureRHIRef TextureRHI_Water = WaterHeightTexture->GetRenderTargetResource()->GetRenderTargetTexture();
                if (TextureRHI_Water)
                {
                    RHICmdList.UpdateTexture2D(TextureRHI_Water, 0, Region, Stride, (const uint8*)GPUData.GetData());
                    UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: Uploaded to WaterHeightTexture"));
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapToGPU: All buffers uploaded (A, B, WaterHeight)"));
        });
}

// Verification function to confirm GPU upload succeeded
void ADynamicTerrain::VerifyGPUUpload()
{
    UE_LOG(LogTemp, Warning, TEXT(">>> VerifyGPUUpload CALLED <<<"));

    if (!HeightRenderTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("VerifyGPUUpload: No render texture!"));
        return;
    }

    EPixelFormat Format = HeightRenderTexture->GetFormat();
    UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload: Texture format = %d (PF_FloatRGBA=%d)"), (int32)Format, (int32)PF_FloatRGBA);

    if (Format != PF_FloatRGBA && Format != PF_A32B32G32R32F)
    {
        UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload: Skipping - format %d not compatible"), (int32)Format);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload: Reading back GPU texture to verify upload..."));

    ENQUEUE_RENDER_COMMAND(VerifyGPUUploadCmd)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            if (!TextureRHI) return;

            TArray<FFloat16Color> SurfaceData;
            FIntRect Rect(0, 0, GPUTextureWidth, GPUTextureHeight);

            RHICmdList.ReadSurfaceFloatData(TextureRHI, Rect, SurfaceData, CubeFace_PosX, 0, 0);

            AsyncTask(ENamedThreads::GameThread, [this, SurfaceData]()
            {
                if (SurfaceData.Num() == 0)
                {
                    UE_LOG(LogTemp, Error, TEXT("VerifyGPUUpload: Readback returned 0 samples!"));
                    return;
                }

                // Compare first 10 samples
                UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload: Comparing %d GPU samples to CPU HeightMap"), SurfaceData.Num());
                UE_LOG(LogTemp, Warning, TEXT("  GPU texture: %dx%d, CPU terrain: %dx%d"),
                       GPUTextureWidth, GPUTextureHeight, TerrainWidth, TerrainHeight);

                float MaxDiff = 0.0f;
                float TotalDiff = 0.0f;
                int32 SampleCount = 0;

                for (int32 i = 0; i < FMath::Min(100, FMath::Min(SurfaceData.Num(), HeightMap.Num())); i++)
                {
                    float GPUVal = SurfaceData[i].R.GetFloat();
                    float CPUVal = HeightMap[i];
                    float Diff = FMath::Abs(GPUVal - CPUVal);
                    MaxDiff = FMath::Max(MaxDiff, Diff);
                    TotalDiff += Diff;
                    SampleCount++;

                    if (i < 10)
                    {
                        // Show ALL channels to debug format issues
                        float R = SurfaceData[i].R.GetFloat();
                        float G = SurfaceData[i].G.GetFloat();
                        float B = SurfaceData[i].B.GetFloat();
                        float A = SurfaceData[i].A.GetFloat();
                        UE_LOG(LogTemp, Warning, TEXT("    [%d] R=%.2f G=%.2f B=%.2f A=%.2f | CPU=%.2f"),
                               i, R, G, B, A, CPUVal);
                    }
                }

                float AvgDiff = SampleCount > 0 ? TotalDiff / SampleCount : 0.0f;
                UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload RESULT: MaxDiff=%.2f AvgDiff=%.2f (from %d samples)"),
                       MaxDiff, AvgDiff, SampleCount);

                if (MaxDiff > 10.0f)
                {
                    UE_LOG(LogTemp, Error, TEXT("VerifyGPUUpload FAILED: GPU data doesn't match CPU! Upload may have failed."));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("VerifyGPUUpload SUCCESS: GPU data matches CPU within tolerance."));
                }
            });
        });

    FlushRenderingCommands();
}

// 2. GPU -> CPU Transfer (one-time on mode switch)

void ADynamicTerrain::SyncCPUToGPU()
{
    if (!HeightRenderTexture || HeightMap.Num() == 0)
        return;

    // CRITICAL: HeightRenderTexture uses PF_FloatRGBA which is 16-bit HALF floats (FFloat16Color)
    // NOT 32-bit floats (FLinearColor)! Using wrong format causes data corruption.
    TArray<FFloat16Color> GPUHeightData;
    GPUHeightData.SetNum(GPUTextureWidth * GPUTextureHeight);

    // Initialize to zero height
    for (int32 i = 0; i < GPUHeightData.Num(); i++)
    {
        GPUHeightData[i] = FFloat16Color(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
    }

    // Copy CPU heightmap to GPU buffer (height in R channel)
    for (int32 Y = 0; Y < TerrainHeight; Y++)
    {
        for (int32 X = 0; X < TerrainWidth; X++)
        {
            int32 CPUIndex = Y * TerrainWidth + X;
            int32 GPUIndex = Y * GPUTextureWidth + X;

            if (CPUIndex < HeightMap.Num() && GPUIndex < GPUHeightData.Num())
            {
                GPUHeightData[GPUIndex] = FFloat16Color(FLinearColor(HeightMap[CPUIndex], 0.0f, 0.0f, 1.0f));
            }
        }
    }

    ENQUEUE_RENDER_COMMAND(UploadHeightToGPU)(
        [this, GPUHeightData](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            if (!TextureRHI) return;

            // Stride for RGBA half-float texture (4 half-floats = 8 bytes per pixel)
            uint32 Stride = GPUTextureWidth * sizeof(FFloat16Color);
            FUpdateTextureRegion2D Region(0, 0, 0, 0, GPUTextureWidth, GPUTextureHeight);

            RHICmdList.UpdateTexture2D(
                TextureRHI,
                0,  // Mip level
                Region,
                Stride,
                (const uint8*)GPUHeightData.GetData()
            );
        });

    UE_LOG(LogTemp, Verbose, TEXT("SyncCPUToGPU: Uploaded %dx%d heightmap to GPU (FFloat16Color format)"),
           TerrainWidth, TerrainHeight);
}


void ADynamicTerrain::SyncHardnessToGPU()
{
    if (!HardnessRenderTexture || !CachedMasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("SyncHardnessToGPU: Missing HardnessRenderTexture or MasterController"));
        return;
    }

    AGeologyController* GeologyCtrl = CachedMasterController->GetGeologyController();
    if (!GeologyCtrl)
    {
        // No geology controller - fill with default hardness (0.5)
        UE_LOG(LogTemp, Warning, TEXT("SyncHardnessToGPU: No GeologyController - using default hardness"));

        TArray<FColor> HardnessData;
        HardnessData.SetNum(GPUTextureWidth * GPUTextureHeight);

        uint8 DefaultHardness = 128; // 0.5 normalized
        for (int32 i = 0; i < HardnessData.Num(); i++)
        {
            HardnessData[i] = FColor(DefaultHardness, 0, 0, 255);
        }

        ENQUEUE_RENDER_COMMAND(UploadDefaultHardness)(
            [this, HardnessData](FRHICommandListImmediate& RHICmdList)
            {
                FTextureRHIRef TextureRHI = HardnessRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
                if (!TextureRHI) return;

                uint32 Stride = GPUTextureWidth * sizeof(FColor);
                FUpdateTextureRegion2D Region(0, 0, 0, 0, GPUTextureWidth, GPUTextureHeight);

                RHICmdList.UpdateTexture2D(
                    TextureRHI,
                    0,
                    Region,
                    Stride,
                    (const uint8*)HardnessData.GetData()
                );
            });
        return;
    }

    // Get geology grid dimensions
    int32 GeoWidth = GeologyCtrl->GeologyGridWidth;
    int32 GeoHeight = GeologyCtrl->GeologyGridHeight;

    if (GeoWidth <= 0 || GeoHeight <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SyncHardnessToGPU: GeologyGrid not initialized"));
        return;
    }

    // Create hardness data buffer (PF_R8G8B8A8 format)
    TArray<FColor> HardnessData;
    HardnessData.SetNum(GPUTextureWidth * GPUTextureHeight);

    // Sample geology grid and resample to terrain resolution
    for (int32 Y = 0; Y < GPUTextureHeight; Y++)
    {
        for (int32 X = 0; X < GPUTextureWidth; X++)
        {
            // Map terrain coordinates to geology grid coordinates
            int32 GeoX = FMath::Clamp(X * GeoWidth / GPUTextureWidth, 0, GeoWidth - 1);
            int32 GeoY = FMath::Clamp(Y * GeoHeight / GPUTextureHeight, 0, GeoHeight - 1);
            int32 GeoIndex = GeoY * GeoWidth + GeoX;

            float Hardness = 0.5f; // Default
            if (GeoIndex < GeologyCtrl->GeologyGrid.Num())
            {
                Hardness = GeologyCtrl->GeologyGrid[GeoIndex].Hardness;
            }

            // Convert to 8-bit (0-255)
            uint8 HardnessByte = FMath::Clamp(FMath::RoundToInt(Hardness * 255.0f), 0, 255);

            int32 TexIndex = Y * GPUTextureWidth + X;
            HardnessData[TexIndex] = FColor(HardnessByte, 0, 0, 255);
        }
    }

    // Upload to GPU
    ENQUEUE_RENDER_COMMAND(UploadHardnessToGPU)(
        [this, HardnessData](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HardnessRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            if (!TextureRHI) return;

            uint32 Stride = GPUTextureWidth * sizeof(FColor);
            FUpdateTextureRegion2D Region(0, 0, 0, 0, GPUTextureWidth, GPUTextureHeight);

            RHICmdList.UpdateTexture2D(
                TextureRHI,
                0,
                Region,
                Stride,
                (const uint8*)HardnessData.GetData()
            );
        });

    UE_LOG(LogTemp, Log, TEXT("SyncHardnessToGPU: Uploaded %dx%d hardness from GeologyGrid (%dx%d)"),
           GPUTextureWidth, GPUTextureHeight, GeoWidth, GeoHeight);
}


// 13.2 GPU â†' CPU READBACK

void ADynamicTerrain::TransferHeightmapFromGPU()
{
    if (!HeightRenderTexture) return;

    // CRITICAL: Check texture format before attempting readback
    EPixelFormat Format = HeightRenderTexture->GetFormat();
    if (Format != PF_FloatRGBA && Format != PF_A32B32G32R32F)
    {
        UE_LOG(LogTemp, Warning, TEXT("TransferHeightmapFromGPU: Skipping - texture format %d not compatible"), (int32)Format);
        return;
    }

    TArray<FFloat16Color> ReadbackData;
    ReadbackData.SetNum(TerrainWidth * TerrainHeight);

    // Use proper UE5.5 render target readback
    FTextureRenderTargetResource* Resource = HeightRenderTexture->GetRenderTargetResource();
    if (!Resource) return;

    FReadSurfaceDataFlags ReadFlags(RCM_UNorm, CubeFace_MAX);
    Resource->ReadFloat16Pixels(ReadbackData);

    // Update CPU heightmap
    for (int32 i = 0; i < FMath::Min(ReadbackData.Num(), HeightMap.Num()); i++)
    {
        HeightMap[i] = ReadbackData[i].R.GetFloat();
    }

    // Skip boundary validation when erosion is enabled - GPU handles heightmap uniformly
    if (!bEnableGPUErosion)
    {
        ValidateAndRepairChunkBoundaries();
    }
}



void ADynamicTerrain::SyncGPUToCPU()
{
    if (!HeightRenderTexture || !bGPUDataDirty)
    {
        return;  // Silent return - this is normal
    }

    // CRITICAL: Check texture format before attempting readback
    EPixelFormat Format = HeightRenderTexture->GetFormat();
    if (Format != PF_FloatRGBA && Format != PF_A32B32G32R32F)
    {
        bGPUDataDirty = false;
        return;
    }

    // Don't sync if we're actively editing
    if (bHasPendingBrush)
    {
        return;
    }

    // CRITICAL: Protect recent user edits from being overwritten
    // After a user edit, skip GPU->CPU sync for a short time to let the edit "settle"
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float TimeSinceEdit = CurrentTime - LastUserEditTime;
    if (TimeSinceEdit < UserEditProtectionDuration)
    {
        // User recently edited - skip this sync to preserve their changes
        // The edit has been uploaded to GPU, erosion will run on it,
        // and we'll sync back after the protection period
        static int32 ProtectionLogCount = 0;
        if (ProtectionLogCount++ < 10)
        {
            UE_LOG(LogTemp, Warning, TEXT("SYNC SKIPPED: User edit protection active (%.2fs remaining)"),
                   UserEditProtectionDuration - TimeSinceEdit);
        }
        return;
    }
    
    if (bPendingGPUReadback)
    {
        GPUReadbackFence.Wait();
        bPendingGPUReadback = false;
    }
    
    bPendingGPUReadback = true;

    // PERFORMANCE: Only backup CPU state when erosion is disabled (for validation)
    // When erosion is enabled, we expect heights to change - no need for expensive copy
    TArray<float> CPUBackup;
    bool bNeedValidation = !bEnableGPUErosion;
    if (bNeedValidation)
    {
        CPUBackup = HeightMap;
    }

    ENQUEUE_RENDER_COMMAND(ReadbackHeightFromGPU)(
        [this, CPUBackup, bNeedValidation](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            if (!TextureRHI) return;
            
            TArray<FFloat16Color> SurfaceData;
            FIntRect Rect(0, 0, GPUTextureWidth, GPUTextureHeight);
            
            RHICmdList.ReadSurfaceFloatData(
                TextureRHI,
                Rect,
                SurfaceData,
                CubeFace_PosX,
                0,
                0
            );
            
            AsyncTask(ENamedThreads::GameThread, [this, SurfaceData, CPUBackup, bNeedValidation]()
            {
                // Validate that GPU data isn't completely flat
                float MinHeight = FLT_MAX;
                float MaxHeight = -FLT_MAX;
                float AvgHeight = 0.0f;
                int32 ValidCount = 0;
                
                for (int32 Y = 0; Y < FMath::Min(TerrainHeight, GPUTextureHeight); Y++)
                {
                    for (int32 X = 0; X < FMath::Min(TerrainWidth, GPUTextureWidth); X++)
                    {
                        int32 GPUIndex = Y * GPUTextureWidth + X;
                        if (GPUIndex < SurfaceData.Num())
                        {
                            float Height = SurfaceData[GPUIndex].R.GetFloat();
                            if (!FMath::IsNaN(Height) && FMath::IsFinite(Height))
                            {
                                MinHeight = FMath::Min(MinHeight, Height);
                                MaxHeight = FMath::Max(MaxHeight, Height);
                                AvgHeight += Height;
                                ValidCount++;
                            }
                        }
                    }
                }
                
                if (ValidCount > 0)
                {
                    AvgHeight /= ValidCount;
                }
                
                // VALIDATION: Check if GPU data is corrupt
                // CRITICAL: Skip ALL validation when erosion is enabled - we trust GPU erosion results
                float HeightRange = MaxHeight - MinHeight;
                bool bGPUDataSeemsFlat = false;
                bool bGPUDataTooNoisy = false;

                // Only check for flat/corrupt data when erosion is DISABLED
                if (!bEnableGPUErosion)
                {
                    bGPUDataSeemsFlat = (HeightRange < 1.0f && FMath::Abs(AvgHeight) < 1.0f);
                }

                // PERFORMANCE: Skip expensive deviation checks when erosion is enabled
                // Erosion legitimately changes heights - no need to validate against old CPU state
                if (bNeedValidation && CPUBackup.Num() > 0)
                {
                    float MaxDeviation = 0.0f;
                    float TotalDeviation = 0.0f;
                    int32 DeviationSamples = 0;

                    // Calculate CPU backup stats (reduced sample count for performance)
                    float CPUMin = TNumericLimits<float>::Max();
                    float CPUMax = TNumericLimits<float>::Lowest();
                    int32 SampleStep = FMath::Max(1, CPUBackup.Num() / 200); // Sample ~200 pixels instead of 1000
                    for (int32 i = 0; i < CPUBackup.Num(); i += SampleStep)
                    {
                        CPUMin = FMath::Min(CPUMin, CPUBackup[i]);
                        CPUMax = FMath::Max(CPUMax, CPUBackup[i]);
                    }
                    float CPURange = CPUMax - CPUMin;

                    // Sample GPU vs CPU deviation (reduced samples)
                    for (int32 i = 0; i < SurfaceData.Num(); i += SampleStep)
                    {
                        int32 Y = i / GPUTextureWidth;
                        int32 X = i % GPUTextureWidth;
                        int32 CPUIndex = Y * TerrainWidth + X;

                        if (CPUIndex < CPUBackup.Num())
                        {
                            float GPUHeight = SurfaceData[i].R.GetFloat();
                            float CPUHeight = CPUBackup[CPUIndex];

                            if (!FMath::IsNaN(GPUHeight) && FMath::IsFinite(GPUHeight))
                            {
                                float Deviation = FMath::Abs(GPUHeight - CPUHeight);
                                MaxDeviation = FMath::Max(MaxDeviation, Deviation);
                                TotalDeviation += Deviation;
                                DeviationSamples++;
                            }
                        }
                    }

                    float AvgDeviation = DeviationSamples > 0 ? TotalDeviation / DeviationSamples : 0.0f;
                    float AllowedDeviation = FMath::Max(CPURange * 0.1f, 100.0f);
                    bGPUDataTooNoisy = (MaxDeviation > AllowedDeviation * 10.0f) || (AvgDeviation > AllowedDeviation);
                }

                // DEBUG: Log only on actual problems (reduced frequency for performance)
                static int32 DiagnosticLogCount = 0;
                if (DiagnosticLogCount < 3 && bGPUDataSeemsFlat)
                {
                    DiagnosticLogCount++;
                    UE_LOG(LogTemp, Warning, TEXT("GPU SYNC: Data appears flat (range: %.2f, avg: %.2f)"),
                           HeightRange, AvgHeight);
                }

                // Log sync status only first few times
                static int32 SyncLogCount = 0;
                if (SyncLogCount++ < 3)
                {
                    UE_LOG(LogTemp, Log, TEXT("GPU SYNC [%d]: Range=%.1f, Avg=%.1f"),
                           SyncLogCount, HeightRange, AvgHeight);
                }

                if (bGPUDataSeemsFlat && CPUBackup.Num() > 0)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GPU sync: RESTORING from CPU backup (flat data)"));
                    HeightMap = CPUBackup;
                    TransferHeightmapToGPU();
                }
                else if (bGPUDataSeemsFlat)
                {
                    // No backup available - this is a problem! GPU data is flat but we can't restore
                    UE_LOG(LogTemp, Error, TEXT("GPU sync: FLAT DATA with no backup! Water/erosion may be broken."));
                }
                // NOTE: Removed "too noisy" rejection - delta-based sync handles this properly
                // The old rejection caused a loop: reject -> overwrite GPU -> erosion runs -> reject again
                else
                {
                    // GPU data looks valid - apply EROSION DELTA only, don't overwrite user edits
                    //
                    // CRITICAL FIX: Only apply erosion where GPU height is LOWER than CPU height
                    // This ensures user edits (raises) are NEVER undone by sync
                    //
                    // The rule is simple: erosion can only LOWER terrain, never raise it
                    // If GPU is higher than CPU, that's fine (deposition or user edit made it to GPU)
                    // If GPU is lower than CPU, apply the difference as erosion

                    int32 ErosionPixelsApplied = 0;
                    int32 PixelsSkipped = 0;

                    for (int32 Y = 0; Y < FMath::Min(TerrainHeight, GPUTextureHeight); Y++)
                    {
                        for (int32 X = 0; X < FMath::Min(TerrainWidth, GPUTextureWidth); X++)
                        {
                            int32 GPUIndex = Y * GPUTextureWidth + X;
                            int32 CPUIndex = Y * TerrainWidth + X;

                            if (GPUIndex < SurfaceData.Num() && CPUIndex < HeightMap.Num())
                            {
                                float GPUHeight = SurfaceData[GPUIndex].R.GetFloat();
                                float CPUHeight = HeightMap[CPUIndex];  // Current CPU (has user edits)

                                // Skip invalid GPU data
                                if (FMath::IsNaN(GPUHeight) || !FMath::IsFinite(GPUHeight))
                                {
                                    continue;
                                }

                                GPUHeight = FMath::Clamp(GPUHeight, -10000.0f, 10000.0f);

                                // SIMPLE RULE: Only apply if GPU is LOWER than CPU
                                // This means erosion happened and we should update CPU
                                // If GPU >= CPU, either no erosion or user raised terrain - keep CPU value
                                const float ErosionThreshold = 0.1f;

                                if (GPUHeight < CPUHeight - ErosionThreshold)
                                {
                                    // GPU is lower = erosion happened, apply it
                                    HeightMap[CPUIndex] = GPUHeight;
                                    ErosionPixelsApplied++;
                                }
                                else
                                {
                                    // GPU >= CPU, keep CPU value (preserves user raises)
                                    PixelsSkipped++;
                                }
                            }
                        }
                    }

                    static int32 DeltaSyncLogCount = 0;
                    if (DeltaSyncLogCount++ < 2)
                    {
                        UE_LOG(LogTemp, Log, TEXT("Erosion sync: %d pixels updated"), ErosionPixelsApplied);
                    }

                    // Sync complete - diagnostic logging removed for performance
                }

                // GPU RENDERING MODE: Skip mesh regeneration entirely!
                // The material samples HeightRenderTexture directly via WPO
                // Heights are already updated in the GPU texture, no CPU mesh work needed
                if (bUseGPUHeightmapRendering && bGPUInitialized)
                {
                    // No mesh regeneration needed - material WPO handles heights
                }
                else if (!bEnableGPUErosion)
                {
                    ValidateAndRepairChunkBoundaries();

                    // Gradual chunk updates when erosion is OFF
                    for (int32 i = 0; i < TerrainChunks.Num(); i++)
                    {
                        PendingChunkUpdates.Add(i);
                    }
                }
                else
                {
                    // Fallback: Force immediate atomic update if GPU rendering is off but erosion is on
                    TArray<int32> AllChunks;
                    AllChunks.Reserve(TerrainChunks.Num());
                    for (int32 i = 0; i < TerrainChunks.Num(); i++)
                    {
                        AllChunks.Add(i);
                    }
                    ForceUpdateChunkGroup(AllChunks);
                }
                
                bPendingGPUReadback = false;
                bGPUDataDirty = false;
            });
        });
    
    GPUReadbackFence.BeginFence();
}


// Lightweight height sync for water - skips validation and mesh updates
void ADynamicTerrain::SyncGPUHeightsForWater()
{
    if (!HeightRenderTexture || !bGPUInitialized || !bEnableGPUErosion)
    {
        return;  // Only needed when erosion is active
    }

    // Simple fast readback - just update HeightMap for water to read
    ENQUEUE_RENDER_COMMAND(FastHeightSyncForWater)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            if (!TextureRHI) return;

            TArray<FFloat16Color> SurfaceData;
            FIntRect Rect(0, 0, GPUTextureWidth, GPUTextureHeight);

            RHICmdList.ReadSurfaceFloatData(TextureRHI, Rect, SurfaceData, CubeFace_PosX, 0, 0);

            AsyncTask(ENamedThreads::GameThread, [this, SurfaceData]()
            {
                // Fast copy - no validation, no mesh updates
                // Just update heights so water can read them
                for (int32 Y = 0; Y < FMath::Min(TerrainHeight, GPUTextureHeight); Y++)
                {
                    for (int32 X = 0; X < FMath::Min(TerrainWidth, GPUTextureWidth); X++)
                    {
                        int32 GPUIndex = Y * GPUTextureWidth + X;
                        int32 CPUIndex = Y * TerrainWidth + X;

                        if (GPUIndex < SurfaceData.Num() && CPUIndex < HeightMap.Num())
                        {
                            float GPUHeight = SurfaceData[GPUIndex].R.GetFloat();
                            if (!FMath::IsNaN(GPUHeight) && FMath::IsFinite(GPUHeight))
                            {
                                HeightMap[CPUIndex] = FMath::Clamp(GPUHeight, -10000.0f, 10000.0f);
                            }
                        }
                    }
                }
            });
        });
}


// ============================================================================
// SECTION 14: GPU TERRAIN - COMPUTE EXECUTION (~250 lines, 5%)
// ============================================================================
/**
 * Compute shader execution, parameter updates, brush system, external connections.
 */

// 14.1 SHADER EXECUTION

void ADynamicTerrain::ExecuteTerrainComputeShader(float DeltaTime)
{
    if (CurrentComputeMode != ETerrainComputeMode::GPU || !bGPUInitialized)
    {
        return;
    }

    // Double buffer validation
    if (!HeightRenderTexture_A || !HeightRenderTexture_B || !ErosionRenderTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("GPU textures not initialized: HeightA=%s, HeightB=%s, Erosion=%s"),
               HeightRenderTexture_A ? TEXT("Valid") : TEXT("Null"),
               HeightRenderTexture_B ? TEXT("Valid") : TEXT("Null"),
               ErosionRenderTexture ? TEXT("Valid") : TEXT("Null"));
        return;
    }

    // ========== SINGLE BUFFER MODE ==========
    // Using single buffer for simplicity - compute reads AND writes to same texture (A).
    // Double buffering caused issues: swap=vibration, copy=data loss.
    // If visual pulsing occurs during erosion, we can revisit with triple buffering.

    // ========== NO THROTTLING - RUN EVERY FRAME ==========
    // Removed 30Hz throttling to match water simulation rate (60Hz)
    // This eliminates visual "jumping" caused by terrain/water rate mismatch
    float ShaderDeltaTime = DeltaTime;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // ===== GET AUTHORITATIVE SHADER PARAMETERS ON GAME THREAD =====
    FVector4f TerrainParams;
    if (CachedMasterController)
    {
        TerrainParams = CachedMasterController->GetShaderTerrainParams();

        // CRITICAL: Validate dimensions match between authority and GPU texture
        int32 AuthWidth = (int32)TerrainParams.X;
        int32 AuthHeight = (int32)TerrainParams.Y;
        if (AuthWidth != GPUTextureWidth || AuthHeight != GPUTextureHeight)
        {
            UE_LOG(LogTemp, Error, TEXT("DIMENSION MISMATCH! Shader expects %dx%d but GPU texture is %dx%d"),
                   AuthWidth, AuthHeight, GPUTextureWidth, GPUTextureHeight);
            // Force shader to use actual GPU texture dimensions
            TerrainParams = FVector4f(GPUTextureWidth, GPUTextureHeight, TerrainParams.Z, TerrainParams.W);
        }
    }
    else
    {
        // Fallback if MasterController not available
        TerrainParams = FVector4f(GPUTextureWidth, GPUTextureHeight, TerrainScale, CurrentTime);
        UE_LOG(LogTemp, Warning, TEXT("ExecuteTerrainComputeShader: No MasterController, using fallback params"));
    }
    
    ENQUEUE_RENDER_COMMAND(TerrainComputeCommand)(
        [this, ShaderDeltaTime, TerrainParams](FRHICommandListImmediate& RHICmdList)
        {
            // ONE-TIME LOG: Confirm shader is dispatching
            static bool bLoggedOnce = false;
            if (!bLoggedOnce && bEnableGPUErosion)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Erosion shader dispatching (logged once)"));
                bLoggedOnce = true;
            }
            
            TShaderMapRef<FTerrainComputeCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            if (!ComputeShader.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("  Terrain compute shader not valid!"));
                return;
            }
            
            FRDGBuilder GraphBuilder(RHICmdList);
            
            // ===== SINGLE BUFFER MODE =====
            // Using HeightReadTexture (A) for both read AND write.
            // This is simpler and avoids sync issues from double buffering.
            // May have some visual artifacts during erosion due to read-write overlap,
            // but avoids the vibration/data-loss issues from buffer swapping/copying.

            // Register single buffer for both UAV (write) and SRV (read)
            FTextureRenderTargetResource* HeightResource = HeightReadTexture->GetRenderTargetResource();
            TRefCountPtr<IPooledRenderTarget> PooledHeightTexture = CreateRenderTarget(
                HeightResource->GetRenderTargetTexture(), TEXT("HeightTextureSingle"));
            FRDGTextureRef HeightTextureRDG = GraphBuilder.RegisterExternalTexture(PooledHeightTexture);

            // For single buffer, both UAV and SRV point to same texture
            // The SRV will read slightly stale data from neighboring pixels (race condition)
            // but this is more stable than the buffer alternation artifacts
            FRDGTextureRef HeightTextureInputRDG = HeightTextureRDG;

            // ===== REGISTER EROSION TEXTURE (OUTPUT) =====
            FTextureRenderTargetResource* ErosionResource = ErosionRenderTexture->GetRenderTargetResource();
            TRefCountPtr<IPooledRenderTarget> PooledErosionTexture = CreateRenderTarget(
                ErosionResource->GetRenderTargetTexture(), TEXT("ErosionTexture"));
            FRDGTextureRef ErosionTextureRDG = GraphBuilder.RegisterExternalTexture(PooledErosionTexture);
            
            // Allocate shader parameters
            FTerrainComputeCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FTerrainComputeCS::FParameters>();
            
            PassParameters->HeightTexture = GraphBuilder.CreateUAV(HeightTextureRDG);  // Write buffer (output)
            PassParameters->HeightTextureInput = GraphBuilder.CreateSRV(HeightTextureInputRDG);  // Read buffer (input)
            PassParameters->TerrainParams = TerrainParams;  // Use captured authoritative params
            
            // ===== WATER DATA CONNECTION FOR EROSION =====
            FRDGTextureRef WaterDepthRDG = nullptr;
            FRDGTextureRef FlowVelocityRDG = nullptr;
            
            // Try to get real water textures if water system is connected
            if (bEnableGPUErosion && ConnectedWaterSystem &&
                ConnectedWaterSystem->ErosionWaterDepthRT &&
                ConnectedWaterSystem->ErosionFlowVelocityRT)
            {
                // Register real water depth texture
                FTextureRenderTargetResource* DepthResource =
                    ConnectedWaterSystem->ErosionWaterDepthRT->GetRenderTargetResource();
                if (DepthResource)
                {
                    TRefCountPtr<IPooledRenderTarget> DepthPool = CreateRenderTarget(
                        DepthResource->GetRenderTargetTexture(),
                        TEXT("RealWaterDepth"));
                    WaterDepthRDG = GraphBuilder.RegisterExternalTexture(DepthPool);
                }
                
                // Register real flow velocity texture
                FTextureRenderTargetResource* VelocityResource =
                    ConnectedWaterSystem->ErosionFlowVelocityRT->GetRenderTargetResource();
                if (VelocityResource)
                {
                    TRefCountPtr<IPooledRenderTarget> VelocityPool = CreateRenderTarget(
                        VelocityResource->GetRenderTargetTexture(),
                        TEXT("RealFlowVelocity"));
                    FlowVelocityRDG = GraphBuilder.RegisterExternalTexture(VelocityPool);
                }
                
                static bool bLoggedRealWater = false;
                if (!bLoggedRealWater)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GPU Erosion: Using REAL water data"));
                    bLoggedRealWater = true;
                }
            }
            
            // Create dummy textures if water data not available
            FRDGTextureDesc DummyDesc = FRDGTextureDesc::Create2D(
                FIntPoint(GPUTextureWidth, GPUTextureHeight),
                PF_R32_FLOAT,
                FClearValueBinding::Black,
                TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable
            );
            
            FRDGTextureRef DummyTexture = GraphBuilder.CreateTexture(DummyDesc, TEXT("DummyWater"));
            AddClearRenderTargetPass(GraphBuilder, DummyTexture);
            
            // Use real water textures if available, otherwise use dummy
            if (!WaterDepthRDG)
            {
                WaterDepthRDG = DummyTexture;
            }
            if (!FlowVelocityRDG)
            {
                FlowVelocityRDG = DummyTexture;
            }
            
            // ===== WIND DATA CONNECTION FOR OROGRAPHIC EFFECTS =====
            FRDGTextureRef WindFieldRDG = nullptr;
            FRDGTextureRef MoistureRDG = nullptr;

            // Try to get real wind texture if atmosphere system is connected
            if (bEnableOrographicEffects && ConnectedAtmosphere &&
                ConnectedAtmosphere->WindFieldTexture)
            {
                FTextureRenderTargetResource* WindResource =
                    ConnectedAtmosphere->WindFieldTexture->GetRenderTargetResource();
                if (WindResource)
                {
                    TRefCountPtr<IPooledRenderTarget> WindPool = CreateRenderTarget(
                        WindResource->GetRenderTargetTexture(),
                        TEXT("RealWindField"));
                    WindFieldRDG = GraphBuilder.RegisterExternalTexture(WindPool);
                }

                // Also get moisture texture if available
                if (ConnectedAtmosphere->MoistureTexture)
                {
                    FTextureRenderTargetResource* MoistureResource =
                        ConnectedAtmosphere->MoistureTexture->GetRenderTargetResource();
                    if (MoistureResource)
                    {
                        TRefCountPtr<IPooledRenderTarget> MoisturePool = CreateRenderTarget(
                            MoistureResource->GetRenderTargetTexture(),
                            TEXT("RealMoisture"));
                        MoistureRDG = GraphBuilder.RegisterExternalTexture(MoisturePool);
                    }
                }

                static bool bLoggedRealWind = false;
                if (!bLoggedRealWind)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GPU Erosion: Using REAL wind/moisture data"));
                    bLoggedRealWind = true;
                }
            }

            // Use real textures if available, otherwise use dummy
            if (!WindFieldRDG)
            {
                WindFieldRDG = DummyTexture;
            }
            if (!MoistureRDG)
            {
                MoistureRDG = DummyTexture;
            }

            // Bind shader resources
            PassParameters->WaterDepthTexture = GraphBuilder.CreateSRV(WaterDepthRDG);
            PassParameters->FlowVelocityTexture = GraphBuilder.CreateSRV(FlowVelocityRDG);
            PassParameters->MoistureTexture = GraphBuilder.CreateSRV(MoistureRDG);
            PassParameters->WindFieldTexture = GraphBuilder.CreateSRV(WindFieldRDG);
            PassParameters->HardnessTexture = GraphBuilder.CreateSRV(DummyTexture);
            PassParameters->ErosionOutputTexture = GraphBuilder.CreateUAV(ErosionTextureRDG);
            
            PassParameters->TextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
            PassParameters->DeltaTime = ShaderDeltaTime;  // Use accumulated time for frame-rate independence
            
            // Set simulation mode based on enabled features
            uint32 SimMode = 0;
            if (bEnableGPUErosion) SimMode |= 1;
            if (bEnableOrographicEffects) SimMode |= 2;
            PassParameters->SimulationMode = SimMode;

            // DEBUG: Log simulation mode once
            static bool bLoggedSimMode = false;
            if (!bLoggedSimMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("GPU Compute SimMode=%d (Erosion=%d, Orographic=%d, Brush=%d)"),
                       SimMode, bEnableGPUErosion ? 1 : 0, bEnableOrographicEffects ? 1 : 0, bHasPendingBrush ? 1 : 0);
                bLoggedSimMode = true;
            }
            
            // Set erosion parameters
            PassParameters->ErosionParams = FVector4f(
                GPUErosionRate,
                GPUDepositionRate,
                1.0f,  // Erosion capacity multiplier
                1.0f   // Hardness multiplier
            );
            
            // Set orographic parameters
            PassParameters->OrographicParams = FVector4f(
                OrographicLiftStrength,
                MoistureCondensationThreshold,
                0.01f, // Precipitation rate
                0.0f   // Unused
            );
            
            // Handle brush operations
            PassParameters->BrushParams = PendingBrushParams;
            PassParameters->BrushActive = bHasPendingBrush ? 1 : 0;

            // DEBUG: Log brush params when active
            if (bHasPendingBrush)
            {
                UE_LOG(LogTemp, Warning,
                       TEXT("SHADER BRUSH ACTIVE: Pos(%.1f,%.1f) Radius=%.1f Strength=%.1f"),
                       PendingBrushParams.X, PendingBrushParams.Y, PendingBrushParams.Z, PendingBrushParams.W);
            }

            UE_LOG(LogTemp, VeryVerbose,
                   TEXT("Shader Params: Dims(%.0f,%.0f) Scale=%.2f BrushActive=%d"),
                   TerrainParams.X, TerrainParams.Y, TerrainParams.Z, bHasPendingBrush ? 1 : 0);
            
            // Dispatch compute shader
            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("TerrainComputeShader"),
                ComputeShader,
                PassParameters,
                FIntVector(
                    FMath::DivideAndRoundUp(GPUTextureWidth, 8),
                    FMath::DivideAndRoundUp(GPUTextureHeight, 8),
                    1
                )
            );
            
            // Execute the graph
            GraphBuilder.Execute();

            // SINGLE BUFFER MODE: No copy needed - compute wrote directly to HeightReadTexture

            // Clear brush flag after execution
            if (bHasPendingBrush)
            {
                bHasPendingBrush = false;
            }
        }
    );

    // CRITICAL FIX: Mark GPU data as dirty so visual sync happens
    bGPUDataDirty = true;
}


// 14.2 PARAMETER UPDATES & BRUSH

void ADynamicTerrain::UpdateGPUShaderParameters(float DeltaTime)
{
    if (!HeightRenderTexture) return;
    
    float Time = GetWorld()->GetTimeSeconds();
    
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent)
        {
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                Chunk.MeshComponent->GetMaterial(0)
            );
            
            if (DynMaterial)
            {
                DynMaterial->SetScalarParameterValue(FName("Time"), Time);
                DynMaterial->SetTextureParameterValue(
                    FName("HeightmapTexture"),
                    HeightRenderTexture
                );
            }
        }
    }
}


void ADynamicTerrain::UpdateGPUBrush(FVector WorldPosition, float Radius, float Strength, bool bRaise)
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateGPUBrush CALLED: Pos=%s, Radius=%.1f"),
               *WorldPosition.ToString(), Radius);
    
    if (CurrentComputeMode != ETerrainComputeMode::GPU || !bGPUInitialized)
        return;
    
    // ===== CRITICAL FIX: Use MasterController as single source of truth =====
    if (!CachedMasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateGPUBrush: No MasterController - cannot transform coordinates!"));
        return;
    }
    
    // Get authoritative coordinate transformation from MasterController
    FVector2D TextureCoords;
    float RadiusInTexels;
    CachedMasterController->WorldBrushToTextureSpace(
        WorldPosition,
        Radius,
        TextureCoords,
        RadiusInTexels
    );
    
    // Match CPU's strength scaling for large brushes
    float AdjustedStrength = Strength;
    if (Radius > 1000.0f)
    {
        AdjustedStrength *= (Radius / 500.0f);
    }
    
    // Store brush parameters in texture space (NOT world space!)
    PendingBrushParams = FVector4f(
        TextureCoords.X,        // X in texture coordinates
        TextureCoords.Y,        // Y in texture coordinates
        RadiusInTexels,         // Radius in texture space
        bRaise ? AdjustedStrength : -AdjustedStrength
    );
    
    bHasPendingBrush = true;
    
    UE_LOG(LogTemp, Log,
           TEXT("GPU Brush: Texture(%.2f, %.2f) Radius=%.2f texels Strength=%.1f"),
           TextureCoords.X, TextureCoords.Y, RadiusInTexels, AdjustedStrength);
    
    // Execute compute shader immediately
    // CRITICAL: Pass non-zero DeltaTime for brush - shader multiplies HeightChange by DeltaTime!
    // Using 1.0 means brush strength is applied directly
    float BrushDeltaTime = 1.0f;

    ExecuteTerrainComputeShader(BrushDeltaTime);

    // Wait for render thread to complete shader execution
    FlushRenderingCommands();

    // Sync GPU to CPU for terrain modifications (async)
    SyncGPUToCPU();
}


// 14.3 EXTERNAL CONNECTIONS

void ADynamicTerrain::ConnectToGPUAtmosphere(UAtmosphericSystem* AtmoSys)
{
    ConnectedAtmosphere = AtmoSys;
    
    if (ConnectedAtmosphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("Connected terrain to atmospheric system"));
    }
}


void ADynamicTerrain::ConnectToGPUWaterSystem(UWaterSystem* WaterSys)
{
    if (!WaterSys)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot connect to null water system"));
        return;
    }
    
    ConnectedWaterSystem = WaterSys;
    
    // Let water system know about the terrain
    WaterSys->ConnectToGPUTerrain(this);
    
    // Verify erosion textures were created
    if (bEnableGPUErosion)
    {
        if (!WaterSys->ErosionWaterDepthRT || !WaterSys->ErosionFlowVelocityRT)
        {
            UE_LOG(LogTemp, Error, TEXT("  CRITICAL: Erosion textures not created after connection!"));
            UE_LOG(LogTemp, Error, TEXT("   Water may need to be initialized first"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  Erosion textures verified: %dx%d"),
                   WaterSys->ErosionWaterDepthRT->SizeX,
                   WaterSys->ErosionWaterDepthRT->SizeY);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("  Terrain connected to GPU water system"));
}




// ============================================================================
// SECTION 15: GPU TERRAIN - VISUAL UPDATES (~400 lines, 9%)
// ============================================================================
/**
 * Chunk material updates, visual synchronization, displacement mapping.
 */

// 15.1 CHUNK MATERIAL UPDATES

void ADynamicTerrain::UpdateGPUChunkMaterialParams(UMaterialInstanceDynamic* DynMaterial,
                                                   const FTerrainChunk& Chunk)
{
    if (!DynMaterial || !HeightRenderTexture) return;

    // Bind current height texture - the stable read buffer (not being written to)
    DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightReadTexture);

    // NOTE: We do NOT set HeightmapTexturePrev because with only 2 buffers,
    // HeightWriteTexture is actively being modified by compute during the frame.
    // Material-side interpolation requires 3 buffers to work correctly.

    // Blend factor always 1.0 - use current buffer fully, no interpolation
    DynMaterial->SetScalarParameterValue(FName("HeightBlendFactor"), 1.0f);

    // CRITICAL: Correct UV mapping that matches CPU chunk layout exactly
    int32 TerrainStartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 TerrainStartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);

    // UV coordinates map chunk's local UVs (0-1) to global heightmap UVs
    // CRITICAL: Use (ChunkSize-1) for scale so the LAST vertex maps to the correct texel
    // Without this, chunk boundaries sample different heightmap locations = visible seams
    float UVStartX = (float)TerrainStartX / (float)TerrainWidth;
    float UVStartY = (float)TerrainStartY / (float)TerrainHeight;
    float UVScaleX = (float)(ChunkSize - 1) / (float)TerrainWidth;
    float UVScaleY = (float)(ChunkSize - 1) / (float)TerrainHeight;

    // Split UV transform into two Float2 parameters for easier material setup
    DynMaterial->SetVectorParameterValue(
        FName("ChunkUVStart"),
        FLinearColor(UVStartX, UVStartY, 0, 0)
    );
    DynMaterial->SetVectorParameterValue(
        FName("ChunkUVScale"),
        FLinearColor(UVScaleX, UVScaleY, 0, 0)
    );

    // Terrain dimensions for height sampling and normal calculation
    // xy = texture dimensions (for texel size), z = terrain scale, w = max height
    DynMaterial->SetVectorParameterValue(
        FName("TerrainParams"),
        FLinearColor(TerrainWidth, TerrainHeight, TerrainScale, MaxTerrainHeight)
    );

    // Texel size in UV space - needed for normal calculation
    // Material samples neighboring texels to compute gradient
    float TexelSizeU = 1.0f / (float)TerrainWidth;
    float TexelSizeV = 1.0f / (float)TerrainHeight;
    DynMaterial->SetVectorParameterValue(
        FName("TexelSize"),
        FLinearColor(TexelSizeU, TexelSizeV, 0, 0)
    );

    // Terrain world bounds for world position calculations
    FVector TerrainOrigin = GetActorLocation();
    float TerrainWorldSizeX = TerrainWidth * TerrainScale;
    float TerrainWorldSizeY = TerrainHeight * TerrainScale;
    DynMaterial->SetVectorParameterValue(
        FName("TerrainWorldBounds"),
        FLinearColor(TerrainOrigin.X, TerrainOrigin.Y, TerrainWorldSizeX, TerrainWorldSizeY)
    );

    // Flag for GPU rendering mode
    DynMaterial->SetScalarParameterValue(FName("UseGPUHeightmap"), bUseGPUHeightmapRendering ? 1.0f : 0.0f);

    DynMaterial->SetScalarParameterValue(FName("ChunkSize"), ChunkSize);
    DynMaterial->SetScalarParameterValue(FName("ChunkOverlap"), ChunkOverlap);

    // Force immediate update
    DynMaterial->SetScalarParameterValue(FName("UpdateTime"), GetWorld()->GetTimeSeconds());

    // Water-specific height texture - updated every frame for continuous water visuals
    // Terrain uses HeightmapTexture (stable buffer, no interpolation with 2-buffer system)
    // Water uses WaterHeightTexture for smoother flow without double-buffer swap artifacts
    if (WaterHeightTexture)
    {
        DynMaterial->SetTextureParameterValue(FName("WaterHeightTexture"), WaterHeightTexture);
    }

    // Water integration if needed
    if (WaterSystem && WaterSystem->bUseShaderWater)
    {
        WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
    }

    // Hardness texture for debug visualization and material effects
    if (HardnessRenderTexture)
    {
        DynMaterial->SetTextureParameterValue(FName("HardnessTexture"), HardnessRenderTexture);
    }

    // Debug visualization flag - when enabled, material blends hardness colors over terrain
    DynMaterial->SetScalarParameterValue(FName("ShowHardnessDebug"), bShowHardnessDebug ? 1.0f : 0.0f);
}


void ADynamicTerrain::UpdateGPUChunkMaterial(FTerrainChunk& Chunk)
{
    UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
        Chunk.MeshComponent->GetMaterial(0)
    );

    if (!DynMaterial && CurrentActiveMaterial)
    {
        DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
        Chunk.MeshComponent->SetMaterial(0, DynMaterial);
    }

    if (DynMaterial)
    {
        // Use the unified parameter update function
        UpdateGPUChunkMaterialParams(DynMaterial, Chunk);
    }
}


// 15.2 VISUAL SYNCHRONIZATION

void ADynamicTerrain::ForceGPUChunkVisualUpdate(const TArray<int32>& ChunkIndices)
{
    if (!HeightRenderTexture) return;

    // Expand to include all neighbors
    TSet<int32> AllChunks;
    for (int32 ChunkIndex : ChunkIndices)
    {
        if (!TerrainChunks.IsValidIndex(ChunkIndex)) continue;

        const FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];

        // Add a 2-chunk radius around each affected chunk for large brushes
        int32 Radius = (ChunkIndices.Num() > 15) ? 2 : 1;

        for (int32 dy = -Radius; dy <= Radius; dy++)
        {
            for (int32 dx = -Radius; dx <= Radius; dx++)
            {
                int32 NX = Chunk.ChunkX + dx;
                int32 NY = Chunk.ChunkY + dy;
                if (NX >= 0 && NX < ChunksX && NY >= 0 && NY < ChunksY)
                {
                    AllChunks.Add(NY * ChunksX + NX);
                }
            }
        }
    }

    // Update all chunks using unified parameter function
    for (int32 ChunkIndex : AllChunks)
    {
        if (TerrainChunks.IsValidIndex(ChunkIndex))
        {
            FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
            if (Chunk.MeshComponent)
            {
                // Get or create material
                UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                    Chunk.MeshComponent->GetMaterial(0));

                if (!DynMaterial && CurrentActiveMaterial)
                {
                    DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
                    Chunk.MeshComponent->SetMaterial(0, DynMaterial);
                }

                if (DynMaterial)
                {
                    // Use unified parameter update function
                    UpdateGPUChunkMaterialParams(DynMaterial, Chunk);

                    // Force mesh to update visuals
                    Chunk.MeshComponent->MarkRenderStateDirty();
                    Chunk.MeshComponent->UpdateBounds();
                }
            }
        }
    }
}

// Helper function to update material parameters (thread-safe)

void ADynamicTerrain::SyncGPUChunkVisuals()
{
    if (!HeightRenderTexture) return;

    // Use the consolidated UpdateGPUChunkMaterialParams function
    // This ensures consistent parameters across all update paths
    for (FTerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.MeshComponent && Chunk.bIsVisible)
        {
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(
                Chunk.MeshComponent->GetMaterial(0)
            );

            if (!DynMaterial && CurrentActiveMaterial)
            {
                DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
                Chunk.MeshComponent->SetMaterial(0, DynMaterial);
            }

            if (DynMaterial)
            {
                // Use the unified parameter update function
                UpdateGPUChunkMaterialParams(DynMaterial, Chunk);
            }
        }
    }
}


// 15.3 GPU VALIDATION



// ============================================================================
// SECTION 16: DEBUGGING & VALIDATION (~250 lines, 5%)
// ============================================================================
/**
 * Debug console commands, performance stats, GPU/authority/erosion debugging.
 */

// 16.1 EROSION DEBUGGING

void ADynamicTerrain::DebugErosion()
{
    UE_LOG(LogTemp, Log, TEXT("=== EROSION DEBUG ==="));
    
    // Check 1: Is system ready?
    UE_LOG(LogTemp, Warning, TEXT("1. GPU Mode: %s"),
           CurrentComputeMode == ETerrainComputeMode::GPU ? TEXT("  YES") : TEXT("  NO (CPU mode)"));
    UE_LOG(LogTemp, Warning, TEXT("2. Erosion Enabled: %s"),
           bEnableGPUErosion ? TEXT("  YES") : TEXT("  NO"));
    
    // Check 2: Water connection
    bool bWaterConnected = (ConnectedWaterSystem != nullptr);
    bool bWaterTexturesExist = bWaterConnected &&
                               ConnectedWaterSystem->ErosionWaterDepthRT &&
                               ConnectedWaterSystem->ErosionFlowVelocityRT;
    
    UE_LOG(LogTemp, Warning, TEXT("3. Water System: %s"),
           bWaterConnected ? TEXT("  Connected") : TEXT("  Not Connected"));
    UE_LOG(LogTemp, Warning, TEXT("4. Water Textures: %s"),
           bWaterTexturesExist ? TEXT("  Exist") : TEXT("  Missing"));
    
    // Check 3: Water flow (if connected)
    if (bWaterConnected && ConnectedWaterSystem->SimulationData.IsValid())
    {
        int32 WaterCells = 0;
        float MaxFlow = 0.0f;
        
        for (int32 i = 0; i < ConnectedWaterSystem->SimulationData.WaterDepthMap.Num(); i++)
        {
            if (ConnectedWaterSystem->SimulationData.WaterDepthMap[i] > 0.01f)
            {
                WaterCells++;
                float VelX = ConnectedWaterSystem->SimulationData.WaterVelocityX[i];
                float VelY = ConnectedWaterSystem->SimulationData.WaterVelocityY[i];
                float Flow = FMath::Sqrt(VelX * VelX + VelY * VelY);
                MaxFlow = FMath::Max(MaxFlow, Flow);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("5. Water Cells: %d"), WaterCells);
        UE_LOG(LogTemp, Warning, TEXT("6. Max Flow Speed: %.2f m/s"), MaxFlow);
        
        if (WaterCells == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("      NO WATER - Add water to see erosion!"));
        }
        else if (MaxFlow < 0.1f)
        {
            UE_LOG(LogTemp, Warning, TEXT("      WATER NOT FLOWING - Needs slope or more water!"));
        }
    }
    
    // Check 4: Erosion parameters
    UE_LOG(LogTemp, Warning, TEXT("7. Erosion Rate: %.3f (default 0.1)"), GPUErosionRate);
    
    if (GPUErosionRate < 0.01f)
    {
        UE_LOG(LogTemp, Warning, TEXT("      TOO LOW - Try 0.5 or 1.0 for visible erosion"));
    }
    
    // Final verdict
    UE_LOG(LogTemp, Warning, TEXT(""));
    if (CurrentComputeMode != ETerrainComputeMode::GPU)
    {
        UE_LOG(LogTemp, Error, TEXT("  EROSION CANNOT WORK - Switch to GPU mode!"));
    }
    else if (!bEnableGPUErosion)
    {
        UE_LOG(LogTemp, Error, TEXT("  EROSION DISABLED - Enable bEnableGPUErosion"));
    }
    else if (!bWaterTexturesExist)
    {
        UE_LOG(LogTemp, Error, TEXT("  WATER TEXTURES MISSING - Need to create them!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT(" EROSION SHOULD BE WORKING!"));
        UE_LOG(LogTemp, Warning, TEXT("   Wait 30-60 seconds and check for terrain changes."));
        UE_LOG(LogTemp, Log, TEXT("   Increase GPUErosionRate to 1.0 for faster results."));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("===================="));
}

// 16.2 GPU DEBUGGING


// 16.3 AUTHORITY DEBUGGING

void ADynamicTerrain::DebugAuthority()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN AUTHORITY DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("bUseGPUTerrain: %s"), bUseGPUTerrain ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("bGPUInitialized: %s"), bGPUInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("bPendingGPUInit: %s"), bPendingGPUInit ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("bEnableGPUErosion: %s"), bEnableGPUErosion ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("CurrentComputeMode: %s"),
           CurrentComputeMode == ETerrainComputeMode::GPU ? TEXT("GPU") :
           CurrentComputeMode == ETerrainComputeMode::CPU ? TEXT("CPU") : TEXT("Switching"));
    UE_LOG(LogTemp, Warning, TEXT("bCPUHasAuthority: %s"), bCPUHasAuthority ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("bGPUDataDirty: %s"), bGPUDataDirty ? TEXT("YES") : TEXT("NO"));

    // GPU Resources
    UE_LOG(LogTemp, Warning, TEXT("--- GPU Resources ---"));
    UE_LOG(LogTemp, Warning, TEXT("HeightRenderTexture: %s"),
           HeightRenderTexture ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("ErosionRenderTexture: %s"),
           ErosionRenderTexture ? TEXT("Valid") : TEXT("NULL"));

    // Water connection
    UE_LOG(LogTemp, Warning, TEXT("--- Water Connection ---"));
    UE_LOG(LogTemp, Warning, TEXT("ConnectedWaterSystem: %s"),
           ConnectedWaterSystem ? TEXT("Connected") : TEXT("NULL"));
    if (ConnectedWaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("  ErosionWaterDepthRT: %s"),
               ConnectedWaterSystem->ErosionWaterDepthRT ? TEXT("Valid") : TEXT("NULL"));
        UE_LOG(LogTemp, Warning, TEXT("  ErosionFlowVelocityRT: %s"),
               ConnectedWaterSystem->ErosionFlowVelocityRT ? TEXT("Valid") : TEXT("NULL"));
    }

    // Atmosphere connection
    UE_LOG(LogTemp, Warning, TEXT("--- Atmosphere Connection ---"));
    UE_LOG(LogTemp, Warning, TEXT("ConnectedAtmosphere: %s"),
           ConnectedAtmosphere ? TEXT("Connected") : TEXT("NULL"));
    if (ConnectedAtmosphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("  WindFieldTexture: %s"),
               ConnectedAtmosphere->WindFieldTexture ? TEXT("Valid") : TEXT("NULL"));
    }

    // Sample heights to verify terrain exists
    UE_LOG(LogTemp, Warning, TEXT("--- Terrain Data ---"));
    if (HeightMap.Num() > 0)
    {
        float MinH = FLT_MAX, MaxH = -FLT_MAX;
        for (const float& H : HeightMap)
        {
            if (!FMath::IsNaN(H))
            {
                MinH = FMath::Min(MinH, H);
                MaxH = FMath::Max(MaxH, H);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("CPU Heights: Min=%.2f, Max=%.2f, Range=%.2f"),
               MinH, MaxH, MaxH - MinH);

        if (MaxH - MinH < 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("WARNING: CPU terrain appears flat!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CPU HeightMap is EMPTY!"));
    }

    UE_LOG(LogTemp, Warning, TEXT("============================"));
}

void ADynamicTerrain::ForceGPUMode()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FORCING GPU MODE ==="));

    // CRITICAL: Validate HeightMap before attempting GPU mode
    int32 ExpectedSize = TerrainWidth * TerrainHeight;
    if (HeightMap.Num() < ExpectedSize || ExpectedSize <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot force GPU mode - HeightMap has %d elements, need %d!"),
               HeightMap.Num(), ExpectedSize);
        UE_LOG(LogTemp, Error, TEXT("Terrain must be fully initialized first."));
        return;
    }

    // Validate HeightMap has actual terrain data
    float MinH = TNumericLimits<float>::Max();
    float MaxH = TNumericLimits<float>::Lowest();
    for (int32 i = 0; i < FMath::Min(1000, HeightMap.Num()); i++)
    {
        MinH = FMath::Min(MinH, HeightMap[i]);
        MaxH = FMath::Max(MaxH, HeightMap[i]);
    }
    float HeightRange = MaxH - MinH;

    if (HeightRange < 1.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot force GPU mode - HeightMap appears flat (range: %.2f)!"),
               HeightRange);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("HeightMap validated: %d elements, range: %.2f"), HeightMap.Num(), HeightRange);

    // Step 1: Enable GPU terrain flag
    bUseGPUTerrain = true;

    // Enable all GPU terrain effects
    bEnableGPUErosion = true;
    bEnableOrographicEffects = true;
    UE_LOG(LogTemp, Warning, TEXT("GPU effects ENABLED - erosion=%d, orographic=%d"),
           bEnableGPUErosion ? 1 : 0, bEnableOrographicEffects ? 1 : 0);

    // Step 2: Initialize GPU if not done
    if (!bGPUInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Initializing GPU resources..."));
        InitializeGPUTerrainWithData();
    }

    // Step 3: Switch compute mode (only if init succeeded)
    if (bGPUInitialized)
    {
        CurrentComputeMode = ETerrainComputeMode::GPU;
        bCPUHasAuthority = false;
        UE_LOG(LogTemp, Warning, TEXT("Switched to GPU mode!"));

        // Force upload CPU data to GPU
        TransferHeightmapToGPU();
        FlushRenderingCommands();

        // Connect water system if available
        if (WaterSystem && !ConnectedWaterSystem)
        {
            ConnectToGPUWaterSystem(WaterSystem);
        }

        // Connect atmosphere if available
        if (AtmosphericSystem && !ConnectedAtmosphere)
        {
            ConnectToGPUAtmosphere(AtmosphericSystem);
        }

        UE_LOG(LogTemp, Warning, TEXT("GPU Mode ACTIVE - Run DebugAuthority to verify"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize GPU resources!"));
        bUseGPUTerrain = false;
        bEnableGPUErosion = false;
    }
}

void ADynamicTerrain::ForceCPUMode()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FORCING CPU MODE ==="));

    // Sync any GPU changes back first
    if (bGPUInitialized && CurrentComputeMode == ETerrainComputeMode::GPU)
    {
        SyncGPUToCPU();
        FlushRenderingCommands();
    }

    // Switch to CPU mode
    CurrentComputeMode = ETerrainComputeMode::CPU;
    bCPUHasAuthority = true;
    bUseGPUTerrain = false;

    // Queue all chunks for update
    for (int32 i = 0; i < TerrainChunks.Num(); i++)
    {
        PendingChunkUpdates.Add(i);
    }

    UE_LOG(LogTemp, Warning, TEXT("CPU Mode ACTIVE"));
}

void ADynamicTerrain::ToggleGPUErosion()
{
    bEnableGPUErosion = !bEnableGPUErosion;
    UE_LOG(LogTemp, Warning, TEXT("=== GPU EROSION %s ==="), bEnableGPUErosion ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Warning, TEXT("Current state: Erosion=%d, Orographic=%d, ComputeMode=%s"),
           bEnableGPUErosion ? 1 : 0,
           bEnableOrographicEffects ? 1 : 0,
           CurrentComputeMode == ETerrainComputeMode::GPU ? TEXT("GPU") : TEXT("CPU"));
}

void ADynamicTerrain::ToggleGPUOrographic()
{
    bEnableOrographicEffects = !bEnableOrographicEffects;
    UE_LOG(LogTemp, Warning, TEXT("=== GPU OROGRAPHIC %s ==="), bEnableOrographicEffects ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Warning, TEXT("Current state: Erosion=%d, Orographic=%d, ComputeMode=%s"),
           bEnableGPUErosion ? 1 : 0,
           bEnableOrographicEffects ? 1 : 0,
           CurrentComputeMode == ETerrainComputeMode::GPU ? TEXT("GPU") : TEXT("CPU"));
}
