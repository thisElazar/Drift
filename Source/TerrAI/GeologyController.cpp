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
    if (!TargetTerrain || !WaterSystem) return;
    
    // Check for springs where water table meets surface
    for (int32 Y = 0; Y < GeologyGridHeight; Y += 4) // Sparse check for performance
    {
        for (int32 X = 0; X < GeologyGridWidth; X += 4)
        {
            int32 Index = GetGridIndex(X, Y);
            const FSimplifiedGeology& Geology = GeologyGrid[Index];
            
            FVector2D GridPos(X, Y);
            FVector WorldPos;
            if (MasterController)
            {
                FVector2D Pos2D = MasterController->ConvertGeologyToWaterGrid(GridPos);
                WorldPos = FVector(Pos2D.X, Pos2D.Y, 0.0f);
            }
            else
            {
                WorldPos = TargetTerrain->TerrainToWorldPosition(GridPos.X, GridPos.Y);
            }
            float SurfaceHeight = TargetTerrain->GetHeightAtPosition(WorldPos);
            float WaterTableHeight = SurfaceHeight - Geology.WaterTableDepth;
            
            // Spring forms when water table above surface
            if (Geology.WaterTableDepth < 0.01f) // Water table at/above surface
            {
                float SpringFlow = SpringFlowRate * DeltaTime;
                
                // Add water to surface
                // Scale up spring flow to match new depth scale
                // 0.01 m³/s in real units = 1 depth unit/s in simulation
                float ScaledSpringFlow = SpringFlow / AMasterWorldController::WATER_DEPTH_SCALE;
                WaterSystem->AddWater(WorldPos, ScaledSpringFlow);
                
                // Lower water table slightly
                GeologyGrid[Index].WaterTableDepth = 0.1f;
                
                UE_LOG(LogTemp, Warning, TEXT("Spring flowing at (%d,%d): %.3f m³/s"),
                    X, Y, SpringFlow / DeltaTime);
            }
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
        
        // Update soil moisture
        float SoilCapacity = GetSoilCapacity(Geology.SurfaceRock);
        float SoilSpace = (1.0f - Geology.SoilMoisture) * SoilCapacity;
        float ToSoil = FMath::Min(WaterAmount, SoilSpace);
        
        Geology.SoilMoisture += ToSoil / SoilCapacity;
        
        // Excess goes to water table
        float ToWaterTable = WaterAmount - ToSoil;
        if (ToWaterTable > 0.0f && MasterController)
        {
            // Convert excess water to groundwater volume using water authority
            float GroundwaterVolume = MasterController->GetGeologyCellWaterVolume(
                ToWaterTable, Geology.Permeability);
            // Convert volume to depth change using cell area
            float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
            float DepthChange = GroundwaterVolume / (CellArea * Geology.Permeability);
            Geology.WaterTableDepth = FMath::Max(0.0f, Geology.WaterTableDepth - DepthChange);
        }
    }
}
