/**
 * ============================================
 * TERRAI GEOLOGY CONTROLLER
 * ============================================
 * Purpose: Rock formation, erosion behaviors, water table simulation
 * Scope: Sand dunes, sediment transport, groundwater flow
 * Integration: Realistic erosion patterns, water table effects
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Map.h"
#include "GeologyController.generated.h"

class UWaterSystem;
class ADynamicTerrain;

UENUM(BlueprintType)
enum class ERockType : uint8
{
    Granite        UMETA(DisplayName = "Granite"),
    Sandstone      UMETA(DisplayName = "Sandstone"),
    Limestone      UMETA(DisplayName = "Limestone"),
    Clay           UMETA(DisplayName = "Clay"),
    Sand           UMETA(DisplayName = "Sand"),
    Silt           UMETA(DisplayName = "Silt"),
    Gravel         UMETA(DisplayName = "Gravel"),
    Bedrock        UMETA(DisplayName = "Bedrock")
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
    float Hardness = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Porosity = 0.3f; // 0-1, affects water table
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Permeability = 0.5f; // 0-1, water flow rate
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ErosionResistance = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LayerColor = FLinearColor::Gray;
};

USTRUCT(BlueprintType)
struct FSandDuneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WindDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SandAccumulation = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stability = 1.0f; // resistance to wind erosion
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AGeologyController : public AActor
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
    TArray<FRockLayer> RockLayers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float SedimentationRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float BedrockDepth = 100.0f;

    // ===== SAND DUNE SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    bool bEnableSandDunes = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    TArray<FSandDuneData> SandDunes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    float WindStrength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    FVector PrevailingWindDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    float SandTransportRate = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    float DuneFormationThreshold = 10.0f;

    // ===== WATER TABLE SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    bool bEnableWaterTable = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float WaterTableDepth = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float GroundwaterFlowRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float WaterTableRechargeRate = 0.05f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Table")
    float SpringFormationThreshold = 20.0f;

    // ===== EROSION & WEATHERING =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableChemicalWeathering = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringRate = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float FrostWedgingEffect = 1.2f;

    // ===== CORE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void UpdateGeologicalProcesses(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void UpdateSandDunes(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void UpdateWaterTable(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void FormSedimentLayer(FVector Location, float Thickness, ERockType RockType);

    // ===== GEOLOGICAL QUERIES =====
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    ERockType GetRockTypeAtLocation(FVector WorldLocation, float Depth = 0.0f) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetRockHardnessAtLocation(FVector WorldLocation, float Depth = 0.0f) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetWaterTableDepthAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Geology")
    float GetGroundwaterFlowAtLocation(FVector WorldLocation) const;

    // ===== SYSTEM COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnWaterFlowChanged(FVector Location, float FlowRate);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnWindChanged(FVector WindDirection, float WindSpeed);
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnErosionOccurred(FVector Location, float ErosionAmount);

    // ===== VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowWaterTable(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowSandDunes(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowRockLayers(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintGeologicalStats() const;

private:
    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;

    // ===== INTERNAL STATE =====
    
    // Use simpler storage for UE5.4 compatibility
    UPROPERTY()
    TArray<FRockLayer> DefaultRockLayers;
    
    UPROPERTY()
    TArray<FVector> WaterTableGrid;
    
    UPROPERTY()
    float GeologicalTimer = 0.0f;
    
    UPROPERTY()
    bool bSystemInitialized = false;

    // ===== INTERNAL FUNCTIONS =====
    
    void InitializeGeologicalLayers();
    void ProcessSedimentation(float DeltaTime);
    void ProcessWeathering(float DeltaTime);
    void ProcessSandTransport(float DeltaTime);
    void ProcessGroundwaterFlow(float DeltaTime);
    void CreateSpring(FVector Location);
    FRockLayer CreateDefaultRockLayer(ERockType Type) const;
    float CalculateErosionResistance(const FRockLayer& Layer) const;
};