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
    
    // Initialize master controller reference
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
            }
        }
    }
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
            
            // Initialize hydraulic head to water table elevation
            Cell.LastTerrainHeight = TerrainHeight;
            Cell.HydraulicHead = TerrainHeight - Cell.WaterTableDepth;
            
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
            Cell.WaterTableDepth = AverageWaterTableDepth;
            Cell.SoilMoisture = 0.2f;
            Cell.Permeability = 0.5f;
        }
    }
}

// ===== WATER CYCLE INTEGRATION =====

void AGeologyController::SetWaterTableDepth(FVector Location, float Depth)
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        GeologyGrid[Index].WaterTableDepth = FMath::Max(0.0f, Depth);
    }
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

float AGeologyController::GetWaterTableDepthAtLocation(FVector Location) const
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        return GeologyGrid[Index].WaterTableDepth;
    }
    
    return AverageWaterTableDepth;
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
    ProcessSurfaceWaterInfiltration(DeltaTime);
    UpdateSimplifiedWaterTable(DeltaTime);
    ProcessGroundwaterDischarge(DeltaTime);
       UpdateHydraulicHeadSystem(DeltaTime);
}

void AGeologyController::ProcessSurfaceWaterInfiltration(float DeltaTime)
{
    // PHASE 3: Realistic infiltration from existing surface water
    if (!WaterSystem || !TargetTerrain) return;
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Processing surface water infiltration"));
    
    for (int32 Y = 0; Y < GeologyGridHeight; Y++)
    {
        for (int32 X = 0; X < GeologyGridWidth; X++)
        {
            int32 Index = GetGridIndex(X, Y);
            FSimplifiedGeology& Geology = GeologyGrid[Index];
            
            // Convert geology grid to world position using master controller
            FVector2D GridPos(X, Y);
            FVector WorldPos;
            if (MasterController)
            {
                FVector2D Pos2D = MasterController->ConvertGeologyToWaterGrid(FVector2D(X, Y));
                WorldPos = FVector(Pos2D.X, Pos2D.Y, 0.0f);
            }
            else
            {
                WorldPos = TargetTerrain->TerrainToWorldPosition(GridPos.X, GridPos.Y);
            }
            
            // Check for existing surface water at this location
            float SurfaceWaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldPos);
            
            if (SurfaceWaterDepth > 0.01f) // Has surface water to infiltrate
            {
                // Calculate maximum infiltration rate based on rock type
                float MaxInfiltrationRate = GetInfiltrationRate(Geology.SurfaceRock);
                
                // Reduce infiltration if soil is already saturated
                float SoilSaturation = Geology.SoilMoisture;
                float EffectiveInfiltrationRate = MaxInfiltrationRate * (1.0f - SoilSaturation);
                
                // Calculate how much water can infiltrate this frame
                float MaxInfiltrationDepth = EffectiveInfiltrationRate * DeltaTime;
                float ActualInfiltrationDepth = FMath::Min(SurfaceWaterDepth, MaxInfiltrationDepth);
                
                if (ActualInfiltrationDepth > 0.001f) // Meaningful infiltration
                {
                    // Remove water from surface
                    WaterSystem->RemoveWater(WorldPos, ActualInfiltrationDepth);
                    
                    // Update soil moisture using water volume authority
                    float SoilCapacity = GetSoilCapacity(Geology.SurfaceRock);
                    float InfiltratedVolume = MasterController ? 
                        MasterController->GetWaterCellVolume(ActualInfiltrationDepth) :
                        ActualInfiltrationDepth;
                    
                    float SoilMoistureIncrease = InfiltratedVolume / SoilCapacity;
                    Geology.SoilMoisture = FMath::Min(1.0f, Geology.SoilMoisture + SoilMoistureIncrease);
                    
                    // Update water table (excess water goes to groundwater)
                    if (Geology.SoilMoisture >= 1.0f)
                    {
                        float ExcessWater = (Geology.SoilMoisture - 1.0f) * SoilCapacity;
                        // Use geology cell water volume conversion with proper porosity
                        float GroundwaterIncrease = MasterController ?
                            MasterController->GetGeologyCellWaterVolume(ExcessWater, Geology.Permeability) :
                            ExcessWater * Geology.Permeability;
                        Geology.WaterTableDepth = FMath::Max(0.0f, Geology.WaterTableDepth - GroundwaterIncrease);
                        Geology.SoilMoisture = 1.0f; // Cap at full saturation
                    }
                    
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Infiltrated %f depth at (%d,%d), soil moisture now %f"),
                           ActualInfiltrationDepth, X, Y, Geology.SoilMoisture);
                }
            }
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
void AGeologyController::UpdateSimplifiedWaterTable(float DeltaTime)
{
    // Copy current depths
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        TempWaterTableDepths[i] = GeologyGrid[i].WaterTableDepth;
    }
    
    // Simple lateral flow between cells
    for (int32 Y = 1; Y < GeologyGridHeight - 1; Y++)
    {
        for (int32 X = 1; X < GeologyGridWidth - 1; X++)
        {
            int32 Index = GetGridIndex(X, Y);
            float CurrentDepth = GeologyGrid[Index].WaterTableDepth;
            
            // Get neighbor depths
            float NeighborSum = 0.0f;
            int32 NeighborCount = 0;
            
            // 4-neighbor averaging
            int32 Neighbors[4] = {
                GetGridIndex(X - 1, Y),
                GetGridIndex(X + 1, Y),
                GetGridIndex(X, Y - 1),
                GetGridIndex(X, Y + 1)
            };
            
            for (int32 NeighborIndex : Neighbors)
            {
                if (NeighborIndex >= 0 && NeighborIndex < GeologyGrid.Num())
                {
                    NeighborSum += GeologyGrid[NeighborIndex].WaterTableDepth;
                    NeighborCount++;
                }
            }
            
            if (NeighborCount > 0)
            {
                float NeighborAvg = NeighborSum / NeighborCount;
                float FlowRate = GroundwaterFlowRate * GeologyGrid[Index].Permeability;
                TempWaterTableDepths[Index] = FMath::Lerp(CurrentDepth, NeighborAvg,
                    FlowRate * DeltaTime);
            }
        }
    }
    
    // Apply new depths
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        GeologyGrid[i].WaterTableDepth = TempWaterTableDepths[i];
    }
}

void AGeologyController::ProcessGroundwaterDischarge(float DeltaTime)
{
    if (!MasterController || !WaterSystem) return;
    
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        FSimplifiedGeology& Geology = GeologyGrid[i];
        
        // Spring discharge where water table at surface
        if (Geology.WaterTableDepth < 0.1f)
        {
            float SpringFlow = SpringFlowRate * DeltaTime;
            int32 X = i % GeologyGridWidth;
            int32 Y = i / GeologyGridWidth;
            FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
            
            // Calculate cell area
            float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale() /
                           (GeologyGridWidth * GeologyGridHeight);
            
            // Route through master controller
            MasterController->TransferGroundwaterToSurface(WorldPos, SpringFlow);
            
            // Update water table
            Geology.WaterTableDepth += SpringFlow / (Geology.Permeability * CellArea);
        }
    }
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
            
            FVector2D GridPos(X, Y);
            FVector WorldPos = TargetTerrain->TerrainToWorldPosition(GridPos.X, GridPos.Y);
            float SurfaceHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
            
            // Draw water table depth
            FVector WaterTablePos = WorldPos;
            WaterTablePos.Z = SurfaceHeight - Cell.WaterTableDepth;
            
            FColor DepthColor = FColor::Blue;
            if (Cell.WaterTableDepth < 1.0f) // Near surface
            {
                DepthColor = FColor::Cyan;
            }
            else if (Cell.WaterTableDepth > 20.0f) // Deep
            {
                DepthColor = FColor::Purple;
            }
            
            // Draw line from surface to water table
            DrawDebugLine(GetWorld(), WorldPos, WaterTablePos, DepthColor, false, 5.0f, 0, 2.0f);
            
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
    float AvgWaterDepth = 0.0f;
    float AvgSoilMoisture = 0.0f;
    int32 SpringCount = 0;
    
    for (const FSimplifiedGeology& Cell : GeologyGrid)
    {
        AvgWaterDepth += Cell.WaterTableDepth;
        AvgSoilMoisture += Cell.SoilMoisture;
        if (Cell.WaterTableDepth < 0.1f) SpringCount++;
    }
    
    AvgWaterDepth /= GeologyGrid.Num();
    AvgSoilMoisture /= GeologyGrid.Num();
    
    FString DebugText = FString::Printf(
        TEXT("Geology Stats:\nAvg Water Table: %.1fm\nAvg Soil Moisture: %.1f%%\nActive Springs: %d"),
        AvgWaterDepth, AvgSoilMoisture * 100.0f, SpringCount);
    
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
            
            Geology.WaterTableDepth = FMath::Max(0.0f, Geology.WaterTableDepth - WaterTableRise);
            Geology.HydraulicHead += WaterTableRise;
        }
    }
}

//Hydraulic Head System

void AGeologyController::UpdateHydraulicHeadSystem(float DeltaTime)
{
    // Step 1: Update hydraulic heads based on current terrain
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        FSimplifiedGeology& Cell = GeologyGrid[i];
        int32 X = i % GeologyGridWidth;
        int32 Y = i / GeologyGridWidth;
        FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
        float CurrentTerrainHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
        
        // Check if terrain changed
        if (FMath::Abs(CurrentTerrainHeight - Cell.LastTerrainHeight) > 0.01f)
        {
            OnTerrainModified(WorldPos, Cell.LastTerrainHeight, CurrentTerrainHeight);
            Cell.LastTerrainHeight = CurrentTerrainHeight;
        }
    }
    
    // Step 2: Lateral groundwater flow
    UpdateLateralGroundwaterFlow(DeltaTime);
    
    // Step 3: Surface water interaction
    ProcessHydraulicSurfaceInteraction(DeltaTime);
}

void AGeologyController::OnTerrainModified(FVector WorldLocation, float OldHeight, float NewHeight)
{
    FVector2D Coords = WorldToGridCoordinates(WorldLocation);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (!IsValidGridCoordinate(X, Y)) return;
    
    int32 Index = GetGridIndex(X, Y);
    FSimplifiedGeology& Cell = GeologyGrid[Index];
    
    // Update water table to maintain same absolute elevation
    float WaterTableElevation = OldHeight - Cell.WaterTableDepth;
    Cell.WaterTableDepth = NewHeight - WaterTableElevation;
    
    // If terrain dropped below water table, we need to add surface water
    if (NewHeight < WaterTableElevation)
    {
        float ExposedWaterDepth = WaterTableElevation - NewHeight;
        
        // Check current surface water
        float CurrentWaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldLocation);
        
        if (CurrentWaterDepth < ExposedWaterDepth)
        {
            // Mark for filling in the surface interaction update
            // (Don't add water immediately to allow for gradual filling)
            UE_LOG(LogTemp, Warning, TEXT("Terrain dropped below water table at %s. Water table at %.1fm, terrain at %.1fm"),
                *WorldLocation.ToString(), WaterTableElevation, NewHeight);
        }
    }
}

void AGeologyController::ProcessHydraulicSurfaceInteraction(float DeltaTime)
{
    if (!WaterSystem || !MasterController) return;
    
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        FSimplifiedGeology& Cell = GeologyGrid[i];
        int32 X = i % GeologyGridWidth;
        int32 Y = i / GeologyGridWidth;
        FVector WorldPos = TargetTerrain->TerrainToWorldPosition(X, Y);
        
        float TerrainHeight = Cell.LastTerrainHeight;
        float WaterTableElevation = Cell.GetWaterTableElevation(TerrainHeight);
        float CurrentWaterDepth = WaterSystem->GetWaterDepthAtPosition(WorldPos);
        float SurfaceWaterElevation = TerrainHeight + CurrentWaterDepth;
        
        // Case 1: Terrain is below water table (groundwater exposed)
        if (TerrainHeight < WaterTableElevation)
        {
            float TargetWaterDepth = WaterTableElevation - TerrainHeight;
            
            // Check for artesian conditions
            if (bEnableArtesianConditions && Cell.HydraulicHead > WaterTableElevation)
            {
                TargetWaterDepth = Cell.HydraulicHead - TerrainHeight;
            }
            
            float DepthDeficit = TargetWaterDepth - CurrentWaterDepth;
            
            if (DepthDeficit > 0.001f)
            {
                // Fill rate based on permeability and deficit
                float FillRate = Cell.Permeability * WaterTableFillMultiplier;
                float WaterToAdd = DepthDeficit * FillRate * DeltaTime;
                
                // Clamp to prevent overshooting
                WaterToAdd = FMath::Min(WaterToAdd, DepthDeficit);
                
                // Add water through master controller
                MasterController->TransferGroundwaterToSurface(WorldPos, WaterToAdd);
                
                // Lower water table slightly as water emerges
                float VolumeExtracted = WaterToAdd * MasterController->GetTerrainScale() *
                                       MasterController->GetTerrainScale();
                float WaterTableDrop = VolumeExtracted / (Cell.StorageCoefficient *
                                      GeologyGridWidth * GeologyGridHeight);
                Cell.WaterTableDepth += WaterTableDrop;
                Cell.HydraulicHead -= WaterTableDrop;
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Filling water table hole: Added %.3fm water at %s"),
                    WaterToAdd, *WorldPos.ToString());
            }
        }
        // Case 2: Surface water above water table (infiltration)
        else if (CurrentWaterDepth > 0.01f && SurfaceWaterElevation > WaterTableElevation)
        {
            float InfiltrationRate = GetInfiltrationRate(Cell.SurfaceRock);
            float MaxInfiltration = CurrentWaterDepth * InfiltrationRate * DeltaTime;
            
            // Remove from surface
            WaterSystem->RemoveWater(WorldPos, MaxInfiltration);
            
            // Raise water table
            float InfiltratedVolume = MaxInfiltration * MasterController->GetTerrainScale() *
                                     MasterController->GetTerrainScale();
            float WaterTableRise = InfiltratedVolume / (Cell.StorageCoefficient *
                                  GeologyGridWidth * GeologyGridHeight);
            Cell.WaterTableDepth = FMath::Max(0.0f, Cell.WaterTableDepth - WaterTableRise);
            Cell.HydraulicHead += WaterTableRise;
        }
    }
}

void AGeologyController::UpdateLateralGroundwaterFlow(float DeltaTime)
{
    // Copy current hydraulic heads for calculation
    TArray<float> NewHydraulicHeads;
    NewHydraulicHeads.SetNum(GeologyGrid.Num());
    
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        NewHydraulicHeads[i] = GeologyGrid[i].HydraulicHead;
    }
    
    // Calculate flow between cells using Darcy's law
    for (int32 Y = 0; Y < GeologyGridHeight; Y++)
    {
        for (int32 X = 0; X < GeologyGridWidth; X++)
        {
            int32 Index = GetGridIndex(X, Y);
            const FSimplifiedGeology& Cell = GeologyGrid[Index];
            
            float TotalFlow = 0.0f;
            
            // Check all 4 neighbors
            TArray<FIntPoint> Neighbors = {
                FIntPoint(X + 1, Y),
                FIntPoint(X - 1, Y),
                FIntPoint(X, Y + 1),
                FIntPoint(X, Y - 1)
            };
            
            for (const FIntPoint& Neighbor : Neighbors)
            {
                if (IsValidGridCoordinate(Neighbor.X, Neighbor.Y))
                {
                    int32 NeighborIndex = GetGridIndex(Neighbor.X, Neighbor.Y);
                    const FSimplifiedGeology& NeighborCell = GeologyGrid[NeighborIndex];
                    
                    // Hydraulic gradient
                    float HeadDifference = Cell.HydraulicHead - NeighborCell.HydraulicHead;
                    float Distance = MasterController->GetTerrainScale(); // Cell spacing
                    float Gradient = HeadDifference / Distance;
                    
                    // Average transmissivity (harmonic mean for accuracy)
                    float AvgTransmissivity = 2.0f * Cell.Transmissivity * NeighborCell.Transmissivity /
                                            (Cell.Transmissivity + NeighborCell.Transmissivity);
                    
                    // Flow rate (m³/s per meter width)
                    float FlowRate = AvgTransmissivity * Gradient * Distance;
                    TotalFlow += FlowRate * DeltaTime;
                }
            }
            
            // Update hydraulic head based on net flow
            float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
            float HeadChange = TotalFlow / (CellArea * Cell.StorageCoefficient);
            NewHydraulicHeads[Index] -= HeadChange;
        }
    }
    
    // Apply new hydraulic heads and update water table depths
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        FSimplifiedGeology& Cell = GeologyGrid[i];
        Cell.HydraulicHead = NewHydraulicHeads[i];
        
        // Update water table depth to match new hydraulic head
        float TerrainHeight = Cell.LastTerrainHeight;
        Cell.WaterTableDepth = TerrainHeight - Cell.HydraulicHead;
    }
}

float AGeologyController::GetHydraulicHeadAtLocation(FVector Location) const
{
    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);
    
    if (IsValidGridCoordinate(X, Y))
    {
        int32 Index = GetGridIndex(X, Y);
        return GeologyGrid[Index].HydraulicHead;
    }
    
    return 0.0f;
}
