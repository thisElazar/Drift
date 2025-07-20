// AtmosphericSystem.h - Scientific Atmospheric Physics Engine
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "MasterController.h"
#include "GeologyController.h" // For ERockType
#include "Materials/MaterialParameterCollection.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AtmosphericSystem.generated.h"


// Forward declarations
class ADynamicTerrain;
class UWaterSystem;
class AGeologyController;
class AEcosystemController;
class UTemporalManager;
class AMasterWorldController;




// ===== ATMOSPHERIC PHYSICS CONSTANTS =====
namespace AtmosphericConstants
{
    constexpr float GAS_CONSTANT = 287.0f;           // J/(kg·K) for dry air
    constexpr float GRAVITY = 9.81f;                 // m/s²
    constexpr float LAPSE_RATE = -6.5f;             // °C/km standard atmosphere
    constexpr float WATER_VAPOR_PRESSURE = 611.2f;  // Pa at 0°C
    constexpr float LATENT_HEAT = 2.26e6f;          // J/kg vaporization
    constexpr float SPECIFIC_HEAT_AIR = 1005.0f;    // J/(kg·K)
    constexpr float CORIOLIS_COEFFICIENT = 1.458e-4f; // rad/s at 45° latitude
}

// Add this to AtmosphericSystem.h, replacing the current FAtmosphericCell struct

// ===== SIMPLIFIED ATMOSPHERIC CELL - CORE WATER CYCLE =====
USTRUCT(BlueprintType)
struct TERRAI_API FSimplifiedAtmosphericCell
{
    GENERATED_BODY()

    // Core state (reduced from 20+ variables to 6)
    float Temperature = 288.15f;         // Kelvin
    float Humidity = 0.5f;               // 0-1 relative
    float MoistureMass = 10.0f;          // kg/m² actual water (KEY VARIABLE)
    float CloudCover = 0.0f;             // 0-1 visual
    float PrecipitationRate = 0.0f;      // mm/hr
    FVector2D WindVector = FVector2D(5, 0); // m/s surface wind

    // Helper functions
    float GetSaturationCapacity() const
    {
        // Simplified Magnus formula
        float TempC = Temperature - 273.15f;
        float SaturationPressure = 6.11f * FMath::Exp((17.27f * TempC) / (TempC + 237.3f));
        return SaturationPressure * 0.01f; // Convert to kg/m²
    }

    bool IsCondensationLevel() const
    {
        return MoistureMass > GetSaturationCapacity();
    }
};

// ===== WEATHER PATTERN SYSTEM =====
UENUM(BlueprintType)
enum class EWeatherPattern : uint8
{
    HighPressure,       // Clear, stable weather
    LowPressure,        // Unstable, stormy weather
    FrontalSystem,      // Moving weather boundaries
    ConvectiveStorm,    // Thunderstorms from heating
    OrographicLift,     // Mountain-induced precipitation
    SeaBreeze,          // Coastal wind patterns
    TemperatureInversion // Atmospheric layering
};

USTRUCT(BlueprintType)
struct TERRAI_API FWeatherPattern
{
    GENERATED_BODY()
    
    EWeatherPattern PatternType = EWeatherPattern::HighPressure;
    FVector2D Center = FVector2D::ZeroVector;        // Pattern center location
    float Intensity = 1.0f;                         // Pattern strength
    float Radius = 10000.0f;                        // Influence radius (meters)
    FVector2D Movement = FVector2D::ZeroVector;      // Pattern movement velocity
    float LifeTime = 3600.0f;                       // Duration in seconds
    float Age = 0.0f;                               // Current age
    
    // Pattern-specific parameters
    TMap<FString, float> Parameters;
};

// ===== CORE ATMOSPHERIC SYSTEM =====
UCLASS(BlueprintType)
class TERRAI_API UAtmosphericSystem : public UObject, public IScalableSystem
{
    GENERATED_BODY()

public:
    UAtmosphericSystem();
    
    // ===== ISCALABLESYSTEM INTERFACE =====
    
    UFUNCTION(BlueprintCallable, Category = "Master Controller Integration")
    virtual void ConfigureFromMaster(const FWorldScalingConfig& Config) override;
    
    UFUNCTION(BlueprintCallable, Category = "Master Controller Integration")
    virtual void SynchronizeCoordinates(const FWorldCoordinateSystem& Coords) override;
    
    UFUNCTION(BlueprintPure, Category = "Master Controller Integration")
    virtual bool IsSystemScaled() const override;
    
    // ===== MASTER CONTROLLER COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Master Controller Integration")
    void RegisterWithMasterController(class AMasterWorldController* Master);
    
    UFUNCTION(BlueprintPure, Category = "Master Controller Integration")
    bool IsRegisteredWithMaster() const { return MasterController != nullptr; }
    
    UFUNCTION(BlueprintPure, Category = "Master Controller Integration")
    FString GetScalingDebugInfo() const;

    // ===== INITIALIZATION =====
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void Initialize(ADynamicTerrain* InTerrain, UWaterSystem* InWaterSystem);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void InitializeAtmosphericGrid();

    // ===== CORE SIMULATION =====
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Physics")
    void UpdateAtmosphericSimulation(float DeltaTime);
    
    // ===== RESET SIMULATION =====
    UFUNCTION(BlueprintCallable)
    void ResetAtmosphere();

    
    // ===== ATMOSPHERIC GRID SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    int32 GridWidth = 32;               // Reduced resolution for 2D grid
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    int32 GridHeight = 32;
    
    // REMOVED: GridLayers - no more 3D grid!
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    float CellSize = 2000.0f;           // Larger cells for simplified grid
    

    // 2D atmospheric state (was 3D)
    TArray<FSimplifiedAtmosphericCell> AtmosphericGrid;

    
    // ===== CLIMATE PARAMETERS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float BaseTemperature = 288.15f;    // Kelvin (15°C)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float SeasonalAmplitude = 10.0f;    // °C seasonal variation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float DiurnalAmplitude = 8.0f;      // °C day/night variation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float AltitudeGradient = 6.5f;      // °C/km cooling with elevation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float LatitudeGradient = 0.5f;      // °C/degree latitude cooling
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector2D WindPattern = FVector2D(5.0f, 0.0f); // m/s prevailing wind

    // ===== WEATHER PATTERNS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Patterns")
    TArray<FWeatherPattern> ActiveWeatherPatterns;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Patterns")
    float PatternGenerationRate = 0.0001f;  // Patterns per second per cell
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Patterns")
    bool bEnableConvectiveStorms = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Patterns")
    bool bEnableFrontalSystems = true;

    // ===== SYSTEM INTERFACES =====
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    UPROPERTY()
    ADynamicTerrain* TerrainSystem = nullptr;
    
    // ===== TEMPORAL INTEGRATION =====
    UPROPERTY()
    UTemporalManager* TemporalManager = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Temporal Settings")
    bool bUseTemporalManager = true;
    
    UPROPERTY(EditAnywhere, Category = "Temporal Settings")
    float TemporalUpdateFrequency = 1.0f;

    // ===== QUERY INTERFACE =====
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetTemperatureAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetPressureAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetHumidityAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    FVector GetWindAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetPrecipitationAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetCloudCoverAt(FVector WorldPosition) const;

    
    // ===== CONTROL INTERFACE =====

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateWeatherEffect(FVector2D Location, float Radius, float Intensity);


    void ProcessCondensationAndPrecipitation(float DeltaTime);
    
    UFUNCTION(CallInEditor, Category = "Testing")
    void TriggerImmediatePrecipitation(FVector2D Location, float Radius, float IntensityMMPerHour);

    // ===== DEBUG VISUALIZATION =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowPressureMap = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowWindVectors = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowHumidityMap = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowWeatherPatterns = false;

    // ===== CLOUD RENDERING SYSTEM =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    bool bEnableCloudRendering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    float CloudAltitude = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    float CloudThickness = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    int32 MaxCloudMeshes = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    UStaticMesh* CloudStaticMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Rendering")
    UMaterialInterface* CloudMaterial = nullptr;

    // ===== PERFORMANCE SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 4.0f;       // Hz - atmospheric updates per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAdaptiveTimeStep = true;
    
    
    // ===== DEBUG COMMANDS =====
        UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
        void DebugAtmosphericState();
        
        UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
        void DebugCloudSystem();
        
        UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
        void ForceGenerateClouds(float Coverage = 0.8f);
        
        UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
        void DebugMoistureDistribution();
        
        UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
        void EnableAtmosphericDebugVisualization(bool bEnable);
    
    

private:
    // ===== MASTER CONTROLLER INTEGRATION =====
    
    UPROPERTY()
    class AMasterWorldController* MasterController = nullptr;
    
    // Scaling state from master controller
    FWorldScalingConfig CurrentWorldConfig;
    FWorldCoordinateSystem CurrentCoordinateSystem;
    bool bIsScaledByMaster = false;
    
    // Master coordinate transformation helpers
    FVector WorldToAtmosphericGrid(FVector WorldPos) const;
    FVector AtmosphericGridToWorld(int32 X, int32 Y, int32 Z) const;
    bool IsMasterCoordinateValid(int32 X, int32 Y, int32 Z) const;
    
    // ===== ATMOSPHERIC PHYSICS =====
    
    // Core physics update steps
 
    void UpdateCloudPhysics(float DeltaTime);
    
    // Thermodynamic calculations
    void CalculateTemperatureField();
    void CalculatePressureField();
    void CalculateHumidityTransport(float DeltaTime);
    void ProcessPhaseChanges(float DeltaTime);
    
 

    // ===== WEATHER PATTERN SYSTEMS =====
    void UpdateWeatherPatterns(float DeltaTime);
    void ApplyWeatherPatternToGrid(const FWeatherPattern& Pattern);
    void GenerateNewWeatherPattern();
    void RemoveExpiredWeatherPatterns();
    
    
    // ===== BOUNDARY CONDITIONS =====
    void ApplyTerrainInteraction();
    void ProcessSurfaceHeatFlux();
    void CalculateEvapotranspiration();
    void ApplyTopographicEffects();
    
  
private:
    // Grid access (now 2D only)
    int32 GetGridIndex(int32 X, int32 Y) const;
    FSimplifiedAtmosphericCell& GetCell(int32 X, int32 Y);
    const FSimplifiedAtmosphericCell& GetCell(int32 X, int32 Y) const;

    // Interpolation for smooth field queries
    float InterpolateField(const TArray<float>& Field, FVector WorldPosition) const;
    FVector InterpolateVectorField(const TArray<FVector>& Field, FVector WorldPosition) const;
    
    // Coordinate transformations
    FVector2D WorldToGridCoordinates(FVector WorldPosition) const;
    FVector GridToWorldCoordinates(int32 X, int32 Y) const;
    
    
    //FVector2D WorldToTerrainCoordinates(FVector WorldPosition) const;
    
    // ===== INTEGRATION WITH OTHER SYSTEMS =====
    void UpdateWaterSystemInterface();
    void UpdateTerrainInterface();
    
    // ===== CLOUD RENDERING INTERNALS =====
    UPROPERTY()
    TArray<UStaticMeshComponent*> CloudMeshes;

    UPROPERTY()
    UMaterialInstanceDynamic* CloudMaterialInstance = nullptr;

    UPROPERTY()
    USceneComponent* AttachParent = nullptr;

    // Cloud rendering functions
    void InitializeCloudRendering(USceneComponent* InAttachParent);
    void UpdateCloudRendering(float DeltaTime);
    void UpdateCloudMeshes();
    void CreateCloudMesh(int32 AtmosphericX, int32 AtmosphericY, float CloudCover);
    void UpdateCloudMesh(UStaticMeshComponent* CloudMesh, float CloudCover, FVector Position);
    UStaticMeshComponent* GetOrCreateCloudMesh();
    void HideUnusedCloudMeshes(int32 UsedMeshCount);
    
    // Cloud data conversion
    float GetCloudCoverageAt(int32 X, int32 Y) const;
    FVector GetCloudWorldPosition(int32 AtmosphericX, int32 AtmosphericY) const;
    FVector GetPlayerPosition() const;

    // ===== PERFORMANCE OPTIMIZATION =====
    float AccumulatedTime = 0.0f;
    int32 CurrentUpdateLayer = 0;       // For spreading computation across frames
    
    // ===== CONSTANTS AND LOOKUP TABLES =====
    static const TArray<float> StandardAtmosphere_Temperature;
    static const TArray<float> StandardAtmosphere_Pressure;
    static const TArray<float> SaturationVaporPressure_LookupTable;
    
    
    
    
    // ===== PHASE 2: MOISTURE TRANSPORT =====
    void AdvectMoisture(float DeltaTime);
    float CalculateSaturationMoisture(float TempKelvin);
    void UpdateCloudCoverFromMoisture();
    
    // ===== PHASE 3: OROGRAPHIC EFFECTS ====
    void ApplyOrographicEffects(float DeltaTime);
    FVector2D GetTerrainGradient(int32 GridX, int32 GridY);
    void ClearConsumedPrecipitation();
    
    // ===== PHASE 4: EVAPORATION SYSTEM  =====
    void ProcessEvaporation(float DeltaTime);
    void ProcessEvapotranspiration(float DeltaTime);
    
    // ===== PHASE 5: INFILTRATION & GROUNDWATER (Coming Soon) =====
    void ProcessPrecipitation(float DeltaTime);
    void ProcessGroundwaterDischarge(float DeltaTime);
    void UpdateSimplifiedWaterTable(float DeltaTime);
    
    void LogPrecipitationActivity();
    
    // ===== PHASE 5: INFILTRATION & GROUNDWATER =====
     // float GetInfiltrationRate(ERockType Rock);
      
      // Constants for geology interaction
      static const int32 GeologyGridWidth;
      static const int32 GeologyGridHeight;
      static const float SpringFlowRate;
    
    void ApplyWeatherPatterns();
};


   
