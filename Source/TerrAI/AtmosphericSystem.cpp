// AtmosphericSystem.cpp - Scientific Atmospheric Physics Implementation
#include "AtmosphericSystem.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "GeologyController.h"
#include "EcosystemController.h"
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
    CellSize = 1000.0f;
  
    
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

// In AtmosphericSystem.cpp

void UAtmosphericSystem::InitializeAtmosphericGrid()
{
    if (!TerrainSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphericSystem: Cannot initialize grid without terrain"));
        return;
    }
    
    // CRITICAL CHANGE: 2D grid with single layer instead of 3D
    int32 TotalCells = GridWidth * GridHeight;
    AtmosphericGrid.SetNum(TotalCells);
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initializing SIMPLIFIED 2D grid %dx%d"),
           GridWidth, GridHeight);
    
    // Initialize with prevailing wind pattern
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[Index];
            
            // Simple initialization
            Cell.Temperature = BaseTemperature;
            Cell.Humidity = 0.5f;
            Cell.MoistureMass = 0.2f;      // kg/m² starting moisture WAS 1.5f too much than .1 nothing at all
            Cell.CloudCover = 0.0f;
            Cell.PrecipitationRate = 0.0f;
            
            // Initialize with prevailing wind pattern
            Cell.WindVector = WindPattern;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Initialized %d cells with simplified atmosphere"),
           TotalCells);
}

// Simplified grid access (2D only)
int32 UAtmosphericSystem::GetGridIndex(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight)
    {
        return 0;
    }
    return Y * GridWidth + X;
}

FSimplifiedAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y)
{
    return AtmosphericGrid[GetGridIndex(X, Y)];
}

const FSimplifiedAtmosphericCell& UAtmosphericSystem::GetCell(int32 X, int32 Y) const
{
    return AtmosphericGrid[GetGridIndex(X, Y)];
}

// ===== CORE SIMULATION =====

void UAtmosphericSystem::UpdateAtmosphericSimulation(float DeltaTime)
{
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("🌧️ UpdateAtmosphericSimulation CALLED with DT=%.4f"), DeltaTime);
      
      if (!TerrainSystem || AtmosphericGrid.Num() == 0)
      {
          UE_LOG(LogTemp, Error, TEXT("🌧️ UpdateAtmosphericSimulation FAILED - no terrain or grid"));
          return;
      }
      
     // UE_LOG(LogTemp, Warning, TEXT("🌧️ Processing atmospheric simulation steps..."));
      
    if (!TerrainSystem || AtmosphericGrid.Num() == 0)
    {
        return;
    }
    
    // LOGICAL ORDER: Transport → Condensation → Terrain Effects → Water Transfer
    
    // 1. Transport moisture with wind
    AdvectMoisture(DeltaTime);
    
    // 2. MAIN PRECIPITATION PROCESS - where rain actually forms
    ProcessCondensationAndPrecipitation(DeltaTime);
    
    // 3. Terrain effects modify existing precipitation patterns
    ApplyOrographicEffects(DeltaTime);
    
    // 4. Transfer precipitation to water system
    //UE_LOG(LogTemp, Warning, TEXT("🌧️ Calling ProcessPrecipitation..."));
    ProcessPrecipitation(DeltaTime);
    
    // 5. Process evaporation (adds moisture back) Evaporation managed in water system and evapotrans is too complex
    //ProcessEvaporation(DeltaTime);
    //ProcessEvapotranspiration(DeltaTime);
    
    // 6. Process springs (groundwater → surface water) managed in geology controller
    //ProcessGroundwaterDischarge(DeltaTime);
    
    // 7. Update visual states
    UpdateCloudCoverFromMoisture();
}

// ===== MAIN PRECIPITATION PROCESS =====
void UAtmosphericSystem::ProcessCondensationAndPrecipitation(float DeltaTime)
{
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[GetGridIndex(X, Y)];
            
            // Calculate saturation point for this temperature
            float SaturationMoisture = CalculateSaturationMoisture(Cell.Temperature);
            
            // CONDENSATION: If moisture exceeds saturation, form precipitation
            if (Cell.MoistureMass > SaturationMoisture)
            {
                float ExcessMoisture = Cell.MoistureMass - SaturationMoisture;
                
                // Condensation rate - how fast excess moisture becomes rain
                float CondensationRate = 0.8f; // 80% of excess converts per frame
                float CondensedMoisture = ExcessMoisture * CondensationRate * DeltaTime;
                
                // Convert condensed moisture to precipitation rate using Water Volume Authority
                float DepthPerSec = AMasterWorldController::MoistureMassToDepth(CondensedMoisture);
                float NewPrecipitation = AMasterWorldController::MetersPerSecondToPrecipitationRate(DepthPerSec);
                
                // ✅ NOW apply rate limiting (after initialization)
                NewPrecipitation = FMath::Min(NewPrecipitation, 75.0f); // Cap at heavy rain
                
                // Update precipitation rate (SET, don't accumulate)
                Cell.PrecipitationRate = NewPrecipitation;
                
                // Remove condensed moisture from atmosphere
                Cell.MoistureMass -= CondensedMoisture;
                
                // Update cloud cover based on precipitation intensity
                Cell.CloudCover = FMath::Clamp(Cell.PrecipitationRate / 20.0f, 0.0f, 1.0f);
                
                // Log significant precipitation events
                if (Cell.PrecipitationRate > 5.0f)
                {
                    static int32 LoggedCells = 0;
                    if (LoggedCells < 5) // Limit spam
                    {
                        UE_LOG(LogTemp, Warning, TEXT("🌧️ CONDENSATION RAIN: %.1f mm/hr at grid (%d,%d)"),
                               Cell.PrecipitationRate, X, Y);
                        LoggedCells++;
                    }
                }
            }
            
            // EVAPORATION: Precipitation naturally decreases over time
            if (Cell.PrecipitationRate > 0.0f)
            {
                float EvaporationRate = 0.001f; // .1% decay per second lowered from 5 than lowered from 1
                Cell.PrecipitationRate *= (1.0f - EvaporationRate * DeltaTime);
                
                // Threshold - stop very light rain
                if (Cell.PrecipitationRate < 0.01f)
                {
                    Cell.PrecipitationRate = 0.0f;
                }
            }
        }
    }
}

// ===== WEATHER PATTERNS =====

void UAtmosphericSystem::CreateWeatherEffect(FVector2D Location, float Radius, float Intensity)
{
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: Creating weather effect - Intensity %.2f at %s"),
           Intensity, *Location.ToString());
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            FVector2D CellPos(X * CellSize, Y * CellSize);
            float Distance = FVector2D::Distance(CellPos, Location);
            
            if (Distance < Radius)
            {
                FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[GetGridIndex(X, Y)];
                
                float Influence = 1.0f - (Distance / Radius);
                
                // ADD MOISTURE (which will condense into precipitation naturally)
                float MoistureBoost = Intensity * Influence * 1.0f; // Significant moisture injection WITH 15.0f
                Cell.MoistureMass += MoistureBoost;
                
                // Boost temperature slightly (storm energy)
                Cell.Temperature += Influence * 2.0f;
                
                // Create convergent wind pattern (storms have inward flow)
                FVector2D ToCenter = (Location - CellPos).GetSafeNormal();
                Cell.WindVector += ToCenter * Influence * Intensity * 3.0f;
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Added %.1f moisture to cell (%d,%d)"),
                       MoistureBoost, X, Y);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✅ Weather effect created - moisture added, precipitation will form naturally"));
}

// ===== SYSTEM INTEGRATION =====
/*
void UAtmosphericSystem::UpdateWaterSystemInterface()
{
    if (!WaterSystem) return;
    
    // Atmosphere's only job: rain becomes surface water
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    
    for (const auto& Cell : AtmosphericGrid)
    {
        if (Cell.PrecipitationRate > 0.01f) // Raining?
        {
            // mm/hr → m/s → simulation units
            float WaterToAdd = Cell.PrecipitationRate * 2.78e-7f / AMasterWorldController::WATER_DEPTH_SCALE * DeltaTime;
            WaterSystem->AddWater(Cell.WorldPosition, WaterToAdd);
        }
    }
}
*/
void UAtmosphericSystem::LogPrecipitationActivity()
{
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Only log every 5 seconds for performance
    if (CurrentTime - LastLogTime < 5.0f) return;
    
    LastLogTime = CurrentTime;
    
    // Count active precipitation cells
    int32 ActiveCells = 0;
    float TotalPrecipitation = 0.0f;
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        if (Cell.PrecipitationRate > 0.1f)
        {
            ActiveCells++;
            TotalPrecipitation += Cell.PrecipitationRate;
        }
    }
    
    if (ActiveCells > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[ATMOSPHERIC] Active precipitation: %d cells, Total: %.1f mm/hr"),
               ActiveCells, TotalPrecipitation);
    }
}

// ===== QUERY INTERFACE =====

float UAtmosphericSystem::GetTemperatureAt(FVector WorldPosition) const
{
    FVector2D GridPos = WorldToGridCoordinates(WorldPosition);
    
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    
    if (X >= 0 && X < GridWidth - 1 && Y >= 0 && Y < GridHeight - 1)
    {
        // Simple bilinear interpolation (no Z component!)
        float FracX = GridPos.X - X;
        float FracY = GridPos.Y - Y;
        
        float T00 = AtmosphericGrid[GetGridIndex(X, Y)].Temperature;
        float T10 = AtmosphericGrid[GetGridIndex(X + 1, Y)].Temperature;
        float T01 = AtmosphericGrid[GetGridIndex(X, Y + 1)].Temperature;
        float T11 = AtmosphericGrid[GetGridIndex(X + 1, Y + 1)].Temperature;
        
        float T0 = FMath::Lerp(T00, T10, FracX);
        float T1 = FMath::Lerp(T01, T11, FracX);
        
        return FMath::Lerp(T0, T1, FracY);
    }
    
    return BaseTemperature;
}

float UAtmosphericSystem::GetHumidityAt(FVector WorldPosition) const
{
    // Similar simplified interpolation
    FVector2D GridPos = WorldToGridCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return AtmosphericGrid[GetGridIndex(X, Y)].Humidity;
    }
    
    return 0.5f;
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
    
    // Update grid dimensions (2D only now!)
    int32 NewGridWidth = Config.AtmosphericConfig.GridWidth;
    int32 NewGridHeight = Config.AtmosphericConfig.GridHeight;
    // NO GridLayers!
    
    UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Resizing atmospheric grid: %dx%d -> %dx%d"),
           GridWidth, GridHeight, NewGridWidth, NewGridHeight);
    
    // Update grid dimensions
    GridWidth = NewGridWidth;
    GridHeight = NewGridHeight;
    CellSize = Config.AtmosphericConfig.CellSize;
    
    // Reinitialize atmospheric grid with new dimensions
    InitializeAtmosphericGrid();
    
    // Mark as scaled by master
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[MASTER INTEGRATION] Atmospheric system configuration complete - Grid: %dx%d, CellSize: %.1fm"),
           GridWidth, GridHeight, CellSize);
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


FVector2D UAtmosphericSystem::WorldToGridCoordinates(FVector WorldPosition) const
{
    return FVector2D(
        WorldPosition.X / CellSize,
        WorldPosition.Y / CellSize
    );
}

float UAtmosphericSystem::GetPressureAt(FVector WorldPosition) const
{
    // Simplified - just return standard pressure
    return 101325.0f;
}

float UAtmosphericSystem::GetCloudCoverAt(FVector WorldPosition) const
{
    FVector2D GridPos = WorldToGridCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return AtmosphericGrid[GetGridIndex(X, Y)].CloudCover;
    }
    
    return 0.0f;
}

float UAtmosphericSystem::GetPrecipitationAt(FVector WorldPosition) const
{
    FVector2D GridPos = WorldToGridCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return AtmosphericGrid[GetGridIndex(X, Y)].PrecipitationRate;
    }
    
    return 0.0f;
}

FVector UAtmosphericSystem::GetWindAt(FVector WorldPosition) const
{
    FVector2D GridPos = WorldToGridCoordinates(WorldPosition);
    int32 X = FMath::FloorToInt(GridPos.X);
    int32 Y = FMath::FloorToInt(GridPos.Y);
    
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        FVector2D Wind2D = AtmosphericGrid[GetGridIndex(X, Y)].WindVector;
        return FVector(Wind2D.X, Wind2D.Y, 0.0f);
    }
    
    return FVector(WindPattern.X, WindPattern.Y, 0.0f);
}

FString UAtmosphericSystem::GetScalingDebugInfo() const
{
    FString DebugInfo;
    DebugInfo += FString::Printf(TEXT("=== ATMOSPHERIC SYSTEM SCALING DEBUG ===\n"));
    DebugInfo += FString::Printf(TEXT("Grid: %dx%d\n"), GridWidth, GridHeight);
    DebugInfo += FString::Printf(TEXT("Cell Size: %.1fm\n"), CellSize);
    DebugInfo += FString::Printf(TEXT("Is Scaled by Master: %s\n"), bIsScaledByMaster ? TEXT("YES") : TEXT("NO"));
    return DebugInfo;
}

float UAtmosphericSystem::GetCloudCoverageAt(int32 X, int32 Y) const
{
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return AtmosphericGrid[GetGridIndex(X, Y)].CloudCover;
    }
    return 0.0f;
}

FVector UAtmosphericSystem::GridToWorldCoordinates(int32 X, int32 Y) const
{
    // ✅ Use SAME coordinate authority as everyone else
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("No coordinate authority"));
        return FVector::ZeroVector;
    }
    
    // Convert atmospheric grid to terrain grid coordinates
    FVector2D WorldDims = MasterController->GetWorldDimensions();
    float TerrainX = (float(X) / GridWidth) * WorldDims.X;
    float TerrainY = (float(Y) / GridHeight) * WorldDims.Y;
    
    // Use MasterController's authoritative coordinate transformation
    return MasterController->TerrainToWorldPosition(FVector2D(TerrainX, TerrainY));
}

void UAtmosphericSystem::AdvectMoisture(float DeltaTime)
{
    // CRITICAL: Use temporary array to avoid feedback
    TArray<float> NewMoisture;
    NewMoisture.SetNum(AtmosphericGrid.Num());
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            FVector2D Wind = AtmosphericGrid[Index].WindVector;
            
            // Semi-Lagrangian advection (stable for game timesteps)
            FVector2D SourcePos = FVector2D(X, Y) - Wind * DeltaTime / CellSize;
            
            // Bilinear interpolation from source
            int32 X0 = FMath::FloorToInt(SourcePos.X);
            int32 Y0 = FMath::FloorToInt(SourcePos.Y);
            int32 X1 = X0 + 1;
            int32 Y1 = Y0 + 1;
            
            // Clamp to grid bounds
            X0 = FMath::Clamp(X0, 0, GridWidth - 1);
            X1 = FMath::Clamp(X1, 0, GridWidth - 1);
            Y0 = FMath::Clamp(Y0, 0, GridHeight - 1);
            Y1 = FMath::Clamp(Y1, 0, GridHeight - 1);
            
            // Interpolation weights
            float FracX = SourcePos.X - X0;
            float FracY = SourcePos.Y - Y0;
            
            // Sample moisture at four corners
            float M00 = AtmosphericGrid[Y0 * GridWidth + X0].MoistureMass;
            float M10 = AtmosphericGrid[Y0 * GridWidth + X1].MoistureMass;
            float M01 = AtmosphericGrid[Y1 * GridWidth + X0].MoistureMass;
            float M11 = AtmosphericGrid[Y1 * GridWidth + X1].MoistureMass;
            
            // Bilinear interpolation
            float M0 = FMath::Lerp(M00, M10, FracX);
            float M1 = FMath::Lerp(M01, M11, FracX);
            NewMoisture[Index] = FMath::Lerp(M0, M1, FracY);
        }
    }
    
    // Apply advected moisture
    for (int32 i = 0; i < AtmosphericGrid.Num(); i++)
        {
            // ✅ Validate new moisture value
            if (FMath::IsFinite(NewMoisture[i]) && NewMoisture[i] >= 0.0f)
            {
                AtmosphericGrid[i].MoistureMass = NewMoisture[i];
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid moisture value at cell %d: %.6f - resetting"), i, NewMoisture[i]);
                AtmosphericGrid[i].MoistureMass = 0.1f; // Reset to safe value
            }
            
            // ✅ Update relative humidity with safe division
            float SaturationCapacity = CalculateSaturationMoisture(AtmosphericGrid[i].Temperature);
            if (SaturationCapacity > 0.001f) // Prevent division by zero
            {
                AtmosphericGrid[i].Humidity = AtmosphericGrid[i].MoistureMass / SaturationCapacity;
            }
            else
            {
                AtmosphericGrid[i].Humidity = 0.5f; // Safe default
                UE_LOG(LogTemp, Warning, TEXT("Zero saturation capacity at cell %d"), i);
            }
            
            // ✅ Clamp humidity to valid range
            AtmosphericGrid[i].Humidity = FMath::Clamp(AtmosphericGrid[i].Humidity, 0.0f, 1.0f);
        }
}

// Helper function for saturation calculations with safety checks
float UAtmosphericSystem::CalculateSaturationMoisture(float TempKelvin)
{
    // ✅ Safety check for valid temperature range
    if (!FMath::IsFinite(TempKelvin) || TempKelvin < 200.0f || TempKelvin > 350.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid temperature in saturation calculation: %.2f K"), TempKelvin);
        return 0.17f; // Return reasonable default (15°C saturation)
    }
    
    // Simplified Magnus formula
    float TempC = TempKelvin - 273.15f;
    
    // ✅ Safety check for division by zero
    float Denominator = TempC + 237.3f;
    if (FMath::Abs(Denominator) < 0.001f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Near-zero denominator in Magnus formula"));
        return 0.17f; // Safe fallback
    }
    
    float SaturationPressure = 6.11f * FMath::Exp(17.27f * TempC / Denominator);
    
    // ✅ Safety check for result
    float Result = SaturationPressure * 0.05f; //increased from .01 for headroom
    if (!FMath::IsFinite(Result) || Result <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid saturation result: %.6f"), Result);
        return 0.17f; // Safe fallback
    }
    
    return Result;
}

void UAtmosphericSystem::UpdateCloudCoverFromMoisture()
{
    for (int32 i = 0; i < AtmosphericGrid.Num(); i++)
    {
        FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[i];
        
        // ✅ Validate temperature first
        if (!FMath::IsFinite(Cell.Temperature) || Cell.Temperature < 200.0f || Cell.Temperature > 350.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid temperature in cell %d: %.2f - resetting"), i, Cell.Temperature);
            Cell.Temperature = 288.15f; // Reset to 15°C
        }
        
        // ✅ Validate moisture mass
        if (!FMath::IsFinite(Cell.MoistureMass) || Cell.MoistureMass < 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid moisture in cell %d: %.6f - resetting"), i, Cell.MoistureMass);
            Cell.MoistureMass = 0.1f; // Reset to safe value
        }
        
        // Simplified cloud formation based on humidity
        float SaturationCapacity = CalculateSaturationMoisture(Cell.Temperature);
        
        // ✅ Safe humidity calculation with division protection
        if (SaturationCapacity > 0.001f)
        {
            Cell.Humidity = FMath::Clamp(Cell.MoistureMass / SaturationCapacity, 0.0f, 1.0f);
        }
        else
        {
            Cell.Humidity = 0.5f; // Safe default
        }
        
        // ✅ Validate humidity result
        if (!FMath::IsFinite(Cell.Humidity))
        {
            Cell.Humidity = 0.5f;
            UE_LOG(LogTemp, Warning, TEXT("NaN humidity in cell %d - reset to 0.5"), i);
        }
        
        // Cloud cover update with frame time protection
        float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        DeltaTime = FMath::Clamp(DeltaTime, 0.001f, 0.1f); // Prevent extreme values
        
        if (Cell.Humidity > 0.8f)
        {
            Cell.CloudCover = FMath::Min(Cell.CloudCover + 0.1f * DeltaTime, 1.0f);
        }
        else
        {
            Cell.CloudCover = FMath::Max(Cell.CloudCover - 0.05f * DeltaTime, 0.0f);
        }
        
        // ✅ Final validation
        Cell.CloudCover = FMath::Clamp(Cell.CloudCover, 0.0f, 1.0f);
    }
}

void UAtmosphericSystem::ApplyOrographicEffects(float DeltaTime)
{
    // Orographic effects MODIFY precipitation based on terrain
    // They don't CREATE precipitation - that happens in ProcessCondensationAndPrecipitation
    
    if (!TerrainSystem) return;
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[GetGridIndex(X, Y)];
            
            FVector WorldPos = GridToWorldCoordinates(X, Y);
            float TerrainHeight = TerrainSystem->GetHeightAtPosition(WorldPos);
            
            // Only apply orographic effects if there's already some weather activity
            if (Cell.MoistureMass > 5.0f || Cell.PrecipitationRate > 0.1f)
            {
                FVector2D TerrainGradient = GetTerrainGradient(X, Y);
                
                // WINDWARD SLOPE EFFECT: Wind blowing upslope increases precipitation
                float UpslopeEffect = FVector2D::DotProduct(Cell.WindVector.GetSafeNormal(), TerrainGradient);
                
                if (UpslopeEffect > 0.0f && TerrainHeight > 200.0f) // Significant elevation
                {
                    // Increase moisture on windward slopes (feeds future precipitation)
                    Cell.MoistureMass += UpslopeEffect * DeltaTime * 3.0f;
                    
                    // Enhance existing precipitation
                    if (Cell.PrecipitationRate > 0.1f)
                    {
                        float OrographicMultiplier = 1.0f + (UpslopeEffect * 0.5f);
                        Cell.PrecipitationRate *= OrographicMultiplier;
                    }
                }
                
                // LEEWARD SLOPE EFFECT: Reduce precipitation on downwind side
                if (UpslopeEffect < -0.1f)
                {
                    Cell.PrecipitationRate *= 0.7f; // Rain shadow effect
                }
                
                // ALTITUDE EFFECT: Higher altitude = more precipitation potential
                if (TerrainHeight > 500.0f)
                {
                    float AltitudeBonus = (TerrainHeight - 500.0f) / 1000.0f; // 0.0 to 1.0+
                    Cell.MoistureMass += AltitudeBonus * DeltaTime * 1.0f;
                }
            }
        }
    }
}

// EFFICIENT terrain gradient calculation
FVector2D UAtmosphericSystem::GetTerrainGradient(int32 GridX, int32 GridY)
{
    FVector WorldPos = GridToWorldCoordinates(GridX, GridY);
    
    // Sample terrain at neighboring points
    float H0 = TerrainSystem->GetHeightAtPosition(WorldPos);
    float Hx = TerrainSystem->GetHeightAtPosition(WorldPos + FVector(CellSize, 0, 0));
    float Hy = TerrainSystem->GetHeightAtPosition(WorldPos + FVector(0, CellSize, 0));
    
    // Return gradient
    return FVector2D((Hx - H0) / CellSize, (Hy - H0) / CellSize);
}

void UAtmosphericSystem::ClearConsumedPrecipitation()
{
    for (int32 i = 0; i < AtmosphericGrid.Num(); i++)
    {
        AtmosphericGrid[i].PrecipitationRate = 0.0f;
    }
}

/*
void UAtmosphericSystem::ProcessEvaporation(float DeltaTime)
{
    if (!WaterSystem) return;
    
    // PERFORMANCE: Can be updated less frequently than main simulation
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[Index];
            FVector WorldPos = GridToWorldCoordinates(X, Y);
            
            // Check for water at this location
            float WaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldPos);
            
            if (WaterDepth > 0.01f) // Has water
            {
                // Penman equation simplified
                float TempC = Cell.Temperature - 273.15f;
                
                // TUNING: These coefficients control evaporation rate
                float BaseEvapRate = 0.001f; // m/s at 20°C, calm wind
                float TempFactor = FMath::Max(0.0f, 1.0f + (TempC - 20.0f) * 0.05f);
                float WindFactor = 1.0f + Cell.WindVector.Size() * 0.1f;
                float HumidityFactor = FMath::Max(0.0f, 1.0f - Cell.Humidity);
                
                float EvaporationRate = BaseEvapRate * TempFactor * WindFactor * HumidityFactor;
                float EvaporatedDepth = EvaporationRate * DeltaTime;
                
                // Convert to moisture mass using Water Volume Authority
                float EvaporatedMass = AMasterWorldController::DepthToMoistureMass(EvaporatedDepth);
                
                // Add to atmosphere
                Cell.MoistureMass += EvaporatedMass;
                
                // Water removal handled by water system's evaporation processing
                // The water system will handle the actual depth reduction
                
                // LOGGING for water balance debugging
                static float TotalEvaporated = 0.0f;
                static float LastLogTime = 0.0f;
                float CurrentTime = TerrainSystem->GetWorld()->GetTimeSeconds();

                if (CurrentTime - LastLogTime >= 5.0f) {
                    TotalEvaporated = 0.0f;  // ✅ RESET ACCUMULATOR
                    LastLogTime = CurrentTime;
                }
                TotalEvaporated += EvaporatedMass * CellSize * CellSize;
                if (FMath::Fmod(TerrainSystem->GetWorld()->GetTimeSeconds(), 5.0f) < DeltaTime)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Evaporation at (%d,%d): %.3f m³/s"), X, Y, TotalEvaporated);
                }
            }
        }
    }
}

void UAtmosphericSystem::ProcessEvapotranspiration(float DeltaTime)
{
    // Get geology controller from master controller's public member
    if (!TerrainSystem || !TerrainSystem->CachedMasterController) return;
    
    AMasterWorldController* Master = TerrainSystem->CachedMasterController;
    if (!Master->GeologyController) return;
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[Index];
            FVector WorldPos = GridToWorldCoordinates(X, Y);
            
            // Get soil moisture from geology's water table system
            float WaterTableDepth = Master->GeologyController->GetWaterTableDepthAtLocation(WorldPos);
            float TerrainHeight = TerrainSystem->GetHeightAtPosition(WorldPos);
            
            // Calculate soil moisture based on water table proximity
            float SoilMoisture = 0.0f;
            if (WaterTableDepth < 10.0f) // Water table within 10m of surface
            {
                SoilMoisture = 1.0f - (WaterTableDepth / 10.0f);
            }
            
            if (SoilMoisture > 0.1f) // Moist soil
            {
                // Transpiration rate (simplified)
                float TranspRate = 0.00001f * SoilMoisture * (1.0f - Cell.Humidity);
                
                // Vegetation factor (check if ecosystem controller available)
                if (Master->EcosystemController)
                {
                    float VegDensity = Master->EcosystemController->GetVegetationDensityAtLocation(WorldPos);
                    TranspRate *= (1.0f + VegDensity * 2.0f); // Plants increase transpiration
                }
                
                float TranspiredMass = TranspRate * CellSize * CellSize * DeltaTime;
                
                // Update atmosphere
                Cell.MoistureMass += TranspiredMass;
            }
        }
    }
}
*/
 
void UAtmosphericSystem::ProcessPrecipitation(float DeltaTime)
{
    AMasterWorldController* Master = TerrainSystem->CachedMasterController;
    if (!Master) return;

    for (int32 i = 0; i < AtmosphericGrid.Num(); i++)
    {
        const FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[i];
        
        if (Cell.PrecipitationRate > 0.01f)
        {
            int32 X = i % GridWidth;
            int32 Y = i / GridWidth;
            FVector2D AtmosGridPos(X, Y);
            FVector2D WaterGridPos;
            
            // Route through master controller
            float ActualWaterAdded = Master->TransferPrecipitationToSurface(
                Cell.PrecipitationRate,
                DeltaTime,
                AtmosGridPos,
                WaterGridPos
            );
            
            // Reduce atmospheric moisture by amount transferred
            AtmosphericGrid[i].MoistureMass -= ActualWaterAdded;
        }
    }
}


void UAtmosphericSystem::UpdateSimplifiedWaterTable(float DeltaTime)
{
    // Simplified water table update - geology controller handles complex flow
    // This just ensures atmospheric system knows about groundwater for evapotranspiration
}

// Add these constants
const int32 UAtmosphericSystem::GeologyGridWidth = 16;  // Lower resolution than atmosphere
const int32 UAtmosphericSystem::GeologyGridHeight = 16;
const float UAtmosphericSystem::SpringFlowRate = 0.01f; // m³/s per meter of head

void UAtmosphericSystem::DebugAtmosphericState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ATMOSPHERIC SYSTEM DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Grid: %dx%d, Cell Size: %.0f"), GridWidth, GridHeight, CellSize);
    UE_LOG(LogTemp, Warning, TEXT("Total Cells: %d"), AtmosphericGrid.Num());
    
    // Calculate statistics
    float TotalMoisture = 0.0f;
    float TotalCloudCover = 0.0f;
    float MaxMoisture = 0.0f;
    float MaxCloudCover = 0.0f;
    int32 CloudyCells = 0;
    int32 PrecipitatingCells = 0;
    
    for (const FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        TotalMoisture += Cell.MoistureMass;
        TotalCloudCover += Cell.CloudCover;
        MaxMoisture = FMath::Max(MaxMoisture, Cell.MoistureMass);
        MaxCloudCover = FMath::Max(MaxCloudCover, Cell.CloudCover);
        
        if (Cell.CloudCover > 0.1f) CloudyCells++;
        if (Cell.PrecipitationRate > 0.1f) PrecipitatingCells++;
    }
    
    float AvgMoisture = TotalMoisture / AtmosphericGrid.Num();
    float AvgCloudCover = TotalCloudCover / AtmosphericGrid.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Moisture - Avg: %.2f kg/m², Max: %.2f kg/m²"), AvgMoisture, MaxMoisture);
    UE_LOG(LogTemp, Warning, TEXT("Cloud Cover - Avg: %.2f%%, Max: %.2f%%"), AvgCloudCover * 100, MaxCloudCover * 100);
    UE_LOG(LogTemp, Warning, TEXT("Cloudy Cells: %d (%.1f%%)"), CloudyCells, (float)CloudyCells / AtmosphericGrid.Num() * 100);
    UE_LOG(LogTemp, Warning, TEXT("Precipitating Cells: %d"), PrecipitatingCells);
    UE_LOG(LogTemp, Warning, TEXT("Cloud Rendering Enabled: %s"), bEnableCloudRendering ? TEXT("YES") : TEXT("NO"));
}

void UAtmosphericSystem::DebugCloudSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CLOUD RENDERING DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cloud Meshes: %d / %d"), CloudMeshes.Num(), MaxCloudMeshes);
    UE_LOG(LogTemp, Warning, TEXT("Cloud Material: %s"), CloudMaterial ? TEXT("SET") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("Cloud Static Mesh: %s"), CloudStaticMesh ? TEXT("SET") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("Attach Parent: %s"), AttachParent ? TEXT("SET") : TEXT("MISSING"));
    
    int32 VisibleClouds = 0;
    for (UStaticMeshComponent* CloudMesh : CloudMeshes)
    {
        if (CloudMesh && CloudMesh->IsVisible())
        {
            VisibleClouds++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Visible Cloud Meshes: %d"), VisibleClouds);
    
    // Check why clouds might not be showing
    if (!CloudStaticMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CLOUD ERROR: No static mesh assigned! Set CloudStaticMesh in Blueprint"));
    }
    if (!CloudMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("CLOUD ERROR: No material assigned! Set CloudMaterial in Blueprint"));
    }
    if (!AttachParent)
    {
        UE_LOG(LogTemp, Error, TEXT("CLOUD ERROR: No attach parent! Clouds have nowhere to spawn"));
    }
}

void UAtmosphericSystem::ForceGenerateClouds(float Coverage)
{
    UE_LOG(LogTemp, Warning, TEXT("Force generating clouds with %.0f%% coverage"), Coverage * 100);
    
    // Set cloud cover for all cells
    for (FSimplifiedAtmosphericCell& Cell : AtmosphericGrid)
    {
        Cell.CloudCover = Coverage;
        Cell.Humidity = 0.9f;
        Cell.MoistureMass = 20.0f; // High moisture
    }
    
    // Force cloud mesh update
    if (bEnableCloudRendering)
    {
        UpdateCloudMeshes();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cloud rendering is disabled! Enable bEnableCloudRendering"));
    }
}

void UAtmosphericSystem::DebugMoistureDistribution()
{
    if (!GetWorld()) return;
    
    // Visualize moisture distribution
    for (int32 Y = 0; Y < GridHeight; Y += 4) // Sample every 4 cells
    {
        for (int32 X = 0; X < GridWidth; X += 4)
        {
            int32 Index = GetGridIndex(X, Y);
            const FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[Index];
            FVector WorldPos = GridToWorldCoordinates(X, Y);
            
            // Draw moisture as blue spheres
            float SphereRadius = FMath::Lerp(10.0f, 100.0f, Cell.MoistureMass / 30.0f);
            FColor MoistureColor = FColor::MakeRedToGreenColorFromScalar(1.0f - Cell.Humidity);
            DrawDebugSphere(GetWorld(), WorldPos + FVector(0, 0, 500), SphereRadius, 12, MoistureColor, false, 5.0f);
            
            // Draw cloud cover as white boxes
            if (Cell.CloudCover > 0.1f)
            {
                float BoxSize = Cell.CloudCover * 200.0f;
                DrawDebugBox(GetWorld(), WorldPos + FVector(0, 0, CloudAltitude),
                    FVector(BoxSize), FColor::White, false, 5.0f);
            }
            
            // Show precipitation as lines
            if (Cell.PrecipitationRate > 0.1f)
            {
                DrawDebugLine(GetWorld(),
                    WorldPos + FVector(0, 0, CloudAltitude),
                    WorldPos,
                    FColor::Cyan, false, 5.0f, 0, 5.0f);
            }
        }
    }
}

void UAtmosphericSystem::EnableAtmosphericDebugVisualization(bool bEnable)
{
    // This could be expanded to show continuous visualization
    if (bEnable)
    {
        DebugMoistureDistribution();
        DebugAtmosphericState();
        DebugCloudSystem();
    }
}

void UAtmosphericSystem::TriggerImmediatePrecipitation(FVector2D Location, float Radius, float IntensityMMPerHour)
{
    UE_LOG(LogTemp, Warning, TEXT("🌧️ DIRECT PRECIPITATION TEST: %.1f mm/hr at %s"),
           IntensityMMPerHour, *Location.ToString());
           
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            FVector2D CellPos(X * CellSize, Y * CellSize);
            float Distance = FVector2D::Distance(CellPos, Location);
            
            if (Distance < Radius)
            {
                FSimplifiedAtmosphericCell& Cell = AtmosphericGrid[GetGridIndex(X, Y)];
                
                float Influence = 1.0f - (Distance / Radius);
                
                // DIRECTLY SET PRECIPITATION (bypass normal condensation process)
                Cell.PrecipitationRate = IntensityMMPerHour * Influence;
                Cell.CloudCover = FMath::Clamp(Cell.PrecipitationRate / 20.0f, 0.0f, 1.0f);
                
                // Also add corresponding moisture for realism
                Cell.MoistureMass += IntensityMMPerHour * Influence * 0.5f;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✅ Direct precipitation set - should see water immediately"));
}

