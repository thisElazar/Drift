/**
 * ============================================
 * DRIFT ECOSYSTEM CONTROLLER - HEADER
 * ============================================
 *
 * PURPOSE:
 * Manages living biological systems including vegetation growth, biome distribution,
 * and species placement. Integrates with water, atmosphere, and terrain systems to
 * create realistic, data-driven ecosystems that respond to environmental conditions.
 *
 * ALPHA SPRINT SCOPE:
 * Basic grass growth system as proof-of-concept for ecosystem patterns:
 * - Water-dependent growth (queries WaterSystem through MasterController authority)
 * - Sparse update pattern (100 instances/frame for performance)
 * - GPU-native rendering (UE5 HISM automatic instancing)
 * - Atmospheric wind integration (connects to AtmosphericSystem)
 * - Spatial grid for density management (O(1) neighbor queries)
 *
 * ARCHITECTURE PATTERNS:
 * - Authority Delegation: MasterController coordinates all systems
 * - Event-Driven Updates: Only updates vegetation when conditions change
 * - Sparse Processing: Updates small subset per frame (avoids frame spikes)
 * - GPU Rendering: CPU logic drives GPU instancing (not GPU compute)
 * - Data Authority: Water/Terrain systems authorize placement decisions
 *
 * RENDERING STRATEGY:
 * CPU (Logic):
 *   - Growth calculations (water availability, health)
 *   - Placement decisions (can grass grow here?)
 *   - Death/removal (health-based lifecycle)
 *
 * GPU (Rendering):
 *   - UE5 HISM (Hierarchical Instanced Static Mesh)
 *   - Automatic draw call batching (10,000 grass = 1 draw call)
 *   - Material-based wind animation (World Position Offset node)
 *   - Frustum culling & LOD (built-in UE5 optimization)
 *
 * INTEGRATION POINTS:
 * - WaterSystem: Moisture queries for growth decisions
 * - AtmosphericSystem: Wind data for material animation
 * - DynamicTerrain: Height/slope validation, coordinate transforms
 * - MasterController: Central authority for all coordinate operations
 * - TemporalManager: Time-scaled updates (geological → real-time)
 *
 * PERFORMANCE CHARACTERISTICS:
 * - CPU overhead: ~0.5-1ms per frame (10,000 grass, sparse updates)
 * - GPU overhead: Negligible (single batched draw call)
 * - Memory: ~128 bytes per grass instance (CPU) + ~64 bytes (GPU)
 * - Scalability: Linear up to ~100,000 instances before optimization needed
 *
 * FILE ORGANIZATION:
 * Section 1: Core Types & Structures
 * Section 2: Public Interface
 * Section 3: IScalableSystem Interface
 * Section 4: Private State & Methods
 *
 * FUTURE ENHANCEMENTS (Post-Alpha):
 * - Multiple vegetation species (trees, shrubs, flowers)
 * - Seed dispersal simulation
 * - Species competition & succession
 * - Advanced LOD streaming
 * - GPU compute for massive placement optimization
 * - Wildlife integration (herbivore grazing patterns)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MasterController.h"
#include "EcosystemController.generated.h"

class UWaterSystem;
class ADynamicTerrain;
class AAtmosphereController;

// ============================================================================
// SECTION 1: CORE TYPES & STRUCTURES
// ============================================================================

/**
 * Biome types supported by the ecosystem
 * Each biome has specific vegetation types, temperature, and humidity ranges
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Desert         UMETA(DisplayName = "Desert"),
    Grassland      UMETA(DisplayName = "Grassland"),
    Forest         UMETA(DisplayName = "Forest"),
    Wetland        UMETA(DisplayName = "Wetland"),
    Tundra         UMETA(DisplayName = "Tundra"),
    Alpine         UMETA(DisplayName = "Alpine"),
    Coastal        UMETA(DisplayName = "Coastal")
};

/**
 * Vegetation species types
 * Alpha sprint: Only Grass implemented
 * Future: All types with species-specific growth patterns
 */
UENUM(BlueprintType)
enum class EVegetationType : uint8
{
    Grass          UMETA(DisplayName = "Grass"),
    Shrub          UMETA(DisplayName = "Shrub"),
    Tree           UMETA(DisplayName = "Tree"),
    Flower         UMETA(DisplayName = "Flower"),
    Moss           UMETA(DisplayName = "Moss"),
    Fern           UMETA(DisplayName = "Fern")
};

/**
 * Configuration data for vegetation species
 * Defines growth requirements and environmental preferences
 */
USTRUCT(BlueprintType)
struct FVegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EVegetationType VegetationType = EVegetationType::Grass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    UStaticMesh* VegetationMesh = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Requirements")
    float MinWaterRequirement = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Requirements")
    float OptimalWaterLevel = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Parameters")
    float GrowthRate = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float MaxDensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D ElevationRange = FVector2D(0.0f, 1000.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<EBiomeType> PreferredBiomes;
};

/**
 * Biome configuration data
 * Defines environmental characteristics and native vegetation
 */
USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EBiomeType BiomeType = EBiomeType::Grassland;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector2D TemperatureRange = FVector2D(10.0f, 30.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector2D HumidityRange = FVector2D(0.3f, 0.8f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 500.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationData> NativeVegetation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    FLinearColor BiomeColor = FLinearColor::Green;
};

/**
 * Runtime instance data for individual grass
 * Tracks growth state, health, and spatial position
 *
 * MEMORY: 52 bytes per instance
 * - Location: 12 bytes (FVector)
 * - GrowthProgress: 4 bytes (float)
 * - Health: 4 bytes (float)
 * - LastUpdateTime: 4 bytes (float)
 * - SoilMoisture: 4 bytes (float)
 * - MeshIndex: 4 bytes (int32)
 * - InstanceIndex: 4 bytes (int32)
 * - Padding: 16 bytes (alignment)
 */
USTRUCT()
struct FGrassInstance
{
    GENERATED_BODY()

    // World position of grass instance
    UPROPERTY()
    FVector Location = FVector::ZeroVector;
    
    // Growth progress: 0.0 (seed) to 1.0 (fully grown)
    UPROPERTY()
    float GrowthProgress = 0.0f;
    
    // Health: 1.0 (healthy) to 0.0 (dead)
    // Dies when water unavailable or flooded
    UPROPERTY()
    float Health = 1.0f;
    
    // Last time this instance was updated (for sparse updates)
    UPROPERTY()
    float LastUpdateTime = 0.0f;
    
    // Cached soil moisture (from water absorption)
    UPROPERTY()
    float SoilMoisture = 0.0f;
    
    // Which HISM component this belongs to (for multi-mesh support)
    UPROPERTY()
    int32 MeshIndex = 0;
    
    // Index in HISM component (for update/removal)
    UPROPERTY()
    int32 InstanceIndex = -1;
};

// ============================================================================
// SECTION 2: MAIN ACTOR CLASS
// ============================================================================

UCLASS(BlueprintType, Blueprintable)
class DRIFT_API AEcosystemController : public AActor, public IScalableSystem
{
    GENERATED_BODY()

public:
    AEcosystemController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    /**
     * Initialize ecosystem with terrain and water references
     * Called by MasterController during Phase 2 initialization
     */
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);

    // ===== VEGETATION SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bEnableVegetationGrowth = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationGrowthRate = 0.2f; // Growth per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationUpdateInterval = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxVegetationInstances = 10000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationData> VegetationTypes;
    
    // ===== GRASS GROWTH PARAMETERS (ALPHA SPRINT) =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float GrassMinMoisture = 0.1f; // Minimum water depth for grass survival
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float GrassMaxFloodDepth = 2.0f; // Max water depth before drowning
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float GrassDeathRate = 0.05f; // Health loss per second without water
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 GrassUpdateBudget = 100; // Grass instances updated per frame
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxGrassPerCell = 20; // Max grass in 10m x 10m cell
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Growth", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float SpatialGridCellSize = 1000.0f; // Cell size for density management (cm)

    // ===== BIOME SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeData> AvailableBiomes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bAutomaticBiomeTransitions = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionSmoothness = 0.5f;

    // ===== ENVIRONMENTAL FACTORS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseTemperature = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseHumidity = 0.6f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SeasonalVariation = 0.2f;

    // ===== CORE VEGETATION FUNCTIONS =====
    
    /**
     * Update all vegetation growth, health, and lifecycle
     * Uses sparse update pattern: only processes UpdateBudget instances per frame
     */
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateVegetation(float DeltaTime);
    
    /**
     * Update biome distribution based on environmental conditions
     */
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateBiomes();
    
    /**
     * Plant a new vegetation instance at location
     * Validates water, terrain, and density requirements
     */
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void PlantSeed(FVector Location, EVegetationType PlantType);
    
    /**
     * Remove all vegetation within radius
     * Used for terrain modification events
     */
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RemoveVegetationInRadius(FVector Location, float Radius);
    
    /**
     * Spawn initial grass coverage across terrain
     * Uses random placement with density constraints
     */
    UFUNCTION(BlueprintCallable, Category = "Grass System")
    void SpawnInitialGrassCoverage(int32 Count);

    // ===== BIOME QUERY FUNCTIONS =====
    
    UFUNCTION(BlueprintPure, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Biomes")
    float GetVegetationDensityAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Biomes")
    float GetTemperatureAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Biomes")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    // ===== SYSTEM COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnWeatherChanged(float Temperature, float Humidity);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnSeasonChanged(float SeasonValue);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnTerrainModified(FVector Location, float Radius);

    // ===== DEBUG & VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawBiomeMap(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowVegetationStats() const;
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowGrassStats() const;

    // ===== ISCALABLESYSTEM INTERFACE =====
    
    virtual void ConfigureFromMaster(const FWorldScalingConfig& Config) override;
    virtual void SynchronizeCoordinates(const FWorldCoordinateSystem& Coords) override;
    virtual bool IsSystemScaled() const override;
    
    // ===== MASTER CONTROLLER INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void RegisterWithMasterController(AMasterWorldController* Master);
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    bool IsRegisteredWithMaster() const;
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    FString GetScalingDebugInfo() const;
    
    // ===== TEMPORAL INTEGRATION =====
    
    /**
     * Main update function called by TemporalManager
     * DeltaTime is pre-scaled by time acceleration
     */
    UFUNCTION(BlueprintCallable, Category = "Temporal Integration")
    void UpdateEcosystemSystem(float DeltaTime);

    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    UPROPERTY()
    AAtmosphereController* AtmosphereController = nullptr;

private:
    // ===== INSTANCED MESH COMPONENTS =====
    
    /**
     * Map of vegetation type to HISM component
     * Alpha sprint: Only Grass populated
     * Each HISM automatically handles GPU instancing
     */
    UPROPERTY(VisibleAnywhere, Category = "Rendering")
    TMap<EVegetationType, UHierarchicalInstancedStaticMeshComponent*> VegetationMeshes;

    // ===== GRASS RUNTIME STATE (ALPHA SPRINT) =====
    
    /**
     * All active grass instances
     * Updated in sparse pattern (UpdateBudget per frame)
     */
    UPROPERTY()
    TArray<FGrassInstance> ActiveGrassInstances;
    
    /**
     * Spatial grid for O(1) density queries
     * Key: Grid cell coordinate (x + y * GridWidth)
     * Value: Indices into ActiveGrassInstances array
     *
     * NOTE: Not a UPROPERTY - UE reflection doesn't support TMap<int32, TArray<int32>>
     * This is runtime-only data that rebuilds on initialization
     */
    TMap<int32, TArray<int32>> GrassSpatialGrid;
    
    /**
     * Current index for sparse updates
     * Increments each frame to cycle through all grass
     */
    UPROPERTY()
    int32 GrassUpdateIndex = 0;

    // ===== LEGACY VEGETATION STATE (PRESERVED) =====
    
    UPROPERTY()
    float VegetationUpdateTimer = 0.0f;
    
    UPROPERTY()
    TArray<FVector> VegetationLocations;
    
    UPROPERTY()
    TArray<EVegetationType> VegetationTypes_Runtime;
    
    UPROPERTY()
    bool bSystemInitialized = false;
    
    // ===== WORLD SCALING STATE =====
    
    UPROPERTY()
    AMasterWorldController* MasterController = nullptr;
    
    FWorldScalingConfig CurrentWorldConfig;
    FWorldCoordinateSystem CurrentCoordinateSystem;
    
    UPROPERTY()
    bool bIsScaledByMaster = false;
    
    UPROPERTY()
    bool bIsRegisteredWithMaster = false;

    // ===== INTERNAL FUNCTIONS - RENDERING =====
    
    /**
     * Initialize all vegetation mesh components
     * Alpha sprint: Sets up grass HISM with UE5 built-in foliage
     */
    void InitializeVegetationMeshes();
    
    /**
     * Update wind parameters for grass material
     * Connects to AtmosphericSystem for wind data
     * Called once per frame, updates all grass materials
     */
    void UpdateWindParameters();
    
    /**
     * Legacy vegetation spawning (preserved)
     */
    void SpawnVegetationInstance(FVector Location, EVegetationType Type);
    
    /**
     * Legacy vegetation removal (preserved)
     */
    void RemoveVegetationInstance(int32 Index, EVegetationType Type);
    
    // ===== INTERNAL FUNCTIONS - GRASS GROWTH (ALPHA SPRINT) =====
    
    /**
     * Update grass growth, health, and lifecycle
     * Uses sparse update pattern: processes UpdateBudget instances per frame
     * Queries water system for moisture, removes dead grass
     */
    void UpdateGrassGrowth(float DeltaTime);
    
    /**
     * Spawn a single grass instance with validation
     * Checks water availability, terrain slope, density
     * Adds to HISM and spatial grid
     */
    void SpawnGrassInstance(FVector Location);
    
    /**
     * Remove grass instance from rendering and tracking
     */
    void RemoveGrassInstance(int32 GrassIndex);
    
    /**
     * Check if grass can grow at location
     * Validates: water depth, terrain slope, density, biome
     */
    bool CanGrassGrowAt(FVector Location) const;
    
    /**
     * Query soil moisture at location
     * Uses water system's absorption model
     */
    float GetSoilMoistureAt(FVector Location) const;
    
    // ===== INTERNAL FUNCTIONS - SPATIAL GRID =====
    
    /**
     * Convert world position to spatial grid cell
     * Returns cell key for TMap lookup
     */
    int32 WorldToGridCell(FVector WorldLocation) const;
    
    /**
     * Get number of grass instances in cell
     * Used for density management
     */
    int32 GetGrassCountInCell(int32 CellKey) const;
    
    // ===== INTERNAL FUNCTIONS - BIOMES (LEGACY) =====
    
    void UpdateVegetationGrowth(float DeltaTime);
    void CalculateBiomeDistribution();
    bool CanVegetationGrowAt(FVector Location, const FVegetationData& VegData) const;
};
