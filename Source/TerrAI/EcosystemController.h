/**
 * ============================================
 * TERRAI ECOSYSTEM CONTROLLER
 * ============================================
 * Purpose: Vegetation, wildlife, and biological systems
 * Scope: Biome management, plant growth, species distribution
 * Integration: Water-dependent growth, climate-based biomes
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MasterController.h"
#include "EcosystemController.generated.h"

class UWaterSystem;
class ADynamicTerrain;

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

USTRUCT(BlueprintType)
struct FVegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVegetationType VegetationType = EVegetationType::Grass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* VegetationMesh = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinWaterRequirement = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OptimalWaterLevel = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrowthRate = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ElevationRange = FVector2D(0.0f, 1000.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EBiomeType> PreferredBiomes;
};

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType = EBiomeType::Grassland;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D TemperatureRange = FVector2D(10.0f, 30.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D HumidityRange = FVector2D(0.3f, 0.8f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ElevationRange = FVector2D(0.0f, 500.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVegetationData> NativeVegetation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BiomeColor = FLinearColor::Green;
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AEcosystemController : public AActor, public IScalableSystem
{
    GENERATED_BODY()

public:
    AEcosystemController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);

    // ===== VEGETATION SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bEnableVegetationGrowth = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationGrowthRate = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationUpdateInterval = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxVegetationInstances = 10000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationData> VegetationTypes;

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

    // ===== CORE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateVegetation(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateBiomes();
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void PlantSeed(FVector Location, EVegetationType PlantType);
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RemoveVegetationInRadius(FVector Location, float Radius);

    // ===== BIOME FUNCTIONS =====
    
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
    
    UFUNCTION(BlueprintCallable, Category = "Temporal Integration")
    void UpdateEcosystemSystem(float DeltaTime);
    
    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;

private:
  

    // ===== INSTANCED MESH COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, Category = "Rendering")
    TMap<EVegetationType, UInstancedStaticMeshComponent*> VegetationMeshes;

    // ===== INTERNAL STATE =====
    
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

    
    // ===== INTERNAL FUNCTIONS =====
    
    void InitializeVegetationMeshes();
    void UpdateVegetationGrowth(float DeltaTime);
    void CalculateBiomeDistribution();
    bool CanVegetationGrowAt(FVector Location, const FVegetationData& VegData) const;
    void SpawnVegetationInstance(FVector Location, EVegetationType Type);
    void RemoveVegetationInstance(int32 Index, EVegetationType Type);
};
