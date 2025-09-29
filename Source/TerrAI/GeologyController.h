// GeologyController.h - Simplified for emergent water cycle behaviors

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrAI.h"
#include "MasterController.h"
#include "GeologyController.generated.h"

// Forward declarations
class ADynamicTerrain;
class UWaterSystem;
class AMasterWorldController;

/**
 * ============================================
 * SIMPLIFIED GEOLOGY TYPES
 * ============================================
 */

UENUM(BlueprintType)
enum class ERockType : uint8
{
    Sand        UMETA(DisplayName = "Sand"),
    Clay        UMETA(DisplayName = "Clay"),
    Silt        UMETA(DisplayName = "Silt"),
    Sandstone   UMETA(DisplayName = "Sandstone"),
    Limestone   UMETA(DisplayName = "Limestone"),
    Granite     UMETA(DisplayName = "Granite"),
    Gravel      UMETA(DisplayName = "Gravel"),
    Shale       UMETA(DisplayName = "Shale"),
    Basalt      UMETA(DisplayName = "Basalt")
};

// ===== USER-CREATED SPRINGS =====

USTRUCT(BlueprintType)
struct FUserSpring
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowRate = 1.0f; // m³/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive = true;
    
    FUserSpring()
    {
        Location = FVector::ZeroVector;
        FlowRate = 1.0f;
        bActive = true;
    }
};

USTRUCT(BlueprintType)
struct FSimplifiedGeology
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERockType SurfaceRock = ERockType::Sandstone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness = 0.5f;  // 0-1 for erosion resistance

    // Removed per-cell water table - using global system instead

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SoilMoisture = 0.2f;  // 0-1 saturation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Permeability = 0.5f;  // 0-1 infiltration rate
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
       float HydraulicHead = 0.0f;  // Absolute elevation of groundwater pressure
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite)
       float Transmissivity = 0.01f;  // m²/s - horizontal flow capacity
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite)
       float StorageCoefficient = 0.3f;  // Effective porosity for water storage
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite)
       float LastTerrainHeight = 0.0f;  // Track terrain changes


    FSimplifiedGeology()
    {
        SurfaceRock = ERockType::Sandstone;
        Hardness = 0.5f;
        SoilMoisture = 0.2f;
        Permeability = 0.5f;
        HydraulicHead = 0.0f;
        Transmissivity = 0.01f;
        StorageCoefficient = 0.3f;
        LastTerrainHeight = 0.0f;
    }
};

/**
 * ============================================
 * SIMPLIFIED GEOLOGY CONTROLLER
 * ============================================
 * Manages water table, soil moisture, and rock properties
 * for emergent water cycle behaviors
 */
UCLASS()
class TERRAI_API AGeologyController : public AActor, public IScalableSystem
{
    GENERATED_BODY()
    
public:
    AGeologyController();

    // ===== COMPATIBILITY WITH EXISTING SYSTEMS =====
    
    // These methods maintain compatibility with existing water system
    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    ERockType GetRockTypeAtLocation(FVector WorldLocation, float Depth) const;
    
    // Simplified erosion handler - no actual erosion in simplified system
    void OnErosionOccurred(FVector Location, float ErosionAmount, ERockType ErodedType) {}
    



protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);

    // ===== SIMPLIFIED GEOLOGY GRID =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geology Grid")
    TArray<FSimplifiedGeology> GeologyGrid;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geology Grid")
    int32 GeologyGridWidth = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geology Grid")
    int32 GeologyGridHeight = 0;

    // ===== WATER TABLE SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    bool bEnableWaterTable = true;
    
    // ===== SIMPLIFIED GLOBAL WATER TABLE =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Water Table", 
        meta = (ClampMin = "-4000.0", ClampMax = "500.0"))
    float GlobalWaterTableElevation = -3500.0f;  // Absolute world height
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Water Table")
    bool bUseProportionalWaterTable = false;  // If true, set as % of terrain height
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Water Table",
        meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bUseProportionalWaterTable"))
    float WaterTableHeightPercent = 0.02f;  // 2% of max terrain height
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Global Water Table")
    float GlobalWaterTableVolume = 0.0f;  // m³ - tracked by water budget

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Water Table")
    float GlobalPorosity = 0.3f;  // 30% void space in rock

    // ===== WATER CYCLE INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void SetWaterTableDepth(FVector Location, float Depth);
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void ReduceSoilMoisture(FVector Location, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    float GetSoilMoistureAt(FVector Location) const;
    
    UFUNCTION(BlueprintPure, Category = "Global Water Table")
    float GetWaterTableDepthAtLocation(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Global Water Table")
    void SetGlobalWaterTableElevation(float NewElevation);
    
    UFUNCTION(BlueprintCallable, Category = "Global Water Table")
    void AddWaterToWaterTable(float VolumeM3);
    
    UFUNCTION(BlueprintCallable, Category = "Global Water Table")
    float RemoveWaterFromWaterTable(float VolumeM3);
    
    UFUNCTION(BlueprintPure, Category = "Global Water Table")
    float GetGroundwaterVolume() const { return GlobalWaterTableVolume; }
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    float GetInfiltrationRate(ERockType Rock) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void ApplyInfiltration(FVector Location, float WaterAmount);
    
    // ===== WATER TABLE INITIALIZATION =====
    UFUNCTION(BlueprintCallable, Category = "Water Table")
    void InitializeWaterTableFill();

    // ===== CORE UPDATE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology Update")
    void UpdateGeologySystem(float DeltaTime);


    UFUNCTION(BlueprintCallable, Category = "Water Table")
    void CheckInitialWaterTable();

    UFUNCTION(BlueprintCallable, Category = "Water Table")
    void OnWaterTableElevationChanged();
    
   // void ProcessSurfaceWaterInfiltration(float DeltaTime);
    void InitializeWaterTable();
    void UpdateWaterTableFromVolume();
    void UpdateWaterTableDebugVisualization();

    // ===== QUERIES =====

    
    UFUNCTION(BlueprintPure, Category = "Geology Query")
    bool IsLocationAboveWaterTable(FVector Location) const;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Springs")
    TArray<FUserSpring> UserSprings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Springs", 
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DefaultSpringFlowRate = 1.0f; // m³/s - Blueprint controllable

    UFUNCTION(BlueprintCallable, Category = "User Springs")
    void AddUserSpring(FVector WorldLocation, float FlowRate = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "User Springs")
    void RemoveUserSpring(FVector WorldLocation, float SearchRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "User Springs")
    void ClearAllUserSprings();

    // Process springs in the update loop
    void ProcessUserSprings(float DeltaTime);

    // ===== ISCALABLESYSTEM INTERFACE =====
    
    virtual void ConfigureFromMaster(const FWorldScalingConfig& Config) override;
    virtual void SynchronizeCoordinates(const FWorldCoordinateSystem& Coords) override;
    virtual bool IsSystemScaled() const override;

    // ===== MASTER CONTROLLER INTEGRATION =====
    
    void RegisterWithMasterController(AMasterWorldController* Master);
    bool IsRegisteredWithMaster() const;
    FString GetScalingDebugInfo() const;

    // ===== VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowWaterTable(bool bEnable);
    
 //   UFUNCTION(BlueprintCallable, Category = "Debug")
 //   void DrawSimplifiedDebugInfo() const;
    
    // Debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowWaterTablePlane = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FColor WaterTablePlaneColor = FColor(0, 100, 255, 128);
    
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    
    // ===== HYDRAULIC PROPERTIES =====
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
           meta = (ClampMin = "0.000001", ClampMax = "0.1"))
       float BaseHydraulicConductivity = 0.001f; // m/s for sandy soil
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
           meta = (ClampMin = "0.1", ClampMax = "10.0"))
       float VerticalConductivityMultiplier = 1.0f; // Often different from horizontal
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
           meta = (ClampMin = "0.1", ClampMax = "10.0"))
       float EffectiveFlowPathLength = 1.0f; // Distance water travels through soil
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance",
           meta = (ClampMin = "0.05", ClampMax = "1.0"))
       float SeepageUpdateInterval = 0.1f; // How often to update seepage (seconds)
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance",
           meta = (ClampMin = "4", ClampMax = "32"))
       int32 SeepageSampleInterval = 8; // Sample every N terrain cells
       
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
           meta = (ClampMin = "0.001", ClampMax = "1.0"))
       float MinimumHeadDifference = 0.01f; // Ignore seepage below this threshold (meters)
       
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
        meta = (ClampMin = "0.001", ClampMax = "1.0"))
    float BaseSeepageFlowRate = 0.1f;  // Base m³/s per seepage point

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Properties",
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SeepageFlowMultiplier = 1.0f;  // Global multiplier for all seepage
    
       UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
       bool bEnableDynamicWaterTable = false; // Allow water table to drop during seepage
/*
   private:
       // Seepage tracking
       float SeepageUpdateTimer = 0.0f;
       TMap<FIntPoint, float> ActiveSeepagePoints; // Track cells with active seepage
       
       // Core seepage function (replaces ProcessWaterTableEmergence)
       void ProcessHydraulicSeepage(float DeltaTime);
       
       // Helper functions
       void UpdateActiveSeepagePoints();
   
       
   public:
       // Called when terrain is modified
       UFUNCTION(BlueprintCallable, Category = "Water Table")
       void OnTerrainModified(const TArray<FVector>& ModifiedPositions);
    */

protected:
    // ===== SYSTEM REFERENCES =====
    

    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    UPROPERTY()
    AMasterWorldController* MasterController = nullptr;

    // ===== SCALING STATE =====
    
    bool bIsScaledByMaster = false;
    bool bIsRegisteredWithMaster = false;
    bool bSystemInitialized = false;
    
    FWorldCoordinateSystem CurrentCoordinateSystem;
    FVector GridOrigin;
    float CellSize = 100.0f;  // Size of each geology cell in world units

    // ===== TEMPORARY ARRAYS FOR COMPUTATION =====
    
    TArray<float> TempWaterTableDepths;
    TArray<float> TempSoilMoisture;
    
private:
    // Grid utility methods
    void InitializeGeologyGrid();
    int32 GetGridIndex(int32 X, int32 Y) const;
    FVector2D WorldToGridCoordinates(const FVector& WorldPosition) const;
    FVector2D GeologyGridToWorldCoordinates(int32 X, int32 Y) const;
    bool IsValidGridCoordinate(int32 X, int32 Y) const;
    bool IsEdgeCell(int32 X, int32 Y) const;
    float GetSoilCapacity(ERockType Rock) const;
    float GetTotalWorldArea() const;
    
    float GetHydraulicConductivityAt(FVector WorldPos) const;
    
    // Grid dimensions
    int32 GridWidth = 32;
    int32 GridHeight = 32;
    
    // Water table initialization state
    bool bWaterTableInitialized = false;
    
    // Debug mesh for water table visualization
    class UProceduralMeshComponent* WaterTableDebugMesh = nullptr;
    


};

