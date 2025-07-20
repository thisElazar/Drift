/**
 * ============================================
 * TERRAI ATMOSPHERE CONTROLLER
 * ============================================
 * Purpose: Weather systems, seasonal cycles, atmospheric effects
 * Scope: Temperature, humidity, wind, precipitation patterns
 * Integration: Drives ecosystem/geology responses, visual weather
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AtmosphereController.generated.h"

class UWaterSystem;
class ADynamicTerrain;

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear          UMETA(DisplayName = "Clear"),
    Cloudy         UMETA(DisplayName = "Cloudy"),
    Rain           UMETA(DisplayName = "Rain"),
    Storm          UMETA(DisplayName = "Storm"),
    Snow           UMETA(DisplayName = "Snow"),
    Fog            UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class ESeason : uint8
{
    Spring         UMETA(DisplayName = "Spring"),
    Summer         UMETA(DisplayName = "Summer"),
    Autumn         UMETA(DisplayName = "Autumn"),
    Winter         UMETA(DisplayName = "Winter")
};

USTRUCT(BlueprintType)
struct FWeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherType WeatherType = EWeatherType::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f; // Celsius
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 0.6f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WindDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindSpeed = 5.0f; // m/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PrecipitationRate = 0.0f; // mm/hr
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCover = 0.3f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Visibility = 10000.0f; // meters
};

USTRUCT(BlueprintType)
struct FSeasonalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseTemperature = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureVariation = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RainProbability = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PrevailingWind = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DayLength = 12.0f; // hours
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphereController();
    
    // ===== TEMPORAL MANAGER INTEGRATION =====
        
        /**
         * Main update function called by MasterController when TemporalManager approves
         * This replaces the Tick-based updates for proper temporal coordination
         */
        UFUNCTION(BlueprintCallable, Category = "Temporal Integration")
        void UpdateAtmosphericSystem(float DeltaTime);
        
        /**
         * Gets the last time this system was updated (for debugging)
         */
        UFUNCTION(BlueprintPure, Category = "Temporal Integration")
        float GetLastUpdateTime() const { return LastSystemUpdateTime; }
        

protected:
    // Override Tick but disable atmospheric updates
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

public:
    //virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);

    // ===== WEATHER SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWeatherData CurrentWeather;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSimulation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 300.0f; // seconds
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control", meta = (CallInEditor))
    void StartPhysicsBasedRain(float Intensity = 5.0f, float Radius = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control", meta = (CallInEditor))
    void StopPhysicsBasedRain();

    // ===== SEASONAL SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    ESeason CurrentSeason = ESeason::Spring;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    float SeasonProgress = 0.0f; // 0-1 through current season
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    float SeasonDuration = 1200.0f; // seconds per season
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    bool bEnableSeasonalCycle = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    TArray<FSeasonalData> SeasonalSettings;

    // ===== TEMPERATURE SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float BaseTemperature = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float DiurnalVariation = 8.0f; // day/night temperature swing
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float AltitudeGradient = 6.5f; // degrees per 1000m altitude
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float TimeOfDay = 12.0f; // 0-24 hours

    // ===== WIND SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector BaseWindDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float BaseWindSpeed = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindVariation = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    bool bEnableGustiness = true;

    // ===== CORE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherSystem(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType NewWeather, float TransitionTime = 5.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerStorm(float Intensity, float Duration);

    // ===== SEASONAL FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Seasons")
    void UpdateSeasonalCycle(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Seasons")
    void SetSeason(ESeason NewSeason);
    
    UFUNCTION(BlueprintCallable, Category = "Seasons")
    void SetTimeOfDay(float Hours);

    // ===== ATMOSPHERIC QUERIES =====
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    float GetTemperatureAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    float GetHumidityAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    FVector GetWindAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    float GetPrecipitationAtLocation(FVector WorldLocation) const;

    // ===== SYSTEM COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void OnTerrainModified(FVector Location, float Radius);

    // ===== WIND CONTROL FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void SetWindDirection(FVector NewDirection);
    
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void SetWindSpeed(float NewSpeed);
    
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void SetWind(FVector WindVector);
    
    UFUNCTION(BlueprintCallable, Category = "Wind Control")
    void UpdateWindImmediate();
    
    // ===== DEBUG & VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowWeatherMap(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintAtmosphereStats() const;
    
    // ===== BLUEPRINT INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor)
    void ApplyBlueprintDefaults();
    
    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    UPROPERTY()
    class UAtmosphericSystem* AtmosphericSystem = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System")
        UStaticMesh* CloudStaticMesh = nullptr;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System")
        UMaterialInterface* CloudMaterial = nullptr;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System")
        bool bEnableCloudRendering = true;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
        float CloudAltitude = 2000.0f;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (ClampMin = "0.1", ClampMax = "1.0"))
        float CloudOpacity = 0.8f;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (ClampMin = "1", ClampMax = "256"))
        int32 MaxCloudMeshes = 64;
    
        
      /*  // Cloud debug commands
        UFUNCTION(BlueprintCallable, Category = "Cloud System", CallInEditor)
        void DebugCloudSystem();
        
        UFUNCTION(BlueprintCallable, Category = "Cloud System", CallInEditor)
        void ForceGenerateClouds(float Coverage = 0.8f);
       
       */
    
    
    /** Apply weather change immediately without transition */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherImmediate(EWeatherType NewWeather);
    
    /** Apply current weather state to physics simulation */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyWeatherToPhysics();
    
    // ===== CONSOLE COMMANDS FOR TESTING =====
    
    /** Set weather to clear immediately */
    UFUNCTION(CallInEditor, Category = "Weather Commands")
    void SetWeatherClear();
    
    /** Set weather to rain immediately */
    UFUNCTION(CallInEditor, Category = "Weather Commands")
    void SetWeatherRain();
    
    /** Set weather to storm immediately */
    UFUNCTION(CallInEditor, Category = "Weather Commands")
    void SetWeatherStorm();
    
    /** Display atmospheric system debug information */
    UFUNCTION(CallInEditor, Category = "Weather Commands")
    void ShowAtmosphericDebug();
    
    // ===== PHASE 1 VALIDATION =====
    
    /** Validate that Phase 1 implementation is working correctly */
    UFUNCTION(CallInEditor, Category = "Testing")
    void ValidatePhase1Implementation();

    
    UPROPERTY()
    bool bSystemInitialized = false;

    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:

    // ===== INTERNAL STATE =====
    
    UPROPERTY()
    float WeatherTimer = 0.0f;
    
    UPROPERTY()
    float SeasonTimer = 0.0f;
    
    UPROPERTY()
    FWeatherData TargetWeather;


    // ===== INTERNAL FUNCTIONS =====
    
    void InitializeSeasonalData();
    void UpdateTemperatureField(float DeltaTime);
    void UpdateWindField(float DeltaTime);
    void UpdatePrecipitation(float DeltaTime);
    void ProcessWeatherTransition(float DeltaTime);
    FWeatherData GenerateRandomWeather() const;
    FSeasonalData GetCurrentSeasonalData() const;
    
    UPROPERTY()
    float LastSystemUpdateTime = 0.0f;
    
    UPROPERTY()
    float WindFieldUpdateTimer = 0.0f;
    
    UPROPERTY()
    float WindFieldUpdateInterval = 0.5f; // Update wind field every 0.5 seconds
    
    // Track last wind values to avoid unnecessary updates
    UPROPERTY()
    FVector LastWindDirection = FVector::ZeroVector;
    
    UPROPERTY()
    float LastWindSpeed = 0.0f;
};
