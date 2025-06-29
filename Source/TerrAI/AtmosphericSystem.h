// AtmosphericSystem.h - Scientific Atmospheric Physics Engine
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "MasterController.h"
#include "Materials/MaterialParameterCollection.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AtmosphericSystem.generated.h"


// Forward declarations
class ADynamicTerrain;
class UWaterSystem;
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

// ===== ATMOSPHERIC CELL - FUNDAMENTAL UNIT =====
USTRUCT(BlueprintType)
struct TERRAI_API FAtmosphericCell
{
    GENERATED_BODY()

    // === THERMODYNAMIC STATE ===
    float Temperature = 288.15f;        // Kelvin (15°C default)
    float Pressure = 101325.0f;         // Pascals (sea level)
    float Density = 1.225f;             // kg/m³
    float Humidity = 0.5f;              // Relative humidity (0-1)
    float WaterVaporMass = 0.0f;        // kg water vapor per m³
    
    // === FLUID DYNAMICS ===
    FVector Velocity = FVector::ZeroVector;     // m/s wind velocity
    FVector Acceleration = FVector::ZeroVector; // m/s² for next frame
    float Vorticity = 0.0f;             // s⁻¹ rotational component
    float Divergence = 0.0f;            // s⁻¹ flow divergence
    
    // === CLOUD PHYSICS ===
    float CloudWaterContent = 0.0f;     // kg/m³ liquid water in clouds
    float CloudDropletSize = 0.0f;      // μm average droplet diameter
    float CloudCoverFraction = 0.0f;    // 0-1 visual cloud coverage
    float PrecipitationRate = 0.0f;     // mm/hr falling rate
    
    // === RADIATIVE PROPERTIES ===
    float SolarRadiation = 0.0f;        // W/m² incoming solar
    float LongwaveRadiation = 0.0f;     // W/m² outgoing thermal
    float Albedo = 0.3f;                // Surface reflectance
    
    // === CALCULATED PROPERTIES ===
    float GetSaturationVaporPressure() const
    {
        // August-Roche-Magnus formula
        float TempC = Temperature - 273.15f;
        return 611.2f * FMath::Exp((17.67f * TempC) / (TempC + 243.5f));
    }
    
    float GetDewPoint() const
    {
        float SatPress = GetSaturationVaporPressure();
        float ActualPress = Humidity * SatPress;
        float Ln = FMath::Loge(ActualPress / 611.2f);
        return 243.5f * Ln / (17.67f - Ln) + 273.15f;
    }
    
    bool IsCondensationLevel() const
    {
        return Temperature <= GetDewPoint() + 0.1f; // Small threshold for stability
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
    int32 GridWidth = 64;               // Atmospheric resolution (lower than terrain)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    int32 GridHeight = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    int32 GridLayers = 12;              // Vertical atmospheric layers
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    float CellSize = 1000.0f;           // Meters per atmospheric cell
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Grid")
    float LayerHeight = 500.0f;         // Meters per vertical layer

    // 3D atmospheric state
    TArray<FAtmosphericCell> AtmosphericGrid;
    
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
    void CreateHighPressureSystem(FVector2D Center, float Strength);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateLowPressureSystem(FVector2D Center, float Strength);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateFrontalSystem(FVector2D Start, FVector2D End, float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetSeasonProgress(float Progress); // 0.0 = winter, 1.0 = next winter
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float Hours); // 0.0 = midnight, 12.0 = noon
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ForceTestPrecipitation(); // Force precipitation for immediate testing
    
    // ===== ATMOSPHERIC BRUSHES =====
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyTemperatureBrush(FVector WorldPosition, float Radius, float TemperatureChange, float Strength = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyHumidityBrush(FVector WorldPosition, float Radius, float HumidityChange, float Strength = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyPressureBrush(FVector WorldPosition, float Radius, float PressureChange, float Strength = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyWindBrush(FVector WorldPosition, float Radius, FVector WindVelocity, float Strength = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyPrecipitationBrush(FVector WorldPosition, float Radius, float RainIntensity, float Duration = 300.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ApplyCloudBrush(FVector WorldPosition, float Radius, float CloudDensity, float Strength = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Brushes")
    void ClearWeatherBrush(FVector WorldPosition, float Radius, float Strength = 1.0f);

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
    void UpdateThermodynamics(float DeltaTime);
    void UpdateFluidDynamics(float DeltaTime);
    void UpdateCloudPhysics(float DeltaTime);
    void UpdateRadiation(float DeltaTime);
    
    // Thermodynamic calculations
    void CalculateTemperatureField();
    void CalculatePressureField();
    void CalculateHumidityTransport(float DeltaTime);
    void ProcessPhaseChanges(float DeltaTime);
    
    // Fluid dynamics (simplified Navier-Stokes)
    void CalculatePressureGradientForce();
    void CalculateCoriolisForce();
    void CalculateViscousForces();
    void AdvectWindField(float DeltaTime);
    void EnforceContinuityEquation();
    
    // Cloud microphysics
    void ProcessCondensation(float DeltaTime);
    void ProcessEvaporation(float DeltaTime);
    void ProcessCollisionCoalescence(float DeltaTime);
    void CalculateCloudDropletGrowth(float DeltaTime);
    
    // ===== WEATHER PATTERN SYSTEMS =====
    void UpdateWeatherPatterns(float DeltaTime);
    void ApplyWeatherPatternToGrid(const FWeatherPattern& Pattern);
    void GenerateNewWeatherPattern();
    void RemoveExpiredWeatherPatterns();
    
    // Pattern-specific physics
    void ApplyHighPressureSystem(const FWeatherPattern& Pattern);
    void ApplyLowPressureSystem(const FWeatherPattern& Pattern);
    void ApplyFrontalSystem(const FWeatherPattern& Pattern);
    void ApplyConvectiveForcing(const FWeatherPattern& Pattern);
    void ApplyOrographicLift();
    
    // ===== BOUNDARY CONDITIONS =====
    void ApplyTerrainInteraction();
    void ProcessSurfaceHeatFlux();
    void CalculateEvapotranspiration();
    void ApplyTopographicEffects();
    
  
    // ===== NUMERICAL METHODS =====
public:
    // Grid access and interpolation
    int32 GetGridIndex(int32 X, int32 Y, int32 Z) const;
    FAtmosphericCell& GetCell(int32 X, int32 Y, int32 Z);
    const FAtmosphericCell& GetCell(int32 X, int32 Y, int32 Z) const;
private:
    // Spatial derivatives for physics calculations
    float CalculateGradientX(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const;
    float CalculateGradientY(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const;
    float CalculateGradientZ(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const;
    float CalculateLaplacian(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const;
    
    // Interpolation for smooth field queries
    float InterpolateField(const TArray<float>& Field, FVector WorldPosition) const;
    FVector InterpolateVectorField(const TArray<FVector>& Field, FVector WorldPosition) const;
    
    // Coordinate transformations
    FVector WorldToGridCoordinates(FVector WorldPosition) const;
    FVector GridToWorldCoordinates(int32 X, int32 Y, int32 Z) const;
    
    // CRITICAL FIX: Add coordinate transform function
    FVector2D WorldToTerrainCoordinates(FVector WorldPosition) const;
    
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
};
