// GeologyController.cpp - Simplified implementation for water cycle

#include "GeologyController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "MasterController.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "ProceduralMeshComponent.h"

AGeologyController::AGeologyController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GeologyRoot"));
}

void AGeologyController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Beginning play"));
    
    // FIRST: Find master controller before anything else
    if (!MasterController)
    {
        TArray<AActor*> MasterControllers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMasterWorldController::StaticClass(), MasterControllers);
        if (MasterControllers.Num() > 0)
        {
            MasterController = Cast<AMasterWorldController>(MasterControllers[0]);
            if (MasterController)
            {
                RegisterWithMasterController(MasterController);
                UE_LOG(LogTemp, Warning, TEXT("GeologyController: Found and registered with MasterController"));
            }
        }
    }
    
    // Find existing systems
    if (!WaterSystem)
    {
        // WaterSystem is typically a component of DynamicTerrain
        if (TargetTerrain)
        {
            WaterSystem = TargetTerrain->WaterSystem;
            if (WaterSystem)
            {
                UE_LOG(LogTemp, Warning, TEXT("GeologyController: Found water system on terrain"));
            }
        }
    }
    
    // Initialize the geology grid
    InitializeGeologyGrid();
    
    // Initialize water table (now MasterController should be available)
    InitializeWaterTable();
    
    // Initial water table checking is now handled by MasterController during initialization
    // to ensure proper timing with other systems
}

void AGeologyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemInitialized && bEnableWaterTable)
    {
        UpdateGeologySystem(DeltaTime);
    }
}

// ===== INITIALIZATION =====

void AGeologyController::Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water)
{
    if (!Terrain)
    {
        UE_LOG(LogTemp, Error, TEXT("GeologyController: Invalid terrain reference"));
        return;
    }
    
    TargetTerrain = Terrain;
    WaterSystem = Water;
    
    InitializeGeologyGrid();
    
    // Try to find MasterController if not already set
    if (!MasterController)
    {
        TArray<AActor*> MasterControllers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMasterWorldController::StaticClass(), MasterControllers);
        if (MasterControllers.Num() > 0)
        {
            MasterController = Cast<AMasterWorldController>(MasterControllers[0]);
            if (MasterController)
            {
                RegisterWithMasterController(MasterController);
                UE_LOG(LogTemp, Warning, TEXT("GeologyController: Found and registered with MasterController during Initialize"));
            }
        }
    }
    
    // Initialize water table AFTER grid is created and MasterController is found
    InitializeWaterTable();
    
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Initialized with %d geology cells (%dx%d)"),
           GeologyGrid.Num(), GeologyGridWidth, GeologyGridHeight);
    
    // Initialize hydraulic heads after grid creation
        for (int32 i = 0; i < GeologyGrid.Num(); i++)
        {
            FSimplifiedGeology& Cell = GeologyGrid[i];
            
            // Get terrain height at this geology cell
            int32 X = i % GeologyGridWidth;
            int32 Y = i / GeologyGridWidth;
            FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
            float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
            
            // Initialize hydraulic head to global water table elevation
            Cell.LastTerrainHeight = TerrainHeight;
            Cell.HydraulicHead = GlobalWaterTableElevation;
            
            // Set transmissivity based on rock type
            switch (Cell.SurfaceRock)
            {
                case ERockType::Gravel:
                    Cell.Transmissivity = 0.1f;     // Very high
                    Cell.StorageCoefficient = 0.25f;
                    break;
                case ERockType::Sand:
                    Cell.Transmissivity = 0.01f;    // High
                    Cell.StorageCoefficient = 0.35f;
                    break;
                case ERockType::Silt:
                    Cell.Transmissivity = 0.001f;   // Medium
                    Cell.StorageCoefficient = 0.40f;
                    break;
                case ERockType::Clay:
                    Cell.Transmissivity = 0.00001f; // Low
                    Cell.StorageCoefficient = 0.45f;
                    break;
                case ERockType::Granite:
                    Cell.Transmissivity = 0.000001f; // Very low
                    Cell.StorageCoefficient = 0.05f;
                    break;
            }
        }
}

void AGeologyController::InitializeGeologyGrid()
{
    // Determine grid dimensions
    if (TargetTerrain)
    {
        GeologyGridWidth = TargetTerrain->TerrainWidth / 4;
        GeologyGridHeight = TargetTerrain->TerrainHeight / 4;
    }
    else
    {
        GeologyGridWidth = GridWidth;
        GeologyGridHeight = GridHeight;
    }
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Initializing simplified geology grid %dx%d"), 
           GeologyGridWidth, GeologyGridHeight);
    
    // Clear and resize arrays
    GeologyGrid.Empty();
    GeologyGrid.SetNum(GeologyGridWidth * GeologyGridHeight);
    
    TempWaterTableDepths.SetNum(GeologyGrid.Num());
    TempSoilMoisture.SetNum(GeologyGrid.Num());
    
    // Initialize each cell
    for (int32 Y = 0; Y < GeologyGridHeight; Y++)
    {
        for (int32 X = 0; X < GeologyGridWidth; X++)
        {
            int32 Index = GetGridIndex(X, Y);
            FSimplifiedGeology& Cell = GeologyGrid[Index];
            
            // Set default values
            Cell.SurfaceRock = ERockType::Sandstone;
            Cell.Hardness = 0.5f;
            Cell.SoilMoisture = 0.2f;
            Cell.Permeability = 0.5f;
        }
    }
}

// ===== WATER CYCLE INTEGRATION =====

void AGeologyController::SetWaterTableDepth(FVector Location, float Depth)
{
    // This function now sets the global water table elevation based on the location's terrain height
    if (!TargetTerrain) return;
    
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(Location);
    SetGlobalWaterTableElevation(TerrainHeight - Depth);
}

void AGeologyController::ReduceSoilMoisture(FVector Location, float Amount)
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        GeologyGrid[Index].SoilMoisture = FMath::Clamp(
            GeologyGrid[Index].SoilMoisture - Amount, 0.0f, 1.0f);
    }
}

float AGeologyController::GetSoilMoistureAt(FVector Location) const
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        return GeologyGrid[Index].SoilMoisture;
    }
    
    return 0.0f;
}

void AGeologyController::UpdateWaterTableDebugVisualization()
{
    if (!bShowWaterTablePlane) 
    {
        if (WaterTableDebugMesh)
        {
            WaterTableDebugMesh->SetVisibility(false);
        }
        return;
    }
    
    if (!WaterTableDebugMesh)
    {
        WaterTableDebugMesh = NewObject<UProceduralMeshComponent>(this);
        WaterTableDebugMesh->SetupAttachment(RootComponent);
        WaterTableDebugMesh->RegisterComponent();
    }
    
    // Create a simple plane at water table elevation
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    
    if (!MasterController) return;
    
    FVector2D WorldSize = MasterController->GetWorldDimensions();
    float Scale = MasterController->GetTerrainScale();
    
    // Create quad
    Vertices.Add(FVector(0, 0, GlobalWaterTableElevation));
    Vertices.Add(FVector(WorldSize.X * Scale, 0, GlobalWaterTableElevation));
    Vertices.Add(FVector(WorldSize.X * Scale, WorldSize.Y * Scale, GlobalWaterTableElevation));
    Vertices.Add(FVector(0, WorldSize.Y * Scale, GlobalWaterTableElevation));
    
    Triangles.Add(0); Triangles.Add(1); Triangles.Add(2);
    Triangles.Add(0); Triangles.Add(2); Triangles.Add(3);
    
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(FVector::UpVector);
        UVs.Add(FVector2D(0, 0));
        Colors.Add(WaterTablePlaneColor);
    }
    
    WaterTableDebugMesh->CreateMeshSection(0, Vertices, Triangles, 
        Normals, UVs, Colors, TArray<FProcMeshTangent>(), false);
    WaterTableDebugMesh->SetVisibility(true);
}

float AGeologyController::GetWaterTableDepthAtLocation(FVector Location) const
{
    // Simple: depth below surface at this location
    if (!TargetTerrain) return 0.0f;
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(Location);
    return TerrainHeight - GlobalWaterTableElevation;
}

float AGeologyController::GetInfiltrationRate(ERockType Rock) const
{
    // Infiltration rates in m/s
    switch (Rock)
    {
        case ERockType::Clay:      return 0.0000014f;  // 5 mm/hr
        case ERockType::Silt:      return 0.0000072f;  // 26 mm/hr
        case ERockType::Sand:      return 0.00005f;    // 180 mm/hr
        case ERockType::Gravel:    return 0.00008f;    // 288 mm/hr
        case ERockType::Granite:   return 0.0000003f;  // 1 mm/hr
        case ERockType::Limestone: return 0.00004f;    // 144 mm/hr (fractured)
        case ERockType::Sandstone: return 0.00002f;    // 72 mm/hr
        case ERockType::Shale:     return 0.000002f;   // 7 mm/hr
        default:                   return 0.00001f;    // 36 mm/hr
    }
}

// ===== CORE UPDATE FUNCTIONS =====

void AGeologyController::UpdateGeologySystem(float DeltaTime)
{
    if (!TargetTerrain || !WaterSystem) return;
    
    // Update in correct order for water conservation
   // ProcessSurfaceWaterInfiltration(DeltaTime);
    
    ProcessWaterTableEmergence(DeltaTime);
    ProcessUserSprings(DeltaTime);  // Process user-created springs

    UpdateWaterTableDebugVisualization();
}

void AGeologyController::ProcessWaterTableEmergence(float DeltaTime)
{
    // Only process after initialization
    if (!bWaterTableInitialized || !bEnableWaterTable || !WaterSystem || ActiveEmergencePoints.Num() == 0) return;
    
    // Process registered emergence points
    TArray<FVector> PointsToRemove;
    
    for (const FVector& EmergencePoint : ActiveEmergencePoints)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(EmergencePoint);
        
        // Check if still below water table
        if (TerrainHeight >= GlobalWaterTableElevation)
        {
            PointsToRemove.Add(EmergencePoint);
            continue;
        }
        
        float CurrentWaterDepth = WaterSystem->GetWaterDepthAtPosition(EmergencePoint);
        float WaterSurfaceHeight = TerrainHeight + CurrentWaterDepth;
        
        // If water surface reached water table, remove from active list
        if (WaterSurfaceHeight >= GlobalWaterTableElevation - 0.1f) // Small tolerance
        {
            PointsToRemove.Add(EmergencePoint);
            continue;
        }
        
        // Add water to reach water table
        float DepthDeficit = GlobalWaterTableElevation - WaterSurfaceHeight;
        float DepthToAdd = DepthDeficit * WaterEmergenceRate * DeltaTime;
        DepthToAdd = FMath::Min(DepthToAdd, DepthDeficit);
        
        // Use proper transfer function through MasterController
        if (MasterController)
        {
            float CellArea = MasterController->GetWaterCellArea();
            float Volume = DepthToAdd * CellArea;
            
            if (MasterController->CanGroundwaterEmerge(Volume))
            {
                // This handles BOTH adding water to surface AND updating water budgets
                MasterController->TransferGroundwaterToSurface(EmergencePoint, Volume);
                // No need to manually RemoveWaterFromWaterTable - the transfer handles it
            }
        }
    }
    
    // Clean up completed points
    for (const FVector& Point : PointsToRemove)
    {
        ActiveEmergencePoints.Remove(Point);
    }
    
    // Log status occasionally
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    if (LogTimer > 5.0f && ActiveEmergencePoints.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Water table maintaining %d active emergence points"), ActiveEmergencePoints.Num());
        LogTimer = 0.0f;
    }
}

void AGeologyController::EmergenceWaterAtPoints(const TArray<FVector>& WorldPositions, UWaterSystem* Water)
{
    if (!Water || !MasterController) return;
    
    for (const FVector& WorldPos : WorldPositions)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
        
        if (TerrainHeight < GlobalWaterTableElevation)
        {
            float TargetDepth = GlobalWaterTableElevation - TerrainHeight;
            float CurrentDepth = Water->GetWaterDepthAtPosition(WorldPos);
            
            if (CurrentDepth < TargetDepth)
            {
                // Calculate how much water to add initially
                float DepthToAdd = (TargetDepth - CurrentDepth) * WaterTableFillMultiplier;
                
                // Cap the initial fill to avoid instant flooding
                const float MaxInitialDepth = 2.0f; // Maximum 2 meters initial fill
                DepthToAdd = FMath::Min(DepthToAdd, MaxInitialDepth);
                
                // Use proper transfer function through MasterController
                if (MasterController)
                {
                    float CellArea = MasterController->GetWaterCellArea();
                    
                    // Validate cell area to prevent invalid volume calculations
                    if (CellArea <= 0.0f || !FMath::IsFinite(CellArea))
                    {
                        UE_LOG(LogTemp, Error, TEXT("Invalid cell area: %.2f - skipping emergence"), CellArea);
                        continue;
                    }
                    
                    float Volume = DepthToAdd * CellArea;
                    
                    // Validate volume before transfer
                    if (!FMath::IsFinite(Volume) || Volume <= 0.0f)
                    {
                        UE_LOG(LogTemp, Error, TEXT("Invalid emergence volume: %.2f - skipping"), Volume);
                        continue;
                    }
                    
                    if (MasterController->CanGroundwaterEmerge(Volume))
                    {
                        // Use the proper transfer function
                        MasterController->TransferGroundwaterToSurface(WorldPos, Volume);
                    }
                }
                
                // If we didn't fully fill to water table level, register for continuous filling
                float NewDepth = CurrentDepth + DepthToAdd;
                if (NewDepth < TargetDepth - 0.1f) // 0.1m tolerance
                {
                    RegisterEmergencePoint(WorldPos);
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Registered emergence point at %s (%.1fm below water table)"),
                           *WorldPos.ToString(), TargetDepth - NewDepth);
                }
            }
        }
    }
    
    // Log summary
    if (WorldPositions.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Processed %d potential emergence points, %d active"),
               WorldPositions.Num(), ActiveEmergencePoints.Num());
    }
}

void AGeologyController::CheckInitialWaterTable()
{
    if (!TargetTerrain || !WaterSystem) 
    {
        UE_LOG(LogTemp, Error, TEXT("CheckInitialWaterTable: Missing TargetTerrain or WaterSystem"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CheckInitialWaterTable: Starting scan for points below water table elevation %.1fm"), GlobalWaterTableElevation);
    
    TArray<FVector> BelowWaterPoints;
    
    // Iterate through terrain chunks directly
    for (const FTerrainChunk& Chunk : TargetTerrain->TerrainChunks)
    {
        int32 ChunkX = Chunk.ChunkX;
        int32 ChunkY = Chunk.ChunkY;
        
        // Sample 9 points per chunk (corners + center)
        for (int32 dy = 0; dy <= 32; dy += 16)
        {
            for (int32 dx = 0; dx <= 32; dx += 16)
            {
                int32 X = ChunkX * 32 + dx;
                int32 Y = ChunkY * 32 + dy;
                
                if (X < TargetTerrain->TerrainWidth && Y < TargetTerrain->TerrainHeight)
                {
                    FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
                    float Height = TargetTerrain->GetHeightAtPosition(WorldPos);
                    
                    if (Height < GlobalWaterTableElevation)
                    {
                        BelowWaterPoints.Add(WorldPos);
                    }
                }
            }
        }
    }
    
    EmergenceWaterAtPoints(BelowWaterPoints, WaterSystem);
}

void AGeologyController::OnWaterTableElevationChanged()
{
    CheckInitialWaterTable();
}

void AGeologyController::RegisterEmergencePoint(FVector WorldPosition)
{
    // Only register if actually below water table
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldPosition);
    if (TerrainHeight < GlobalWaterTableElevation)
    {
        ActiveEmergencePoints.AddUnique(WorldPosition);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Registered emergence point at %s"), *WorldPosition.ToString());
    }
}

void AGeologyController::UnregisterEmergencePoint(FVector WorldPosition)
{
    ActiveEmergencePoints.Remove(WorldPosition);
}
/*
void AGeologyController::ProcessSurfaceWaterInfiltration(float DeltaTime)
{
    if (!WaterSystem || !MasterController) return;
    
    // Static timer to reduce frequency
    static float InfiltrationTimer = 0.0f;
    InfiltrationTimer += DeltaTime;
    
    if (InfiltrationTimer < 0.5f) return; // 2Hz update
    
    // Get total surface water volume
    float TotalWaterVolume = WaterSystem->GetTotalWaterVolume();
    if (TotalWaterVolume <= 0.01f)
    {
        InfiltrationTimer = 0.0f;
        return;
    }
    
    // Uniform infiltration: NOW LINKED TO WATER SYSTEM, WAS 0.00002 m/s 
    float InfiltrationDepth = WaterSystem->WaterAbsorptionRate * InfiltrationTimer;
    float FractionToInfiltrate = FMath::Min(InfiltrationDepth / WaterSystem->GetAverageDepth(), 0.1f);
    
    // Apply uniform removal
    float InfiltratedVolume = TotalWaterVolume * FractionToInfiltrate;
    WaterSystem->ApplyUniformDepthReduction(InfiltrationDepth);
    
    // Add to water table
    AddWaterToWaterTable(InfiltratedVolume);
    
    InfiltrationTimer = 0.0f;
}
*/

void AGeologyController::InitializeWaterTableFill()
{
    if (bWaterTableInitialized || !TargetTerrain || !WaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeWaterTableFill: Already initialized or missing components"));
        return;
    }
        
    UE_LOG(LogTemp, Warning, TEXT("Filling initial water table at elevation %.1fm"), 
           GlobalWaterTableElevation);
    
    // Find all points below water table
    TArray<FVector> BelowWaterPoints;
    
    // Sample terrain at reasonable intervals
    const int32 SampleInterval = 16; // Every 16 units
    for (int32 Y = 0; Y < TargetTerrain->TerrainHeight; Y += SampleInterval)
    {
        for (int32 X = 0; X < TargetTerrain->TerrainWidth; X += SampleInterval)
        {
            FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
            float Height = TargetTerrain->GetHeightAtPosition(WorldPos);
            
            if (Height < GlobalWaterTableElevation)
            {
                float DepthToFill = GlobalWaterTableElevation - Height;
                
                // Add water directly - this is initial world state, not a "transfer"
                WaterSystem->AddWater(WorldPos, DepthToFill);
                
                // Register for continuous emergence if not fully filled
                RegisterEmergencePoint(WorldPos);
            }
        }
    }
    
    bWaterTableInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Water table fill complete - %d emergence points active"), 
           ActiveEmergencePoints.Num());
}

// ===== USER SPRING FUNCTIONS =====

void AGeologyController::AddUserSpring(FVector WorldLocation, float FlowRate)
{
    FUserSpring NewSpring;
    NewSpring.Location = WorldLocation;
    NewSpring.FlowRate = (FlowRate > 0.0f) ? FlowRate : DefaultSpringFlowRate;
    NewSpring.bActive = true;
    
    UserSprings.Add(NewSpring);
    
    UE_LOG(LogTemp, Warning, TEXT("Added user spring at %s with flow rate %.2f m³/s"), 
        *WorldLocation.ToString(), NewSpring.FlowRate);
}

void AGeologyController::RemoveUserSpring(FVector WorldLocation, float SearchRadius)
{
    for (int32 i = UserSprings.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(UserSprings[i].Location, WorldLocation) <= SearchRadius)
        {
            UE_LOG(LogTemp, Warning, TEXT("Removed user spring at %s"), 
                *UserSprings[i].Location.ToString());
            UserSprings.RemoveAt(i);
        }
    }
}

void AGeologyController::ClearAllUserSprings()
{
    UserSprings.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all user springs"));
}

void AGeologyController::ProcessUserSprings(float DeltaTime)
{
    if (!MasterController || UserSprings.Num() == 0) return;
    
    for (FUserSpring& Spring : UserSprings)
    {
        if (!Spring.bActive) continue;
        
        // Calculate volume to add this frame
        float VolumeToAdd = Spring.FlowRate * DeltaTime;
        
        // Check if we have enough groundwater
        if (MasterController->CanGroundwaterEmerge(VolumeToAdd))
        {
            // Transfer from groundwater to surface
            MasterController->TransferGroundwaterToSurface(Spring.Location, VolumeToAdd);
        }
        else
        {
            // Not enough groundwater - disable spring
            Spring.bActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Spring at %s ran dry - insufficient groundwater"), 
                *Spring.Location.ToString());
        }
    }
}

FVector2D AGeologyController::GeologyGridToWorldCoordinates(int32 X, int32 Y) const
{
    // Use master controller for grid conversions
    if (MasterController)
    {
        return MasterController->ConvertGeologyToWaterGrid(FVector2D(X, Y));
    }
    
    // Fallback to manual calculation if no master controller
    float CellSizeX = (TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale) / GeologyGridWidth;
    float CellSizeY = (TargetTerrain->TerrainHeight * TargetTerrain->TerrainScale) / GeologyGridHeight;
    
    FVector TerrainOrigin = TargetTerrain->GetActorLocation();
    
    return FVector2D(
        TerrainOrigin.X + (X * CellSizeX),
        TerrainOrigin.Y + (Y * CellSizeY)
    );
}
void AGeologyController::InitializeWaterTable()
{
    if (bUseProportionalWaterTable && TargetTerrain)
    {
        // Get max terrain height by sampling the terrain
        float MaxTerrainHeight = 0.0f;
        for (int32 Y = 0; Y < TargetTerrain->TerrainHeight; Y += 10)
        {
            for (int32 X = 0; X < TargetTerrain->TerrainWidth; X += 10)
            {
                FVector Pos = TargetTerrain->TerrainToWorldPosition(X, Y);
                float Height = TargetTerrain->GetHeightAtPosition(Pos);
                MaxTerrainHeight = FMath::Max(MaxTerrainHeight, Height);
            }
        }
        GlobalWaterTableElevation = MaxTerrainHeight * WaterTableHeightPercent;
    }
    
    // Defer volume calculation if MasterController isn't ready yet
    if (!MasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Water table elevation set to %.1fm - volume calculation deferred until MasterController ready"), 
            GlobalWaterTableElevation);
        return;
    }
    
    // Calculate initial volume based on world size
    float WorldArea = GetTotalWorldArea();
    if (WorldArea <= 0.0f || !FMath::IsFinite(WorldArea))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid world area calculated: %.2f - using default"), WorldArea);
        // Use a reasonable default based on terrain dimensions
        if (TargetTerrain)
        {
            float TerrainScale = TargetTerrain->TerrainScale;
            WorldArea = TargetTerrain->TerrainWidth * TerrainScale * TargetTerrain->TerrainHeight * TerrainScale;
        }
        else
        {
            WorldArea = 1000000.0f; // 1km² default
        }
    }
    
    float AverageDepth = 100.0f; // Assume 100m average saturated thickness
    GlobalWaterTableVolume = WorldArea * AverageDepth * GlobalPorosity;
    
    // Validate the calculated volume
    if (!FMath::IsFinite(GlobalWaterTableVolume) || GlobalWaterTableVolume <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid water table volume calculated: %.2f - using default"), GlobalWaterTableVolume);
        GlobalWaterTableVolume = 1000000.0f; // Default 1 million m³
    }
    
    // Register initial groundwater with master controller
    MasterController->SetInitialGroundwater(GlobalWaterTableVolume);
    
    UE_LOG(LogTemp, Warning, TEXT("Water table initialized at elevation %.1fm with %.0f m³ (World Area: %.0f m²)"), 
        GlobalWaterTableElevation, GlobalWaterTableVolume, WorldArea);
}


void AGeologyController::UpdateWaterTableFromVolume()
{
    if (!MasterController) return;
    
    // Get current volume from master controller
    GlobalWaterTableVolume = MasterController->GetGroundwaterVolume();
    
    // Calculate new elevation: Volume = Area * Height * Porosity
    float WorldArea = GetTotalWorldArea();
    float EffectiveHeight = GlobalWaterTableVolume / (WorldArea * GlobalPorosity);
    
    // Water table elevation is height above bedrock (assumed at z=0)
    GlobalWaterTableElevation = EffectiveHeight;
}

void AGeologyController::SetGlobalWaterTableElevation(float NewElevation)
{
    GlobalWaterTableElevation = NewElevation;
    
    // Update volume to match new elevation
    if (MasterController)
    {
        float WorldArea = GetTotalWorldArea();
        GlobalWaterTableVolume = NewElevation * WorldArea * GlobalPorosity;
        MasterController->SetInitialGroundwater(GlobalWaterTableVolume);
    }
}

void AGeologyController::AddWaterToWaterTable(float VolumeM3)
{
    if (VolumeM3 <= 0.0f) return; // Skip zero volumes
    
    // Skip tiny volumes to avoid log spam
    if (VolumeM3 < 0.001f)
    {
        GlobalWaterTableVolume += VolumeM3;
        return; // Skip calculations and logging
    }
    
    // Add to total volume
    GlobalWaterTableVolume += VolumeM3;
    
    // Calculate new water table elevation
    float WorldArea = GetTotalWorldArea();
    float EffectiveArea = WorldArea * GlobalPorosity;
    
    // Height change = Volume / (Area * Porosity)
    float HeightChange = VolumeM3 / EffectiveArea;
    GlobalWaterTableElevation += HeightChange;
    
    // Only log significant additions
    if (VolumeM3 > 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Added %.1f m³ to water table, new elevation: %.2f m"),
            VolumeM3, GlobalWaterTableElevation);
    }
}

float AGeologyController::RemoveWaterFromWaterTable(float VolumeM3)
{
    // Can't remove more than we have
    float ActualRemoval = FMath::Min(VolumeM3, GlobalWaterTableVolume);
    
    // Remove from total volume
    GlobalWaterTableVolume -= ActualRemoval;
    
    // Calculate new water table elevation
    float WorldArea = GetTotalWorldArea();
    float EffectiveArea = WorldArea * GlobalPorosity;
    
    // Height change = Volume / (Area * Porosity)
    float HeightChange = ActualRemoval / EffectiveArea;
    GlobalWaterTableElevation -= HeightChange;
    
    return ActualRemoval;
}


// ===== QUERIES =====

ERockType AGeologyController::GetRockTypeAtLocation(FVector Location, float Depth) const
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        return GeologyGrid[Index].SurfaceRock;
    }
    
    return ERockType::Sandstone;
}

bool AGeologyController::IsLocationAboveWaterTable(FVector Location) const
{
    float WaterTableDepth = GetWaterTableDepthAtLocation(Location);
    return WaterTableDepth > 0.0f;
}

// ===== INTERNAL HELPERS =====

FVector2D AGeologyController::WorldToGridCoordinates(const FVector& WorldPosition) const
{
    // ✅ Use SAME coordinate authority as everyone else
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("No coordinate authority"));
        return FVector2D::ZeroVector;
    }
    
    // Get authoritative terrain coordinates
    FVector2D TerrainCoords = MasterController->WorldToTerrainCoordinates(WorldPosition);
    
    // Convert to geology grid coordinates
    FVector2D WorldDims = MasterController->GetWorldDimensions();
    float GeologyX = (TerrainCoords.X / WorldDims.X) * GeologyGridWidth;
    float GeologyY = (TerrainCoords.Y / WorldDims.Y) * GeologyGridHeight;
    
    return FVector2D(GeologyX, GeologyY);
}

bool AGeologyController::IsValidGridCoordinate(int32 X, int32 Y) const
{
    return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

int32 AGeologyController::GetGridIndex(int32 X, int32 Y) const
{
    X = FMath::Clamp(X, 0, GridWidth - 1);
    Y = FMath::Clamp(Y, 0, GridHeight - 1);
    return Y * GridWidth + X;
}

float AGeologyController::GetSoilCapacity(ERockType Rock) const
{
    // Return soil moisture capacity based on rock type
    switch (Rock)
    {
        case ERockType::Clay:
            return 0.4f;  // High capacity
        case ERockType::Silt:
            return 0.35f;
        case ERockType::Sand:
            return 0.15f; // Low capacity
        case ERockType::Gravel:
            return 0.1f;  // Very low
        case ERockType::Granite:
        case ERockType::Basalt:
            return 0.05f; // Minimal
        default:
            return 0.2f;  // Default medium capacity
    }
}


bool AGeologyController::IsEdgeCell(int32 X, int32 Y) const
{
    return X == 0 || X == GeologyGridWidth - 1 || Y == 0 || Y == GeologyGridHeight - 1;
}

float AGeologyController::GetTotalWorldArea() const
{
    if (!MasterController) return 1000000.0f;  // Default 1 km²
    
    FVector2D WorldDims = MasterController->GetWorldDimensions();
    float TerrainScale = MasterController->GetTerrainScale();
    
    return WorldDims.X * WorldDims.Y * TerrainScale * TerrainScale;
}

// ===== ISCALABLESYSTEM INTERFACE =====

void AGeologyController::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuring from master"));
    
    // Update grid dimensions if terrain reference exists
    if (TargetTerrain)
    {
        GeologyGridWidth = Config.TerrainWidth;
        GeologyGridHeight = Config.TerrainHeight;
        CellSize = Config.GeologyConfig.ErosionCellSize; // Use erosion cell size
        
        // Reinitialize grid with new dimensions
        InitializeGeologyGrid();
    }
    
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuration complete - Grid: %dx%d"),
           GeologyGridWidth, GeologyGridHeight);
}

void AGeologyController::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    CurrentCoordinateSystem = Coords;
    GridOrigin = Coords.WorldOrigin;
}

bool AGeologyController::IsSystemScaled() const
{
    return bIsScaledByMaster && bIsRegisteredWithMaster;
}

// ===== MASTER CONTROLLER INTEGRATION =====

void AGeologyController::RegisterWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("[GEOLOGY SCALING] Cannot register with null master controller"));
        return;
    }
    
    MasterController = Master;
    bIsRegisteredWithMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] GeologyController registered with master controller"));
}

bool AGeologyController::IsRegisteredWithMaster() const
{
    return bIsRegisteredWithMaster && MasterController != nullptr;
}

FString AGeologyController::GetScalingDebugInfo() const
{
    FString DebugInfo = TEXT("Geology System Scaling:\n");
    DebugInfo += FString::Printf(TEXT("  - Registered: %s\n"), IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("  - Scaled: %s\n"), IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("  - Grid Size: %dx%d\n"), GeologyGridWidth, GeologyGridHeight);
    DebugInfo += FString::Printf(TEXT("  - Total Cells: %d\n"), GeologyGrid.Num());
    return DebugInfo;
}

// ===== VISUALIZATION =====

void AGeologyController::ShowWaterTable(bool bEnable)
{
    if (!bEnable || !GetWorld() || !TargetTerrain) return;
    
    // Sample visualization every 10 cells
    for (int32 Y = 0; Y < GeologyGridHeight; Y += 10)
    {
        for (int32 X = 0; X < GeologyGridWidth; X += 10)
        {
            int32 Index = GetGridIndex(X, Y);
            const FSimplifiedGeology& Cell = GeologyGrid[Index];
            
            FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
            float SurfaceHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
            
            // Draw water table plane intersection
            if (SurfaceHeight > GlobalWaterTableElevation)
            {
                // Above water table - draw down to it
                FVector WaterTablePos = WorldPos;
                WaterTablePos.Z = GlobalWaterTableElevation;
                DrawDebugLine(GetWorld(), WorldPos, WaterTablePos, FColor::Blue, false, 5.0f, 0, 2.0f);
            }
            else
            {
                // Below water table - draw as cyan
                DrawDebugSphere(GetWorld(), WorldPos, 50.0f, 8, FColor::Cyan, false, 5.0f);
            }
            
            // Show soil moisture as sphere size
            float SphereRadius = 10.0f + (Cell.SoilMoisture * 40.0f);
            DrawDebugSphere(GetWorld(), WorldPos + FVector(0, 0, 20), SphereRadius,
                8, FColor::Green, false, 5.0f);
        }
    }
}

void AGeologyController::DrawSimplifiedDebugInfo() const
{
    if (!GetWorld() || !TargetTerrain) return;
    
    // Calculate averages
    float AvgSoilMoisture = 0.0f;
    int32 SpringCount = 0;
    
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        AvgSoilMoisture += GeologyGrid[i].SoilMoisture;
        // Check if terrain below water table
        int32 X = i % GeologyGridWidth;
        int32 Y = i / GeologyGridWidth;
        FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
        if (TerrainHeight < GlobalWaterTableElevation) SpringCount++;
    }
    
    AvgSoilMoisture /= GeologyGrid.Num();
    
    FString DebugText = FString::Printf(
           TEXT("Geology Stats:\nWater Table Elevation: %.1fm\nAvg Soil Moisture: %.1f%%\nActive Springs: %d\nActive Emergence Points: %d\nWater Volume: %.0f m³"),
           GlobalWaterTableElevation, AvgSoilMoisture * 100.0f, SpringCount,
           ActiveEmergencePoints.Num(), GlobalWaterTableVolume);

    
    GEngine->AddOnScreenDebugMessage(10, 5.0f, FColor::Yellow, DebugText);
}


void AGeologyController::ApplyInfiltration(FVector Location, float WaterAmount)
{
    if (!MasterController) return;
    
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        FSimplifiedGeology& Geology = GeologyGrid[Index];
        
        // Update soil moisture (existing code)
        float SoilCapacity = GetSoilCapacity(Geology.SurfaceRock);
        float SoilSpace = (1.0f - Geology.SoilMoisture) * SoilCapacity;
        float ToSoil = FMath::Min(WaterAmount, SoilSpace);
        
        Geology.SoilMoisture += ToSoil / SoilCapacity;
        
        // Excess goes to water table
        float ToWaterTable = WaterAmount - ToSoil;
        if (ToWaterTable > 0.0f && MasterController)
        {
            // Raise water table and hydraulic head
            float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
            float WaterTableRise = ToWaterTable / (CellArea * Geology.StorageCoefficient);
            
            // Add to global water table instead of per-cell
            if (MasterController)
            {
                MasterController->TransferSurfaceToGroundwater( Location, ToWaterTable);
            }
        }
    }
}
