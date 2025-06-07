// AtmosphericSystem.cpp - Scientific Atmospheric Physics Implementation
#include "AtmosphericSystem.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Async/ParallelFor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"

UAtmosphericSystem::UAtmosphericSystem()
{
    // Initialize default atmospheric settings
    GridWidth = 64;
    GridHeight = 64;
    GridLayers = 12;
    CellSize = 1000.0f;
    LayerHeight = 500.0f;
    
    // Climate defaults for temperate zone
    BaseTemperature = 288.15f;  // 15°C
    SeasonalAmplitude = 10.0f;
    DiurnalAmplitude = 8.0f;
    AltitudeGradient = 6.5f;
    LatitudeGradient = 0.5f;
    WindPattern = FVector2D(5.0f, 0.0f);
    
    // Weather pattern settings
    PatternGenerationRate = 0.0001f;
    bEnableConvectiveStorms = true;
    bEnableFrontalSystems = true;
    
    // Performance settings
    UpdateFrequency = 4.0f;
    bUseAdaptiveTimeStep = true;
    AccumulatedTime = 0.0f;
    CurrentUpdateLayer = 0;
    
    // Cloud rendering defaults
    bEnableCloudRendering = true;
    CloudAltitude = 2000.0f;
    CloudThickness = 1000.0f;
    CloudOpacity = 0.8f;
    MaxCloudMeshes = 64;
    CloudStaticMesh = nullptr;
    CloudMaterial = nullptr;
    CloudMaterialInstance = nullptr;
    AttachParent = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized"));
}

// ===== INITIALIZATION =====

void UAtmosphericSystem::Initialize(ADynamicTerrain* InTerrain, UWaterSystem* InWaterSystem)
{
    if (!InTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphericSystem: Cannot initialize without terrain"));
        return;
    }
    
    TerrainSystem = InTerrain;
    WaterSystem = InWaterSystem;
    
    // Match atmospheric grid to terrain scale
    float TerrainWidth = InTerrain->TerrainWidth * InTerrain->TerrainScale;
    float TerrainHeight = InTerrain->TerrainHeight * InTerrain->TerrainScale;
    
    // Adjust grid to cover terrain area
    GridWidth = FMath::Max(32, FMath::RoundToInt(TerrainWidth / CellSize));
    GridHeight = FMath::Max(32, FMath::RoundToInt(TerrainHeight / CellSize));
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Grid size %dx%dx%d covering %.1fkm terrain"),
           GridWidth, GridHeight, GridLayers, TerrainWidth / 1000.0f);
    
    InitializeAtmosphericGrid();
    
    // Initialize cloud rendering system
    InitializeCloudRendering(InTerrain->GetRootComponent());
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized with %d atmospheric cells"),
           AtmosphericGrid.Num());
}

void UAtmosphericSystem::InitializeAtmosphericGrid()
{
    int32 TotalCells = GridWidth * GridHeight * GridLayers;
    AtmosphericGrid.SetNum(TotalCells);
    
    // Initialize each atmospheric cell with realistic values
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        float Altitude = Z * LayerHeight;
        
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            for (int32 X = 0; X < GridWidth; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                // Temperature decreases with altitude (lapse rate)
                Cell.Temperature = BaseTemperature - (Altitude / 1000.0f) * AltitudeGradient;
                
                // Pressure decreases exponentially with altitude (barometric formula)
                Cell.Pressure = 101325.0f * FMath::Pow(1.0f - (0.0065f * Altitude) / 288.15f, 5.255f);
                
                // Density from ideal gas law
                Cell.Density = Cell.Pressure / (AtmosphericConstants::GAS_CONSTANT * Cell.Temperature);
                
                // Humidity decreases with altitude
                Cell.Humidity = FMath::Max(0.1f, 0.7f * FMath::Exp(-Altitude / 3000.0f));
                
                // Initial water vapor mass
                float SaturationPressure = Cell.GetSaturationVaporPressure();
                Cell.WaterVaporMass = (Cell.Humidity * SaturationPressure) / 
                                     (AtmosphericConstants::GAS_CONSTANT * Cell.Temperature) * 0.622f;
                
                // Initialize wind with prevailing pattern + some variation
                float WindVariation = FMath::RandRange(-2.0f, 2.0f);
                Cell.Velocity = FVector(
                    WindPattern.X + WindVariation,
                    WindPattern.Y + WindVariation * 0.5f,
                    0.0f
                );
                
                // Initialize other properties
                Cell.CloudWaterContent = 0.0f;
                Cell.CloudCoverFraction = 0.0f;
                Cell.PrecipitationRate = 0.0f;
                Cell.Albedo = 0.3f;
                
                // Calculate initial radiation based on altitude
                Cell.SolarRadiation = 1361.0f * FMath::Pow(0.7f, Z); // Atmospheric absorption
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized %d cells with realistic atmosphere"),
           TotalCells);
}

// ===== CORE SIMULATION =====

void UAtmosphericSystem::UpdateAtmosphericSimulation(float DeltaTime)
{
    if (!TerrainSystem || AtmosphericGrid.Num() == 0)
    {
        return;
    }
    
    // Accumulate time for frequency control
    AccumulatedTime += DeltaTime;
    float TimeStep = 1.0f / UpdateFrequency;
    
    if (AccumulatedTime >= TimeStep)
    {
        float ActualDeltaTime = bUseAdaptiveTimeStep ? AccumulatedTime : TimeStep;
        
        // Core atmospheric physics updates
        UpdateThermodynamics(ActualDeltaTime);
        UpdateFluidDynamics(ActualDeltaTime);
        UpdateCloudPhysics(ActualDeltaTime);
        UpdateRadiation(ActualDeltaTime);
        
        // Weather pattern systems
        UpdateWeatherPatterns(ActualDeltaTime);
        
        // Boundary interactions
        ApplyTerrainInteraction();
        
        // Interface with other systems
        UpdateWaterSystemInterface();
        
        // Update cloud rendering system
        if (bEnableCloudRendering)
        {
            UpdateCloudRendering(ActualDeltaTime);
        }
        
        AccumulatedTime = 0.0f;
    }
}

// ===== THERMODYNAMICS =====

void UAtmosphericSystem::UpdateThermodynamics(float DeltaTime)
{
    CalculateTemperatureField();
    CalculatePressureField();
    CalculateHumidityTransport(DeltaTime);
    ProcessPhaseChanges(DeltaTime);
}

void UAtmosphericSystem::CalculateTemperatureField()
{
    if (!TerrainSystem || !TerrainSystem->GetWorld())
    {
        return;
    }
    
    UWorld* World = TerrainSystem->GetWorld();
    float TimeOfDay = FMath::Fmod(World->GetTimeSeconds() / 3600.0f, 24.0f);
    float SeasonProgress = FMath::Fmod(World->GetTimeSeconds() / (24.0f * 3600.0f * 365.0f), 1.0f);
    
    float SolarElevation = FMath::Sin((TimeOfDay - 6.0f) * PI / 12.0f);
    float SeasonalFactor = FMath::Cos(SeasonProgress * 2.0f * PI);
    
    ParallelFor(GridLayers, [&](int32 Z)
    {
        float Altitude = Z * LayerHeight;
        
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            for (int32 X = 0; X < GridWidth; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                float BaseTemp = BaseTemperature - (Altitude / 1000.0f) * AltitudeGradient;
                float SeasonalTemp = BaseTemp + SeasonalAmplitude * SeasonalFactor;
                float DiurnalFactor = FMath::Max(0.0f, SolarElevation);
                float SurfaceWeight = FMath::Exp(-Altitude / 1000.0f);
                float DiurnalTemp = SeasonalTemp + DiurnalAmplitude * DiurnalFactor * SurfaceWeight;
                
                Cell.Temperature = FMath::FInterpTo(Cell.Temperature, DiurnalTemp, 0.1f, 0.1f);
                
                if (Z == 0)
                {
                    Cell.SolarRadiation = 1361.0f * FMath::Max(0.0f, SolarElevation) * 
                                         (1.0f - Cell.CloudCoverFraction * 0.8f);
                }
            }
        }
    });
}

void UAtmosphericSystem::CalculatePressureField()
{
    // Update pressure based on temperature changes (hydrostatic equilibrium)
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            // Start from surface and work upward
            for (int32 Z = 1; Z < GridLayers; Z++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                int32 BelowIndex = GetGridIndex(X, Y, Z - 1);
                
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                const FAtmosphericCell& BelowCell = AtmosphericGrid[BelowIndex];
                
                // Barometric formula for pressure decrease
                float DeltaAltitude = LayerHeight;
                float AvgTemp = (Cell.Temperature + BelowCell.Temperature) * 0.5f;
                float PressureRatio = FMath::Exp(-AtmosphericConstants::GRAVITY * DeltaAltitude / 
                                                (AtmosphericConstants::GAS_CONSTANT * AvgTemp));
                
                Cell.Pressure = BelowCell.Pressure * PressureRatio;
                
                // Update density from ideal gas law
                Cell.Density = Cell.Pressure / (AtmosphericConstants::GAS_CONSTANT * Cell.Temperature);
            }
        }
    }
}

void UAtmosphericSystem::CalculateHumidityTransport(float DeltaTime)
{
    // TODO: Humidity advection with wind
}

void UAtmosphericSystem::ProcessPhaseChanges(float DeltaTime)
{
    // TODO: Condensation and evaporation
}

// ===== FLUID DYNAMICS =====

void UAtmosphericSystem::UpdateFluidDynamics(float DeltaTime)
{
    CalculatePressureGradientForce();
    CalculateCoriolisForce();
    CalculateViscousForces();
    AdvectWindField(DeltaTime);
    EnforceContinuityEquation();
}

void UAtmosphericSystem::CalculatePressureGradientForce()
{
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = 1; Y < GridHeight - 1; Y++)
        {
            for (int32 X = 1; X < GridWidth - 1; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                float PressureLeft = AtmosphericGrid[GetGridIndex(X - 1, Y, Z)].Pressure;
                float PressureRight = AtmosphericGrid[GetGridIndex(X + 1, Y, Z)].Pressure;
                float PressureUp = AtmosphericGrid[GetGridIndex(X, Y - 1, Z)].Pressure;
                float PressureDown = AtmosphericGrid[GetGridIndex(X, Y + 1, Z)].Pressure;
                
                FVector PressureForce;
                PressureForce.X = -(PressureRight - PressureLeft) / (2.0f * CellSize * Cell.Density);
                PressureForce.Y = -(PressureDown - PressureUp) / (2.0f * CellSize * Cell.Density);
                PressureForce.Z = 0.0f;
                
                Cell.Acceleration = PressureForce;
            }
        }
    }
}

void UAtmosphericSystem::CalculateCoriolisForce()
{
    float CoriolisParameter = 2.0f * AtmosphericConstants::CORIOLIS_COEFFICIENT;
    
    for (FAtmosphericCell& Cell : AtmosphericGrid)
    {
        FVector CoriolisForce;
        CoriolisForce.X = CoriolisParameter * Cell.Velocity.Y;
        CoriolisForce.Y = -CoriolisParameter * Cell.Velocity.X;
        CoriolisForce.Z = 0.0f;
        
        Cell.Acceleration += CoriolisForce;
    }
}

void UAtmosphericSystem::CalculateViscousForces()
{
    float ViscosityCoefficient = 0.01f;
    
    for (FAtmosphericCell& Cell : AtmosphericGrid)
    {
        Cell.Acceleration += -Cell.Velocity * ViscosityCoefficient;
    }
}

void UAtmosphericSystem::AdvectWindField(float DeltaTime)
{
    for (FAtmosphericCell& Cell : AtmosphericGrid)
    {
        Cell.Velocity += Cell.Acceleration * DeltaTime;
        Cell.Velocity *= 0.99f;
        
        float MaxWindSpeed = 50.0f;
        if (Cell.Velocity.Size() > MaxWindSpeed)
        {
            Cell.Velocity = Cell.Velocity.GetSafeNormal() * MaxWindSpeed;
        }
    }
}

void UAtmosphericSystem::EnforceContinuityEquation()
{
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = 1; Y < GridHeight - 1; Y++)
        {
            for (int32 X = 1; X < GridWidth - 1; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                float VelLeft = AtmosphericGrid[GetGridIndex(X - 1, Y, Z)].Velocity.X;
                float VelRight = AtmosphericGrid[GetGridIndex(X + 1, Y, Z)].Velocity.X;
                float VelUp = AtmosphericGrid[GetGridIndex(X, Y - 1, Z)].Velocity.Y;
                float VelDown = AtmosphericGrid[GetGridIndex(X, Y + 1, Z)].Velocity.Y;
                
                Cell.Divergence = (VelRight - VelLeft) / (2.0f * CellSize) + 
                                 (VelDown - VelUp) / (2.0f * CellSize);
            }
        }
    }
}

// ===== CLOUD PHYSICS =====

void UAtmosphericSystem::UpdateCloudPhysics(float DeltaTime)
{
    // Enhanced cloud and precipitation generation
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            for (int32 X = 0; X < GridWidth; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                // Cloud formation at condensation level
                if (Cell.IsCondensationLevel())
                {
                    // Generate clouds based on humidity and altitude
                    float Altitude = Z * LayerHeight;
                    float CloudFormationRate = 0.1f;
                    
                    if (Altitude >= 1000.0f && Altitude <= 4000.0f) // Typical cloud altitudes
                    {
                        float HumidityFactor = FMath::Clamp((Cell.Humidity - 0.6f) / 0.4f, 0.0f, 1.0f);
                        float AltitudeFactor = 1.0f - FMath::Abs(Altitude - 2500.0f) / 1500.0f; // Peak at 2.5km
                        AltitudeFactor = FMath::Clamp(AltitudeFactor, 0.0f, 1.0f);
                        
                        // Increase cloud cover
                        Cell.CloudCoverFraction += CloudFormationRate * HumidityFactor * AltitudeFactor * DeltaTime;
                        Cell.CloudCoverFraction = FMath::Clamp(Cell.CloudCoverFraction, 0.0f, 1.0f);
                        
                        // Generate cloud water content
                        Cell.CloudWaterContent = Cell.CloudCoverFraction * 0.5f; // kg/m³
                    }
                }
                else
                {
                    // Gradual cloud dissipation
                    Cell.CloudCoverFraction *= 0.99f;
                    Cell.CloudWaterContent *= 0.99f;
                }
                
                // Precipitation model
                if (Cell.CloudCoverFraction > 0.3f && Cell.Humidity > 0.8f)
                {
                    // Generate precipitation based on cloud density and humidity
                    float ExcessHumidity = Cell.Humidity - 0.8f;
                    Cell.PrecipitationRate = ExcessHumidity * Cell.CloudCoverFraction * 20.0f; // mm/hr
                    
                    // Reduce humidity as water precipitates
                    Cell.Humidity -= ExcessHumidity * 0.1f * DeltaTime;
                    
                    // Reduce cloud water content
                    Cell.CloudWaterContent *= 0.98f;
                }
                else
                {
                    // Gradually reduce precipitation
                    Cell.PrecipitationRate *= 0.95f;
                }
            }
        }
    }
}

void UAtmosphericSystem::ProcessCondensation(float DeltaTime)
{
    // TODO: Water vapor to cloud droplets
}

void UAtmosphericSystem::ProcessEvaporation(float DeltaTime)
{
    // TODO: Cloud droplets back to vapor
}

void UAtmosphericSystem::ProcessCollisionCoalescence(float DeltaTime)
{
    // TODO: Droplet growth and precipitation
}

void UAtmosphericSystem::CalculateCloudDropletGrowth(float DeltaTime)
{
    // TODO: Detailed cloud microphysics
}

// ===== RADIATION =====

void UAtmosphericSystem::UpdateRadiation(float DeltaTime)
{
    // TODO: Solar and thermal radiation
}

// ===== WEATHER PATTERNS =====

void UAtmosphericSystem::UpdateWeatherPatterns(float DeltaTime)
{
    // TODO: High/low pressure systems, fronts, storms
}

void UAtmosphericSystem::ApplyWeatherPatternForces()
{
    // TODO: Apply pattern effects to atmospheric grid
}

void UAtmosphericSystem::ApplyHighPressureSystem(const FWeatherPattern& Pattern)
{
    // TODO: Clear, stable weather effects
}

void UAtmosphericSystem::ApplyLowPressureSystem(const FWeatherPattern& Pattern)
{
    // TODO: Stormy, unstable weather effects
}

void UAtmosphericSystem::ApplyFrontalSystem(const FWeatherPattern& Pattern)
{
    // TODO: Weather front effects
}

void UAtmosphericSystem::ApplyConvectiveForcing(const FWeatherPattern& Pattern)
{
    // TODO: Thunderstorm effects
}

void UAtmosphericSystem::GenerateNewWeatherPatterns()
{
    // TODO: Create new weather systems
}

void UAtmosphericSystem::RemoveExpiredWeatherPatterns()
{
    // TODO: Remove old weather patterns
}

void UAtmosphericSystem::ApplyOrographicLift()
{
    // TODO: Mountain effects on air masses
}

// ===== SYSTEM INTEGRATION =====

void UAtmosphericSystem::UpdateWaterSystemInterface()
{
    if (!WaterSystem || !TerrainSystem)
    {
        return;
    }
    
    // Apply precipitation to water system
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            float Precipitation = GetPrecipitationAt(GridToWorldCoordinates(X, Y, 0));
            
            if (Precipitation > 0.1f) // Minimum threshold
            {
                // Convert precipitation rate to water amount
                float WaterAmount = Precipitation * 0.001f; // mm/hr to m/s approximation
                
                // Apply to multiple terrain points within this atmospheric cell
                int32 TerrainPointsPerCell = 8;
                for (int32 i = 0; i < TerrainPointsPerCell; i++)
                {
                    FVector2D RandomOffset(
                        FMath::RandRange(-CellSize * 0.5f, CellSize * 0.5f),
                        FMath::RandRange(-CellSize * 0.5f, CellSize * 0.5f)
                    );
                    
                    FVector WorldPos = GridToWorldCoordinates(X, Y, 0);
                    WorldPos.X += RandomOffset.X;
                    WorldPos.Y += RandomOffset.Y;
                    
                    WaterSystem->AddWater(WorldPos, WaterAmount);
                }
            }
        }
    }
}

void UAtmosphericSystem::ApplyTerrainInteraction()
{
    // TODO: Terrain effects on atmosphere
}

void UAtmosphericSystem::ProcessSurfaceHeatFlux()
{
    // TODO: Surface heating effects
}

void UAtmosphericSystem::CalculateEvapotranspiration()
{
    // TODO: Water evaporation from surface
}

void UAtmosphericSystem::ApplyTopographicEffects()
{
    // TODO: Terrain-driven atmospheric effects
}

void UAtmosphericSystem::UpdateTerrainInterface()
{
    // TODO: Interface with terrain system
}

// ===== QUERY INTERFACE =====

float UAtmosphericSystem::GetTemperatureAt(FVector WorldPosition) const
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    int32 Z = FMath::FloorToInt(GridPos.Z);
    
    if (X >= 0 && X < GridWidth - 1 && Y >= 0 && Y < GridHeight - 1 && Z >= 0 && Z < GridLayers - 1)
    {
        // Trilinear interpolation
        float FracX = GridPos.X - X;
        float FracY = GridPos.Y - Y;
        float FracZ = GridPos.Z - Z;
        
        float T000 = AtmosphericGrid[GetGridIndex(X, Y, Z)].Temperature;
        float T100 = AtmosphericGrid[GetGridIndex(X + 1, Y, Z)].Temperature;
        float T010 = AtmosphericGrid[GetGridIndex(X, Y + 1, Z)].Temperature;
        float T110 = AtmosphericGrid[GetGridIndex(X + 1, Y + 1, Z)].Temperature;
        float T001 = AtmosphericGrid[GetGridIndex(X, Y, Z + 1)].Temperature;
        float T101 = AtmosphericGrid[GetGridIndex(X + 1, Y, Z + 1)].Temperature;
        float T011 = AtmosphericGrid[GetGridIndex(X, Y + 1, Z + 1)].Temperature;
        float T111 = AtmosphericGrid[GetGridIndex(X + 1, Y + 1, Z + 1)].Temperature;
        
        float T00 = FMath::Lerp(T000, T100, FracX);
        float T10 = FMath::Lerp(T010, T110, FracX);
        float T01 = FMath::Lerp(T001, T101, FracX);
        float T11 = FMath::Lerp(T011, T111, FracX);
        
        float T0 = FMath::Lerp(T00, T10, FracY);
        float T1 = FMath::Lerp(T01, T11, FracY);
        
        return FMath::Lerp(T0, T1, FracZ);
    }
    
    return BaseTemperature;
}

float UAtmosphericSystem::GetPressureAt(FVector WorldPosition) const
{
    // TODO: Interpolate pressure at position
    return 101325.0f;
}

float UAtmosphericSystem::GetHumidityAt(FVector WorldPosition) const
{
    // TODO: Interpolate humidity at position
    return 0.5f;
}

FVector UAtmosphericSystem::GetWindAt(FVector WorldPosition) const
{
    // TODO: Interpolate wind at position
    return FVector::ZeroVector;
}

float UAtmosphericSystem::GetPrecipitationAt(FVector WorldPosition) const
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 X = FMath::Clamp(FMath::RoundToInt(GridPos.X), 0, GridWidth - 1);
    int32 Y = FMath::Clamp(FMath::RoundToInt(GridPos.Y), 0, GridHeight - 1);
    
    // Average precipitation through atmospheric column
    float TotalPrecipitation = 0.0f;
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        TotalPrecipitation += AtmosphericGrid[GetGridIndex(X, Y, Z)].PrecipitationRate;
    }
    
    return TotalPrecipitation / GridLayers;
}

float UAtmosphericSystem::GetCloudCoverAt(FVector WorldPosition) const
{
    // TODO: Interpolate cloud cover at position
    return 0.0f;
}

// ===== CONTROL INTERFACE =====

void UAtmosphericSystem::CreateHighPressureSystem(FVector2D Center, float Strength)
{
    FWeatherPattern NewPattern;
    NewPattern.PatternType = EWeatherPattern::HighPressure;
    NewPattern.Center = Center;
    NewPattern.Intensity = Strength;
    NewPattern.Radius = 15000.0f;
    NewPattern.LifeTime = 7200.0f; // 2 hours
    NewPattern.Movement = FVector2D(2.0f, 1.0f);
    NewPattern.Age = 0.0f;
    
    ActiveWeatherPatterns.Add(NewPattern);
    UE_LOG(LogTemp, Warning, TEXT("Created high pressure system at (%.0f, %.0f)"), Center.X, Center.Y);
}

void UAtmosphericSystem::CreateLowPressureSystem(FVector2D Center, float Strength)
{
    FWeatherPattern NewPattern;
    NewPattern.PatternType = EWeatherPattern::LowPressure;
    NewPattern.Center = Center;
    NewPattern.Intensity = Strength;
    NewPattern.Radius = 20000.0f;
    NewPattern.LifeTime = 10800.0f; // 3 hours
    NewPattern.Movement = FVector2D(-1.0f, 2.0f);
    NewPattern.Age = 0.0f;
    
    ActiveWeatherPatterns.Add(NewPattern);
    UE_LOG(LogTemp, Warning, TEXT("Created low pressure system at (%.0f, %.0f)"), Center.X, Center.Y);
}

void UAtmosphericSystem::CreateFrontalSystem(FVector2D Start, FVector2D End, float Intensity)
{
    // TODO: Create weather front
}

void UAtmosphericSystem::SetSeasonProgress(float Progress)
{
    // TODO: Set seasonal progression
}

void UAtmosphericSystem::SetTimeOfDay(float Hours)
{
    // TODO: Set time of day for solar heating
}

// ===== UTILITY FUNCTIONS =====

int32 UAtmosphericSystem::GetGridIndex(int32 X, int32 Y, int32 Z) const
{
    if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight || Z < 0 || Z >= GridLayers)
    {
        return 0; // Return valid index as fallback
    }
    return Z * GridWidth * GridHeight + Y * GridWidth + X;
}

FAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y, int32 Z)
{
    return AtmosphericGrid[GetGridIndex(X, Y, Z)];
}

const FAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y, int32 Z) const
{
    return AtmosphericGrid[GetGridIndex(X, Y, Z)];
}

FVector UAtmosphericSystem::WorldToGridCoordinates(FVector WorldPosition) const
{
    return FVector(
        WorldPosition.X / CellSize,
        WorldPosition.Y / CellSize,
        WorldPosition.Z / LayerHeight
    );
}

FVector UAtmosphericSystem::GridToWorldCoordinates(int32 X, int32 Y, int32 Z) const
{
    return FVector(
        X * CellSize,
        Y * CellSize,
        Z * LayerHeight
    );
}

// ===== NUMERICAL METHODS - STUBS =====

float UAtmosphericSystem::CalculateGradientX(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const
{
    // TODO: Calculate spatial derivative in X direction
    return 0.0f;
}

float UAtmosphericSystem::CalculateGradientY(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const
{
    // TODO: Calculate spatial derivative in Y direction
    return 0.0f;
}

float UAtmosphericSystem::CalculateGradientZ(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const
{
    // TODO: Calculate spatial derivative in Z direction
    return 0.0f;
}

float UAtmosphericSystem::CalculateLaplacian(const TArray<float>& Field, int32 X, int32 Y, int32 Z) const
{
    // TODO: Calculate Laplacian operator
    return 0.0f;
}

float UAtmosphericSystem::InterpolateField(const TArray<float>& Field, FVector WorldPosition) const
{
    // TODO: Trilinear interpolation
    return 0.0f;
}

FVector UAtmosphericSystem::InterpolateVectorField(const TArray<FVector>& Field, FVector WorldPosition) const
{
    // TODO: Vector field interpolation
    return FVector::ZeroVector;
}

// ===== CLOUD RENDERING SYSTEM IMPLEMENTATION =====

void UAtmosphericSystem::InitializeCloudRendering(USceneComponent* InAttachParent)
{
    AttachParent = InAttachParent;
    
    // Create dynamic material instance if cloud material is set
    if (CloudMaterial)
    {
        CloudMaterialInstance = UMaterialInstanceDynamic::Create(CloudMaterial, this);
        if (CloudMaterialInstance)
        {
            CloudMaterialInstance->SetScalarParameterValue(FName("CloudOpacity"), CloudOpacity);
            UE_LOG(LogTemp, Warning, TEXT("CloudRenderingSystem: Created dynamic material instance"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CloudRenderingSystem: Initialized with %d max cloud meshes"), MaxCloudMeshes);
}

void UAtmosphericSystem::UpdateCloudRendering(float DeltaTime)
{
    if (!AttachParent)
    {
        return;
    }
    
    UpdateCloudMeshes();
}

void UAtmosphericSystem::UpdateCloudMeshes()
{
    int32 UsedMeshes = 0;
    FVector PlayerPosition = GetPlayerPosition();
    float MaxRenderDistance = 15000.0f; // Only render clouds within 15km of player
    
    // Process atmospheric grid to create cloud meshes
    for (int32 Y = 0; Y < GridHeight && UsedMeshes < MaxCloudMeshes; Y += 2) // Skip every other cell for performance
    {
        for (int32 X = 0; X < GridWidth && UsedMeshes < MaxCloudMeshes; X += 2)
        {
            float CloudCoverage = GetCloudCoverageAt(X, Y);
            
            if (CloudCoverage > 0.1f) // Only create clouds with significant coverage
            {
                FVector CloudPosition = GetCloudWorldPosition(X, Y);
                float DistanceToPlayer = FVector::Distance(PlayerPosition, CloudPosition);
                
                if (DistanceToPlayer <= MaxRenderDistance) // Only render clouds near player
                {
                    UStaticMeshComponent* CloudMesh = GetOrCreateCloudMesh();
                    if (CloudMesh)
                    {
                        UpdateCloudMesh(CloudMesh, CloudCoverage, CloudPosition);
                        UsedMeshes++;
                    }
                }
            }
        }
    }
    
    // Hide unused cloud meshes
    HideUnusedCloudMeshes(UsedMeshes);
    
    if (UsedMeshes > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("CloudRenderingSystem: Updated %d cloud meshes"), UsedMeshes);
    }
}

float UAtmosphericSystem::GetCloudCoverageAt(int32 X, int32 Y) const
{
    // Sample multiple atmospheric layers and average cloud coverage
    float TotalCloudCover = 0.0f;
    int32 CloudLayers = 0;
    
    for (int32 Z = 4; Z < 8; Z++) // Typical cloud altitude layers (2-4km)
    {
        int32 Index = GetGridIndex(X, Y, Z);
        if (Index >= 0 && Index < AtmosphericGrid.Num())
        {
            const FAtmosphericCell& Cell = AtmosphericGrid[Index];
            TotalCloudCover += Cell.CloudCoverFraction;
            CloudLayers++;
        }
    }
    
    return CloudLayers > 0 ? TotalCloudCover / CloudLayers : 0.0f;
}

FVector UAtmosphericSystem::GetCloudWorldPosition(int32 AtmosphericX, int32 AtmosphericY) const
{
    return FVector(
        AtmosphericX * CellSize,
        AtmosphericY * CellSize,
        CloudAltitude
    );
}

FVector UAtmosphericSystem::GetPlayerPosition() const
{
    if (!TerrainSystem || !TerrainSystem->GetWorld())
    {
        return FVector::ZeroVector;
    }
    
    UWorld* World = TerrainSystem->GetWorld();
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        return PC->GetPawn()->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}

UStaticMeshComponent* UAtmosphericSystem::GetOrCreateCloudMesh()
{
    // Find an unused cloud mesh
    for (UStaticMeshComponent* CloudMesh : CloudMeshes)
    {
        if (CloudMesh && !CloudMesh->IsVisible())
        {
            return CloudMesh;
        }
    }
    
    // Create new cloud mesh if under limit
    if (CloudMeshes.Num() < MaxCloudMeshes && AttachParent)
    {
        FString MeshName = FString::Printf(TEXT("CloudMesh_%d"), CloudMeshes.Num());
        UStaticMeshComponent* NewCloudMesh = NewObject<UStaticMeshComponent>(AttachParent, *MeshName);
        
        if (NewCloudMesh)
        {
            NewCloudMesh->SetupAttachment(AttachParent);
            
            // Set cloud static mesh (will need to be assigned in Blueprint)
            if (CloudStaticMesh)
            {
                NewCloudMesh->SetStaticMesh(CloudStaticMesh);
            }
            
            // Set cloud material
            if (CloudMaterialInstance)
            {
                NewCloudMesh->SetMaterial(0, CloudMaterialInstance);
            }
            else if (CloudMaterial)
            {
                NewCloudMesh->SetMaterial(0, CloudMaterial);
            }
            
            NewCloudMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            NewCloudMesh->SetCastShadow(true); // Clouds cast shadows for realism
            NewCloudMesh->RegisterComponent();
            
            CloudMeshes.Add(NewCloudMesh);
            
            UE_LOG(LogTemp, Warning, TEXT("CloudRenderingSystem: Created cloud mesh %d"), CloudMeshes.Num());
            return NewCloudMesh;
        }
    }
    
    return nullptr;
}

void UAtmosphericSystem::UpdateCloudMesh(UStaticMeshComponent* CloudMesh, float CloudCover, FVector Position)
{
    if (!CloudMesh)
    {
        return;
    }
    
    // Update position
    CloudMesh->SetWorldLocation(Position);
    
    // Update scale based on cloud coverage
    float CloudScale = FMath::Lerp(0.5f, 3.0f, CloudCover);
    CloudMesh->SetWorldScale3D(FVector(CloudScale));
    
    // Update material parameters if we have a dynamic material instance
    if (CloudMaterialInstance)
    {
        float OpacityValue = CloudCover * CloudOpacity;
        CloudMaterialInstance->SetScalarParameterValue(FName("CloudOpacity"), OpacityValue);
        CloudMaterialInstance->SetScalarParameterValue(FName("CloudDensity"), CloudCover);
        
        // Update material on the mesh
        CloudMesh->SetMaterial(0, CloudMaterialInstance);
    }
    
    // Make visible
    CloudMesh->SetVisibility(true);
}

void UAtmosphericSystem::HideUnusedCloudMeshes(int32 UsedMeshCount)
{
    for (int32 i = UsedMeshCount; i < CloudMeshes.Num(); i++)
    {
        if (CloudMeshes[i])
        {
            CloudMeshes[i]->SetVisibility(false);
        }
    }
}
