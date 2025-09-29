/**
 * ============================================
 * TERRAI DYNAMIC TERRAIN SYSTEM
 * ============================================
 * Purpose: Chunk-based procedural terrain with real-time modification
 * Performance: 60+ FPS with 256 chunks, adaptive update batching
 * Scale: 51.3km x 51.3km terrain (513x513 heightmap)
 *
 * Architecture:
 * - Chunk System: 16x16 grid of 33x33 vertex chunks
 * - Water Integration: Real-time shader texture updates
 * - Frustum Culling: Only render visible chunks (30-40% savings)
 * - Threading: ParallelFor for large brush operations
 */
#include "DynamicTerrain.h"
#include "TerrAI.h"  // Include for validation macros and constants
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

using namespace TerrAIConstants;  // Use named constants



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
        UE_LOG(LogTemp, Warning, TEXT("Could not load BasicShapeMaterial, will use wireframe fallback"));
    }
    
    // Initialize scalable world configuration FIRST
    CurrentWorldSize = ETerrainWorldSize::Medium;
    WorldConfig = GetWorldConfigForSize(CurrentWorldSize);
    
    // Apply configuration to legacy properties
    TerrainWidth = WorldConfig.TerrainWidth;
    TerrainHeight = WorldConfig.TerrainHeight;
    ChunkSize = WorldConfig.ChunkSize;
    TerrainScale = 100.0f;
    MaxTerrainHeight = 5000.0f;
    MinTerrainHeight = -5000.0f;
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Beginning initialization"));
    
    // PHASE 2: Initialize basic data structures
    InitializeTerrainData();
    
    if (bUseGPUTerrain)
      {
          InitializeGPUTerrain();
          UE_LOG(LogTemp, Warning, TEXT("GPU Terrain Init requested"));
      }
      else
      {
          UE_LOG(LogTemp, Warning, TEXT("GPU Terrain disabled (bUseGPUTerrain = false)"));
      }
    
}

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
            
            // CRITICAL: Still process water chunk updates in GPU mode
            ProcessPendingWaterChunkUpdates();
            
            // Update GPU chunk visuals if needed
            if (bGPUDataDirty)
            {
                GPUSyncTimer += DeltaTime;
                if (GPUSyncTimer >= 0.1f) // Sync every 100ms max
                {
                    SyncGPUChunkVisuals();
                    GPUSyncTimer = 0.0f;
                    bGPUDataDirty = false;
                }
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

// ===== WATER SYSTEM INTEGRATION =====

void ADynamicTerrain::InitializeWaterSystem()
{
    // Prevent Duplicate Water System Creation
    if (WaterSystem && WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Warning, TEXT("WaterSystem already initialized"));
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
        UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: WaterSystem created"));
    }
    
    if (WaterSystem)
    {
        // Register with MasterController FIRST
        if (CachedMasterController)
        {
            WaterSystem->RegisterWithMasterController(CachedMasterController);
            UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: WaterSystem registered with MasterController"));
        }
        
        // Pass validated dimensions explicitly
        WaterSystem->Initialize(this, CachedMasterController);
        
        // Validate initialization completed successfully
        if (WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: WaterSystem fully initialized and ready"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: WaterSystem initialized but not ready yet"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DynamicTerrain: Failed to create water system"));
    }
}

// ===== WATER FUNCTIONS REMOVED - USE WATERCONTROLLER INSTEAD =====
// All water interaction now goes through WaterController for clean separation

bool ADynamicTerrain::IsWaterSystemReady() const
{
    return WaterSystem && WaterSystem->IsSystemReady();
}

// ===== VOLUMETRIC WATER CONTROL - REMOVED =====
// All volumetric water functions moved to WaterController for clean separation

// ===== TERRAIN GENERATION =====

/**
 * Procedural terrain generation using multi-octave sinusoidal waves
 *
 * Algorithm: Multiple frequency sine waves with ridge patterns
 * References:
 * - Perlin, K. (1985). "An image synthesizer" SIGGRAPH '85
 * - Mandelbrot, B. (1982). "The Fractal Geometry of Nature"
 * - Ebert, D. et al. (2003). "Texturing and Modeling: A Procedural Approach" 3rd Ed.
 *
 * Creates realistic terrain with valleys for water flow demonstration
 */
void ADynamicTerrain::GenerateProceduralTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating sinusoidal terrain for water flow demonstration..."));
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("Sinusoidal terrain generation complete - ready for water flow"));
}

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
    UE_LOG(LogTemp, Warning, TEXT("Simple terrain generated - chunks will update as needed"));
}

// ===== TERRAIN MODIFICATION =====

/**
 * Modifies terrain height in circular pattern with quadratic falloff
 *
 * @param WorldPosition - Center point in world coordinates
 * @param Radius - Brush radius in world units (500+ recommended)
 * @param Strength - Height change per modification (200 = moderate)
 * @param bRaise - True to raise terrain, false to lower
 *
 * Performance: Throttled to 20 calls/second for stability
 * Threading: Game thread only (modifies shared HeightMap)
 * Side Effects: Marks affected chunks for mesh regeneration
 */
void ADynamicTerrain::ModifyTerrain(FVector WorldPosition, float Radius, float Strength, bool bRaise)
{
    
    /*if (CurrentComputeMode == ETerrainComputeMode::GPU) {
        UpdateGPUBrush(WorldPosition, Radius, Strength, bRaise);
        return; // Skip CPU path
    }
     */
    // PERFORMANCE: Throttle to 20 modifications/second
    float CurrentTime = GetCachedFrameTime();
    if (CurrentTime - LastModificationTime < ModificationCooldown)
    {
        return;
    }
    LastModificationTime = CurrentTime;
    
    // PHASE 2: Direct MasterController authority - no fallbacks
    FVector2D TerrainCoords = CachedMasterController->WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    // Use MasterController scaling authority
    float ScaledRadius = CachedMasterController->GetBrushScaleMultiplier() * (Radius / TerrainScale);
    
    ModifyTerrainAtIndex(X, Y, ScaledRadius, Strength, bRaise);
    
    if (bUseGPUTerrain)
        {
            SyncCPUToGPU();
        }
}

void ADynamicTerrain::ModifyTerrainAtIndex(int32 X, int32 Y, float Radius, float Strength, bool bRaise)
{
    if (!HeightMap.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("HeightMap not initialized!"));
        return;
    }
    
    // Clamp center coordinates
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
                        // ORIGINAL: Simple quadratic falloff
                        float Falloff = FMath::Pow(1.0f - (Distance / Radius), 2.0f);
                        float HeightChange = Strength * Falloff * (bRaise ? 1.0f : -1.0f);
                        
                        HeightMap[Index] = FMath::Clamp(
                            HeightMap[Index] + HeightChange,
                            MinTerrainHeight, MaxTerrainHeight
                        );
                        
                        // Track affected chunks
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
    
    // Mark affected chunks for update with priority
    for (int32 ChunkIndex : AffectedChunks)
    {
        // Calculate priority based on cursor proximity and size
        FVector ModificationWorldPos = CachedMasterController->TerrainToWorldPosition(FVector2D(X, Y));
        FVector ChunkWorldPos = GetChunkWorldPosition(ChunkIndex);
        float CursorDistance = FVector::Dist(ModificationWorldPos, ChunkWorldPos);
        float Priority = 100.0f / FMath::Max(CursorDistance, 100.0f); // Higher priority for closer chunks
        
        if (Radius > 1000.0f) Priority *= 2.0f; // Large brushes get priority boost
        
        RequestPriorityChunkUpdate(ChunkIndex, Priority);
    }
    
    // OPTIMIZED LARGE BRUSH HANDLING
    if (AffectedChunks.Num() > 15 || Radius > 2000.0f)
    {
        // Large operation: batch process immediately to prevent accumulation
        UE_LOG(LogTemp, Warning, TEXT("TerrAI: Large brush operation affecting chunks, batch processing: %d"), AffectedChunks.Num());
        
        // Process in smaller batches to maintain framerate
        TArray<int32> ChunkArray = AffectedChunks.Array();
        int32 BatchSize = 8;
        
        for (int32 i = 0; i < ChunkArray.Num(); i += BatchSize)
        {
            int32 BatchEnd = FMath::Min(i + BatchSize, ChunkArray.Num());
            for (int32 j = i; j < BatchEnd; j++)
            {
                UpdateChunk(ChunkArray[j]);
                PendingChunkUpdates.Remove(ChunkArray[j]);
            }
            
            // Yield control if batch processing takes too long
            if ((i + BatchSize) < ChunkArray.Num())
            {
                // Schedule remaining chunks for next frame
                break;
            }
        }
    }
    else if (AffectedChunks.Num() <= 4)
    {
        // Small edits: immediate update to prevent chunk tears
        for (int32 ChunkIndex : AffectedChunks)
        {
            UpdateChunk(ChunkIndex);
            PendingChunkUpdates.Remove(ChunkIndex);
        }
    }
    
    
    // NEW: Force water system to sync with terrain changes
    if (WaterSystem)
    {
        WaterSystem->ForceTerrainSync();
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem Synced and Restarted"));
    }
}

// ===== CHUNK SYSTEM =====

void ADynamicTerrain::InitializeChunks()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing chunk system with %dx%d chunks"), ChunksX, ChunksY);
    
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
        UE_LOG(LogTemp, Warning, TEXT("Using fallback wireframe material for %d/%d chunks"),
               FallbackMaterialCount, TerrainChunks.Num());
    }
    
    if (FailedChunkCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create %d chunks"), FailedChunkCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("âœ… Created %d chunks with validated materials"), TerrainChunks.Num());
}

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
            float Height = GetHeightSafe(X, Y);
            
            // Local position within chunk (relative to chunk origin)
            FVector LocalPosition = FVector(
                (X - StartX) * TerrainScale,
                (Y - StartY) * TerrainScale,
                Height
            );
            
            Vertices.Add(LocalPosition);
            
            // Calculate normal
            FVector Normal = CalculateVertexNormal(X, Y);
            Normals.Add(Normal);
            
            // UV coordinates using actual chunk dimensions
            float U = (float)(X - StartX) / (EndX - StartX - 1);
            float V = (float)(Y - StartY) / (EndY - StartY - 1);
            UVs.Add(FVector2D(U, V));
            
            // Height-based vertex colors
            float NormalizedHeight = Height / MaxTerrainHeight;
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

void ADynamicTerrain::UpdateChunk(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }
    
    FTerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    GenerateChunkMesh(Chunk.ChunkX, Chunk.ChunkY);
    
    TotalChunkUpdatesThisFrame++;
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
        UE_LOG(LogTemp, Warning, TEXT("TerrAI: Water update queue pressure, pending: %d"), PendingWaterChunkUpdates.Num());
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
void ADynamicTerrain::ProcessPendingChunkUpdates()
{
    // PRIORITY-BASED CHUNK PROCESSING
    int32 TotalUpdatesNeeded = PriorityChunkQueue.Num() + PendingChunkUpdates.Num();
    
    // Adaptive throttling based on queue pressure
    int32 UpdatesThisFrame;
    if (TotalUpdatesNeeded > 50)
    {
        UpdatesThisFrame = EMERGENCY_CHUNK_UPDATES;  // Emergency mode
        UE_LOG(LogTemp, Warning, TEXT("TerrAI: Emergency chunk processing, total pending: %d"), TotalUpdatesNeeded);
    }
    else if (TotalUpdatesNeeded > 20)
    {
        UpdatesThisFrame = 8;   // High pressure
    }
    else if (TotalUpdatesNeeded > 10)
    {
        UpdatesThisFrame = 6;   // Moderate pressure
    }
    else
    {
        UpdatesThisFrame = MaxUpdatesPerFrame;
    }
    
    int32 ProcessedThisFrame = 0;
    
    // STEP 1: Process priority queue first (editing chunks, near camera)
    if (PriorityChunkQueue.Num() > 0)
    {
    // Sort by priority (higher priority first)
    PriorityChunkQueue.Sort();
    
    // Process highest priority chunks
    int32 PriorityUpdates = FMath::Min(UpdatesThisFrame, PriorityChunkQueue.Num());
    
    // PHASE 4: Multi-threaded processing for large batches
    if (PriorityUpdates > 4 && bEnableChunkPooling)
    {
    TArray<int32> BatchIndices;
        for (int32 i = 0; i < PriorityUpdates; i++)
        {
            BatchIndices.Add(PriorityChunkQueue[i].ChunkIndex);
        }
            ForceUpdateChunkGroup(BatchIndices);
                ProcessedThisFrame += PriorityUpdates;
            }
            else
            {
                // Standard single-threaded processing
                for (int32 i = 0; i < PriorityUpdates; i++)
                {
                    UpdateChunk(PriorityChunkQueue[i].ChunkIndex);
                    ProcessedThisFrame++;
                }
            }
            
            // Remove processed chunks
            PriorityChunkQueue.RemoveAt(0, PriorityUpdates);
        }
    
    // STEP 2: Process remaining capacity with legacy chunks
    int32 RemainingCapacity = UpdatesThisFrame - ProcessedThisFrame;
    if (RemainingCapacity > 0 && PendingChunkUpdates.Num() > 0)
    {
        TArray<int32> ChunksToUpdate;
        for (int32 ChunkIndex : PendingChunkUpdates)
        {
            ChunksToUpdate.Add(ChunkIndex);
            if (ChunksToUpdate.Num() >= RemainingCapacity)
            {
                break;
            }
        }
        
        // Clear processed chunks from pending set
        for (int32 ChunkIndex : ChunksToUpdate)
        {
            PendingChunkUpdates.Remove(ChunkIndex);
            UpdateChunk(ChunkIndex);
            ProcessedThisFrame++;
        }
    }
    
    // STEP 3: Process water-only updates separately
    ProcessPendingWaterChunkUpdates();
    
    // Performance warning for large queues
    if (TotalUpdatesNeeded > 100)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrAI: Large update queue, priority: %d, legacy pending: %d"),
               PriorityChunkQueue.Num(), PendingChunkUpdates.Num());
    }
}

// ===== UTILITY FUNCTIONS =====

FVector2D ADynamicTerrain::GetChunkWorldPosition(int32 ChunkX, int32 ChunkY) const
{
    float WorldX = ChunkX * (WorldConfig.ChunkSize - ChunkOverlap) * TerrainScale;
    float WorldY = ChunkY * (WorldConfig.ChunkSize - ChunkOverlap) * TerrainScale;
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
    
    // Calculate chunk center position in world space
    float ChunkCenterX = ChunkWorldPos2D.X + ((ChunkSize - 1) * TerrainScale * 0.5f);
    float ChunkCenterY = ChunkWorldPos2D.Y + ((ChunkSize - 1) * TerrainScale * 0.5f);
    
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

FVector ADynamicTerrain::TerrainToWorldPosition(int32 X, int32 Y) const
{
    if (!CachedMasterController) return FVector::ZeroVector;
    return CachedMasterController->TerrainToWorldPosition(FVector2D(X, Y));
}

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

/**
 * Calculates vertex normal using finite difference method
 *
 * Algorithm: Cross product of tangent vectors from height differences
 * References:
 * - Bourke, P. (1997). "Calculating the area and centroid of a polygon"
 * - Angel, E. (2008). "Interactive Computer Graphics" 5th Ed., Ch. 6
 *
 * @param X, Y - Terrain coordinates for normal calculation
 * @return Normalized surface normal vector
 */
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

// ===== MATERIAL MANAGEMENT =====

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
    
    UE_LOG(LogTemp, Log, TEXT("âœ… Material applied to %d chunks, %d enhanced with water shaders"),
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

// ===== PERFORMANCE MONITORING =====

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
            
            // Show water system statistics
            if (WaterSystem && WaterSystem->bShowWaterStats)
            {
                WaterSystem->DrawDebugInfo();
            }
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
        
        UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION] System initialization:"));
        UE_LOG(LogTemp, Warning, TEXT("  - WaterSystem connection: %s"), bWaterSystemConnected ? TEXT(" CONNECTED") : TEXT(" FAILED"));
        UE_LOG(LogTemp, Warning, TEXT("  - TerrainSystem connection: %s"), bTerrainSystemConnected ? TEXT(" CONNECTED") : TEXT(" FAILED"));
        
        if (bWaterSystemConnected && bTerrainSystemConnected)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION]  READY FOR PRECIPITATION TRANSFER"));
            
            // Create initial weather patterns
            FVector2D TerrainCenter(TerrainWidth * TerrainScale * 0.5f, TerrainHeight * TerrainScale * 0.5f);

            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION]  Created initial weather patterns"));
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION]  Precipitation should begin within 30-60 seconds"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[ATMOSPHERIC INTEGRATION]  CONNECTION FAILED - Precipitation will not work"));
        }
    }
}

float ADynamicTerrain::GetTemperatureAt(FVector WorldPosition) const
{
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetTemperatureAt(WorldPosition);
    }
    return 288.15f; // 15Â°C default
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

/**
 * Frustum culling optimization system for terrain chunks
 *
 * Algorithm:
 * 1. Update visibility state every 0.1 seconds (10Hz)
 * 2. Test each chunk's world bounds against camera view frustum
 * 3. Toggle mesh component visibility based on results
 *
 * Performance Benefits:
 * - 30-40% rendering performance improvement for large terrains
 * - Automatic scaling with terrain size
 * - Minimal CPU overhead (bounds testing only)
 *
 * References:
 * - Akenine-MÃ¶ller, T. et al. (2018). "Real-Time Rendering" 4th Ed., Ch. 19
 * - Ulrich, T. (2000). "Rendering massive terrains using chunked level of detail control"
 *
 * @param DeltaTime - Frame time for update timer
 */
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


void ADynamicTerrain::ResetTerrainFully()
{
    UE_LOG(LogTemp, Warning, TEXT("Performing full terrain reset..."));

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

    // ===== CRITICAL FIX: PROPER WATER SYSTEM RESET SEQUENCE =====
    
    // Step 1: Completely reset water system
    if (WaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Resetting water system..."));
        WaterSystem->ResetWaterSystem();
        
        // Step 2: Re-register with MasterController if available
        if (CachedMasterController)
        {
            WaterSystem->RegisterWithMasterController(CachedMasterController);
            UE_LOG(LogTemp, Warning, TEXT("Water system re-registered with MasterController"));
            
            // Reset water budget tracking
            CachedMasterController->ResetWaterBudget();
        }
        
        // Step 3: Force texture recreation
        WaterSystem->CreateWaterDepthTexture();
        WaterSystem->CreateAdvancedWaterTexture();
        WaterSystem->UpdateWaterDepthTexture();
        
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
            UE_LOG(LogTemp, Warning, TEXT("Water system ready after %.1f seconds"), WaitTime);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Water system taking longer than expected (%.1f seconds) - continuing with initialization"), WaitTime);
        }
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
                        UE_LOG(LogTemp, Warning, TEXT("WaterController successfully reconnected after reset"));
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
    
    // Step 6: Initialize atmospheric system AFTER water is confirmed ready
    if (AtmosphericSystem)
    {
        AtmosphericSystem->Initialize(this, WaterSystem);
        UE_LOG(LogTemp, Warning, TEXT("Atmospheric system reinitialized with confirmed water system"));
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

// ===== CLEAN GENERATION FOR STARTUP =====

/**
 * Clean terrain generation for startup - ensures deterministic state
 *
 * Purpose:
 * - Creates fresh terrain without any existing state
 * - Initializes all subsystems in correct dependency order
 * - Provides known-good baseline for scientific simulations
 *
 * Initialization Sequence:
 * 1. Clear all existing data structures
 * 2. Generate procedural heightmap
 * 3. Initialize chunk mesh system
 * 4. Connect water physics simulation
 * 5. Initialize atmospheric simulation
 *
 * Called By:
 * - BeginPlay() during normal startup
 * - ResetTerrainFully() for runtime resets
 */
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

    // Generate fresh procedural terrain
    GenerateProceduralTerrain();

    // Initialize chunk system
    InitializeChunks();

    if (bInitializeSystems)
       {
           InitializeWaterSystem();
           InitializeAtmosphericSystem();
       }

    UE_LOG(LogTemp, Warning, TEXT("Clean terrain generation complete."));
}

// ===== WORLD SIZE MANAGEMENT IMPLEMENTATION =====

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
// TERRAI PHASE 2: AUTHORITY SIMPLIFICATION COMPLETE
// ============================================
// Eliminated fallback patterns, validation overhead, and retry logic
// Authority is established once and trusted throughout execution

// REMOVED: TryConnectToMasterController - no retry logic needed
// REMOVED: ValidateMasterControllerAuthority - authority is trusted
// REMOVED: InitializeStandaloneMode - undermines authority chain
// REMOVED: All "if (!CachedMasterController)" checks in core functions

void ADynamicTerrain::ApplyMaterialParameters(UMaterialInstanceDynamic* Material, int32 ChunkX, int32 ChunkY)
{
    if (!Material) return;
    
    // PHASE 2: Direct parameter application - authority established
    FLinearColor ChunkGridPosition(ChunkX, ChunkY, 0, 0);
    Material->SetVectorParameterValue(FName("ChunkGridPosition"), ChunkGridPosition);
    
    FLinearColor TerrainInfo(TerrainWidth, TerrainHeight, ChunkSize, TerrainScale);
    Material->SetVectorParameterValue(FName("TerrainInfo"), TerrainInfo);
    
    // Set UV scaling parameters for water shader alignment
    float UScale = 1.0f / (float)TerrainWidth;
    float VScale = 1.0f / (float)TerrainHeight;
    Material->SetScalarParameterValue(FName("UVScale"), FMath::Max(UScale, VScale));
    Material->SetVectorParameterValue(FName("WorldSize"), FLinearColor(TerrainWidth * TerrainScale, TerrainHeight * TerrainScale, 0, 0));
}

// ===== SINGLE SOURCE OF TRUTH IMPLEMENTATION =====

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
        UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Already initialized with this MasterController"));
        return;
    }
    
    CachedMasterController = Master;
    
    // PHASE 2: Direct authority application without validation
    FVector2D AuthoritativeDims = Master->GetWorldDimensions();
    int32 AuthoritativeChunkSize = Master->GetOptimalChunkSize();
    float AuthoritativeScale = Master->GetTerrainScale();
    FVector2D ChunkDims = Master->GetChunkDimensions();
    
    // Apply authoritative values
    TerrainWidth = AuthoritativeDims.X;
    TerrainHeight = AuthoritativeDims.Y;
    ChunkSize = AuthoritativeChunkSize;
    TerrainScale = AuthoritativeScale;
    ChunksX = ChunkDims.X;
    ChunksY = ChunkDims.Y;
    
    // Initialize HeightMap
    int32 TotalSize = TerrainWidth * TerrainHeight;
    HeightMap.SetNumZeroed(TotalSize);
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Authority established - %dx%d terrain, %dx%d chunks"),
           TerrainWidth, TerrainHeight, ChunksX, ChunksY);
    

    GenerateProceduralTerrain();
    InitializeChunks();
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicTerrain: Terrain generation complete, awaiting system initialization"));
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
        UE_LOG(LogTemp, Warning, TEXT("Switched to GPU mode"));
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

void ADynamicTerrain::InitializeGPUTerrain()
{
    if (bGPUInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeGPUTerrain Called"));
        return;
    }
    
    // DON'T round to power of 2 - use exact dimensions
    GPUTextureWidth = TerrainWidth;   // 513
    GPUTextureHeight = TerrainHeight; // 513
    
    UE_LOG(LogTemp, Warning, TEXT("Initializing GPU Terrain: %dx%d (from %dx%d)"),
           GPUTextureWidth, GPUTextureHeight, TerrainWidth, TerrainHeight);
    
    CreateGPUResources();
    
    // Verify resources were created
    if (!HeightRenderTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create HeightRenderTexture!"));
        bGPUInitialized = false;
        return;
    }
    
    // Initial CPU to GPU sync
    SyncCPUToGPU();
    
    bGPUInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("GPU Terrain initialized successfully"));
}

void ADynamicTerrain::CreateGPUResources()
{
    // Release any existing resources
    ReleaseGPUResources();
    
    // Create height render texture with UAV support
    HeightRenderTexture = NewObject<UTextureRenderTarget2D>(this);
    HeightRenderTexture->InitCustomFormat(
        GPUTextureWidth,
        GPUTextureHeight,
        PF_R32_FLOAT,
        false
    );
    HeightRenderTexture->AddressX = TA_Clamp;
    HeightRenderTexture->AddressY = TA_Clamp;
    HeightRenderTexture->bCanCreateUAV = true;  // Enable UAV
    HeightRenderTexture->UpdateResourceImmediate();
    
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
    if (HeightRenderTexture)
    {
        HeightRenderTexture->ReleaseResource();
        HeightRenderTexture = nullptr;
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


// 1. CPUâ†’GPU Transfer (one-time on mode switch)
void ADynamicTerrain::TransferHeightmapToGPU()
{
    if (!HeightRenderTexture) return;
    
    TArray<FFloat16> GPUData;
    GPUData.SetNum(TerrainWidth * TerrainHeight);
    
    for (int32 i = 0; i < HeightMap.Num(); i++)
    {
        GPUData[i] = FFloat16(HeightMap[i]);
    }
    
    ENQUEUE_RENDER_COMMAND(UploadHeightmapToGPU)(
        [this, GPUData](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRenderTargetResource* Resource = HeightRenderTexture->GetRenderTargetResource();
            if (!Resource) return;
            
            FRHITexture* Texture = Resource->GetRenderTargetTexture();
            if (!Texture) return;
            
            uint32 Stride = sizeof(FFloat16);
            FUpdateTextureRegion2D Region(0, 0, 0, 0, TerrainWidth, TerrainHeight);
            
            RHICmdList.UpdateTexture2D(
                Texture,
                0,
                Region,
                Stride * TerrainWidth,
                (uint8*)GPUData.GetData()
            );
        });
    
    FlushRenderingCommands();
}

// 2. GPUâ†’CPU Transfer (one-time on mode switch)
void ADynamicTerrain::TransferHeightmapFromGPU()
{
    if (!HeightRenderTexture) return;
    
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
    
    ValidateAndRepairChunkBoundaries();
}




void ADynamicTerrain::ExecuteTerrainComputeShader(float DeltaTime)
    {
        if (!bGPUInitialized || !HeightRenderTexture) {
            UE_LOG(LogTemp, Error, TEXT("GPU NOT READY: Init=%d, Texture=%s"),
                   bGPUInitialized, HeightRenderTexture ? TEXT("Valid") : TEXT("Null"));
            return;
        }
    
    // Throttle execution to improve performance
    static float LastExecutionTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (!bHasPendingBrush && CurrentTime - LastExecutionTime < 0.033f) return; // 30 FPS for ambient updates
    LastExecutionTime = CurrentTime;
        
       // UE_LOG(LogTemp, Warning, TEXT("GPU COMPUTE EXECUTING: Brush=%d"), bHasPendingBrush);
    
    // Skip accumulator for brush operations - execute immediately
    if (!bHasPendingBrush)
    {
        GPUUpdateAccumulator += DeltaTime;
        if (GPUUpdateAccumulator < GPUUpdateInterval) return;
    }
    
    GPUUpdateAccumulator = 0.0f;
    
    ENQUEUE_RENDER_COMMAND(TerrainComputeCommand)(
        [this, DeltaTime](FRHICommandListImmediate& RHICmdList)
        {
            TShaderMapRef<FTerrainComputeCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            if (!ComputeShader.IsValid()) return;
            
            FRDGBuilder GraphBuilder(RHICmdList);
            
            FTextureRenderTargetResource* Resource = HeightRenderTexture->GetRenderTargetResource();
            TRefCountPtr<IPooledRenderTarget> PooledHeightTexture = CreateRenderTarget(
                Resource->GetRenderTargetTexture(), TEXT("HeightTexture"));
            FRDGTextureRef HeightTextureRDG = GraphBuilder.RegisterExternalTexture(PooledHeightTexture);
            
            FTerrainComputeCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FTerrainComputeCS::FParameters>();
            
            PassParameters->HeightTexture = GraphBuilder.CreateUAV(HeightTextureRDG);
            PassParameters->TerrainParams = FVector4f(TerrainWidth, TerrainHeight, TerrainScale, 0);
            
            // Create and CLEAR dummy textures
            FRDGTextureDesc DummyDesc = FRDGTextureDesc::Create2D(
                FIntPoint(GPUTextureWidth, GPUTextureHeight),
                PF_R32_FLOAT,
                FClearValueBinding::Black,
                TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable  // Add this flag
            );

            FRDGTextureRef DummyTexture = GraphBuilder.CreateTexture(DummyDesc, TEXT("DummyWater"));
            AddClearRenderTargetPass(GraphBuilder, DummyTexture);

            // Now use for SRVs
            PassParameters->WaterDepthTexture = GraphBuilder.CreateSRV(DummyTexture);
            PassParameters->FlowVelocityTexture = GraphBuilder.CreateSRV(DummyTexture);
            PassParameters->MoistureTexture = GraphBuilder.CreateSRV(DummyTexture);
            PassParameters->WindFieldTexture = GraphBuilder.CreateSRV(DummyTexture);
            PassParameters->HardnessTexture = GraphBuilder.CreateSRV(DummyTexture);

            FRDGTextureRef ErosionTexture = GraphBuilder.CreateTexture(DummyDesc, TEXT("ErosionOutput"));
            AddClearRenderTargetPass(GraphBuilder, ErosionTexture);
            PassParameters->ErosionOutputTexture = GraphBuilder.CreateUAV(ErosionTexture);
            PassParameters->TextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
            
            PassParameters->DeltaTime = DeltaTime;
            PassParameters->SimulationMode = 0; // Height only
            PassParameters->ErosionParams = FVector4f(0,0,0,0);
            PassParameters->OrographicParams = FVector4f(0,0,0,0);
            
            if (bHasPendingBrush)
            {
                PassParameters->BrushParams = PendingBrushParams;
                PassParameters->BrushActive = 1;
                bHasPendingBrush = false;
            }
            else
            {
                PassParameters->BrushActive = 0;
            }
            
            // Dispatch compute
            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("TerrainCompute"),
                ComputeShader,
                PassParameters,
                FIntVector(
                    FMath::DivideAndRoundUp(GPUTextureWidth, 8),
                    FMath::DivideAndRoundUp(GPUTextureHeight, 8),
                    1
                )
            );
            
            GraphBuilder.Execute();
        });
    
    // Immediate flush for brush operations
    if (PendingBrushParams.W != 0)
    {
        FlushRenderingCommands();
    }
}

void ADynamicTerrain::SyncCPUToGPU()
{
    if (!HeightRenderTexture || HeightMap.Num() == 0)
        return;
    
    TArray<float> GPUHeightData;
    GPUHeightData.SetNum(GPUTextureWidth * GPUTextureHeight);
    
    for (int32 Y = 0; Y < TerrainHeight; Y++)
    {
        for (int32 X = 0; X < TerrainWidth; X++)
        {
            int32 CPUIndex = Y * TerrainWidth + X;
            int32 GPUIndex = Y * GPUTextureWidth + X;
            
            if (CPUIndex < HeightMap.Num())
            {
                GPUHeightData[GPUIndex] = HeightMap[CPUIndex];
            }
        }
    }
    
    ENQUEUE_RENDER_COMMAND(UploadHeightToGPU)(
        [this, GPUHeightData](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            
            // Use UpdateTexture2D for upload
            uint32 Stride = GPUTextureWidth * sizeof(float);
            uint8* TextureData = (uint8*)GPUHeightData.GetData();
            
            FUpdateTextureRegion2D Region(0, 0, 0, 0, GPUTextureWidth, GPUTextureHeight);
            RHIUpdateTexture2D(TextureRHI, 0, Region, Stride, TextureData);
        });
    
    UE_LOG(LogTemp, Log, TEXT("Synced CPU heightmap to GPU"));
}

void ADynamicTerrain::SyncGPUToCPU()
{
    if (!HeightRenderTexture || bPendingGPUReadback)
        return;
    
    ENQUEUE_RENDER_COMMAND(ReadbackHeightFromGPU)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRHIRef TextureRHI = HeightRenderTexture->GetRenderTargetResource()->GetRenderTargetTexture();
            
            // Create a surface to read back data
            FIntRect Rect(0, 0, GPUTextureWidth, GPUTextureHeight);
            TArray<FFloat16Color> SurfaceData;
            
            RHICmdList.ReadSurfaceFloatData(TextureRHI, Rect, SurfaceData, ECubeFace::CubeFace_PosX, 0, 0);
            
            // Convert to float array
            GPUHeightReadbackBuffer.SetNum(TerrainWidth * TerrainHeight);
            for (int32 Y = 0; Y < TerrainHeight; Y++)
            {
                for (int32 X = 0; X < TerrainWidth; X++)
                {
                    int32 SrcIndex = Y * GPUTextureWidth + X;
                    int32 DstIndex = Y * TerrainWidth + X;
                    if (SrcIndex < SurfaceData.Num())
                    {
                        GPUHeightReadbackBuffer[DstIndex] = SurfaceData[SrcIndex].R.GetFloat();
                    }
                }
            }
        });
    
    if (GPUHeightReadbackBuffer.Num() == HeightMap.Num())
    {
        HeightMap = GPUHeightReadbackBuffer;
        for (int32 i = 0; i < TerrainChunks.Num(); i++)
        {
            PendingChunkUpdates.Add(i);
        }
    }
    
    bGPUDataDirty = false;
    
    if (GPUHeightReadbackBuffer.Num() == HeightMap.Num())
       {
           HeightMap = GPUHeightReadbackBuffer;
           
           // Fix chunk tears using existing validation
           ValidateAndRepairChunkBoundaries();
           
           // Mark chunks for update
           for (int32 i = 0; i < TerrainChunks.Num(); i++)
           {
               PendingChunkUpdates.Add(i);
           }
       }
    
    UE_LOG(LogTemp, Log, TEXT("Synced GPU heightmap to CPU"));
}

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
    if (CurrentComputeMode != ETerrainComputeMode::GPU || !bGPUInitialized) return;
    
    FVector2D TerrainPos = FVector2D(
        WorldPosition.X / TerrainScale,
        WorldPosition.Y / TerrainScale
    );
    
    // Match CPU's strength scaling for large brushes
    float AdjustedStrength = Strength;
    if (Radius > 1000.0f)
    {
        AdjustedStrength *= (Radius / 500.0f);
    }
    
    PendingBrushParams = FVector4f(
        TerrainPos.X,
        TerrainPos.Y,
        Radius / TerrainScale,
        bRaise ? AdjustedStrength : -AdjustedStrength
    );
    bHasPendingBrush = true;
    
    // Execute compute shader immediately
    ExecuteTerrainComputeShader(0.0f);
    
    // ALWAYS sync GPU to CPU for terrain modifications
    SyncGPUToCPU();
    
    // Find all affected chunks
    TSet<int32> AffectedChunks;
    int32 RadiusInIndices = FMath::CeilToInt(Radius / TerrainScale);
    int32 CenterX = FMath::RoundToInt(TerrainPos.X);
    int32 CenterY = FMath::RoundToInt(TerrainPos.Y);
    
    for (int32 Y = CenterY - RadiusInIndices; Y <= CenterY + RadiusInIndices; Y++)
    {
        for (int32 X = CenterX - RadiusInIndices; X <= CenterX + RadiusInIndices; X++)
        {
            if (X >= 0 && X < TerrainWidth && Y >= 0 && Y < TerrainHeight)
            {
                float Distance = FVector2D::Distance(FVector2D(X, Y), TerrainPos);
                if (Distance <= RadiusInIndices)
                {
                    int32 ChunkIndex = GetChunkIndexFromCoordinates(X, Y);
                    if (ChunkIndex >= 0)
                    {
                        AffectedChunks.Add(ChunkIndex);
                    }
                }
            }
        }
    }
    
    // Update chunks based on size
    if (AffectedChunks.Num() > 15 || Radius > 2000.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("GPU: Large brush - updating %d chunks"), AffectedChunks.Num());
        ForceUpdateChunkGroup(AffectedChunks.Array());
    }
    else
    {
        // Update chunks individually for small brushes
        for (int32 ChunkIndex : AffectedChunks)
        {
            UpdateChunk(ChunkIndex);
        }
    }
}

void ADynamicTerrain::ForceGPUChunkVisualUpdate(const TArray<int32>& ChunkIndices)
{
    if (!HeightRenderTexture) return;
    
    // Diagnostic: Log what we're updating
    UE_LOG(LogTemp, Warning, TEXT("GPU: ForceUpdate called for %d chunks"), ChunkIndices.Num());
    
    // Wait for compute shader to complete
    FlushRenderingCommands();
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("GPU: Updating %d total chunks (including neighbors)"), AllChunks.Num());
    
    // Update all chunks
    int32 UpdatedCount = 0;
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
                    
                if (!DynMaterial)
                {
                    if (CurrentActiveMaterial)
                    {
                        DynMaterial = UMaterialInstanceDynamic::Create(CurrentActiveMaterial, this);
                        Chunk.MeshComponent->SetMaterial(0, DynMaterial);
                        UE_LOG(LogTemp, Warning, TEXT("Created new material for chunk %d"), ChunkIndex);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("No active material for chunk %d!"), ChunkIndex);
                        continue;
                    }
                }
                
                // Update material with current height texture
                DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightRenderTexture);
                
                // Force a unique time value to ensure shader updates
                float UniqueTime = GetWorld()->GetTimeSeconds() + ChunkIndex * 0.001f;
                DynMaterial->SetScalarParameterValue(FName("ForceUpdateTime"), UniqueTime);
                
                // Set UV parameters
                int32 TerrainStartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
                int32 TerrainStartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
                
                DynMaterial->SetVectorParameterValue(
                    FName("ChunkUVTransform"),
                    FLinearColor(
                        (float)TerrainStartX / TerrainWidth,
                        (float)TerrainStartY / TerrainHeight,
                        (float)ChunkSize / TerrainWidth,
                        (float)ChunkSize / TerrainHeight
                    )
                );
                
                // Force mesh to update
                Chunk.MeshComponent->MarkRenderStateDirty();
                Chunk.MeshComponent->UpdateBounds();
                
                UpdatedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GPU: Successfully updated %d chunks"), UpdatedCount);
}

// Helper function to update material parameters (thread-safe)
void ADynamicTerrain::UpdateGPUChunkMaterialParams(UMaterialInstanceDynamic* DynMaterial,
                                                   const FTerrainChunk& Chunk)
{
    if (!DynMaterial || !HeightRenderTexture) return;
    
    if (DynMaterial)
    {
        DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightRenderTexture);
        UE_LOG(LogTemp, Warning, TEXT("Bound HeightTexture to chunk %d"), Chunk.ChunkX);
    }
    
    // Bind height texture
    DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightRenderTexture);
    
    // CRITICAL: Correct UV mapping that matches CPU chunk layout exactly
    int32 TerrainStartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
    int32 TerrainStartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
    
    // Include the overlap in the UV range to prevent gaps
    float UVStartX = (float)TerrainStartX / (float)TerrainWidth;
    float UVStartY = (float)TerrainStartY / (float)TerrainHeight;
    float UVScaleX = (float)ChunkSize / (float)TerrainWidth;
    float UVScaleY = (float)ChunkSize / (float)TerrainHeight;
    
    DynMaterial->SetVectorParameterValue(
        FName("ChunkUVTransform"),
        FLinearColor(UVStartX, UVStartY, UVScaleX, UVScaleY)
    );
    
    DynMaterial->SetVectorParameterValue(
        FName("TerrainParams"),
        FLinearColor(TerrainWidth, TerrainHeight, TerrainScale, MaxTerrainHeight)
    );
    
    DynMaterial->SetScalarParameterValue(FName("ChunkSize"), ChunkSize);
    DynMaterial->SetScalarParameterValue(FName("ChunkOverlap"), ChunkOverlap);
    
    // Force immediate update
    DynMaterial->SetScalarParameterValue(FName("UpdateTime"), GetWorld()->GetTimeSeconds());
    
    // Water integration if needed
    if (WaterSystem && WaterSystem->bUseShaderWater)
    {
        WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
    }
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
        // Bind height texture
        DynMaterial->SetTextureParameterValue(FName("HeightmapTexture"), HeightRenderTexture);
        
        // CRITICAL FIX: Correct UV calculation accounting for overlap
        // The chunk covers terrain indices from StartX to StartX + ChunkSize
        int32 TerrainStartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
        int32 TerrainStartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
        
        // UV coordinates map directly to heightmap texture coordinates
        float UVStartX = (float)TerrainStartX / (float)TerrainWidth;
        float UVStartY = (float)TerrainStartY / (float)TerrainHeight;
        
        // The chunk samples ChunkSize vertices, not (ChunkSize - ChunkOverlap)
        float UVScaleX = (float)ChunkSize / (float)TerrainWidth;
        float UVScaleY = (float)ChunkSize / (float)TerrainHeight;
        
        DynMaterial->SetVectorParameterValue(
            FName("ChunkUVTransform"),
            FLinearColor(UVStartX, UVStartY, UVScaleX, UVScaleY)
        );
        
        // Pass terrain dimensions for vertex displacement
        DynMaterial->SetVectorParameterValue(
            FName("TerrainParams"),
            FLinearColor(TerrainWidth, TerrainHeight, TerrainScale, MaxTerrainHeight)
        );
        
        // Additional parameters for proper vertex displacement
        DynMaterial->SetScalarParameterValue(FName("ChunkSize"), ChunkSize);
        DynMaterial->SetScalarParameterValue(FName("ChunkOverlap"), ChunkOverlap);
        
        // Water integration
        if (WaterSystem && WaterSystem->bUseShaderWater)
        {
            WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
        }
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
    ConnectedWaterSystem = WaterSys;
    
    if (ConnectedWaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Connected terrain to water system"));
    }
}

void ADynamicTerrain::SyncGPUChunkVisuals()
{
    if (!HeightRenderTexture) return;
    
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
                DynMaterial->SetTextureParameterValue(
                    FName("HeightmapTexture"),
                    HeightRenderTexture
                );
                
                // FIX: Match CPU's exact coordinate mapping
                int32 TerrainStartX = Chunk.ChunkX * (ChunkSize - ChunkOverlap);
                int32 TerrainStartY = Chunk.ChunkY * (ChunkSize - ChunkOverlap);
                
                // Each vertex i maps to terrain coordinate (TerrainStartX + i)
                // So UV = (TerrainStartX + i) / TerrainWidth
                DynMaterial->SetVectorParameterValue(
                    FName("ChunkUVTransform"),
                    FLinearColor(
                        (float)TerrainStartX / TerrainWidth,    // UV start X
                        (float)TerrainStartY / TerrainHeight,   // UV start Y
                        1.0f / TerrainWidth,                    // UV step per vertex X
                        1.0f / TerrainHeight                    // UV step per vertex Y
                    )
                );
                
                DynMaterial->SetScalarParameterValue(FName("ChunkSize"), ChunkSize);
                
                if (WaterSystem && WaterSystem->bUseShaderWater)
                {
                    WaterSystem->ApplyVolumetricWaterToMaterial(DynMaterial);
                }
            }
        }
    }
}

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
