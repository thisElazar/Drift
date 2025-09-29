// AtmosphericSystem.cpp - Pure Physics and Weather Logic Implementation
#include "AtmosphericSystem.h"
#include "MasterController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "Engine/World.h"

UAtmosphericSystem::UAtmosphericSystem()
{
    // Initialize seasonal data
    FSeasonalData Spring;
    Spring.BaseTemperature = 15.0f;
    Spring.TemperatureVariation = 10.0f;
    Spring.RainProbability = 0.4f;
    Spring.PrevailingWind = FVector(1, 0.5f, 0);
    SeasonalSettings.Add(Spring);
    
    FSeasonalData Summer;
    Summer.BaseTemperature = 25.0f;
    Summer.TemperatureVariation = 8.0f;
    Summer.RainProbability = 0.2f;
    Summer.PrevailingWind = FVector(0.5f, 1, 0);
    SeasonalSettings.Add(Summer);
    
    FSeasonalData Fall;
    Fall.BaseTemperature = 12.0f;
    Fall.TemperatureVariation = 12.0f;
    Fall.RainProbability = 0.5f;
    Fall.PrevailingWind = FVector(-0.5f, 0.5f, 0);
    SeasonalSettings.Add(Fall);
    
    FSeasonalData Winter;
    Winter.BaseTemperature = 2.0f;
    Winter.TemperatureVariation = 6.0f;
    Winter.RainProbability = 0.6f;
    Winter.PrevailingWind = FVector(-1, 0, 0);
    SeasonalSettings.Add(Winter);
}

// ===== INITIALIZATION =====

void UAtmosphericSystem::Initialize(ADynamicTerrain* InTerrain, UWaterSystem* InWaterSystem)
{
    TargetTerrain = InTerrain;
    TerrainSystem = InTerrain;
    WaterSystem = InWaterSystem;
    CellSize = GridCellSize;
    
    // Initialize the grid
    InitializeAtmosphericGrid();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized with terrain and water systems"));
}

void UAtmosphericSystem::InitializeAtmosphericGrid()
{
    FScopeLock Lock(&GridDataLock);
    
    int32 TotalCells = GridResolutionX * GridResolutionY;
    AtmosphericGrid.Empty(TotalCells);
    AtmosphericGrid.SetNum(TotalCells);
    
    // Initialize each cell
    for (int32 Y = 0; Y < GridResolutionY; Y++)
    {
        for (int32 X = 0; X < GridResolutionX; X++)
        {
            int32 Index = GetGridIndex(X, Y);
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[Index];
            
            Cell.GridX = X;
            Cell.GridY = Y;
            Cell.Temperature = 288.15f;  // 15°C
            Cell.Humidity = 0.5f;
            Cell.MoistureMass = 0.0f;
            Cell.CloudCover = 0.0f;
            Cell.PrecipitationRate = 0.0f;
            Cell.Pressure = 101325.0f;
            Cell.WindVector = FVector2D::ZeroVector;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Grid initialized with %dx%d cells"),
           GridResolutionX, GridResolutionY);
}

void UAtmosphericSystem::RegisterWithMasterController(AMasterWorldController* Controller)
{
    MasterController = Controller;
    if (MasterController)
    {
        // TemporalManager would be set here if needed
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Registered with MasterController"));
    }
}

void UAtmosphericSystem::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    // Handle world scaling configuration
    // This is called by existing architecture
    bSystemScaled = true;
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Configured from master"));
}

void UAtmosphericSystem::SynchronizeCoordinates(const FWorldCoordinateSystem& CoordSystem)
{
    // Handle coordinate synchronization
    // This is called by existing architecture
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Coordinates synchronized"));
}

void UAtmosphericSystem::ConnectToTerrainSystem(ADynamicTerrain* InTerrain, UWaterSystem* InWaterSystem)
{
    TargetTerrain = InTerrain;
    TerrainSystem = InTerrain;
    WaterSystem = InWaterSystem;
    
    // Update aliases
    CellSize = GridCellSize;
    GridWidth = GridResolutionX;
    GridHeight = GridResolutionY;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Connected to terrain and water systems"));
}

// ===== MAIN UPDATE =====

void UAtmosphericSystem::UpdateAtmosphericSimulation(float DeltaTime)
{
    // Update weather state machine
    WeatherTimer += DeltaTime;
    if (WeatherTimer >= WeatherChangeInterval)
    {
        FWeatherData NewWeather = GenerateRandomWeather();
        SetWeather(NewWeather.WeatherType, WeatherTransitionTime);
        WeatherTimer = 0.0f;
    }
    
    // Process weather transition
    ProcessWeatherTransition(DeltaTime);
    
    // Update seasonal cycle
    UpdateSeasonalCycle(DeltaTime);
    
    // Run physics simulation
    UpdateAtmosphericPhysics(DeltaTime);
    
    // Transfer precipitation to surface
    TransferPrecipitationToSurface();
}

void UAtmosphericSystem::UpdateAtmosphericPhysics(float DeltaTime)
{
    FScopeLock Lock(&GridDataLock);
    
    // Apply weather patterns to grid
    ApplyWeatherToGrid();
    
    // Process condensation and precipitation
    ProcessCondensationAndPrecipitation(DeltaTime);
    
    // Advect moisture with wind
    AdvectMoisture(DeltaTime);
    
    // Apply terrain effects
    ApplyOrographicEffects(DeltaTime);
    
    // Process evaporation from water bodies
    ProcessEvaporation(DeltaTime);
    
    // Update cloud coverage
    UpdateCloudPhysics(DeltaTime);
}

// ===== WEATHER STATE MANAGEMENT =====

void UAtmosphericSystem::SetWeather(EWeatherType NewWeather, float TransitionTime)
{
    TargetWeather.WeatherType = NewWeather;
    WeatherTransitionTime = TransitionTime;
    WeatherTransitionProgress = 0.0f;
    
    // Set target weather parameters
    switch (NewWeather)
    {
    case EWeatherType::Clear:
        TargetWeather.CloudCover = 0.1f;
        TargetWeather.PrecipitationRate = 0.0f;
        TargetWeather.Humidity = 0.4f;
        TargetWeather.Visibility = 15000.0f;
        TargetWeather.WindSpeed = 3.0f;
        break;
        
    case EWeatherType::Cloudy:
        TargetWeather.CloudCover = 0.7f;
        TargetWeather.PrecipitationRate = 0.0f;
        TargetWeather.Humidity = 0.6f;
        TargetWeather.Visibility = 8000.0f;
        TargetWeather.WindSpeed = 5.0f;
        break;
        
    case EWeatherType::Rain:
        TargetWeather.CloudCover = 0.9f;
        TargetWeather.PrecipitationRate = 5.0f;
        TargetWeather.Humidity = 0.8f;
        TargetWeather.Visibility = 3000.0f;
        TargetWeather.WindSpeed = 8.0f;
        break;
        
    case EWeatherType::Storm:
        TargetWeather.CloudCover = 1.0f;
        TargetWeather.PrecipitationRate = 15.0f;
        TargetWeather.Humidity = 0.9f;
        TargetWeather.Visibility = 1000.0f;
        TargetWeather.WindSpeed = 20.0f;
        break;
        
    case EWeatherType::Snow:
        TargetWeather.CloudCover = 0.8f;
        TargetWeather.PrecipitationRate = 2.0f;
        TargetWeather.Humidity = 0.7f;
        TargetWeather.Temperature = 268.15f;  // -5°C
        TargetWeather.Visibility = 2000.0f;
        TargetWeather.WindSpeed = 6.0f;
        break;
        
    case EWeatherType::Fog:
        TargetWeather.CloudCover = 0.6f;
        TargetWeather.PrecipitationRate = 0.0f;
        TargetWeather.Humidity = 0.95f;
        TargetWeather.Visibility = 500.0f;
        TargetWeather.WindSpeed = 2.0f;
        break;
    }
    
    // Apply seasonal wind direction
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    TargetWeather.WindDirection = SeasonData.PrevailingWind;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Weather changing to %d"), (int32)NewWeather);
}

void UAtmosphericSystem::SetWeatherImmediate(EWeatherType NewWeather)
{
    SetWeather(NewWeather, 0.0f);
    CurrentWeather = TargetWeather;
    WeatherTransitionProgress = 1.0f;
    
    // Immediately apply to grid
    ApplyWeatherToGrid();
    
    // Broadcast change
    OnWeatherChanged.Broadcast(CurrentWeather);
}

void UAtmosphericSystem::ProcessWeatherTransition(float DeltaTime)
{
    if (WeatherTransitionProgress >= 1.0f)
        return;
    
    WeatherTransitionProgress += DeltaTime / WeatherTransitionTime;
    WeatherTransitionProgress = FMath::Clamp(WeatherTransitionProgress, 0.0f, 1.0f);
    
    // Interpolate weather parameters
    float Alpha = WeatherTransitionProgress;
    CurrentWeather.CloudCover = FMath::Lerp(CurrentWeather.CloudCover, TargetWeather.CloudCover, Alpha);
    CurrentWeather.PrecipitationRate = FMath::Lerp(CurrentWeather.PrecipitationRate, TargetWeather.PrecipitationRate, Alpha);
    CurrentWeather.Humidity = FMath::Lerp(CurrentWeather.Humidity, TargetWeather.Humidity, Alpha);
    CurrentWeather.Temperature = FMath::Lerp(CurrentWeather.Temperature, TargetWeather.Temperature, Alpha);
    CurrentWeather.WindSpeed = FMath::Lerp(CurrentWeather.WindSpeed, TargetWeather.WindSpeed, Alpha);
    CurrentWeather.Visibility = FMath::Lerp(CurrentWeather.Visibility, TargetWeather.Visibility, Alpha);
    
    // Slerp wind direction
    CurrentWeather.WindDirection = FMath::Lerp(CurrentWeather.WindDirection, TargetWeather.WindDirection, Alpha).GetSafeNormal();
    
    if (WeatherTransitionProgress >= 1.0f)
    {
        CurrentWeather.WeatherType = TargetWeather.WeatherType;
        OnWeatherChanged.Broadcast(CurrentWeather);
    }
}

void UAtmosphericSystem::ApplyWeatherToGrid()
{
    // Apply current weather state to all grid cells
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        // Base values from weather
        Cell.CloudCover = CurrentWeather.CloudCover;
        Cell.Humidity = CurrentWeather.Humidity;
        
        // Wind with some variation
        FVector2D BaseWind = FVector2D(CurrentWeather.WindDirection.X, CurrentWeather.WindDirection.Y) * CurrentWeather.WindSpeed;
        
        // Check for global wind override (from mouse control)
        if (bGlobalWindActive)
        {
            BaseWind = GlobalWindOverride;
        }
        
        // Add slight variation
        float Variation = FMath::FRandRange(0.8f, 1.2f);
        Cell.WindVector = BaseWind * Variation;
        
        // Temperature with height variation
        Cell.Temperature = CurrentWeather.Temperature;
        if (TargetTerrain)
        {
            FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
            float Height = TargetTerrain->GetHeightAtPosition(WorldPos);
            Cell.Temperature -= Height * 0.0065f;  // Lapse rate: -6.5K per 1000m
        }
        
        // Precipitation based on cloud cover and moisture
        if (CurrentWeather.PrecipitationRate > 0.0f && Cell.CloudCover > 0.7f)
        {
            Cell.PrecipitationRate = CurrentWeather.PrecipitationRate * Cell.CloudCover;
        }
        else
        {
            Cell.PrecipitationRate = 0.0f;
        }
    }
}

// ===== PHYSICS SIMULATION =====

void UAtmosphericSystem::ProcessCondensationAndPrecipitation(float DeltaTime)
{
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        // Calculate saturation vapor pressure
        float SaturationVaporPressure = 611.0f * FMath::Exp((17.27f * (Cell.Temperature - 273.15f)) /
                                                            (Cell.Temperature - 273.15f + 237.3f));
        
        // Check for condensation
        float ActualVaporPressure = Cell.Humidity * SaturationVaporPressure;
        if (ActualVaporPressure > SaturationVaporPressure)
        {
            // Excess moisture condenses
            float ExcessMoisture = (ActualVaporPressure - SaturationVaporPressure) / SaturationVaporPressure;
            Cell.MoistureMass += ExcessMoisture * 0.01f * DeltaTime;
            
            // Update cloud cover
            Cell.CloudCover = FMath::Min(Cell.CloudCover + ExcessMoisture * DeltaTime, 1.0f);
        }
        
        // Check for precipitation
        if (Cell.MoistureMass > 0.005f && Cell.CloudCover > 0.7f)
        {
            float PrecipitationAmount = Cell.MoistureMass * 0.1f * DeltaTime;
            Cell.PrecipitationRate = PrecipitationAmount / DeltaTime;
            Cell.MoistureMass -= PrecipitationAmount;
        }
    }
}

void UAtmosphericSystem::AdvectMoisture(float DeltaTime)
{
    FScopeLock Lock(&GridDataLock);
    
    // Create temporary buffer for advected values
    TArray<float> NewMoisture;
    NewMoisture.SetNum(AtmosphericGrid.Num());
    
    for (int32 Y = 0; Y < GridResolutionY; Y++)
    {
        for (int32 X = 0; X < GridResolutionX; X++)
        {
            const FSimplifiedAtmosphericCell& Cell = GetCell(X, Y);
            
            // Semi-Lagrangian advection
            FVector2D BacktracePos = FVector2D(X, Y) - Cell.WindVector * DeltaTime / GridCellSize;
            
            // Bilinear interpolation at backtrace position
            int32 X0 = FMath::FloorToInt(BacktracePos.X);
            int32 Y0 = FMath::FloorToInt(BacktracePos.Y);
            
            // Clamp to grid bounds
            X0 = FMath::Clamp(X0, 0, GridResolutionX - 2);
            Y0 = FMath::Clamp(Y0, 0, GridResolutionY - 2);
            
            float fx = BacktracePos.X - X0;
            float fy = BacktracePos.Y - Y0;
            
            // Interpolate moisture from surrounding cells
            float M00 = GetCell(X0, Y0).MoistureMass;
            float M10 = GetCell(X0 + 1, Y0).MoistureMass;
            float M01 = GetCell(X0, Y0 + 1).MoistureMass;
            float M11 = GetCell(X0 + 1, Y0 + 1).MoistureMass;
            
            NewMoisture[GetGridIndex(X, Y)] =
                M00 * (1-fx) * (1-fy) +
                M10 * fx * (1-fy) +
                M01 * (1-fx) * fy +
                M11 * fx * fy;
        }
    }
    
    // Copy back advected values
    for (int32 i = 0; i < AtmosphericGrid.Num(); i++)
    {
        AtmosphericGrid[i].MoistureMass = NewMoisture[i];
    }
}

void UAtmosphericSystem::ProcessEvaporation(float DeltaTime)
{
    if (!WaterSystem) return;
    
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
        
        // Check for water presence
        float WaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldPos);
        if (WaterDepth > 0.01f)
        {
            // Calculate evaporation rate based on temperature and wind
            float EvaporationRate = 0.001f * (Cell.Temperature / 288.15f) *
                                   (1.0f + Cell.WindVector.Size() / 10.0f);
            
            // Add moisture to atmosphere
            Cell.MoistureMass += EvaporationRate * DeltaTime;
            Cell.Humidity = FMath::Min(Cell.Humidity + EvaporationRate * 0.1f * DeltaTime, 1.0f);
        }
    }
}

// ===== WIND CONTROL =====

void UAtmosphericSystem::SetGlobalWind(FVector2D WindVector)
{
    GlobalWindOverride = WindVector;
    bGlobalWindActive = true;
    
    // Broadcast wind change
    FVector Wind3D(WindVector.X, WindVector.Y, 0);
    OnWindChanged.Broadcast(Wind3D.GetSafeNormal(), WindVector.Size());
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Global wind set to %s"), *WindVector.ToString());
}

void UAtmosphericSystem::SetGlobalWindPattern(FVector WindVector3D)
{
    SetGlobalWind(FVector2D(WindVector3D.X, WindVector3D.Y));
}

void UAtmosphericSystem::ApplyWindImpulse(FVector2D Location, FVector2D WindVector, float Radius, float Strength)
{
    FScopeLock Lock(&GridDataLock);
    
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
        FVector2D CellPos(WorldPos.X, WorldPos.Y);
        
        float Distance = FVector2D::Distance(CellPos, Location);
        if (Distance < Radius)
        {
            float Falloff = 1.0f - (Distance / Radius);
            Cell.WindVector += WindVector * Strength * Falloff;
        }
    }
}

// ===== DATA QUERY INTERFACE =====

TArray<FAtmosphericCellData> UAtmosphericSystem::GetCellsInRadius(FVector WorldPosition, float Radius) const
{
    FScopeLock Lock(&GridDataLock);
    TArray<FAtmosphericCellData> Results;
    
    float RadiusSq = Radius * Radius;
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector CellWorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
        float DistSq = FVector::DistSquared(CellWorldPos, WorldPosition);
        
        if (DistSq <= RadiusSq)
        {
            FAtmosphericCellData Data;
            Data.WorldPosition = CellWorldPos;
            Data.Temperature = Cell.Temperature;
            Data.Humidity = Cell.Humidity;
            Data.CloudCover = Cell.CloudCover;
            Data.PrecipitationRate = Cell.PrecipitationRate;
            Data.WindVector = Cell.WindVector;
            Data.MoistureMass = Cell.MoistureMass;
            Results.Add(Data);
        }
    }
    
    return Results;
}

TArray<FAtmosphericCellData> UAtmosphericSystem::GetAllCellsWithClouds(float MinCloudCover) const
{
    FScopeLock Lock(&GridDataLock);
    TArray<FAtmosphericCellData> Results;
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        if (Cell.CloudCover >= MinCloudCover)
        {
            FAtmosphericCellData Data;
            Data.WorldPosition = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
            Data.Temperature = Cell.Temperature;
            Data.Humidity = Cell.Humidity;
            Data.CloudCover = Cell.CloudCover;
            Data.PrecipitationRate = Cell.PrecipitationRate;
            Data.WindVector = Cell.WindVector;
            Data.MoistureMass = Cell.MoistureMass;
            Results.Add(Data);
        }
    }
    
    return Results;
}

TArray<FAtmosphericCellData> UAtmosphericSystem::GetAllCellsWithPrecipitation(float MinRate) const
{
    FScopeLock Lock(&GridDataLock);
    TArray<FAtmosphericCellData> Results;
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        if (Cell.PrecipitationRate >= MinRate)
        {
            FAtmosphericCellData Data;
            Data.WorldPosition = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
            Data.Temperature = Cell.Temperature;
            Data.Humidity = Cell.Humidity;
            Data.CloudCover = Cell.CloudCover;
            Data.PrecipitationRate = Cell.PrecipitationRate;
            Data.WindVector = Cell.WindVector;
            Data.MoistureMass = Cell.MoistureMass;
            Results.Add(Data);
        }
    }
    
    return Results;
}

FAtmosphericCellData UAtmosphericSystem::GetDataAtWorldPosition(FVector WorldPos) const
{
    FScopeLock Lock(&GridDataLock);
    
    FVector2D GridPos = WorldToGridCoordinates(WorldPos);
    int32 X = FMath::Clamp((int32)GridPos.X, 0, GridResolutionX - 1);
    int32 Y = FMath::Clamp((int32)GridPos.Y, 0, GridResolutionY - 1);
    
    const FSimplifiedAtmosphericCell& Cell = GetCell(X, Y);
    
    FAtmosphericCellData Data;
    Data.WorldPosition = GridToWorldCoordinates(X, Y);
    Data.Temperature = Cell.Temperature;
    Data.Humidity = Cell.Humidity;
    Data.CloudCover = Cell.CloudCover;
    Data.PrecipitationRate = Cell.PrecipitationRate;
    Data.WindVector = Cell.WindVector;
    Data.MoistureMass = Cell.MoistureMass;
    
    return Data;
}

float UAtmosphericSystem::GetWeatherIntensity() const
{
    switch (CurrentWeather.WeatherType)
    {
    case EWeatherType::Clear: return 0.0f;
    case EWeatherType::Cloudy: return 0.2f;
    case EWeatherType::Rain: return 0.6f;
    case EWeatherType::Storm: return 1.0f;
    case EWeatherType::Snow: return 0.4f;
    case EWeatherType::Fog: return 0.3f;
    default: return 0.0f;
    }
}

// ===== PRECIPITATION INTERFACE =====

void UAtmosphericSystem::TransferPrecipitationToSurface()
{
    if (!WaterSystem || !MasterController) return;
    
    FScopeLock Lock(&GridDataLock);
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        if (Cell.PrecipitationRate > 0.0f)
        {
            FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
            
            // Transfer to water system via master controller
            float VolumeToAdd = Cell.PrecipitationRate * GridCellSize * GridCellSize * 0.001f;
            MasterController->TransferAtmosphereToSurface(WorldPos, VolumeToAdd);
        }
    }
}

// ===== HELPER METHODS =====

int32 UAtmosphericSystem::GetGridIndex(int32 X, int32 Y) const
{
    return Y * GridResolutionX + X;
}

FSimplifiedAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y)
{
    return AtmosphericGrid[GetGridIndex(X, Y)];
}

const FSimplifiedAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y) const
{
    return AtmosphericGrid[GetGridIndex(X, Y)];
}

FVector2D UAtmosphericSystem::WorldToGridCoordinates(FVector WorldPosition) const
{
    if (!TargetTerrain) return FVector2D::ZeroVector;
    
    FVector TerrainOrigin = TargetTerrain->GetActorLocation();
    float TerrainWidth = TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale;
    float TerrainHeight = TargetTerrain->TerrainHeight * TargetTerrain->TerrainScale;
    
    float X = (WorldPosition.X - TerrainOrigin.X + TerrainWidth * 0.5f) / TerrainWidth;
    float Y = (WorldPosition.Y - TerrainOrigin.Y + TerrainHeight * 0.5f) / TerrainHeight;
    
    X *= GridResolutionX;
    Y *= GridResolutionY;
    
    return FVector2D(X, Y);
}

FVector UAtmosphericSystem::GridToWorldCoordinates(int32 X, int32 Y) const
{
    if (!TargetTerrain) return FVector::ZeroVector;
    
    FVector TerrainOrigin = TargetTerrain->GetActorLocation();
    float TerrainWidth = TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale;
    float TerrainHeight = TargetTerrain->TerrainHeight * TargetTerrain->TerrainScale;
    
    float WorldX = (X / (float)GridResolutionX) * TerrainWidth - TerrainWidth * 0.5f + TerrainOrigin.X;
    float WorldY = (Y / (float)GridResolutionY) * TerrainHeight - TerrainHeight * 0.5f + TerrainOrigin.Y;
    float WorldZ = TargetTerrain->GetHeightAtPosition(FVector(WorldX, WorldY, 0));
    
    return FVector(WorldX, WorldY, WorldZ);
}

// Additional helper methods implementation...
void UAtmosphericSystem::UpdateSeasonalCycle(float DeltaTime)
{
    SeasonTimer += DeltaTime;
    
    if (SeasonTimer >= SeasonDuration)
    {
        int32 NextSeason = ((int32)CurrentSeason + 1) % 4;
        CurrentSeason = (ESeason)NextSeason;
        SeasonTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Season changed to %d"), (int32)CurrentSeason);
    }
}

FWeatherData UAtmosphericSystem::GenerateRandomWeather() const
{
    FWeatherData NewWeather;
    FSeasonalData SeasonData = GetCurrentSeasonalData();
    
    float RandomValue = FMath::FRand();
    
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

FSeasonalData UAtmosphericSystem::GetCurrentSeasonalData() const
{
    if (SeasonalSettings.Num() > (int32)CurrentSeason)
    {
        return SeasonalSettings[(int32)CurrentSeason];
    }
    
    return FSeasonalData();
}

// ===== ADDITIONAL METHODS FOR COMPATIBILITY =====

void UAtmosphericSystem::CreateWeatherEffect(FVector2D Location, float Radius, float Intensity)
{
    FScopeLock Lock(&GridDataLock);
    
    // Apply weather effect to grid cells within radius
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
        FVector2D CellPos(WorldPos.X, WorldPos.Y);
        
        float Distance = FVector2D::Distance(CellPos, Location);
        if (Distance < Radius)
        {
            float Falloff = 1.0f - (Distance / Radius);
            
            // Apply intensity to moisture
            if (Intensity > 0)
            {
                Cell.MoistureMass += Intensity * Falloff * 0.01f;
                Cell.CloudCover = FMath::Min(Cell.CloudCover + Intensity * Falloff * 0.1f, 1.0f);
            }
            else
            {
                // Negative intensity removes moisture
                Cell.MoistureMass = FMath::Max(Cell.MoistureMass + Intensity * Falloff * 0.01f, 0.0f);
                Cell.CloudCover = FMath::Max(Cell.CloudCover + Intensity * Falloff * 0.1f, 0.0f);
            }
        }
    }
}

void UAtmosphericSystem::DebugDrawAtmosphericState() const
{
    // Debug visualization - would draw grid cells with weather info
    // Implementation depends on your debug drawing system
}

float UAtmosphericSystem::GetTemperatureAt(FVector WorldPosition) const
{
    FAtmosphericCellData Data = GetDataAtWorldPosition(WorldPosition);
    return Data.Temperature;
}

float UAtmosphericSystem::GetHumidityAt(FVector WorldPosition) const
{
    FAtmosphericCellData Data = GetDataAtWorldPosition(WorldPosition);
    return Data.Humidity;
}

float UAtmosphericSystem::GetPrecipitationAt(FVector WorldPosition) const
{
    FAtmosphericCellData Data = GetDataAtWorldPosition(WorldPosition);
    return Data.PrecipitationRate;
}

FVector UAtmosphericSystem::GetWindAt(FVector WorldPosition) const
{
    FAtmosphericCellData Data = GetDataAtWorldPosition(WorldPosition);
    return FVector(Data.WindVector.X, Data.WindVector.Y, 0);
}

float UAtmosphericSystem::GetAverageWind() const
{
    FScopeLock Lock(&GridDataLock);
    
    FVector2D AvgWind = FVector2D::ZeroVector;
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        AvgWind += Cell.WindVector;
    }
    
    if (AtmosphericGrid.Num() > 0)
    {
        AvgWind /= AtmosphericGrid.Num();
    }
    
    return AvgWind.Size();
}

const FSimplifiedAtmosphericCell* UAtmosphericSystem::GetCellDirect(int32 X, int32 Y) const
{
    if (X >= 0 && X < GridResolutionX && Y >= 0 && Y < GridResolutionY)
    {
        return &AtmosphericGrid[GetGridIndex(X, Y)];
    }
    return nullptr;
}

void UAtmosphericSystem::UpdateCloudPhysics(float DeltaTime)
{
    // Cloud physics update
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        // Simple cloud evolution based on moisture
        if (Cell.MoistureMass > 0.001f)
        {
            Cell.CloudCover = FMath::Min(Cell.CloudCover + DeltaTime * 0.1f, 1.0f);
        }
        else
        {
            Cell.CloudCover = FMath::Max(Cell.CloudCover - DeltaTime * 0.05f, 0.0f);
        }
    }
}

void UAtmosphericSystem::ApplyOrographicEffects(float DeltaTime)
{
    if (!TargetTerrain) return;
    
    // Orographic lift effect
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector WorldPos = GridToWorldCoordinates(Cell.GridX, Cell.GridY);
        float Height = TargetTerrain->GetHeightAtPosition(WorldPos);
        
        // Higher terrain causes more condensation
        if (Height > 100.0f)
        {
            float HeightFactor = Height / 1000.0f;
            Cell.MoistureMass += HeightFactor * 0.001f * DeltaTime;
        }
    }
}

void UAtmosphericSystem::TriggerStorm(float Intensity, float Duration)
{
    SetWeather(EWeatherType::Storm, 2.0f);
    TargetWeather.PrecipitationRate = 10.0f * Intensity;
    TargetWeather.WindSpeed = 20.0f * (1.0f + Intensity);
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Storm triggered - Intensity:%.2f Duration:%.1fs"),
           Intensity, Duration);
}

void UAtmosphericSystem::SetSeason(ESeason NewSeason)
{
    CurrentSeason = NewSeason;
    SeasonTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Season set to %d"), (int32)NewSeason);
}
