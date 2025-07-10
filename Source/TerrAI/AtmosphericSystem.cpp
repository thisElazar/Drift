// AtmosphericSystem.cpp - Scientific Atmospheric Physics Implementation
#include "AtmosphericSystem.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "TemporalManager.h"
#include "MasterController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
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
    
    // CRITICAL FIX: Get MasterController reference immediately
    if (InTerrain->CachedMasterController)
    {
        MasterController = InTerrain->CachedMasterController;
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Connected to MasterController"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: No MasterController - coordinate transforms disabled"));
    }
    
    // Match atmospheric grid to terrain scale
    float TerrainWidth = InTerrain->TerrainWidth * InTerrain->TerrainScale;
    float TerrainHeight = InTerrain->TerrainHeight * InTerrain->TerrainScale;
    
    // Adjust grid to cover terrain area
    GridWidth = FMath::Max(32, FMath::RoundToInt(TerrainWidth / CellSize));
    GridHeight = FMath::Max(32, FMath::RoundToInt(TerrainHeight / CellSize));
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Grid size %dx%dx%d covering %.1fkm terrain"),
           GridWidth, GridHeight, GridLayers, TerrainWidth / 1000.0f);
    
    InitializeAtmosphericGrid();
    
    // Get temporal manager reference
    if (bUseTemporalManager)
    {
        UGameInstance* GameInstance = InTerrain->GetWorld()->GetGameInstance();
        if (GameInstance)
        {
            TemporalManager = GameInstance->GetSubsystem<UTemporalManager>();
            if (TemporalManager)
            {
                // Register atmospheric system with 60:1 time scale (1 minute = 1 second)
                TemporalManager->RegisterSystem(ESystemType::Atmospheric, 60.0f);
                UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Registered with TemporalManager (60:1 scale)"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: TemporalManager not available, using real-time"));
                bUseTemporalManager = false;
            }
        }
    }
    
    // Initialize cloud rendering system
    InitializeCloudRendering(InTerrain->GetRootComponent());
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized with %d atmospheric cells"),
           AtmosphericGrid.Num());
    
    // CRITICAL FIX: Late registration with MasterController
       if (InTerrain && InTerrain->CachedMasterController && !IsRegisteredWithMaster())
       {
           AMasterWorldController* Master = InTerrain->CachedMasterController;
           
           // Register ourselves
           RegisterWithMasterController(Master);
           
           // Apply scaling if needed
           if (!IsSystemScaled())
           {
               // Get current world scaling config
               const FWorldScalingConfig& Config = Master->GetWorldScalingConfig();
               ConfigureFromMaster(Config);
               
               // Synchronize coordinates
               const FWorldCoordinateSystem& Coords = Master->GetWorldCoordinateSystem();
               SynchronizeCoordinates(Coords);
               
               UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Late registration and scaling applied"));
           }
       }
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
    
    float ScaledDeltaTime = DeltaTime;
    float AtmosphericTime = 0.0f;
    
    // Use temporal manager if available
    if (bUseTemporalManager && TemporalManager)
    {
        // Check if atmospheric system should update this frame
        if (!TemporalManager->ShouldSystemUpdate(ESystemType::Atmospheric, TemporalUpdateFrequency))
        {
            return;
        }
        
        // Get temporally scaled delta time
        ScaledDeltaTime = TemporalManager->GetSystemDeltaTime(ESystemType::Atmospheric, DeltaTime);
        AtmosphericTime = TemporalManager->GetSystemTime(ESystemType::Atmospheric);
    }
    else
    {
        // Fallback to accumulating real time
        AccumulatedTime += DeltaTime;
        AtmosphericTime = AccumulatedTime;
        
        // Use adaptive time stepping
        if (bUseAdaptiveTimeStep)
        {
            float TimeStep = 1.0f / UpdateFrequency;
            if (AccumulatedTime < TimeStep)
            {
                return;
            }
            ScaledDeltaTime = AccumulatedTime;
            AccumulatedTime = 0.0f;
        }
    }
    
    // Core atmospheric physics updates (using scaled time)
    UpdateThermodynamics(ScaledDeltaTime);
    UpdateFluidDynamics(ScaledDeltaTime);
    UpdateCloudPhysics(ScaledDeltaTime);
    UpdateRadiation(ScaledDeltaTime);
    
    // Weather pattern systems
    UpdateWeatherPatterns(ScaledDeltaTime);
    
    // Boundary interactions
    ApplyTerrainInteraction();
    
    // Interface with other systems
    UpdateWaterSystemInterface();
    
    // Update cloud rendering system
    if (bEnableCloudRendering)
    {
        UpdateCloudRendering(ScaledDeltaTime);
    }
    
    // Debug logging with temporal information
    static float LastDebugTime = 0.0f;
    if (bUseTemporalManager && TemporalManager && AtmosphericTime - LastDebugTime > 60.0f) // Log every simulated minute
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("AtmosphericSystem: Simulated time %.2f, Real time scale %.2fx"), 
               AtmosphericTime, TemporalManager->GetSystemTimeScale(ESystemType::Atmospheric));
        LastDebugTime = AtmosphericTime;
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
                
                // Enhanced precipitation generation
                if (Cell.CloudCoverFraction > 0.3f && Cell.Humidity > 0.7f)
                {
                    // Calculate precipitation based on cloud water content and conditions
                    float BaseRate = Cell.CloudWaterContent * 100.0f; // Convert to mm/hr
                    
                    // Temperature factor (more rain in warmer conditions)
                    float TempFactor = FMath::Clamp((Cell.Temperature - 0.0f) / 20.0f, 0.1f, 1.0f);
                    
                    // Apply weather pattern influence
                    float PatternBoost = 1.0f;
                    for (const FWeatherPattern& Pattern : ActiveWeatherPatterns)
                    {
                        FVector2D GridPos(X * CellSize, Y * CellSize);
                        float Distance = FVector2D::Distance(GridPos, Pattern.Center);
                        
                        if (Distance < Pattern.Radius)
                        {
                            float Influence = 1.0f - (Distance / Pattern.Radius);
                            if (Pattern.PatternType == EWeatherPattern::LowPressure)
                            {
                                PatternBoost += Influence * Pattern.Intensity * 2.0f;
                            }
                        }
                    }
                    
                    // Calculate final precipitation rate
                    Cell.PrecipitationRate = BaseRate * TempFactor * PatternBoost;
                    
                    // Reduce cloud water as it precipitates
                    Cell.CloudWaterContent *= (1.0f - 0.1f * DeltaTime);
                    
                    // Increase precipitation if supersaturated
                    if (Cell.Humidity > 0.95f)
                    {
                        Cell.PrecipitationRate *= 2.0f;
                        Cell.Humidity = 0.9f; // Remove excess humidity
                    }
                }
                else
                {
                    // Gradually reduce precipitation
                    Cell.PrecipitationRate *= 0.95f;
                    if (Cell.PrecipitationRate < 0.01f)
                    {
                        Cell.PrecipitationRate = 0.0f;
                    }
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
    // Update existing patterns
    for (FWeatherPattern& Pattern : ActiveWeatherPatterns)
    {
        // Move patterns across the landscape
        Pattern.Center += Pattern.Movement * DeltaTime * 10.0f; // 10x speed for visible movement
        Pattern.Age += DeltaTime;
        
        // Gradually weaken patterns over time
        Pattern.Intensity *= (1.0f - DeltaTime * 0.01f); // 1% decay per second
        
        // Apply pattern effects to atmospheric grid
        // * USE Just Single Implementation// ApplyWeatherPatternToGrid(Pattern);
    }
    
    // Remove expired patterns
    ActiveWeatherPatterns.RemoveAll([](const FWeatherPattern& Pattern) {
        return Pattern.Age > 1800.0f || Pattern.Intensity < 0.1f; // Remove after 30 min or weak
    });
    
    // CRITICAL: Remove expired patterns to prevent memory issues
    ActiveWeatherPatterns.RemoveAll([DeltaTime](FWeatherPattern& Pattern) {
        Pattern.Age += DeltaTime;
        return Pattern.Age > Pattern.LifeTime;
    });
    
    // CRITICAL: Limit total number of active patterns
    const int32 MaxActivePatterns = 10;
    if (ActiveWeatherPatterns.Num() > MaxActivePatterns)
    {
        // Remove oldest patterns
        ActiveWeatherPatterns.Sort([](const FWeatherPattern& A, const FWeatherPattern& B) {
            return A.Age > B.Age;
        });
        ActiveWeatherPatterns.SetNum(MaxActivePatterns);
    }
    
    // Update pattern positions
    for (FWeatherPattern& Pattern : ActiveWeatherPatterns)
    {
        Pattern.Center += Pattern.Movement * DeltaTime;
    }
    
    // Apply patterns to atmospheric grid
    ApplyWeatherPatterns();

    
    // Generate new patterns occasionally (every 5-10 minutes)
    static float PatternTimer = 0.0f;
    PatternTimer += DeltaTime;
    if (PatternTimer > 300.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        GenerateNewWeatherPattern();
        PatternTimer = 0.0f;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WEATHER] %d active patterns"), ActiveWeatherPatterns.Num());
}

void UAtmosphericSystem::ApplyWeatherPatterns()
{
    // Apply all active weather patterns to the atmospheric grid
    for (const FWeatherPattern& Pattern : ActiveWeatherPatterns)
    {
        ApplyWeatherPatternToGrid(Pattern);
    }
}

void UAtmosphericSystem::ApplyWeatherPatternToGrid(const FWeatherPattern& Pattern)
{
    float CellSize = (TerrainSystem->TerrainWidth * TerrainSystem->TerrainScale) / GridWidth;
    
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            for (int32 X = 0; X < GridWidth; X++)
            {
                int32 Index = GetGridIndex(X, Y, Z);
                FAtmosphericCell& Cell = AtmosphericGrid[Index];
                
                // Calculate distance from pattern center
                FVector2D GridPos(X * CellSize, Y * CellSize);
                float DistanceToPattern = FVector2D::Distance(GridPos, Pattern.Center);
                
                if (DistanceToPattern < Pattern.Radius)
                {
                    float PatternInfluence = 1.0f - (DistanceToPattern / Pattern.Radius);
                    float AltitudeFactor = 1.0f;
                    
                    // Height-based effects (more activity at mid-altitudes)
                    float Altitude = Z * LayerHeight;
                    if (Altitude >= 1000.0f && Altitude <= 8000.0f)
                    {
                        AltitudeFactor = 1.0f - FMath::Abs(Altitude - 4000.0f) / 4000.0f;
                        AltitudeFactor = FMath::Clamp(AltitudeFactor, 0.3f, 1.0f);
                    }
                    else
                    {
                        AltitudeFactor = 0.1f; // Minimal activity outside optimal altitude
                    }
                    
                    if (Pattern.PatternType == EWeatherPattern::LowPressure)
                    {
                        // Low pressure systems create rising air, clouds, and precipitation
                        float Effect = PatternInfluence * Pattern.Intensity * AltitudeFactor;
                        
                        Cell.Humidity = FMath::Min(Cell.Humidity + Effect * 0.2f, 1.0f);
                        Cell.CloudCoverFraction = FMath::Min(Cell.CloudCoverFraction + Effect * 0.3f, 1.0f);
                        Cell.CloudWaterContent = FMath::Min(Cell.CloudWaterContent + Effect * 0.5f, 2.0f);
                        Cell.PrecipitationRate = FMath::Min(Cell.PrecipitationRate + Effect * 8.0f, 25.0f);
                        
                        // Add some turbulence
                        Cell.Pressure -= Effect * 500.0f; // Lower pressure
                        Cell.Temperature += Effect * 2.0f; // Slight warming from condensation
                    }
                    else if (Pattern.PatternType == EWeatherPattern::HighPressure)
                    {
                        // High pressure systems create descending air, clear skies
                        float Effect = PatternInfluence * Pattern.Intensity * AltitudeFactor;
                        
                        Cell.CloudCoverFraction = FMath::Max(Cell.CloudCoverFraction - Effect * 0.2f, 0.0f);
                        Cell.PrecipitationRate = FMath::Max(Cell.PrecipitationRate - Effect * 3.0f, 0.0f);
                        Cell.Pressure += Effect * 300.0f; // Higher pressure
                    }
                }
            }
        }
    }
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

void UAtmosphericSystem::GenerateNewWeatherPattern()
{
    //UE_LOG(LogTemp, Warning, TEXT("GenerateNewWeatherPatternsRemoved");
           /*
    if (!TerrainSystem) return;
    
    FWeatherPattern NewPattern;
    
    // Random position within terrain bounds
    float TerrainWorldWidth = TerrainSystem->TerrainWidth * TerrainSystem->TerrainScale;
    float TerrainWorldHeight = TerrainSystem->TerrainHeight * TerrainSystem->TerrainScale;
    
    NewPattern.Center = FVector2D(
        FMath::RandRange(0.0f, TerrainWorldWidth),
        FMath::RandRange(0.0f, TerrainWorldHeight)
    );
    
    // Random pattern type (70% low pressure for more rain)
    NewPattern.PatternType = (FMath::RandRange(0.0f, 1.0f) < 0.7f) ? 
        EWeatherPattern::LowPressure : EWeatherPattern::HighPressure;
    
    // Random properties
    NewPattern.Intensity = FMath::RandRange(0.5f, 2.0f);
    NewPattern.Radius = FMath::RandRange(5000.0f, 15000.0f);
    NewPattern.Age = 0.0f;
    
    // Random movement direction
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Speed = FMath::RandRange(2.0f, 8.0f); // m/s typical weather system speed
    NewPattern.Movement = FVector2D(FMath::Cos(Angle) * Speed, FMath::Sin(Angle) * Speed);
    
    ActiveWeatherPatterns.Add(NewPattern);
    
    UE_LOG(LogTemp, Warning, TEXT("[WEATHER] Created new %s system at (%.0f, %.0f)"),
           (NewPattern.PatternType == EWeatherPattern::LowPressure) ? TEXT("LOW") : TEXT("HIGH"),
           NewPattern.Center.X, NewPattern.Center.Y);
    */
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
        return; // Fail silently without spam
    }
    
    // CRITICAL FIX: Don't try coordinate transforms without MasterController
    if (!MasterController)
    {
        static float LastErrorTime = 0.0f;
        float CurrentTime = TerrainSystem->GetWorld() ? TerrainSystem->GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTime - LastErrorTime > 5.0f) // Only log every 5 seconds
        {
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC] Waiting for MasterController connection..."));
            LastErrorTime = CurrentTime;
        }
        return;
    }
    
    // DEBUGGING: Check if atmospheric simulation is actually generating precipitation
    static float LastDebugTime = 0.0f;
    float CurrentTime = TerrainSystem->GetWorld() ? TerrainSystem->GetWorld()->GetTimeSeconds() : 0.0f;
    bool bShouldLogDebug = (CurrentTime - LastDebugTime) > 10.0f; // Every 10 seconds
    
    if (bShouldLogDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC DEBUG] UpdateWaterSystemInterface called - Time: %.1fs"), CurrentTime);
        UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC DEBUG] Active weather patterns: %d"), ActiveWeatherPatterns.Num());
        
        // Check if any cells have precipitation
        int32 CellsWithPrecipitation = 0;
        float MaxPrecipitation = 0.0f;
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            for (int32 X = 0; X < GridWidth; X++)
            {
                float Precipitation = GetPrecipitationAt(GridToWorldCoordinates(X, Y, 0));
                if (Precipitation > 0.01f)
                {
                    CellsWithPrecipitation++;
                    MaxPrecipitation = FMath::Max(MaxPrecipitation, Precipitation);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC DEBUG] Cells with precipitation: %d, Max: %.3f mm/hr"), 
               CellsWithPrecipitation, MaxPrecipitation);
        LastDebugTime = CurrentTime;
    }
    
    // OPTIMIZED PRECIPITATION APPLICATION: Collect affected chunks first
    TSet<int32> AffectedChunks;
    float TotalPrecipitationApplied = 0.0f;
    int32 CellsWithPrecipitation = 0;
    
    // Apply precipitation to water system and collect chunk updates
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            float Precipitation = GetPrecipitationAt(GridToWorldCoordinates(X, Y, 0));
            
            if (Precipitation > 0.1f) // Minimum threshold (0.1 mm/hr)
            {
                // CRITICAL FIX: Proper precipitation rate conversion
                // mm/hr to m/s: divide by 3,600,000 (3600 seconds * 1000 mm/m)
                // Then multiply by DeltaTime to get actual water depth addition per frame
                float WaterDepthPerSecond = Precipitation / 3600000.0f; // mm/hr to m/s
                
                // Get frame time from world context
                float DeltaTime = 0.016f; // Default 60 FPS fallback
                if (TerrainSystem && TerrainSystem->GetWorld())
                {
                    DeltaTime = TerrainSystem->GetWorld()->GetDeltaSeconds();
                }
                
                float WaterAmount = WaterDepthPerSecond * DeltaTime;
                
                // Apply to multiple terrain points within this atmospheric cell for natural distribution
                int32 TerrainPointsPerCell = FMath::RandRange(4, 12); // Variable rain density
                for (int32 i = 0; i < TerrainPointsPerCell; i++)
                {
                    // Create natural precipitation pattern with some clustering
                    FVector2D RandomOffset(
                        FMath::RandRange(-CellSize * 0.4f, CellSize * 0.4f),
                        FMath::RandRange(-CellSize * 0.4f, CellSize * 0.4f)
                    );
                    
                    FVector WorldPos = GridToWorldCoordinates(X, Y, 0);
                    WorldPos.X += RandomOffset.X;
                    WorldPos.Y += RandomOffset.Y;
                    
                    // CRITICAL: Apply rain to water system
                    // Divide water amount by number of points to maintain correct total
                    WaterSystem->AddWater(WorldPos, WaterAmount / TerrainPointsPerCell);
                    TotalPrecipitationApplied += WaterAmount / TerrainPointsPerCell;
                    
                    // OPTIMIZATION: Track affected chunks for batch update
                    FVector2D TerrainCoords = MasterController->WorldToTerrainCoordinates(WorldPos);
                    int32 ChunkIndex = TerrainSystem->GetChunkIndexFromCoordinates(
                        FMath::FloorToInt(TerrainCoords.X), 
                        FMath::FloorToInt(TerrainCoords.Y)
                    );
                    if (ChunkIndex >= 0)
                    {
                        AffectedChunks.Add(ChunkIndex);
                    }
                }
                
                CellsWithPrecipitation++;
            }
        }
    }
    
    // CRITICAL OPTIMIZATION: Use new batch water update system
    if (AffectedChunks.Num() > 0)
    {
        TerrainSystem->BatchUpdateWaterChunks(AffectedChunks.Array());
        
        if (bShouldLogDebug)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION] Batch updated %d water chunks for precipitation"), AffectedChunks.Num());
        }
    }
    
    // Log precipitation integration activity (throttled)
    static float LastPrecipitationLogTime = 0.0f;
    if (TerrainSystem && TerrainSystem->GetWorld())
    {
        float CurrentTime = TerrainSystem->GetWorld()->GetTimeSeconds();
        if (TotalPrecipitationApplied > 0.0f && (CurrentTime - LastPrecipitationLogTime) > 5.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION] Applied precipitation: %.6f m total to %d cells, updated %d chunks"),
                   TotalPrecipitationApplied, CellsWithPrecipitation, AffectedChunks.Num());
            LastPrecipitationLogTime = CurrentTime;
        }
        else if (TotalPrecipitationApplied == 0.0f && (CurrentTime - LastPrecipitationLogTime) > 15.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ATMOSPHERIC INTEGRATION] No precipitation generated yet - atmospheric simulation may need more time"));
            LastPrecipitationLogTime = CurrentTime;
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
    if (!TerrainSystem || AtmosphericGrid.Num() == 0)
    {
        return 0.0f;
    }
    
    // Convert world position to grid coordinates
    FVector LocalPosition = TerrainSystem->GetActorTransform().InverseTransformPosition(WorldPosition);
    
    float CellSize = (TerrainSystem->TerrainWidth * TerrainSystem->TerrainScale) / GridWidth;
    int32 X = FMath::Clamp(FMath::FloorToInt(LocalPosition.X / CellSize), 0, GridWidth - 1);
    int32 Y = FMath::Clamp(FMath::FloorToInt(LocalPosition.Y / CellSize), 0, GridHeight - 1);
    
    // CRITICAL FIX: Return MAXIMUM precipitation across all layers, not average
    float MaxPrecipitation = 0.0f;
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        int32 Index = GetGridIndex(X, Y, Z);
        if (Index >= 0 && Index < AtmosphericGrid.Num())
        {
            MaxPrecipitation = FMath::Max(MaxPrecipitation, AtmosphericGrid[Index].PrecipitationRate);
        }
    }
    
    return MaxPrecipitation;
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
    UE_LOG(LogTemp, VeryVerbose, TEXT("Created high pressure system at (%.0f, %.0f)"), Center.X, Center.Y);
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
    UE_LOG(LogTemp, VeryVerbose, TEXT("Created low pressure system at (%.0f, %.0f)"), Center.X, Center.Y);
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

void UAtmosphericSystem::ForceTestPrecipitation()
{
    UE_LOG(LogTemp, Warning, TEXT("[ATMOS] TEST PRECIPITATION REMOVED"));
    /*
    // Force precipitation in central region for immediate testing
    int32 CenterX = GridWidth / 2;
    int32 CenterY = GridHeight / 2;
    int32 TestRadius = 8; // 16x16 cell area
    
    for (int32 Y = CenterY - TestRadius; Y <= CenterY + TestRadius; Y++)
    {
        for (int32 X = CenterX - TestRadius; X <= CenterX + TestRadius; X++)
        {
            if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
            {
                for (int32 Z = 1; Z < GridLayers - 1; Z++) // Skip ground and top layer
                {
                    int32 Index = GetGridIndex(X, Y, Z);
                    if (Index >= 0 && Index < AtmosphericGrid.Num())
                    {
                        AtmosphericGrid[Index].CloudCoverFraction = 0.9f;
                        AtmosphericGrid[Index].CloudWaterContent = 2.0f;
                        AtmosphericGrid[Index].Humidity = 0.95f;
                        AtmosphericGrid[Index].PrecipitationRate = 12.0f; // Heavy rain
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[ATMOS] Test precipitation active - check water system"));
*/
}

// ===== ATMOSPHERIC BRUSHES =====

void UAtmosphericSystem::ApplyTemperatureBrush(FVector WorldPosition, float Radius, float TemperatureChange, float Strength)
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 RadiusInCells = FMath::CeilToInt(Radius / CellSize);
    
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = CenterY - RadiusInCells; Y <= CenterY + RadiusInCells; Y++)
        {
            for (int32 X = CenterX - RadiusInCells; X <= CenterX + RadiusInCells; X++)
            {
                if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
                {
                    float Distance = FMath::Sqrt((float)(FMath::Square(X - CenterX) + FMath::Square(Y - CenterY)));
                    if (Distance <= RadiusInCells)
                    {
                        float Falloff = 1.0f - (Distance / RadiusInCells);
                        Falloff = FMath::Pow(Falloff, 2.0f); // Quadratic falloff
                        
                        int32 Index = GetGridIndex(X, Y, Z);
                        AtmosphericGrid[Index].Temperature += TemperatureChange * Falloff * Strength;
                        AtmosphericGrid[Index].Temperature = FMath::Clamp(AtmosphericGrid[Index].Temperature, 200.0f, 350.0f);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BRUSH] Applied temperature brush: %.1f°C at (%.0f, %.0f)"), 
           TemperatureChange, WorldPosition.X, WorldPosition.Y);
}

void UAtmosphericSystem::ApplyHumidityBrush(FVector WorldPosition, float Radius, float HumidityChange, float Strength)
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 RadiusInCells = FMath::CeilToInt(Radius / CellSize);
    
    for (int32 Z = 0; Z < 6; Z++) // Focus on lower atmosphere
    {
        for (int32 Y = CenterY - RadiusInCells; Y <= CenterY + RadiusInCells; Y++)
        {
            for (int32 X = CenterX - RadiusInCells; X <= CenterX + RadiusInCells; X++)
            {
                if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
                {
                    float Distance = FMath::Sqrt((float)(FMath::Square(X - CenterX) + FMath::Square(Y - CenterY)));
                    if (Distance <= RadiusInCells)
                    {
                        float Falloff = 1.0f - (Distance / RadiusInCells);
                        
                        int32 Index = GetGridIndex(X, Y, Z);
                        AtmosphericGrid[Index].Humidity += HumidityChange * Falloff * Strength;
                        AtmosphericGrid[Index].Humidity = FMath::Clamp(AtmosphericGrid[Index].Humidity, 0.0f, 1.0f);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BRUSH] Applied humidity brush: %.2f at (%.0f, %.0f)"), 
           HumidityChange, WorldPosition.X, WorldPosition.Y);
}

void UAtmosphericSystem::ApplyPrecipitationBrush(FVector WorldPosition, float Radius, float RainIntensity, float Duration)
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 RadiusInCells = FMath::CeilToInt(Radius / CellSize);
    
    // Create temporary weather pattern for sustained rain
    FWeatherPattern RainPattern;
    RainPattern.PatternType = EWeatherPattern::LowPressure;
    RainPattern.Center = FVector2D(WorldPosition.X, WorldPosition.Y);
    RainPattern.Intensity = RainIntensity / 10.0f; // Scale intensity
    RainPattern.Radius = Radius;
    RainPattern.Movement = FVector2D::ZeroVector; // Stationary
    RainPattern.LifeTime = Duration;
    RainPattern.Age = 0.0f;
    
    ActiveWeatherPatterns.Add(RainPattern);
    
    // Immediate precipitation effect
    for (int32 Z = 2; Z < 8; Z++) // Rain formation layers
    {
        for (int32 Y = CenterY - RadiusInCells; Y <= CenterY + RadiusInCells; Y++)
        {
            for (int32 X = CenterX - RadiusInCells; X <= CenterX + RadiusInCells; X++)
            {
                if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
                {
                    float Distance = FMath::Sqrt((float)FMath::Square(X - CenterX) + FMath::Square(Y - CenterY));
                    if (Distance <= RadiusInCells)
                    {
                        float Falloff = 1.0f - (Distance / RadiusInCells);
                        
                        int32 Index = GetGridIndex(X, Y, Z);
                        AtmosphericGrid[Index].CloudCoverFraction = FMath::Min(AtmosphericGrid[Index].CloudCoverFraction + 0.8f * Falloff, 1.0f);
                        AtmosphericGrid[Index].CloudWaterContent = FMath::Min(AtmosphericGrid[Index].CloudWaterContent + 1.5f * Falloff, 3.0f);
                        AtmosphericGrid[Index].Humidity = FMath::Min(AtmosphericGrid[Index].Humidity + 0.3f * Falloff, 1.0f);
                        AtmosphericGrid[Index].PrecipitationRate = FMath::Max(AtmosphericGrid[Index].PrecipitationRate, RainIntensity * Falloff);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BRUSH] Applied precipitation brush: %.1f mm/hr for %.0fs at (%.0f, %.0f)"), 
           RainIntensity, Duration, WorldPosition.X, WorldPosition.Y);
}

void UAtmosphericSystem::ApplyCloudBrush(FVector WorldPosition, float Radius, float CloudDensity, float Strength)
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 RadiusInCells = FMath::CeilToInt(Radius / CellSize);
    
    for (int32 Z = 3; Z < 9; Z++) // Cloud formation layers
    {
        for (int32 Y = CenterY - RadiusInCells; Y <= CenterY + RadiusInCells; Y++)
        {
            for (int32 X = CenterX - RadiusInCells; X <= CenterX + RadiusInCells; X++)
            {
                if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
                {
                    float Distance = FMath::Sqrt((float)FMath::Square(X - CenterX) + FMath::Square(Y - CenterY));
                    if (Distance <= RadiusInCells)
                    {
                        float Falloff = 1.0f - (Distance / RadiusInCells);
                        
                        int32 Index = GetGridIndex(X, Y, Z);
                        AtmosphericGrid[Index].CloudCoverFraction += CloudDensity * Falloff * Strength;
                        AtmosphericGrid[Index].CloudCoverFraction = FMath::Clamp(AtmosphericGrid[Index].CloudCoverFraction, 0.0f, 1.0f);
                        
                        if (CloudDensity > 0.0f)
                        {
                            AtmosphericGrid[Index].CloudWaterContent += CloudDensity * 0.5f * Falloff * Strength;
                            AtmosphericGrid[Index].CloudWaterContent = FMath::Clamp(AtmosphericGrid[Index].CloudWaterContent, 0.0f, 2.0f);
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BRUSH] Applied cloud brush: %.2f density at (%.0f, %.0f)"), 
           CloudDensity, WorldPosition.X, WorldPosition.Y);
}

void UAtmosphericSystem::ClearWeatherBrush(FVector WorldPosition, float Radius, float Strength)
{
    FVector GridPos = WorldToGridCoordinates(WorldPosition);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 RadiusInCells = FMath::CeilToInt(Radius / CellSize);
    
    for (int32 Z = 0; Z < GridLayers; Z++)
    {
        for (int32 Y = CenterY - RadiusInCells; Y <= CenterY + RadiusInCells; Y++)
        {
            for (int32 X = CenterX - RadiusInCells; X <= CenterX + RadiusInCells; X++)
            {
                if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
                {
                    float Distance = FMath::Sqrt((float)FMath::Square(X - CenterX) + FMath::Square(Y - CenterY));
                    if (Distance <= RadiusInCells)
                    {
                        float Falloff = 1.0f - (Distance / RadiusInCells);
                        float ClearStrength = Falloff * Strength;
                        
                        int32 Index = GetGridIndex(X, Y, Z);
                        AtmosphericGrid[Index].CloudCoverFraction *= (1.0f - ClearStrength);
                        AtmosphericGrid[Index].CloudWaterContent *= (1.0f - ClearStrength);
                        AtmosphericGrid[Index].PrecipitationRate *= (1.0f - ClearStrength * 0.5f);
                        
                        // Restore normal humidity
                        float TargetHumidity = 0.5f + (Z * LayerHeight) / 10000.0f * 0.3f; // Altitude-based
                        AtmosphericGrid[Index].Humidity = FMath::FInterpTo(AtmosphericGrid[Index].Humidity, TargetHumidity, 1.0f, ClearStrength);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BRUSH] Applied clear weather brush at (%.0f, %.0f)"), 
           WorldPosition.X, WorldPosition.Y);
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

// ===== ATMOSPHERIC EDITING FUNCTIONS =====

void UAtmosphericSystem::ApplyWindBrush(FVector Position, float Radius, FVector WindForce, float Intensity)
{
    if (AtmosphericGrid.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Cannot apply wind brush - grid not initialized"));
        return;
    }
    
    // Apply wind brush effect to atmospheric grid
    FVector GridPos = WorldToGridCoordinates(Position);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 CenterZ = FMath::RoundToInt(GridPos.Z);
    
    float GridRadius = Radius / CellSize;
    int32 IntRadius = FMath::CeilToInt(GridRadius);
    
    for (int32 Z = FMath::Max(0, CenterZ - IntRadius); Z <= FMath::Min(GridLayers - 1, CenterZ + IntRadius); Z++)
    {
        for (int32 Y = FMath::Max(0, CenterY - IntRadius); Y <= FMath::Min(GridHeight - 1, CenterY + IntRadius); Y++)
        {
            for (int32 X = FMath::Max(0, CenterX - IntRadius); X <= FMath::Min(GridWidth - 1, CenterX + IntRadius); X++)
            {
                float Distance = FVector::Dist(FVector(X, Y, Z), FVector(CenterX, CenterY, CenterZ));
                
                if (Distance <= GridRadius)
                {
                    // Calculate falloff based on distance
                    float Falloff = 1.0f - (Distance / GridRadius);
                    Falloff = FMath::Pow(Falloff, 2.0f); // Smooth falloff
                    
                    int32 Index = GetGridIndex(X, Y, Z);
                    if (Index >= 0 && Index < AtmosphericGrid.Num())
                    {
                        FAtmosphericCell& Cell = AtmosphericGrid[Index];
                        
                        // Apply wind force with falloff and intensity
                        FVector ScaledForce = WindForce * Falloff * Intensity;
                        Cell.Velocity += ScaledForce;
                        
                        // Clamp to reasonable wind speeds
                        float MaxWindSpeed = 100.0f; // m/s (hurricane force)
                        if (Cell.Velocity.Size() > MaxWindSpeed)
                        {
                            Cell.Velocity = Cell.Velocity.GetSafeNormal() * MaxWindSpeed;
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied wind brush at %.1f,%.1f,%.1f with force %.1f,%.1f,%.1f"),
           Position.X, Position.Y, Position.Z, WindForce.X, WindForce.Y, WindForce.Z);
}

void UAtmosphericSystem::ApplyPressureBrush(FVector Position, float Radius, float PressureDelta, float Intensity)
{
    if (AtmosphericGrid.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Cannot apply pressure brush - grid not initialized"));
        return;
    }
    
    // Apply pressure brush effect to atmospheric grid
    FVector GridPos = WorldToGridCoordinates(Position);
    int32 CenterX = FMath::RoundToInt(GridPos.X);
    int32 CenterY = FMath::RoundToInt(GridPos.Y);
    int32 CenterZ = FMath::RoundToInt(GridPos.Z);
    
    float GridRadius = Radius / CellSize;
    int32 IntRadius = FMath::CeilToInt(GridRadius);
    
    for (int32 Z = FMath::Max(0, CenterZ - IntRadius); Z <= FMath::Min(GridLayers - 1, CenterZ + IntRadius); Z++)
    {
        for (int32 Y = FMath::Max(0, CenterY - IntRadius); Y <= FMath::Min(GridHeight - 1, CenterY + IntRadius); Y++)
        {
            for (int32 X = FMath::Max(0, CenterX - IntRadius); X <= FMath::Min(GridWidth - 1, CenterX + IntRadius); X++)
            {
                float Distance = FVector::Dist(FVector(X, Y, Z), FVector(CenterX, CenterY, CenterZ));
                
                if (Distance <= GridRadius)
                {
                    // Calculate falloff based on distance
                    float Falloff = 1.0f - (Distance / GridRadius);
                    Falloff = FMath::Pow(Falloff, 2.0f); // Smooth falloff
                    
                    int32 Index = GetGridIndex(X, Y, Z);
                    if (Index >= 0 && Index < AtmosphericGrid.Num())
                    {
                        FAtmosphericCell& Cell = AtmosphericGrid[Index];
                        
                        // Apply pressure change with falloff and intensity
                        float ScaledPressureDelta = PressureDelta * Falloff * Intensity;
                        Cell.Pressure += ScaledPressureDelta;
                        
                        // Clamp pressure to reasonable atmospheric range
                        Cell.Pressure = FMath::Clamp(Cell.Pressure, 50000.0f, 120000.0f); // 500-1200 hPa
                        
                        // Update density based on new pressure (ideal gas law)
                        if (Cell.Temperature > 0.0f)
                        {
                            Cell.Density = Cell.Pressure / (AtmosphericConstants::GAS_CONSTANT * Cell.Temperature);
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied pressure brush at %.1f,%.1f,%.1f with delta %.1f Pa"),
           Position.X, Position.Y, Position.Z, PressureDelta);
}

void UAtmosphericSystem::ResetAtmosphere()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting Atmospheric System..."));

    // Clear simulation data
    AtmosphericGrid.Empty();
    ActiveWeatherPatterns.Empty();

    // Reset timers and layer trackers
    AccumulatedTime = 0.0f;
    CurrentUpdateLayer = 0;

    // Clear cloud mesh components if any
    for (UStaticMeshComponent* CloudMesh : CloudMeshes)
    {
        if (CloudMesh)
        {
            CloudMesh->DestroyComponent();
        }
    }
    CloudMeshes.Empty();

    // Reinitialize system
    if (TerrainSystem)
    {
        Initialize(TerrainSystem, WaterSystem); // Fully re-initialize from terrain
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ResetAtmosphere: TerrainSystem not set, skipping initialization."));
    }

    UE_LOG(LogTemp, Warning, TEXT("Atmospheric System reset complete."));
}

// ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====

void UAtmosphericSystem::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Atmospheric system configuring from master controller"));
    
    // Store configuration
    CurrentWorldConfig = Config;
    
    // Calculate atmospheric grid dimensions based on terrain
    int32 NewGridWidth = FMath::Max(32, Config.AtmosphericConfig.GridWidth);
    int32 NewGridHeight = FMath::Max(32, Config.AtmosphericConfig.GridHeight);
    int32 NewGridLayers = FMath::Max(8, Config.AtmosphericConfig.GridLayers);
    
    // Check if we need to resize atmospheric grid
    bool bNeedsResize = (GridWidth != NewGridWidth || GridHeight != NewGridHeight || GridLayers != NewGridLayers);
    
    if (bNeedsResize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Resizing atmospheric grid: %dx%dx%d -> %dx%dx%d"),
               GridWidth, GridHeight, GridLayers, NewGridWidth, NewGridHeight, NewGridLayers);
        
        // Update grid dimensions
        GridWidth = NewGridWidth;
        GridHeight = NewGridHeight;
        GridLayers = NewGridLayers;
        
        // Update cell size to match terrain coverage
        float TerrainWorldWidth = Config.TerrainWidth * Config.TerrainScale;
        float TerrainWorldHeight = Config.TerrainHeight * Config.TerrainScale;
        CellSize = FMath::Max(TerrainWorldWidth / GridWidth, TerrainWorldHeight / GridHeight);
        
        // Reinitialize atmospheric grid with new dimensions
        InitializeAtmosphericGrid();
        
        UE_LOG(LogTemp, Log, TEXT("[MASTER INTEGRATION] Atmospheric grid resized - CellSize: %.1fm"), CellSize);
    }
    
    // Mark as scaled by master
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Atmospheric system configuration complete - Grid: %dx%dx%d, CellSize: %.1fm"),
           GridWidth, GridHeight, GridLayers, CellSize);
}

void UAtmosphericSystem::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[MASTER INTEGRATION] Atmospheric system synchronizing coordinates"));
    
    // Store coordinate system
    CurrentCoordinateSystem = Coords;
    
    // Update coordinate transformation parameters
    if (bIsScaledByMaster)
    {
        // Master coordinates override local terrain coordinates
        UE_LOG(LogTemp, VeryVerbose, TEXT("[MASTER INTEGRATION] Using master coordinate system for atmosphere"));
    }
}

bool UAtmosphericSystem::IsSystemScaled() const
{
    return bIsScaledByMaster && MasterController != nullptr;
}

void UAtmosphericSystem::RegisterWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("[MASTER INTEGRATION] Cannot register with null master controller"));
        return;
    }
    
    MasterController = Master;
    
    UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Atmospheric system registered with master controller"));
}

FString UAtmosphericSystem::GetScalingDebugInfo() const
{
    if (!IsSystemScaled())
    {
        return TEXT("Atmospheric system: NOT SCALED (using local coordinates)");
    }
    
    return FString::Printf(TEXT("Atmospheric system: SCALED by master - Grid: %dx%dx%d, CellSize: %.1fm, Layers: %d"),
                          GridWidth, GridHeight, GridLayers, CellSize, GridLayers);
}

// ===== MASTER COORDINATE TRANSFORMATION HELPERS =====

FVector UAtmosphericSystem::WorldToAtmosphericGrid(FVector WorldPos) const
{
    if (!IsSystemScaled())
    {
        // Fall back to local atmospheric coordinates
        return WorldToGridCoordinates(WorldPos);
    }
    
    // Use master controller coordinate system
    FVector LocalPos = WorldPos - CurrentCoordinateSystem.WorldOrigin;
    return FVector(
        LocalPos.X / CellSize,
        LocalPos.Y / CellSize,
        LocalPos.Z / LayerHeight
    );
}

FVector UAtmosphericSystem::AtmosphericGridToWorld(int32 X, int32 Y, int32 Z) const
{
    if (!IsSystemScaled())
    {
        // Fall back to local atmospheric coordinates
        return GridToWorldCoordinates(X, Y, Z);
    }
    
    // Use master controller coordinate system
    FVector WorldPos = CurrentCoordinateSystem.WorldOrigin;
    WorldPos.X += X * CellSize;
    WorldPos.Y += Y * CellSize;
    WorldPos.Z += Z * LayerHeight;
    return WorldPos;
}

bool UAtmosphericSystem::IsMasterCoordinateValid(int32 X, int32 Y, int32 Z) const
{
    if (!IsSystemScaled())
    {
        return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight && Z >= 0 && Z < GridLayers;
    }
    
    return X >= 0 && X < CurrentWorldConfig.AtmosphericConfig.GridWidth &&
           Y >= 0 && Y < CurrentWorldConfig.AtmosphericConfig.GridHeight &&
           Z >= 0 && Z < CurrentWorldConfig.AtmosphericConfig.GridLayers;
}

// CRITICAL FIX: Add coordinate transform function
FVector2D UAtmosphericSystem::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("[ATMOSPHERIC] Cannot transform coordinates - no MasterController"));
        return FVector2D::ZeroVector;
    }
    
    return MasterController->WorldToTerrainCoordinates(WorldPosition);
}


