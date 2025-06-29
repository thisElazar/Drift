/**
 * ============================================
 * TERRAI GEOLOGY CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "GeologyController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AGeologyController::AGeologyController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GeologyRoot"));
    
    // Initialize default rock layers
    RockLayers.Empty();
    
    // Surface layer - typically sand/soil
    FRockLayer SurfaceLayer;
    SurfaceLayer.RockType = ERockType::Sand;
    SurfaceLayer.Thickness = 5.0f;
    SurfaceLayer.Hardness = 0.3f;
    SurfaceLayer.Porosity = 0.4f;
    SurfaceLayer.Permeability = 0.8f;
    SurfaceLayer.ErosionResistance = 0.2f;
    SurfaceLayer.LayerColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f); // Sandy color
    RockLayers.Add(SurfaceLayer);
    
    // Sedimentary layer
    FRockLayer SedimentaryLayer;
    SedimentaryLayer.RockType = ERockType::Sandstone;
    SedimentaryLayer.Thickness = 20.0f;
    SedimentaryLayer.Hardness = 0.6f;
    SedimentaryLayer.Porosity = 0.2f;
    SedimentaryLayer.Permeability = 0.4f;
    SedimentaryLayer.ErosionResistance = 0.5f;
    SedimentaryLayer.LayerColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    RockLayers.Add(SedimentaryLayer);
    
    // Bedrock layer
    FRockLayer BedrockLayer;
    BedrockLayer.RockType = ERockType::Granite;
    BedrockLayer.Thickness = 50.0f;
    BedrockLayer.Hardness = 0.9f;
    BedrockLayer.Porosity = 0.05f;
    BedrockLayer.Permeability = 0.1f;
    BedrockLayer.ErosionResistance = 0.9f;
    BedrockLayer.LayerColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Gray
    RockLayers.Add(BedrockLayer);
    
    // Initialize default sand dunes
    SandDunes.Empty();
    
    // Water table defaults
    WaterTableDepth = 50.0f;
    GroundwaterFlowRate = 0.1f;
    WaterTableRechargeRate = 0.05f;
}

void AGeologyController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeGeologicalLayers();
}

void AGeologyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemInitialized && bEnableGeologicalProcesses)
    {
        GeologicalTimer += DeltaTime;
        
        // Update geological processes at slower intervals
        if (GeologicalTimer >= 1.0f) // Every second
        {
            UpdateGeologicalProcesses(DeltaTime);
            GeologicalTimer = 0.0f;
        }
    }
}

// ===== INITIALIZATION =====

void AGeologyController::Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water)
{
    if (!Terrain || !Water)
    {
        UE_LOG(LogTemp, Error, TEXT("GeologyController: Invalid initialization parameters"));
        return;
    }
    
    TargetTerrain = Terrain;
    WaterSystem = Water;
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Successfully initialized"));
    
    // Initialize water table grid
    WaterTableGrid.Empty();
    
    // Create a simplified water table grid (could be expanded)
    for (int32 x = -10; x <= 10; x++)
    {
        for (int32 y = -10; y <= 10; y++)
        {
            FVector GridPoint = TargetTerrain->GetActorLocation();
            GridPoint.X += x * 500.0f;
            GridPoint.Y += y * 500.0f;
            GridPoint.Z = TargetTerrain->GetHeightAtPosition(GridPoint) - WaterTableDepth;
            WaterTableGrid.Add(GridPoint);
        }
    }
}

// ===== CORE FUNCTIONS =====

void AGeologyController::UpdateGeologicalProcesses(float DeltaTime)
{
    if (!TargetTerrain || !WaterSystem) return;
    
    if (bEnableSandDunes)
    {
        UpdateSandDunes(DeltaTime);
    }
    
    if (bEnableWaterTable)
    {
        UpdateWaterTable(DeltaTime);
    }
    
    ProcessSedimentation(DeltaTime);
    ProcessWeathering(DeltaTime);
    ProcessSandTransport(DeltaTime);
    ProcessGroundwaterFlow(DeltaTime);
}

void AGeologyController::UpdateSandDunes(float DeltaTime)
{
    for (FSandDuneData& Dune : SandDunes)
    {
        // Simulate wind erosion and sand transport
        float WindEffect = WindStrength * DeltaTime;
        
        // Move sand based on wind direction
        FVector SandMovement = PrevailingWindDirection * SandTransportRate * WindEffect;
        
        // Update dune position slightly
        Dune.Location += SandMovement * 0.1f; // Slow movement
        
        // Adjust height based on wind patterns
        float HeightChange = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * WindEffect * 0.5f;
        Dune.Height += HeightChange;
        Dune.Height = FMath::Max(Dune.Height, 10.0f); // Minimum height
        
        // Update sand accumulation
        Dune.SandAccumulation += SandTransportRate * WindEffect;
        
        // Create new dune if accumulation is high enough
        if (Dune.SandAccumulation > DuneFormationThreshold)
        {
            FSandDuneData NewDune;
            NewDune.Location = Dune.Location + (PrevailingWindDirection * 300.0f);
            NewDune.Height = 20.0f;
            NewDune.Width = 150.0f;
            NewDune.WindDirection = PrevailingWindDirection;
            NewDune.SandAccumulation = 0.0f;
            NewDune.Stability = 0.8f;
            
            SandDunes.Add(NewDune);
            Dune.SandAccumulation = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("GeologyController: New sand dune formed at %s"), *NewDune.Location.ToString());
        }
    }
}

void AGeologyController::UpdateWaterTable(float DeltaTime)
{
    if (!WaterSystem) return;
    
    // Update water table levels based on surface water and precipitation
    for (FVector& WaterPoint : WaterTableGrid)
    {
        FVector SurfaceLocation = WaterPoint;
        SurfaceLocation.Z = TargetTerrain->GetHeightAtPosition(WaterPoint);
        
        float SurfaceWaterDepth = WaterSystem->GetWaterDepthAtPosition(SurfaceLocation);
        
        // Recharge water table from surface water
        if (SurfaceWaterDepth > 0.1f)
        {
            float Recharge = WaterTableRechargeRate * DeltaTime * SurfaceWaterDepth;
            WaterPoint.Z += Recharge * 0.1f; // Slow recharge
        }
        
        // Natural drainage
        WaterPoint.Z -= 0.01f * DeltaTime; // Slow drainage
        
        // Don't let water table go below bedrock
        float BedrockLevel = SurfaceLocation.Z - BedrockDepth;
        WaterPoint.Z = FMath::Max(WaterPoint.Z, BedrockLevel);
        
        // Create springs where water table intersects surface
        if (WaterPoint.Z > SurfaceLocation.Z - SpringFormationThreshold)
        {
            CreateSpring(SurfaceLocation);
        }
    }
}

void AGeologyController::FormSedimentLayer(FVector Location, float Thickness, ERockType RockType)
{
    FRockLayer NewLayer = CreateDefaultRockLayer(RockType);
    NewLayer.Thickness = Thickness;
    
    // For now, just add to default layers (simplified for UE5.4 compatibility)
    DefaultRockLayers.Add(NewLayer);
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Formed %d sediment layer (%.1fm thick) at %s"), 
           (int32)RockType, Thickness, *Location.ToString());
}

// ===== GEOLOGICAL QUERIES =====

ERockType AGeologyController::GetRockTypeAtLocation(FVector WorldLocation, float Depth) const
{
    // Use default rock layers for now (simplified for UE5.4)
    if (RockLayers.Num() > 0)
    {
        float CurrentDepth = 0.0f;
        
        for (const FRockLayer& Layer : RockLayers)
        {
            CurrentDepth += Layer.Thickness;
            if (Depth <= CurrentDepth)
            {
                return Layer.RockType;
            }
        }
        
        // Return bottom layer if deeper
        return RockLayers[RockLayers.Num() - 1].RockType;
    }
    
    // Fallback based on depth
    if (Depth < 5.0f) return ERockType::Sand;
    else if (Depth < 25.0f) return ERockType::Sandstone;
    else return ERockType::Granite;
}

float AGeologyController::GetRockHardnessAtLocation(FVector WorldLocation, float Depth) const
{
    ERockType RockType = GetRockTypeAtLocation(WorldLocation, Depth);
    
    // Return hardness based on rock type
    switch (RockType)
    {
    case ERockType::Sand: return 0.2f;
    case ERockType::Clay: return 0.3f;
    case ERockType::Silt: return 0.25f;
    case ERockType::Sandstone: return 0.6f;
    case ERockType::Limestone: return 0.7f;
    case ERockType::Granite: return 0.9f;
    case ERockType::Gravel: return 0.4f;
    case ERockType::Bedrock: return 1.0f;
    default: return 0.5f;
    }
}

float AGeologyController::GetWaterTableDepthAtLocation(FVector WorldLocation) const
{
    // Find closest water table point
    float ClosestDistance = FLT_MAX;
    FVector ClosestWaterPoint = FVector::ZeroVector;
    
    for (const FVector& WaterPoint : WaterTableGrid)
    {
        float Distance = FVector::Dist2D(WorldLocation, WaterPoint);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestWaterPoint = WaterPoint;
        }
    }
    
    if (ClosestDistance < FLT_MAX)
    {
        float SurfaceHeight = TargetTerrain ? TargetTerrain->GetHeightAtPosition(WorldLocation) : WorldLocation.Z;
        return SurfaceHeight - ClosestWaterPoint.Z;
    }
    
    return WaterTableDepth; // Default depth
}

float AGeologyController::GetGroundwaterFlowAtLocation(FVector WorldLocation) const
{
    float WaterTableDepth_Local = GetWaterTableDepthAtLocation(WorldLocation);
    ERockType RockType = GetRockTypeAtLocation(WorldLocation, WaterTableDepth_Local);
    
    // Flow rate depends on rock permeability
    float BaseFlow = GroundwaterFlowRate;
    
    switch (RockType)
    {
    case ERockType::Sand: BaseFlow *= 2.0f; break;
    case ERockType::Gravel: BaseFlow *= 3.0f; break;
    case ERockType::Clay: BaseFlow *= 0.2f; break;
    case ERockType::Granite: BaseFlow *= 0.1f; break;
    case ERockType::Limestone: BaseFlow *= 1.5f; break;
    default: break;
    }
    
    return BaseFlow;
}

// ===== SYSTEM COORDINATION =====

void AGeologyController::OnWaterFlowChanged(FVector Location, float FlowRate)
{
    // Increase erosion in areas with high water flow
    if (FlowRate > 1.0f)
    {
        ERockType LocalRockType = GetRockTypeAtLocation(Location);
        float ErosionAmount = FlowRate * 0.1f * (1.0f - GetRockHardnessAtLocation(Location));
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Water erosion at %s, amount %.3f"), 
               *Location.ToString(), ErosionAmount);
    }
}

void AGeologyController::OnWindChanged(FVector WindDirection, float WindSpeed)
{
    PrevailingWindDirection = WindDirection.GetSafeNormal();
    WindStrength = WindSpeed;
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Wind updated - Direction:%s Speed:%.1f"), 
           *WindDirection.ToString(), WindSpeed);
}

void AGeologyController::OnErosionOccurred(FVector Location, float ErosionAmount)
{
    // Create sediment based on erosion
    ERockType ErodedRockType = GetRockTypeAtLocation(Location);
    
    // Convert harder rocks to softer sediments
    ERockType SedimentType = ERockType::Sand;
    switch (ErodedRockType)
    {
    case ERockType::Granite: SedimentType = ERockType::Gravel; break;
    case ERockType::Sandstone: SedimentType = ERockType::Sand; break;
    case ERockType::Limestone: SedimentType = ERockType::Silt; break;
    default: SedimentType = ERockType::Sand; break;
    }
    
    // Find downstream location to deposit sediment
    FVector DownstreamLocation = Location + FVector(100.0f, 0.0f, -10.0f); // Simplified
    FormSedimentLayer(DownstreamLocation, ErosionAmount * 0.1f, SedimentType);
}

// ===== VISUALIZATION =====

void AGeologyController::ShowWaterTable(bool bEnable)
{
    if (!bEnable) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing water table"));
    
    for (const FVector& WaterPoint : WaterTableGrid)
    {
        if (TargetTerrain)
        {
            FVector SurfacePoint = WaterPoint;
            SurfacePoint.Z = TargetTerrain->GetHeightAtPosition(WaterPoint);
            
            // Draw line from surface to water table
            DrawDebugLine(GetWorld(), SurfacePoint, WaterPoint, FColor::Blue, false, 5.0f, 0, 2.0f);
            DrawDebugSphere(GetWorld(), WaterPoint, 25.0f, 8, FColor::Cyan, false, 5.0f);
        }
    }
}

void AGeologyController::ShowSandDunes(bool bEnable)
{
    if (!bEnable) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing sand dunes"));
    
    for (const FSandDuneData& Dune : SandDunes)
    {
        FColor DuneColor = FColor::Yellow;
        DrawDebugSphere(GetWorld(), Dune.Location, Dune.Height, 12, DuneColor, false, 5.0f);
        
        // Show wind direction
        FVector WindEnd = Dune.Location + (Dune.WindDirection * 200.0f);
        DrawDebugDirectionalArrow(GetWorld(), Dune.Location, WindEnd, 50.0f, FColor::Orange, false, 5.0f, 0, 3.0f);
    }
}

void AGeologyController::ShowRockLayers(bool bEnable)
{
    if (!bEnable) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing rock layers"));
    
    // Simple cross-section visualization - using simplified data
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Rock layers visualization with %d default layers"), DefaultRockLayers.Num());
}

void AGeologyController::PrintGeologicalStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== GEOLOGICAL STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Rock Layers: %d"), RockLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Sand Dunes: %d"), SandDunes.Num());
    UE_LOG(LogTemp, Warning, TEXT("Water Table Points: %d"), WaterTableGrid.Num());
    UE_LOG(LogTemp, Warning, TEXT("Geological Locations: %d"), DefaultRockLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Water Table Depth: %.1fm"), WaterTableDepth);
    UE_LOG(LogTemp, Warning, TEXT("Wind Strength: %.2f"), WindStrength);
    UE_LOG(LogTemp, Warning, TEXT("Bedrock Depth: %.1fm"), BedrockDepth);
}

// ===== PRIVATE FUNCTIONS =====

void AGeologyController::InitializeGeologicalLayers()
{
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Initializing geological layers"));
    
    // Initialize default rock layers (simplified for UE5.4)
    DefaultRockLayers = RockLayers;
}

void AGeologyController::ProcessSedimentation(float DeltaTime)
{
    // Simple sedimentation process
    float SedimentAmount = SedimentationRate * DeltaTime;
    
    if (SedimentAmount > 0.001f && WaterSystem)
    {
        // Add sediment where water is slow-moving
        // This is a simplified implementation
    }
}

void AGeologyController::ProcessWeathering(float DeltaTime)
{
    if (!bEnableChemicalWeathering) return;
    
    // Apply weathering to exposed rock surfaces
    float WeatheringAmount = WeatheringRate * DeltaTime;
    
    // This would typically modify the terrain heightmap slightly
    // For now, just log the process
    if (WeatheringAmount > 0.001f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Weathering process active"));
    }
}

void AGeologyController::ProcessSandTransport(float DeltaTime)
{
    if (!bEnableSandDunes) return;
    
    // Move sand based on wind patterns
    for (FSandDuneData& Dune : SandDunes)
    {
        float TransportRate = SandTransportRate * WindStrength * DeltaTime;
        
        // Erode sand from windward side, deposit on leeward side
        Dune.SandAccumulation += TransportRate;
    }
}

void AGeologyController::ProcessGroundwaterFlow(float DeltaTime)
{
    if (!bEnableWaterTable) return;
    
    // Simulate groundwater flow between grid points
    for (int32 i = 0; i < WaterTableGrid.Num(); i++)
    {
        FVector& CurrentPoint = WaterTableGrid[i];
        
        // Find neighboring points and flow toward lower elevations
        for (int32 j = 0; j < WaterTableGrid.Num(); j++)
        {
            if (i == j) continue;
            
            FVector& NeighborPoint = WaterTableGrid[j];
            float Distance = FVector::Dist(CurrentPoint, NeighborPoint);
            
            if (Distance < 1000.0f && NeighborPoint.Z < CurrentPoint.Z)
            {
                // Flow toward lower neighbor
                float FlowAmount = GroundwaterFlowRate * DeltaTime * 0.01f;
                CurrentPoint.Z -= FlowAmount;
                NeighborPoint.Z += FlowAmount * 0.5f; // Some flow loss
            }
        }
    }
}

void AGeologyController::CreateSpring(FVector Location)
{
    if (!WaterSystem) return;
    
    // Add water to the surface at spring location
    WaterSystem->AddWater(Location, 5.0f);
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Spring created at %s"), *Location.ToString());
}

FRockLayer AGeologyController::CreateDefaultRockLayer(ERockType Type) const
{
    FRockLayer Layer;
    Layer.RockType = Type;
    
    switch (Type)
    {
    case ERockType::Sand:
        Layer.Hardness = 0.2f;
        Layer.Porosity = 0.4f;
        Layer.Permeability = 0.8f;
        Layer.ErosionResistance = 0.1f;
        Layer.LayerColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        break;
        
    case ERockType::Clay:
        Layer.Hardness = 0.3f;
        Layer.Porosity = 0.5f;
        Layer.Permeability = 0.1f;
        Layer.ErosionResistance = 0.4f;
        Layer.LayerColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
        break;
        
    case ERockType::Granite:
        Layer.Hardness = 0.9f;
        Layer.Porosity = 0.05f;
        Layer.Permeability = 0.1f;
        Layer.ErosionResistance = 0.9f;
        Layer.LayerColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
        break;
        
    default:
        Layer.Hardness = 0.5f;
        Layer.Porosity = 0.3f;
        Layer.Permeability = 0.5f;
        Layer.ErosionResistance = 0.5f;
        Layer.LayerColor = FLinearColor::Gray;
        break;
    }
    
    Layer.Thickness = 10.0f; // Default thickness
    
    return Layer;
}

float AGeologyController::CalculateErosionResistance(const FRockLayer& Layer) const
{
    // Combine hardness and erosion resistance
    return (Layer.Hardness + Layer.ErosionResistance) * 0.5f;
}

// ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====

void AGeologyController::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuring from master controller"));
    
    CurrentWorldConfig = Config;
    
    // Configure erosion grid based on terrain size
    int32 NewErosionGridWidth = FMath::Max(64, Config.TerrainWidth / 4);
    int32 NewErosionGridHeight = FMath::Max(64, Config.TerrainHeight / 4);
    
    // Calculate cell size to cover terrain area
    float TerrainWorldSizeX = Config.TerrainWidth * Config.TerrainScale;
    float TerrainWorldSizeY = Config.TerrainHeight * Config.TerrainScale;
    
    float NewErosionCellSize = TerrainWorldSizeX / NewErosionGridWidth;
    
    // Update geological time scale
    float NewTimeScale = Config.GeologyConfig.GeologicalTimeScale;
    
    UE_LOG(LogTemp, Log, TEXT("[GEOLOGY SCALING] Erosion grid: %dx%d, Cell size: %.1fm, Time scale: %.1fx"),
           NewErosionGridWidth, NewErosionGridHeight, NewErosionCellSize, NewTimeScale);
    
    // Regenerate water table grid with new dimensions
    WaterTableGrid.Empty();
    
    int32 GridPointsX = FMath::Max(8, NewErosionGridWidth / 8); // Coarser than erosion grid
    int32 GridPointsY = FMath::Max(8, NewErosionGridHeight / 8);
    
    for (int32 x = 0; x < GridPointsX; x++)
    {
        for (int32 y = 0; y < GridPointsY; y++)
        {
            FVector GridPoint = CurrentCoordinateSystem.WorldOrigin;
            GridPoint.X += (x * NewErosionCellSize * 8.0f) - (TerrainWorldSizeX * 0.5f);
            GridPoint.Y += (y * NewErosionCellSize * 8.0f) - (TerrainWorldSizeY * 0.5f);
            
            if (TargetTerrain)
            {
                GridPoint.Z = TargetTerrain->GetHeightAtPosition(GridPoint) - WaterTableDepth;
            }
            else
            {
                GridPoint.Z = CurrentCoordinateSystem.WorldOrigin.Z - WaterTableDepth;
            }
            
            WaterTableGrid.Add(GridPoint);
        }
    }
    
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuration complete - Water table grid: %d points"),
           WaterTableGrid.Num());
}

void AGeologyController::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[GEOLOGY SCALING] Synchronizing coordinates"));
    
    CurrentCoordinateSystem = Coords;
    
    // Update existing geological features to use new coordinate system
    // For sand dunes, adjust positions relative to new origin
    for (FSandDuneData& Dune : SandDunes)
    {
        // Convert to relative coordinates, then back to world coordinates
        FVector RelativePos = Dune.Location - CurrentCoordinateSystem.WorldOrigin;
        Dune.Location = CurrentCoordinateSystem.WorldOrigin + RelativePos;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[GEOLOGY SCALING] Coordinate synchronization complete"));
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
    
    if (bIsScaledByMaster)
    {
        DebugInfo += FString::Printf(TEXT("  - Erosion Grid: %dx%d\n"), 
                                   CurrentWorldConfig.GeologyConfig.ErosionGridWidth,
                                   CurrentWorldConfig.GeologyConfig.ErosionGridHeight);
        DebugInfo += FString::Printf(TEXT("  - Cell Size: %.1fm\n"), CurrentWorldConfig.GeologyConfig.ErosionCellSize);
        DebugInfo += FString::Printf(TEXT("  - Time Scale: %.1fx\n"), CurrentWorldConfig.GeologyConfig.GeologicalTimeScale);
        DebugInfo += FString::Printf(TEXT("  - Water Table Points: %d\n"), WaterTableGrid.Num());
    }
    else
    {
        DebugInfo += TEXT("  - Not yet configured by master\n");
    }
    
    return DebugInfo;
}
