/**
 * ============================================
 * TERRAI ATMOSPHERE CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "AtmosphereController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AAtmosphereController::AAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AtmosphereRoot"));
    
    // Initialize default weather
    CurrentWeather.WeatherType = EWeatherType::Clear;
    CurrentWeather.Temperature = 20.0f;
    CurrentWeather.Humidity = 0.6f;
    CurrentWeather.WindDirection = FVector(1, 0, 0);
    CurrentWeather.WindSpeed = 5.0f;
    CurrentWeather.PrecipitationRate = 0.0f;
    CurrentWeather.CloudCover = 0.3f;
    CurrentWeather.Visibility = 10000.0f;
    
    TargetWeather = CurrentWeather;
    
    // Initialize seasonal settings
    SeasonalSettings.SetNum(4);
    
    // Spring
    SeasonalSettings[0].BaseTemperature = 15.0f;
    SeasonalSettings[0].TemperatureVariation = 8.0f;
    SeasonalSettings[0].RainProbability = 0.4f;
    SeasonalSettings[0].PrevailingWind = FVector(1, 0, 0);
    SeasonalSettings[0].DayLength = 12.0f;
    
    // Summer
    SeasonalSettings[1].BaseTemperature = 25.0f;
    SeasonalSettings[1].TemperatureVariation = 10.0f;
    SeasonalSettings[1].RainProbability = 0.2f;
    SeasonalSettings[1].PrevailingWind = FVector(0.7f, 0.7f, 0);
    SeasonalSettings[1].DayLength = 14.0f;
    
    // Autumn
    SeasonalSettings[2].BaseTemperature = 12.0f;
    SeasonalSettings[2].TemperatureVariation = 6.0f;
    SeasonalSettings[2].RainProbability = 0.5f;
    SeasonalSettings[2].PrevailingWind = FVector(-1, 0, 0);
    SeasonalSettings[2].DayLength = 10.0f;
    
    // Winter
    SeasonalSettings[3].BaseTemperature = 2.0f;
    SeasonalSettings[3].TemperatureVariation = 4.0f;
    SeasonalSettings[3].RainProbability = 0.3f;
    SeasonalSettings[3].PrevailingWind = FVector(-0.7f, -0.7f, 0);
    SeasonalSettings[3].DayLength = 8.0f;
}

void AAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSeasonalData();
    
    // CRITICAL: Apply blueprint defaults to current weather
    ApplyBlueprintDefaults();
}

void AAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // CRITICAL: Do NOT update weather/wind systems here!
     // All atmospheric updates are now controlled by TemporalManager
     // through the UpdateAtmosphericSystem function
     
     // You can still do non-simulation tasks here if needed
     // For example: debug visualization updates
}

// NEW: Main update function called by MasterController
void AAtmosphereController::UpdateAtmosphericSystem(float DeltaTime)
{
    // In AtmosphereController::UpdateAtmosphericSystem() at the very start
    UE_LOG(LogTemp, Warning, TEXT("UpdateAtmosphericSystem: bSystemInitialized=%d"), bSystemInitialized);
    if (!bSystemInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAtmosphericSystem FAILURE"));
        return;
        
       
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UpdateAtmosphericSystem called with DT=%.4f"), DeltaTime);
    
    // Track update time for debugging
    LastSystemUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Update weather simulation if enabled
    if (bEnableWeatherSimulation)
    {
        UpdateWeatherSystem(DeltaTime);
    }
    
    // Update seasonal cycle if enabled
    if (bEnableSeasonalCycle)
    {
        UpdateSeasonalCycle(DeltaTime);
    }
    
    // Log update for debugging (optional)
    UE_LOG(LogTemp, VeryVerbose, TEXT("AtmosphereController: System updated at %.2f (dt=%.4f)"),
           LastSystemUpdateTime, DeltaTime);
}

// ===== INITIALIZATION =====

void AAtmosphereController::Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water)
{
    if (!Terrain || !Water)
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphereController: Invalid initialization parameters"));
        return;
    }
    
    TargetTerrain = Terrain;
    WaterSystem = Water;
    
    // Get AtmosphericSystem from terrain
    if (Terrain)
    {
        AtmosphericSystem = Terrain->GetAtmosphericSystem();
        if (AtmosphericSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Connected to AtmosphericSystem"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: No AtmosphericSystem found on terrain"));
        }
        
    }
    
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Successfully initialized"));
}

// ===== WEATHER SYSTEM =====

void AAtmosphereController::UpdateWeatherSystem(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    
    // Check for weather changes
    if (WeatherTimer >= WeatherChangeInterval)
    {
        // Generate new random weather based on season
        FWeatherData NewWeather = GenerateRandomWeather();
        SetWeather(NewWeather.WeatherType, 10.0f);
        WeatherTimer = 0.0f;
    }
    
    // Process ongoing weather transition
    ProcessWeatherTransition(DeltaTime);
    
    // Update atmospheric fields
    UpdateTemperatureField(DeltaTime);
    UpdateWindField(DeltaTime);
    UpdatePrecipitation(DeltaTime);
}

void AAtmosphereController::SetWeather(EWeatherType NewWeather, float TransitionTime)
{
    TargetWeather.WeatherType = NewWeather;
    
    switch (NewWeather)
    {
    case EWeatherType::Clear:
        TargetWeather.CloudCover = 0.1f;
        TargetWeather.PrecipitationRate = 0.0f;
        TargetWeather.Visibility = 15000.0f;
        break;
        
    case EWeatherType::Cloudy:
        TargetWeather.CloudCover = 0.7f;
        TargetWeather.PrecipitationRate = 0.0f;
        TargetWeather.Visibility = 8000.0f;
        break;
        
    case EWeatherType::Rain:
        TargetWeather.CloudCover = 0.9f;
        TargetWeather.PrecipitationRate = 5.0f;
        TargetWeather.Visibility = 3000.0f;
        break;
        
    case EWeatherType::Storm:
        TargetWeather.CloudCover = 1.0f;
        TargetWeather.PrecipitationRate = 15.0f;
        TargetWeather.WindSpeed = BaseWindSpeed * 3.0f;
        TargetWeather.Visibility = 1000.0f;
        break;
        
    case EWeatherType::Snow:
        TargetWeather.CloudCover = 0.8f;
        TargetWeather.PrecipitationRate = 2.0f;
        TargetWeather.Temperature = -2.0f;
        TargetWeather.Visibility = 2000.0f;
        break;
        
    case EWeatherType::Fog:
        TargetWeather.CloudCover = 0.6f;
        TargetWeather.Humidity = 0.95f;
        TargetWeather.Visibility = 500.0f;
        break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Weather changing to %d"), (int32)NewWeather);
}

void AAtmosphereController::TriggerStorm(float Intensity, float Duration)
{
    SetWeather(EWeatherType::Storm);
    
    TargetWeather.PrecipitationRate = 10.0f * Intensity;
    TargetWeather.WindSpeed = BaseWindSpeed * (2.0f + Intensity);
    
    // Schedule return to normal weather
    WeatherTimer = WeatherChangeInterval - Duration;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Storm triggered - Intensity:%.2f Duration:%.1fs"), Intensity, Duration);
}

// ===== SEASONAL FUNCTIONS =====

void AAtmosphereController::UpdateSeasonalCycle(float DeltaTime)
{
    SeasonTimer += DeltaTime;
    UE_LOG(LogTemp, Warning, TEXT("UpdateSeasonalCycle: Timer=%.2f/%.2f"), SeasonTimer, SeasonDuration);
       
    
    if (SeasonTimer >= SeasonDuration)
    {
        // Advance to next season
        int32 NextSeason = ((int32)CurrentSeason + 1) % 4;
        CurrentSeason = (ESeason)NextSeason;
        SeasonTimer = 0.0f;
        SeasonProgress = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Season changed to %d"), (int32)CurrentSeason);
    }
    else
    {
        SeasonProgress = SeasonTimer / SeasonDuration;
    }
    
    // Update base temperature based on season
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    BaseTemperature = SeasonData.BaseTemperature;
    BaseWindDirection = SeasonData.PrevailingWind;
}

void AAtmosphereController::SetSeason(ESeason NewSeason)
{
    CurrentSeason = NewSeason;
    SeasonTimer = 0.0f;
    SeasonProgress = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Season manually set to %d"), (int32)NewSeason);
}

void AAtmosphereController::SetTimeOfDay(float Hours)
{
    TimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (TimeOfDay < 0.0f) TimeOfDay += 24.0f;
}

// ===== ATMOSPHERIC QUERIES =====

float AAtmosphereController::GetTemperatureAtLocation(FVector WorldLocation) const
{
    // Use physics-based temperature from AtmosphericSystem if available
    if (AtmosphericSystem)
    {
        float KelvinTemp = AtmosphericSystem->GetTemperatureAt(WorldLocation);
        return KelvinTemp - 273.15f; // Convert K to C
    }
    
    // Fallback to simple calculation
    float Temperature = BaseTemperature;
    
    // Altitude effect
    float Altitude = WorldLocation.Z;
    Temperature -= (Altitude / 1000.0f) * AltitudeGradient;
    
    // Diurnal variation
    float DayCycle = FMath::Sin((TimeOfDay / 24.0f) * 2.0f * PI - PI/2.0f);
    Temperature += DayCycle * DiurnalVariation * 0.5f;
    
    // Weather effect
    switch (CurrentWeather.WeatherType)
    {
    case EWeatherType::Rain:
        Temperature -= 3.0f;
        break;
    case EWeatherType::Snow:
        Temperature = FMath::Min(Temperature, 0.0f);
        break;
    case EWeatherType::Clear:
        Temperature += 2.0f;
        break;
    }
    
    return Temperature;
}

float AAtmosphereController::GetHumidityAtLocation(FVector WorldLocation) const
{
    // Use physics-based humidity from AtmosphericSystem if available
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetHumidityAt(WorldLocation);
    }
    
    // Fallback to simple calculation
    float Humidity = CurrentWeather.Humidity;
    
    // Water proximity effect
    if (WaterSystem)
    {
        float WaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldLocation);
        if (WaterDepth > 0.1f)
        {
            Humidity += 0.2f;
        }
    }
    
    // Altitude effect (humidity decreases with altitude)
    float Altitude = WorldLocation.Z;
    Humidity -= (Altitude / 5000.0f) * 0.3f;
    
    return FMath::Clamp(Humidity, 0.0f, 1.0f);
}

FVector AAtmosphereController::GetWindAtLocation(FVector WorldLocation) const
{
    // Use physics-based wind from AtmosphericSystem if available
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetWindAt(WorldLocation);
    }
    
    // Fallback to simple calculation
    FVector Wind = CurrentWeather.WindDirection * CurrentWeather.WindSpeed;
    
    // Add terrain influence
    if (TargetTerrain)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldLocation);
        float WindModifier = 1.0f + (TerrainHeight / 1000.0f) * 0.5f;
        Wind *= WindModifier;
    }
    
    // Add gustiness
    if (bEnableGustiness)
    {
        float GustFactor = 1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * WindVariation;
        Wind *= GustFactor;
    }
    
    return Wind;
}

float AAtmosphereController::GetPrecipitationAtLocation(FVector WorldLocation) const
{
    // Use physics-based precipitation from AtmosphericSystem if available
    if (AtmosphericSystem)
    {
        return AtmosphericSystem->GetPrecipitationAt(WorldLocation);
    }
    
    // Fallback to simple calculation
    float Precipitation = CurrentWeather.PrecipitationRate;
    
    // Orographic effect (more rain on windward slopes)
    if (TargetTerrain && Precipitation > 0.0f)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldLocation);
        if (TerrainHeight > 500.0f)
        {
            Precipitation *= 1.5f; // More rain at higher elevations
        }
    }
    
    return Precipitation;
}

// ===== WIND CONTROL FUNCTIONS =====

void AAtmosphereController::SetWindDirection(FVector NewDirection)
{
    BaseWindDirection = NewDirection.GetSafeNormal();
    CurrentWeather.WindDirection = BaseWindDirection;
    
    // Immediately update wind field
    UpdateWindImmediate();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Wind direction set to %s"), *BaseWindDirection.ToString());
}

void AAtmosphereController::SetWindSpeed(float NewSpeed)
{
    BaseWindSpeed = FMath::Clamp(NewSpeed, 0.0f, 100.0f);
    CurrentWeather.WindSpeed = BaseWindSpeed;
    
    // Immediately update wind field
    UpdateWindImmediate();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Wind speed set to %.1f m/s"), BaseWindSpeed);
}

void AAtmosphereController::SetWind(FVector WindVector)
{
    float Speed = WindVector.Size();
    if (Speed > 0.01f)
    {
        BaseWindDirection = WindVector.GetSafeNormal();
        BaseWindSpeed = FMath::Clamp(Speed, 0.0f, 100.0f);
        
        CurrentWeather.WindDirection = BaseWindDirection;
        CurrentWeather.WindSpeed = BaseWindSpeed;
        
        // Immediately update wind field
        UpdateWindImmediate();
        
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Wind set to Dir:%s Speed:%.1f"), 
               *BaseWindDirection.ToString(), BaseWindSpeed);
    }
}

void AAtmosphereController::UpdateWindImmediate()
{
    if (AtmosphericSystem && TargetTerrain)
    {
        // Force immediate wind update (bypass timer)
        WindFieldUpdateTimer = WindFieldUpdateInterval;
        
        // Apply wind pattern to entire terrain area
        FVector TerrainCenter = TargetTerrain->GetActorLocation();
        float TerrainSize = FMath::Max(TargetTerrain->TerrainWidth, TargetTerrain->TerrainHeight) * TargetTerrain->TerrainScale;
        
        AtmosphericSystem->ApplyWindBrush(
            TerrainCenter,
            TerrainSize,
            FVector(CurrentWeather.WindDirection * CurrentWeather.WindSpeed),
            1.0f
        );
        
        // Update tracking
        LastWindDirection = CurrentWeather.WindDirection;
        LastWindSpeed = CurrentWeather.WindSpeed;
        
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Wind field updated immediately"));
    }
}

// ===== SYSTEM COORDINATION =====

void AAtmosphereController::OnTerrainModified(FVector Location, float Radius)
{
    // Terrain modifications might affect local wind patterns
    UE_LOG(LogTemp, VeryVerbose, TEXT("AtmosphereController: Terrain modification at %s"), *Location.ToString());
}

// ===== DEBUG & VISUALIZATION =====

void AAtmosphereController::ShowWeatherMap(bool bEnable)
{
    if (!bEnable) return;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Showing weather map"));
    
    // Simple weather visualization
    if (TargetTerrain)
    {
        for (int32 x = -10; x <= 10; x++)
        {
            for (int32 y = -10; y <= 10; y++)
            {
                FVector TestLocation = TargetTerrain->GetActorLocation();
                TestLocation.X += x * 1000.0f;
                TestLocation.Y += y * 1000.0f;
                TestLocation.Z = TargetTerrain->GetHeightAtPosition(TestLocation);
                
                float Temperature = GetTemperatureAtLocation(TestLocation);
                float Humidity = GetHumidityAtLocation(TestLocation);
                
                // Color based on temperature
                FColor TempColor = FColor::Blue;
                if (Temperature > 20.0f) TempColor = FColor::Red;
                else if (Temperature > 10.0f) TempColor = FColor::Yellow;
                else if (Temperature > 0.0f) TempColor = FColor::Green;
                
                DrawDebugSphere(GetWorld(), TestLocation, 100.0f, 8, TempColor, false, 5.0f);
                
                // Wind vectors
                FVector Wind = GetWindAtLocation(TestLocation);
                FVector WindEnd = TestLocation + (Wind * 10.0f);
                DrawDebugDirectionalArrow(GetWorld(), TestLocation, WindEnd, 25.0f, FColor::White, false, 5.0f, 0, 2.0f);
            }
        }
    }
}

void AAtmosphereController::PrintAtmosphereStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ATMOSPHERE STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("bSystemInitialized: %s"), bSystemInitialized ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("Current Weather: %d"), (int32)CurrentWeather.WeatherType);
    UE_LOG(LogTemp, Warning, TEXT("Temperature: %.1f°C"), CurrentWeather.Temperature);
    UE_LOG(LogTemp, Warning, TEXT("Humidity: %.2f"), CurrentWeather.Humidity);
    UE_LOG(LogTemp, Warning, TEXT("Wind Speed: %.1f m/s"), CurrentWeather.WindSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Precipitation: %.1f mm/hr"), CurrentWeather.PrecipitationRate);
    UE_LOG(LogTemp, Warning, TEXT("Season: %d (Progress: %.2f)"), (int32)CurrentSeason, SeasonProgress);
    UE_LOG(LogTemp, Warning, TEXT("Time of Day: %.1f hours"), TimeOfDay);
}

// ===== BLUEPRINT INTEGRATION =====

void AAtmosphereController::ApplyBlueprintDefaults()
{
    CurrentWeather.WindSpeed = BaseWindSpeed;
    CurrentWeather.WindDirection = BaseWindDirection.GetSafeNormal();
    CurrentWeather.Temperature = BaseTemperature;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Applied blueprint defaults - Wind: %.1f m/s, Temp: %.1f°C"), 
           BaseWindSpeed, BaseTemperature);
    
    // For runtime updates
    if (bSystemInitialized && AtmosphericSystem)
    {
        UpdateWindImmediate();
    }
}

#if WITH_EDITOR
void AAtmosphereController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    FName PropertyName = PropertyChangedEvent.Property ? 
        PropertyChangedEvent.Property->GetFName() : NAME_None;
    
    // Apply all blueprint defaults when any property changes
    ApplyBlueprintDefaults();
    
    // Immediate updates for critical properties
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AAtmosphereController, BaseWindSpeed) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AAtmosphereController, BaseWindDirection))
    {
        UpdateWindImmediate();
    }
}
#endif

// ===== PRIVATE FUNCTIONS =====

void AAtmosphereController::InitializeSeasonalData()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Initializing seasonal data"));
    
    // Apply current seasonal settings
    if (SeasonalSettings.Num() > (int32)CurrentSeason)
    {
        FSeasonalData CurrentSeasonData = SeasonalSettings[(int32)CurrentSeason];
        BaseTemperature = CurrentSeasonData.BaseTemperature;
        BaseWindDirection = CurrentSeasonData.PrevailingWind;
    }
}

void AAtmosphereController::UpdateTemperatureField(float DeltaTime)
{
    // Update temperature based on time of day and season
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    
    float TargetTemp = SeasonData.BaseTemperature;
    float DayCycle = FMath::Sin((TimeOfDay / 24.0f) * 2.0f * PI - PI/2.0f);
    TargetTemp += DayCycle * DiurnalVariation * 0.5f;
    
    // Smooth temperature transition
    CurrentWeather.Temperature = FMath::FInterpTo(CurrentWeather.Temperature, TargetTemp, DeltaTime, 0.1f);
}

void AAtmosphereController::UpdateWindField(float DeltaTime)
{
    // Update wind based on seasonal patterns and weather
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    FVector TargetWind = SeasonData.PrevailingWind * BaseWindSpeed;
    
    // Weather modifications
    switch (CurrentWeather.WeatherType)
    {
    case EWeatherType::Storm:
        TargetWind *= 3.0f;
        break;
    case EWeatherType::Clear:
        TargetWind *= 0.7f;
        break;
    }
    
    // Smooth wind transition
    CurrentWeather.WindDirection = FMath::VInterpTo(CurrentWeather.WindDirection, TargetWind.GetSafeNormal(), DeltaTime, 0.5f);
    CurrentWeather.WindSpeed = FMath::FInterpTo(CurrentWeather.WindSpeed, TargetWind.Size(), DeltaTime, 0.5f);
    
    // OPTIMIZATION: Only update wind field if it has changed significantly
    bool bWindChanged = !CurrentWeather.WindDirection.Equals(LastWindDirection, 0.1f) ||
                       !FMath::IsNearlyEqual(CurrentWeather.WindSpeed, LastWindSpeed, 0.5f);
    
    // OPTIMIZATION: Use timer to limit wind field updates
    WindFieldUpdateTimer += DeltaTime;
    bool bShouldUpdateWindField = WindFieldUpdateTimer >= WindFieldUpdateInterval;
    
    if (bWindChanged && bShouldUpdateWindField && AtmosphericSystem && TargetTerrain)
    {
        // Apply wind pattern to entire terrain area
        FVector TerrainCenter = TargetTerrain->GetActorLocation();
        float TerrainSize = FMath::Max(TargetTerrain->TerrainWidth, TargetTerrain->TerrainHeight) * TargetTerrain->TerrainScale;
        
        // Use wind brush to update atmospheric physics
        AtmosphericSystem->ApplyWindBrush(
            TerrainCenter,
            TerrainSize,
            FVector(CurrentWeather.WindDirection * CurrentWeather.WindSpeed),
            1.0f // Full strength
        );
        
        // Update tracking variables
        LastWindDirection = CurrentWeather.WindDirection;
        LastWindSpeed = CurrentWeather.WindSpeed;
        WindFieldUpdateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Wind field updated - Dir: %s, Speed: %.1f"),
               *CurrentWeather.WindDirection.ToString(), CurrentWeather.WindSpeed);
    }
}

void AAtmosphereController::UpdatePrecipitation(float DeltaTime)
{
    if (!AtmosphericSystem || !TargetTerrain) return;
    
    // CRITICAL FIX: Track when we last created weather patterns to avoid spam
    static float LastWeatherPatternTime = -1.0f;
    static EWeatherType LastWeatherType = EWeatherType::Clear;
    static bool bRainActive = false;
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Only create new weather patterns when weather type changes or every 60 seconds
    bool bShouldUpdatePatterns = (CurrentWeather.WeatherType != LastWeatherType) ||
                                 (CurrentTime - LastWeatherPatternTime > 60.0f);
    
    if (bShouldUpdatePatterns)
    {
        LastWeatherPatternTime = CurrentTime;
        LastWeatherType = CurrentWeather.WeatherType;
        
        // Get terrain center for weather pattern placement
        FVector2D TerrainCenter(TargetTerrain->GetActorLocation().X, TargetTerrain->GetActorLocation().Y);
        
        // Apply weather patterns based on current weather type
        switch (CurrentWeather.WeatherType)
        {
            case EWeatherType::Clear:
                // Create high pressure to clear skies (only once per weather change)
                AtmosphericSystem->CreateHighPressureSystem(TerrainCenter, 2.0f);
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created high pressure system for clear weather"));
                break;
                
            case EWeatherType::Cloudy:
                // Light low pressure for cloud formation
                AtmosphericSystem->CreateLowPressureSystem(TerrainCenter, 0.5f);
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created low pressure system for cloudy weather"));
                break;
                
            case EWeatherType::Rain:
                // Moderate low pressure for rain
                AtmosphericSystem->CreateLowPressureSystem(TerrainCenter, 1.5f);
                // Add rain brush for immediate effect
                if (CurrentWeather.PrecipitationRate > 0.1f)
                {
                    AtmosphericSystem->ApplyPrecipitationBrush(
                        FVector(TerrainCenter.X, TerrainCenter.Y, 0),
                        10000.0f, // 10km radius
                        CurrentWeather.PrecipitationRate,
                        300.0f); // 5 minute duration
                }
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created rain system"));
                break;
                
            case EWeatherType::Storm:
                // Strong low pressure for storms
                AtmosphericSystem->CreateLowPressureSystem(TerrainCenter, 3.0f);
                // Multiple rain brushes for storm effect
                AtmosphericSystem->ApplyPrecipitationBrush(
                    FVector(TerrainCenter.X, TerrainCenter.Y, 0),
                    15000.0f, // 15km radius
                    CurrentWeather.PrecipitationRate,
                    600.0f); // 10 minute duration
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created storm system"));
                break;
                
            case EWeatherType::Snow:
                // Cold low pressure for snow
                AtmosphericSystem->CreateLowPressureSystem(TerrainCenter, 1.0f);
                AtmosphericSystem->ApplyTemperatureBrush(
                    FVector(TerrainCenter.X, TerrainCenter.Y, 0),
                    20000.0f, // 20km radius
                    -10.0f, // Cold temperature
                    1.0f);
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created snow system"));
                break;
                
            case EWeatherType::Fog:
                // High humidity for fog
                AtmosphericSystem->ApplyHumidityBrush(
                    FVector(TerrainCenter.X, TerrainCenter.Y, 0),
                    10000.0f, // 10km radius
                    0.4f, // High humidity increase
                    1.0f);
                UE_LOG(LogTemp, Log, TEXT("AtmosphereController: Created fog system"));
                break;
        }
    }
    /* REMOVED FOR DEBUG TESTING ON RAIN AUTHORITY
    // CRITICAL FIX: Only call StartRain/StopRain when state actually changes
    bool bShouldRain = (WaterSystem && CurrentWeather.PrecipitationRate > 0.1f);
    
    if (bShouldRain && !bRainActive)
    {
        WaterSystem->StartRain(CurrentWeather.PrecipitationRate / 10.0f);
        bRainActive = true;
    }
    else if (!bShouldRain && bRainActive)
    {
        WaterSystem->StopRain();
        bRainActive = false;
    }     */
}

void AAtmosphereController::ProcessWeatherTransition(float DeltaTime)
{
    // Smooth transition between current and target weather
    float TransitionSpeed = WeatherTransitionSpeed * DeltaTime;
    
    CurrentWeather.Temperature = FMath::FInterpTo(CurrentWeather.Temperature, TargetWeather.Temperature, DeltaTime, TransitionSpeed);
    CurrentWeather.Humidity = FMath::FInterpTo(CurrentWeather.Humidity, TargetWeather.Humidity, DeltaTime, TransitionSpeed);
    CurrentWeather.CloudCover = FMath::FInterpTo(CurrentWeather.CloudCover, TargetWeather.CloudCover, DeltaTime, TransitionSpeed);
    CurrentWeather.PrecipitationRate = FMath::FInterpTo(CurrentWeather.PrecipitationRate, TargetWeather.PrecipitationRate, DeltaTime, TransitionSpeed);
    CurrentWeather.Visibility = FMath::FInterpTo(CurrentWeather.Visibility, TargetWeather.Visibility, DeltaTime, TransitionSpeed);
}

FWeatherData AAtmosphereController::GenerateRandomWeather() const
{
    FWeatherData NewWeather = CurrentWeather;
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    
    // Random weather based on seasonal probability
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    
    if (RandomValue < SeasonData.RainProbability)
    {
        NewWeather.WeatherType = EWeatherType::Rain;
    }
    else if (RandomValue < SeasonData.RainProbability + 0.1f)
    {
        NewWeather.WeatherType = EWeatherType::Storm;
    }
    else if (RandomValue < SeasonData.RainProbability + 0.3f)
    {
        NewWeather.WeatherType = EWeatherType::Cloudy;
    }
    else
    {
        NewWeather.WeatherType = EWeatherType::Clear;
    }
    
    return NewWeather;
}

FSeasonalData AAtmosphereController::GetCurrentSeasonalData() const
{
    if (SeasonalSettings.Num() > (int32)CurrentSeason)
    {
        return SeasonalSettings[(int32)CurrentSeason];
    }
    
    // Fallback default seasonal data
    FSeasonalData DefaultSeason;
    DefaultSeason.BaseTemperature = 15.0f;
    DefaultSeason.TemperatureVariation = 8.0f;
    DefaultSeason.RainProbability = 0.3f;
    DefaultSeason.PrevailingWind = FVector(1, 0, 0);
    DefaultSeason.DayLength = 12.0f;
    
    return DefaultSeason;
}
        
void AAtmosphereController::StartPhysicsBasedRain(float Intensity, float Radius)
{
    if (!AtmosphericSystem || !TargetTerrain) return;
    
    // Set weather to rain
    SetWeather(EWeatherType::Rain, 1.0f);
    CurrentWeather.PrecipitationRate = Intensity;
    
    // Apply precipitation brush for immediate effect
    FVector TerrainCenter = TargetTerrain->GetActorLocation();
    AtmosphericSystem->ApplyPrecipitationBrush(
        TerrainCenter,
        Radius,
        Intensity,
        300.0f // 5 minute duration
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Started physics-based rain: %.1f mm/hr"), Intensity);
}

void AAtmosphereController::StopPhysicsBasedRain()
{
    SetWeather(EWeatherType::Clear, 5.0f);
    CurrentWeather.PrecipitationRate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped physics-based rain"));
}
