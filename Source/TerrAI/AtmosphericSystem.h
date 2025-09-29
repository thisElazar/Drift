// AtmosphericSystem.h - Pure Physics and Weather Logic
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AtmosphericSystem.generated.h"

// Forward declarations
class AMasterWorldController;
class UTemporalManager;
class UStaticMesh;
class UMaterialInterface;

// Struct forward declarations for compatibility
struct FWorldScalingConfig;
struct FWorldCoordinateSystem;

// Weather types
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear,
    Cloudy,
    Rain,
    Storm,
    Snow,
    Fog
};

// Season types
UENUM(BlueprintType)
enum class ESeason : uint8
{
    Spring,
    Summer,
    Fall,
    Winter
};

// Atmospheric cell data for grid simulation
USTRUCT(BlueprintType)
struct FSimplifiedAtmosphericCell
{
    GENERATED_BODY()

    float Temperature = 288.15f;  // Kelvin
    float Humidity = 0.5f;
    float MoistureMass = 0.0f;
    float CloudCover = 0.0f;
    float PrecipitationRate = 0.0f;
    FVector2D WindVector = FVector2D::ZeroVector;
    float Pressure = 101325.0f;  // Pascals
    int32 GridX = 0;
    int32 GridY = 0;
};

// Weather state data
USTRUCT(BlueprintType)
struct FWeatherData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EWeatherType WeatherType = EWeatherType::Clear;
    
    UPROPERTY(BlueprintReadOnly)
    float Temperature = 288.15f;
    
    UPROPERTY(BlueprintReadOnly)
    float Humidity = 0.5f;
    
    UPROPERTY(BlueprintReadOnly)
    float CloudCover = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float PrecipitationRate = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    FVector WindDirection = FVector(1, 0, 0);
    
    UPROPERTY(BlueprintReadOnly)
    float WindSpeed = 5.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float Visibility = 10000.0f;
};

// Seasonal configuration
USTRUCT(BlueprintType)
struct FSeasonalData
{
    GENERATED_BODY()

    float BaseTemperature = 15.0f;
    float TemperatureVariation = 8.0f;
    float RainProbability = 0.3f;
    FVector PrevailingWind = FVector(1, 0, 0);
    float DayLength = 12.0f;
};

// Thread-safe data query structure
USTRUCT(BlueprintType)
struct FAtmosphericCellData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector WorldPosition;
    
    UPROPERTY(BlueprintReadOnly)
    float Temperature = 288.15f;
    
    UPROPERTY(BlueprintReadOnly)
    float Humidity = 0.5f;
    
    UPROPERTY(BlueprintReadOnly)
    float CloudCover = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float PrecipitationRate = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    FVector2D WindVector;
    
    UPROPERTY(BlueprintReadOnly)
    float MoistureMass = 0.0f;
    
    FAtmosphericCellData() = default;
};



// Event delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, const FWeatherData&, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWindChanged, FVector, Direction, float, Speed);

UCLASS()
class TERRAI_API UAtmosphericSystem : public UObject
{
    GENERATED_BODY()

public:
    UAtmosphericSystem();

    // ===== INITIALIZATION =====
    void InitializeAtmosphericGrid();
    void RegisterWithMasterController(AMasterWorldController* Controller);
    void ConnectToTerrainSystem(class ADynamicTerrain* InTerrain, class UWaterSystem* InWaterSystem);
    
    // Methods expected by existing architecture
    void ConfigureFromMaster(const struct FWorldScalingConfig& Config);
    void SynchronizeCoordinates(const struct FWorldCoordinateSystem& CoordSystem);  // Changed from FMasterCoordinateSystem
    void ConnectToWaterSystem(class UWaterSystem* InWaterSystem) { WaterSystem = InWaterSystem; }
    
    // Initialize method expected by DynamicTerrain
    void Initialize(class ADynamicTerrain* InTerrain, class UWaterSystem* InWaterSystem);
    
    // Visual assets (kept here for compatibility but not used by physics)
    UPROPERTY()
    UStaticMesh* CloudStaticMesh = nullptr;
    
    UPROPERTY()
    UMaterialInterface* CloudMaterial = nullptr;
    
    UPROPERTY()
    bool bEnableCloudRendering = false;
    
    UPROPERTY()
    float CloudAltitude = 2000.0f;
    
    UPROPERTY()
    float CloudOpacity = 1.0f;
    
    UPROPERTY()
    int32 MaxCloudMeshes = 256;

    // ===== CONFIGURABLE GRID =====
    UPROPERTY(EditAnywhere, Category = "Grid Configuration")
    int32 GridResolutionX = 32;
    
    UPROPERTY(EditAnywhere, Category = "Grid Configuration")
    int32 GridResolutionY = 32;
    
    UPROPERTY(EditAnywhere, Category = "Grid Configuration")
    float GridCellSize = 100.0f;

    // ===== MAIN UPDATE =====
    void UpdateAtmosphericSimulation(float DeltaTime);

    // ===== WEATHER STATE MANAGEMENT =====
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FWeatherData CurrentWeather;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FWeatherData TargetWeather;
    
    UPROPERTY(EditAnywhere, Category = "Weather")
    float WeatherTransitionTime = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Weather")
    float WeatherChangeInterval = 300.0f;  // 5 minutes
    
    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(EWeatherType NewWeather, float TransitionTime = 10.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherImmediate(EWeatherType NewWeather);
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void TriggerStorm(float Intensity, float Duration);

    // ===== SEASONAL SYSTEM =====
    UPROPERTY(BlueprintReadOnly, Category = "Seasonal")
    ESeason CurrentSeason = ESeason::Spring;
    
    UPROPERTY(EditAnywhere, Category = "Seasonal")
    TArray<FSeasonalData> SeasonalSettings;
    
    UPROPERTY(EditAnywhere, Category = "Seasonal")
    float SeasonDuration = 600.0f;  // 10 minutes per season
    
    UFUNCTION(BlueprintCallable, Category = "Seasonal")
    void SetSeason(ESeason NewSeason);
    
    void UpdateSeasonalCycle(float DeltaTime);

    // ===== WIND CONTROL =====
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void SetGlobalWind(FVector2D WindVector);
    
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void ApplyWindImpulse(FVector2D Location, FVector2D WindVector, float Radius, float Strength);
    
    // Mouse wind control support
    void SetGlobalWindPattern(FVector WindVector3D);

    // ===== DATA QUERY INTERFACE =====
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    TArray<FAtmosphericCellData> GetCellsInRadius(FVector WorldPosition, float Radius) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    TArray<FAtmosphericCellData> GetAllCellsWithClouds(float MinCloudCover = 0.1f) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    TArray<FAtmosphericCellData> GetAllCellsWithPrecipitation(float MinRate = 0.1f) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    FAtmosphericCellData GetDataAtWorldPosition(FVector WorldPos) const;
    
    // Direct cell access (thread-safe const methods)
    const FSimplifiedAtmosphericCell* GetCellDirect(int32 X, int32 Y) const;
    
    // Weather state queries
    UFUNCTION(BlueprintCallable, Category = "Weather Query")
    EWeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }
    
    UFUNCTION(BlueprintCallable, Category = "Weather Query")
    float GetWeatherIntensity() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weather Query")
    FWeatherData GetFullWeatherState() const { return CurrentWeather; }
    
    // Simple world-space queries
    float GetTemperatureAt(FVector WorldPosition) const;
    float GetHumidityAt(FVector WorldPosition) const;
    float GetPrecipitationAt(FVector WorldPosition) const;
    FVector GetWindAt(FVector WorldPosition) const;
    float GetAverageWind() const;
    
    // Additional methods expected by existing systems
    void CreateWeatherEffect(FVector2D Location, float Radius, float Intensity);
    void DebugDrawAtmosphericState() const;
    void SetTimeOfDay(float Hours) { TimeOfDay = Hours; }
    
    // Methods to match existing architecture expectations
    bool IsSystemScaled() const { return bSystemScaled; }
    bool IsRegisteredWithMaster() const { return MasterController != nullptr; }
    FString GetScalingDebugInfo() const { return FString::Printf(TEXT("Grid: %dx%d, CellSize: %.1f"), GridResolutionX, GridResolutionY, GridCellSize); }
    
    // Wind query method expected by WaterSystem
    FVector GetWindAtLocation(FVector WorldLocation) const { return GetWindAt(WorldLocation); }
    
    // Add CellSize property that MasterController expects
    float GetCellSize() const { return GridCellSize; }
    float CellSize = 100.0f;  // Alias for GridCellSize
    
    // Add TerrainSystem property that DynamicTerrain expects
    UPROPERTY()
    class ADynamicTerrain* TerrainSystem = nullptr;

    // ===== EVENT DELEGATES =====
    UPROPERTY(BlueprintAssignable)
    FOnWeatherChanged OnWeatherChanged;
    
    UPROPERTY(BlueprintAssignable)
    FOnWindChanged OnWindChanged;
    
    // References (made public for compatibility)
    UPROPERTY()
    class UWaterSystem* WaterSystem = nullptr;

    
    
    // Grid management
    int32 GetGridIndex(int32 X, int32 Y) const;
    FSimplifiedAtmosphericCell& GetCell(int32 X, int32 Y);
    const FSimplifiedAtmosphericCell& GetCell(int32 X, int32 Y) const;
    FVector2D WorldToGridCoordinates(FVector WorldPosition) const;
    FVector GridToWorldCoordinates(int32 X, int32 Y) const;
    
    

    int32 GetGridWidth() const { return GridResolutionX; }
    int32 GetGridHeight() const { return GridResolutionY; }
    int32 GridWidth = 32;  // Alias
    int32 GridHeight = 32;  // Alias
    
    // Grid data
    TArray<FSimplifiedAtmosphericCell> AtmosphericGrid;
    mutable FCriticalSection GridDataLock;
    
    
private:

    
    // References
    UPROPERTY()
    AMasterWorldController* MasterController = nullptr;
    
    UPROPERTY()
    UTemporalManager* TemporalManager = nullptr;
    
    UPROPERTY()
    class ADynamicTerrain* TargetTerrain = nullptr;
    
    // Remove duplicate WaterSystem declaration since it's now public
    
    // Internal state
    float WeatherTimer = 0.0f;
    float SeasonTimer = 0.0f;
    float WeatherTransitionProgress = 0.0f;
    float TimeOfDay = 12.0f;
    FVector2D GlobalWindOverride = FVector2D::ZeroVector;
    bool bGlobalWindActive = false;
    bool bSystemScaled = false;  // Track if system has been scaled
    
    // Physics simulation methods
    void UpdateAtmosphericPhysics(float DeltaTime);
    void ProcessCondensationAndPrecipitation(float DeltaTime);
    void AdvectMoisture(float DeltaTime);
    void ApplyOrographicEffects(float DeltaTime);
    void ProcessEvaporation(float DeltaTime);
    void UpdateCloudPhysics(float DeltaTime);
    
    // Weather system methods
    void ProcessWeatherTransition(float DeltaTime);
    void ApplyWeatherToGrid();
    FWeatherData GenerateRandomWeather() const;
    FSeasonalData GetCurrentSeasonalData() const;
    

    
    // Precipitation interface
    void TransferPrecipitationToSurface();
    
    // Helper methods
    float InterpolateField(const TArray<float>& Field, FVector WorldPosition) const;
    FVector InterpolateVectorField(const TArray<FVector>& Field, FVector WorldPosition) const;
    
public:
    // GPU Resources (add these properties)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* WindFieldTexture = nullptr;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* MoistureTexture = nullptr;
    
    // Add this method
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetRainShadowIntensity(float Intensity) { RainShadowIntensity = Intensity; }
    
private:
    float RainShadowIntensity = 0.8f;
};
