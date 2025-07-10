/**
 * ============================================
 * TERRAI GEOLOGY CONTROLLER (REFACTORED)
 * ============================================
 * Purpose: Rock formation, stratigraphy, water table, and geological processes
 * Scope: Bedrock geology, mineral deposits, erosion, weathering
 * Sand dunes moved to separate SandDuneController
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Map.h"
#include "MasterController.h"
#include "GeologyController.generated.h"

class UWaterSystem;
class ADynamicTerrain;
class ASandDuneController;

UENUM(BlueprintType)
enum class ERockType : uint8
{
    Granite        UMETA(DisplayName = "Granite"),
    Sandstone      UMETA(DisplayName = "Sandstone"),
    Limestone      UMETA(DisplayName = "Limestone"),
    Shale          UMETA(DisplayName = "Shale"),
    Basalt         UMETA(DisplayName = "Basalt"),
    Marble         UMETA(DisplayName = "Marble"),
    Quartzite      UMETA(DisplayName = "Quartzite"),
    Clay           UMETA(DisplayName = "Clay"),
    Sand           UMETA(DisplayName = "Sand"),
    Silt           UMETA(DisplayName = "Silt"),
    Gravel         UMETA(DisplayName = "Gravel"),
    Bedrock        UMETA(DisplayName = "Bedrock")
};

UENUM(BlueprintType)
enum class EGeologicalProcess : uint8
{
    Sedimentation  UMETA(DisplayName = "Sedimentation"),
    Erosion        UMETA(DisplayName = "Erosion"),
    Weathering     UMETA(DisplayName = "Weathering"),
    Metamorphism   UMETA(DisplayName = "Metamorphism"),
    Volcanic       UMETA(DisplayName = "Volcanic Activity"),
    Tectonic       UMETA(DisplayName = "Tectonic Movement"),
    Glacial        UMETA(DisplayName = "Glacial Action")
};

USTRUCT(BlueprintType)
struct FRockLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERockType RockType = ERockType::Sandstone;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thickness = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Depth = 0.0f; // Depth from surface
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness = 1.0f; // 0-1, affects erosion
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Porosity = 0.3f; // 0-1, affects water table
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Permeability = 0.5f; // 0-1, water flow rate
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ErosionResistance = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Age = 0.0f; // Geological age in millions of years
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LayerColor = FLinearColor::Gray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bContainsFossils = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasMineralDeposits = false;
};

USTRUCT(BlueprintType)
struct FGeologicalColumn
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRockLayer> Layers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDepth = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasFaultLine = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FaultDisplacement = 0.0f;
};

USTRUCT(BlueprintType)
struct FMineralDeposit
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MineralType = "Iron";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Concentration = 0.5f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1000.0f; // m³
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Depth = 50.0f;
};

USTRUCT(BlueprintType)
struct FWaterTableData
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float WaterLevel = 0.0f; // Absolute Z height
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float FlowVelocity = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector FlowDirection = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsArtesian = false;
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AGeologyController : public AActor, public IScalableSystem
{
    GENERATED_BODY()

public:
    AGeologyController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);

    // ===== ROCK FORMATION SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    bool bEnableGeologicalProcesses = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    TArray<FGeologicalColumn> GeologicalColumns;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float SedimentationRate = 0.1f; // meters per geological time unit
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float BedrockDepth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float GeologicalTimeScale = 1000000.0f; // 1 million years per time unit

    // ===== STRATIGRAPHY =====
    
    UFUNCTION(BlueprintCallable, Category = "Stratigraphy")
    void GenerateStratigraphicColumn(FVector Location);
    
    // C++ only - pointers cannot be exposed to Blueprint
    FGeologicalColumn* GetColumnAtLocation(FVector Location);
    const FGeologicalColumn* GetColumnAtLocation(FVector Location) const;
    
    // Blueprint-friendly version that returns a copy
    UFUNCTION(BlueprintPure, Category = "Stratigraphy")
    FGeologicalColumn GetColumnAtLocationCopy(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Stratigraphy")
    void AddGeologicalLayer(FVector Location, const FRockLayer& NewLayer);

    // ===== WATER TABLE SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    bool bEnableWaterTable = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    TArray<FWaterTableData> WaterTableGrid;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float AverageWaterTableDepth = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float GroundwaterFlowRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float WaterTableRechargeRate = 0.05f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float SpringFormationThreshold = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float ArtesianPressureThreshold = 30.0f;

    // ===== EROSION & WEATHERING =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableChemicalWeathering = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableMechanicalWeathering = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float ChemicalWeatheringRate = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MechanicalWeatheringRate = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float FrostWedgingEffect = 1.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float AcidRainPH = 5.6f;

    // ===== MINERAL DEPOSITS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    TArray<FMineralDeposit> MineralDeposits;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    bool bGenerateMineralDeposits = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    float MineralFormationProbability = 0.1f;

    // ===== CORE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void UpdateGeologicalProcesses(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void UpdateWaterTable(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void FormSedimentLayer(FVector Location, float Thickness, ERockType RockType);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void ApplyMetamorphism(FVector Location, float Pressure, float Temperature);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void CreateFaultLine(FVector Start, FVector End, float Displacement);

    // ===== GEOLOGICAL QUERIES =====
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    ERockType GetRockTypeAtLocation(FVector WorldLocation, float Depth = 0.0f) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetRockHardnessAtLocation(FVector WorldLocation, float Depth = 0.0f) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetWaterTableDepthAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetGroundwaterFlowAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    bool IsLocationAboveWaterTable(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    TArray<FMineralDeposit> GetNearbyMineralDeposits(FVector Location, float Radius) const;

    // ===== SYSTEM COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnWaterFlowChanged(FVector Location, float FlowRate);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnErosionOccurred(FVector Location, float ErosionAmount, ERockType ErodedType);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnTemperatureChanged(float NewTemperature);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    ASandDuneController* GetSandDuneController() const { return SandDuneController; }
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void SetSandDuneController(ASandDuneController* Controller);

    // ===== VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowWaterTable(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowRockLayers(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowMineralDeposits(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowGeologicalColumns(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintGeologicalStats() const;

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

private:
    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    UPROPERTY()
    ASandDuneController* SandDuneController = nullptr;

    // ===== INTERNAL STATE =====
    
    UPROPERTY()
    float GeologicalTimer = 0.0f;
    
    UPROPERTY()
    float CurrentTemperature = 20.0f; // Celsius
    
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

    // ===== INTERNAL FUNCTIONS =====
    
    void InitializeGeologicalColumns();
    void ProcessSedimentation(float DeltaTime);
    void ProcessWeathering(float DeltaTime);
    void ProcessErosion(float DeltaTime);
    void ProcessGroundwaterFlow(float DeltaTime);
    void UpdateAquiferPressure(float DeltaTime);
    void CreateSpring(FVector Location);
    void GenerateMineralDeposit(FVector Location, ERockType HostRock);
    FRockLayer CreateDefaultRockLayer(ERockType Type) const;
    float CalculateErosionResistance(const FRockLayer& Layer) const;
    ERockType GetMetamorphicEquivalent(ERockType Original) const;
    void ProcessAccumulatedSediment();
    
    // Performance optimization - accumulate sediment before creating layers
    UPROPERTY()
    TMap<FIntVector, float> AccumulatedSediment;
    
    UPROPERTY()
    float SedimentAccumulationTimer = 0.0f;
    
    UPROPERTY()
    float SedimentFormationInterval = 10.0f; // Only form layers every 10 seconds
    
    UPROPERTY()
    float MinimumSedimentThickness = 1.0f; // Don't create layers smaller than 1m
};
