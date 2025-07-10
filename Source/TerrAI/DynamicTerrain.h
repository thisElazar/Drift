// DynamicTerrain.h - Clean Terrain System (Water System Separate)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MasterController.h"
#include "DynamicTerrain.generated.h"

// Forward declarations to reduce header dependencies
class UProceduralMeshComponent;
class USceneComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UWaterSystem;
class UAtmosphericSystem;
class AMasterWorldController;
class ULocalPlayer;
class FSceneView;

// World size configuration system
UENUM(BlueprintType)
enum class ETerrainWorldSize : uint8
{
    Small    UMETA(DisplayName = "Small (257x257)"),
    Medium   UMETA(DisplayName = "Medium (513x513)"),
    Large    UMETA(DisplayName = "Large (1025x1025)"),
    Massive  UMETA(DisplayName = "Massive (2049x2049)")
};

// Scalable world configuration
USTRUCT(BlueprintType)
struct TERRAI_API FWorldSizeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 TerrainWidth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 TerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ChunkSize;        // Fixed at 16 for optimal memory usage

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ChunksX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ChunksY;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EditingScale;     // Brush scaling factor

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float LODBias;          // Performance scaling

    FWorldSizeConfig()
        : TerrainWidth(257), TerrainHeight(257), ChunkSize(16)
        , ChunksX(16), ChunksY(16), EditingScale(1.0f), LODBias(1.0f)
    {}

    // Calculate total chunk count
    int32 GetTotalChunks() const { return ChunksX * ChunksY; }
};


// Terrain chunk structure for organized mesh management
USTRUCT(BlueprintType)
struct FTerrainChunk
{
    GENERATED_BODY()

    // Mesh component for this chunk
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProceduralMeshComponent* MeshComponent = nullptr;

    // Chunk grid position
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ChunkX = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ChunkY = 0;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bNeedsUpdate = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsActive = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LastUpdateTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsVisible = true;

    FTerrainChunk()
    {
        MeshComponent = nullptr;
        ChunkX = 0;
        ChunkY = 0;
        bNeedsUpdate = false;
        bIsActive = true;
        LastUpdateTime = 0.0f;
        bIsVisible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API ADynamicTerrain : public AActor
{
    GENERATED_BODY()
    
public:
    ADynamicTerrain();
    
    // ===== AUTHORITY CONSOLIDATION =====
    
    UPROPERTY()
    class AMasterWorldController* CachedMasterController = nullptr;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    
    

    // ===== CORE TERRAIN FUNCTIONS =====
    
    /** Generates simple sinusoidal terrain for quick testing */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateSimpleTerrain();

    /** Generates complex multi-octave procedural terrain with realistic features */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateProceduralTerrain();

    /** 
     * Modifies terrain height at world position with circular brush pattern
     * @param WorldPosition - Center point in world coordinates
     * @param Radius - Brush radius in world units (500+ recommended)
     * @param Strength - Height change per modification (200 = moderate)
     * @param bRaise - True to raise terrain, false to lower
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void ModifyTerrain(FVector WorldPosition, float Radius, float Strength, bool bRaise = true);

    /** 
     * Modifies terrain height at terrain grid coordinates
     * @param X,Y - Terrain grid coordinates (0 to TerrainWidth/Height)
     * @param Radius - Brush radius in terrain units
     * @param Strength - Height change amount
     * @param bRaise - True to raise terrain, false to lower
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void ModifyTerrainAtIndex(int32 X, int32 Y, float Radius, float Strength, bool bRaise = true);

    // ===== UTILITY FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable)
    void ResetTerrainFully();
    
    // Clean generation for startup (automatic reset)
    void PerformCleanGeneration(bool bInitializeSystems = true);

    // ===== MISSING FUNCTION DECLARATIONS =====
    UFUNCTION(BlueprintCallable, Category = "Master Controller")
    void InitializeWithMasterController(AMasterWorldController* Master);
    
    UFUNCTION(BlueprintCallable, Category = "Master Controller") 
    bool ValidateMasterControllerAuthority() const;
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    FVector TerrainToWorldPosition(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtPosition(FVector WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtIndex(int32 X, int32 Y) const;

    // Safe height map access methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightSafe(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    void SetHeightSafe(int32 X, int32 Y, float Height);

    // ===== CHUNK MANAGEMENT =====
    
    /** 
     * Updates a single terrain chunk's mesh geometry
     * @param ChunkIndex - Linear index into TerrainChunks array
     */
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void UpdateChunk(int32 ChunkIndex);

    /** 
     * Converts terrain coordinates to chunk index
     * @param X,Y - Terrain grid coordinates
     * @return Linear chunk index, or -1 if coordinates are invalid
     */
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    int32 GetChunkIndexFromCoordinates(int32 X, int32 Y) const;

    /** 
     * Marks chunk for update in next processing cycle
     * @param ChunkIndex - Chunk to mark for update
     */
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void MarkChunkForUpdate(int32 ChunkIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void MarkChunkForWaterUpdate(int32 ChunkIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void BatchUpdateWaterChunks(const TArray<int32>& ChunkIndices);
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void UpdateChunkWaterOnly(int32 ChunkIndex);
    
    // Priority-based chunk updates
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void RequestPriorityChunkUpdate(int32 ChunkIndex, float Priority);
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    float CalculateChunkDistance(int32 ChunkIndex, FVector FromPosition) const;
    
    // ===== CHUNK BOUNDARY INTEGRITY SYSTEM =====
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void ValidateAndRepairChunkBoundaries();
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    TArray<int32> GetNeighboringChunks(int32 ChunkIndex, bool bIncludeDiagonals = true) const;
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void UpdateChunkGroupAtomic(const TArray<int32>& ChunkIndices);
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    bool ValidateChunkBoundaryIntegrity(int32 ChunkIndex) const;
    
    // ===== VOLUMETRIC WATER SUPPORT FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    FVector GetChunkWorldPosition(int32 ChunkIndex) const;

    // ===== WATER SYSTEM INTEGRATION =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water System")
    UWaterSystem* WaterSystem;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric System")
    UAtmosphericSystem* AtmosphericSystem;
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void InitializeWaterSystem();
    
    // ===== WATER SYSTEM ACCESS ONLY =====
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsWaterSystemReady() const;
  
    // ===== ATMOSPHERIC SYSTEM INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void InitializeAtmosphericSystem();
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    float GetTemperatureAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    float GetPrecipitationAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    FVector GetWindAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateWeatherSystem(int32 WeatherType, FVector2D Center, float Strength);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    UAtmosphericSystem* GetAtmosphericSystem() const { return AtmosphericSystem; }

    // ===== SCALABLE WORLD CONFIGURATION =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Configuration")
    ETerrainWorldSize CurrentWorldSize = ETerrainWorldSize::Medium;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Configuration")
    FWorldSizeConfig WorldConfig;
    
    // ===== TERRAIN DIMENSIONS - SYNCHRONIZED FROM MASTERCONTROLLER =====
    // These are working properties synchronized with MasterController authority (NO DEFAULTS)
    int32 TerrainWidth;  // Synchronized with MasterController
    int32 TerrainHeight; // Synchronized with MasterController  
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float TerrainScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float MaxTerrainHeight = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float MinTerrainHeight = -10000.0f;

    // ===== SCALABLE CHUNK SYSTEM =====
    
    int32 ChunkSize = 32;  // Synchronized with MasterController authority

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
    int32 ChunkOverlap = 2;  // 2 vertex overlap for seamless chunks (enhanced for authority integration)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
    int32 MaxUpdatesPerFrame = 2;  // Reduced for better performance


    // ===== PERFORMANCE SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ModificationCooldown = 0.05f; // 20 modifications per second max

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowPerformanceStats = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowChunkBounds = false;
    
    // ===== CHUNK UPDATE OPTIMIZATION =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePrecipitationOptimization = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxWaterUpdatesPerFrame = 16; // Higher throughput for water-only updates

    // ===== PUBLIC CHUNK ACCESS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk System")
    TArray<FTerrainChunk> TerrainChunks;

    int32 ChunksX = 0;  // Synchronized with MasterController authority
    int32 ChunksY = 0;  // Synchronized with MasterController authority
    
    // ===== FRUSTUM CULLING SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrustumCulling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingUpdateRate = 0.1f;
    
    // ===== PHASE 4: CHUNK POOLING & PERFORMANCE =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableChunkPooling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ChunkPoolSize = 100;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdvancedLOD = false; // Disabled by default for performance
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
    UMaterialInterface* CurrentActiveMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* DefaultTerrainMaterial = nullptr;
    
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetActiveMaterial(UMaterialInterface* Material);
    
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetWaterVolumeMaterial(UMaterialInterface* Material);
    
    // ===== WORLD SIZE MANAGEMENT =====
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void SetWorldSize(ETerrainWorldSize NewSize);
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void SetCustomWorldSize(int32 Width, int32 Height, int32 CustomChunkSize = 32);
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void ApplyWorldConfiguration(const FWorldSizeConfig& Config);
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    FWorldSizeConfig GetWorldConfigForSize(ETerrainWorldSize Size) const;
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    FWorldSizeConfig CreateCustomWorldConfig(int32 Width, int32 Height, int32 CustomChunkSize = 32) const;
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void MigrateToScalableSystem();
    
    // Performance optimization helpers
    float GetCachedFrameTime() const;

private:
    // ===== INTERNAL COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere)
    USceneComponent* TerrainRoot;
    
    // ===== TERRAIN DATA =====
    TArray<float> HeightMap;
    
    // ===== CHUNK MANAGEMENT =====
    TSet<int32> PendingChunkUpdates;
    TSet<int32> PendingWaterChunkUpdates; // Separate queue for water-only updates
    
    // Priority-based chunk update system
    struct FChunkUpdateRequest
    {
        int32 ChunkIndex;
        float Priority;
        float RequestTime;
        
        bool operator<(const FChunkUpdateRequest& Other) const
        {
            return Priority > Other.Priority; // Higher priority first
        }
    };
    
    TArray<FChunkUpdateRequest> PriorityChunkQueue;
    
    // ===== PERFORMANCE OPTIMIZATION =====
    
    float LastModificationTime = 0.0f;
    int32 TotalChunkUpdatesThisFrame = 0;
    float StatUpdateTimer = 0.0f;
    
    // Frustum culling variables
    float CullingUpdateTimer = 0.0f;
    int32 CurrentVisibleChunks = 0;
    
    // ===== INTERNAL FUNCTIONS =====
    
    // Terrain generation and modification
    void InitializeChunks();
    void GenerateChunkMesh(int32 ChunkX, int32 ChunkY);
    void ProcessPendingChunkUpdates();
    void ProcessPendingWaterChunkUpdates(); // Add missing declaration
    void UpdatePerformanceStats(float DeltaTime);
    
    // Material parameter consolidation
    void ApplyMaterialParameters(UMaterialInstanceDynamic* Material, int32 ChunkX, int32 ChunkY);
    
    // Helper functions
    FVector2D GetChunkWorldPosition(int32 ChunkX, int32 ChunkY) const;
    FVector CalculateVertexNormal(int32 X, int32 Y) const;
    FLinearColor GetHeightBasedColor(float NormalizedHeight) const;
    
    // Frustum culling functions
    void UpdateFrustumCulling(float DeltaTime);
    bool IsChunkVisible(const FTerrainChunk& Chunk, const FSceneView* View) const;
    FBoxSphereBounds GetChunkWorldBounds(const FTerrainChunk& Chunk) const;
    
    // Thread-safe height map modifications
    FCriticalSection HeightMapMutex;
    
    // ===== CHUNK BOUNDARY SYSTEM =====
    
    /** Boundary validation tolerance for tear detection */
    static constexpr float BOUNDARY_HEIGHT_TOLERANCE = 5.0f;
    
    /** Maximum neighbors to update atomically */
    static constexpr int32 MAX_ATOMIC_NEIGHBORS = 9;
    
    /** Validate chunk coordinate calculations for consistency */
    bool ValidateChunkCoordinateConsistency() const;
    
    /** Repair detected boundary tears between chunks */
    void RepairBoundaryTear(int32 ChunkA, int32 ChunkB, bool bIsVerticalBoundary);
    
    /** Get chunk boundary vertices for validation */
    TArray<FVector> GetChunkBoundaryVertices(int32 ChunkIndex, int32 BoundaryEdge) const;
    
    /** Force immediate mesh update for chunk group */
    void ForceUpdateChunkGroup(const TArray<int32>& ChunkIndices);
    
    // ===== PHASE 4: CHUNK POOLING SYSTEM =====
    
    /** Pool of reusable procedural mesh components */
    TArray<UProceduralMeshComponent*> ChunkMeshPool;
    
    /** Get pooled mesh component for chunk */
    UProceduralMeshComponent* GetPooledMeshComponent();
    
    /** Return mesh component to pool */
    void ReturnMeshComponentToPool(UProceduralMeshComponent* MeshComponent);
    
    /** Advanced LOD calculation based on distance and importance */
    int32 CalculateChunkLOD(int32 ChunkIndex, FVector CameraLocation) const;
    
    /** Update chunk LOD based on camera distance */
    void UpdateChunkLOD(int32 ChunkIndex, int32 NewLOD);
    
    // Frame time caching system (eliminates 20+ GetTimeSeconds calls per frame)
    mutable float CachedFrameTime = 0.0f;
    mutable int32 CachedFrameNumber = -1;
    
    // String allocation optimization (reduces GC pressure)
    mutable FString CachedChunkNameBuffer;
    mutable FString CachedDebugStringBuffer;
    
    // SINGLE SOURCE OF TRUTH HELPERS
    void InitializeTerrainData();
    
 
};
