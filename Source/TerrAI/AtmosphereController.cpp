/**
 * ============================================
 * TERRAI ATMOSPHERE CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "AtmosphereController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AAtmosphereController::AAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
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
}

void AAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemInitialized)
    {
        if (bEnableWeatherSimulation)
        {
            UpdateWeatherSystem(DeltaTime);
        }
        
        if (bEnableSeasonalCycle)
        {
            UpdateSeasonalCycle(DeltaTime);
        }
    }
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
    UE_LOG(LogTemp, Warning, TEXT("Current Weather: %d"), (int32)CurrentWeather.WeatherType);
    UE_LOG(LogTemp, Warning, TEXT("Temperature: %.1f°C"), CurrentWeather.Temperature);
    UE_LOG(LogTemp, Warning, TEXT("Humidity: %.2f"), CurrentWeather.Humidity);
    UE_LOG(LogTemp, Warning, TEXT("Wind Speed: %.1f m/s"), CurrentWeather.WindSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Precipitation: %.1f mm/hr"), CurrentWeather.PrecipitationRate);
    UE_LOG(LogTemp, Warning, TEXT("Season: %d (Progress: %.2f)"), (int32)CurrentSeason, SeasonProgress);
    UE_LOG(LogTemp, Warning, TEXT("Time of Day: %.1f hours"), TimeOfDay);
}

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
}

void AAtmosphereController::UpdatePrecipitation(float DeltaTime)
{
    // Start precipitation if conditions are right
    if (WaterSystem && CurrentWeather.PrecipitationRate > 0.1f)
    {
        WaterSystem->StartRain(CurrentWeather.PrecipitationRate / 10.0f);
    }
    else if (WaterSystem)
    {
        WaterSystem->StopRain();
    }
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
        
