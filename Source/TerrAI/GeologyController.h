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

USTRUCT(BlueprintType)
struct FSimplifiedGeology
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERockType SurfaceRock = ERockType::Sandstone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness = 0.5f;  // 0-1 for erosion resistance

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterTableDepth = 10.0f;  // meters below surface

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SoilMoisture = 0.2f;  // 0-1 saturation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Permeability = 0.5f;  // 0-1 infiltration rate

    FSimplifiedGeology()
    {
        SurfaceRock = ERockType::Sandstone;
        Hardness = 0.5f;
        WaterTableDepth = 10.0f;
        SoilMoisture = 0.2f;
        Permeability = 0.5f;
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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float AverageWaterTableDepth = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float GroundwaterFlowRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float SpringFlowRate = 0.01f;  // m³/s per meter of head

    // ===== WATER CYCLE INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void SetWaterTableDepth(FVector Location, float Depth);
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void ReduceSoilMoisture(FVector Location, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    float GetSoilMoistureAt(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    float GetWaterTableDepthAtLocation(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    float GetInfiltrationRate(ERockType Rock) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Cycle")
    void ApplyInfiltration(FVector Location, float WaterAmount);

    // ===== CORE UPDATE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology Update")
    void UpdateGeologySystem(float DeltaTime);
    
    void UpdateSimplifiedWaterTable(float DeltaTime);
    void ProcessSurfaceWaterInfiltration(float DeltaTime);
    void ProcessGroundwaterDischarge(float DeltaTime);

    // ===== QUERIES =====
    
  //  UFUNCTION(BlueprintPure, Category = "Geology Query")
  //  ERockType e(FVector Location) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology Query")
    bool IsLocationAboveWaterTable(FVector Location) const;

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
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawSimplifiedDebugInfo() const;
    
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    

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
    float GetSoilCapacity(ERockType Rock) const;
    
    // Grid dimensions
    int32 GridWidth = 32;
    int32 GridHeight = 32;

};

