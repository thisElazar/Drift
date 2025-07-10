/**
 * ============================================
 * TERRAI GEOLOGY CONTROLLER - IMPLEMENTATION (REFACTORED)
 * ============================================
 * Sand dune functionality moved to SandDuneController
 * Enhanced with stratigraphy, minerals, and advanced geology
 */
#include "GeologyController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "SandDuneController.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    // Define custom colors that FColor doesn't have
    const FColor ColorGold = FColor(255, 215, 0);      // Gold
    const FColor ColorOrange = FColor(255, 165, 0);    // Orange
}

AGeologyController::AGeologyController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GeologyRoot"));
    
    // Initialize with a basic stratigraphic column
    FGeologicalColumn DefaultColumn;
    
    // Surface layer - soil/regolith
    FRockLayer SurfaceLayer;
    SurfaceLayer.RockType = ERockType::Sand;
    SurfaceLayer.Thickness = 2.0f;
    SurfaceLayer.Depth = 0.0f;
    SurfaceLayer.Hardness = 0.2f;
    SurfaceLayer.Porosity = 0.45f;
    SurfaceLayer.Permeability = 0.9f;
    SurfaceLayer.ErosionResistance = 0.1f;
    SurfaceLayer.Age = 0.001f; // Very recent
    SurfaceLayer.LayerColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    DefaultColumn.Layers.Add(SurfaceLayer);
    
    // Sedimentary sequence
    FRockLayer SedimentaryLayer1;
    SedimentaryLayer1.RockType = ERockType::Sandstone;
    SedimentaryLayer1.Thickness = 15.0f;
    SedimentaryLayer1.Depth = 2.0f;
    SedimentaryLayer1.Hardness = 0.6f;
    SedimentaryLayer1.Porosity = 0.2f;
    SedimentaryLayer1.Permeability = 0.4f;
    SedimentaryLayer1.ErosionResistance = 0.5f;
    SedimentaryLayer1.Age = 50.0f; // 50 million years
    SedimentaryLayer1.LayerColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    SedimentaryLayer1.bContainsFossils = true;
    DefaultColumn.Layers.Add(SedimentaryLayer1);
    
    FRockLayer SedimentaryLayer2;
    SedimentaryLayer2.RockType = ERockType::Limestone;
    SedimentaryLayer2.Thickness = 25.0f;
    SedimentaryLayer2.Depth = 17.0f;
    SedimentaryLayer2.Hardness = 0.7f;
    SedimentaryLayer2.Porosity = 0.15f;
    SedimentaryLayer2.Permeability = 0.3f;
    SedimentaryLayer2.ErosionResistance = 0.6f;
    SedimentaryLayer2.Age = 200.0f;
    SedimentaryLayer2.LayerColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    SedimentaryLayer2.bContainsFossils = true;
    DefaultColumn.Layers.Add(SedimentaryLayer2);
    
    FRockLayer SedimentaryLayer3;
    SedimentaryLayer3.RockType = ERockType::Shale;
    SedimentaryLayer3.Thickness = 30.0f;
    SedimentaryLayer3.Depth = 42.0f;
    SedimentaryLayer3.Hardness = 0.5f;
    SedimentaryLayer3.Porosity = 0.1f;
    SedimentaryLayer3.Permeability = 0.05f;
    SedimentaryLayer3.ErosionResistance = 0.4f;
    SedimentaryLayer3.Age = 300.0f;
    SedimentaryLayer3.LayerColor = FLinearColor(0.4f, 0.4f, 0.35f, 1.0f);
    SedimentaryLayer3.bHasMineralDeposits = true;
    DefaultColumn.Layers.Add(SedimentaryLayer3);
    
    // Basement rock
    FRockLayer BedrockLayer;
    BedrockLayer.RockType = ERockType::Granite;
    BedrockLayer.Thickness = 1000.0f; // Essentially infinite
    BedrockLayer.Depth = 72.0f;
    BedrockLayer.Hardness = 0.9f;
    BedrockLayer.Porosity = 0.02f;
    BedrockLayer.Permeability = 0.01f;
    BedrockLayer.ErosionResistance = 0.95f;
    BedrockLayer.Age = 1000.0f; // 1 billion years
    BedrockLayer.LayerColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    BedrockLayer.bHasMineralDeposits = true;
    DefaultColumn.Layers.Add(BedrockLayer);
    
    DefaultColumn.TotalDepth = BedrockDepth;
    DefaultColumn.Location = FVector::ZeroVector;
    
    GeologicalColumns.Add(DefaultColumn);
    
    // Set default parameters
    AverageWaterTableDepth = 50.0f;
    GroundwaterFlowRate = 0.1f;
    WaterTableRechargeRate = 0.05f;
    CurrentTemperature = 20.0f;
}

void AGeologyController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeGeologicalColumns();
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
            UpdateGeologicalProcesses(DeltaTime * GeologicalTimeScale);
            GeologicalTimer = 0.0f;
        }
        
        // Water table updates more frequently
        if (bEnableWaterTable)
        {
            UpdateWaterTable(DeltaTime);
        }
        
        // PERFORMANCE FIX: Process accumulated sediment periodically
        SedimentAccumulationTimer += DeltaTime;
        if (SedimentAccumulationTimer >= SedimentFormationInterval)
        {
            ProcessAccumulatedSediment();
            SedimentAccumulationTimer = 0.0f;
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
    
    // Generate initial geological columns across the terrain
    InitializeGeologicalColumns();
    
    // Initialize water table grid
    WaterTableGrid.Empty();
    int32 GridSize = 20; // Simplified grid
    float GridSpacing = (TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale) / GridSize;
    
    for (int32 x = 0; x < GridSize; x++)
    {
        for (int32 y = 0; y < GridSize; y++)
        {
            FWaterTableData WaterPoint;
            WaterPoint.Location = TargetTerrain->GetActorLocation();
            WaterPoint.Location.X += (x - GridSize/2) * GridSpacing;
            WaterPoint.Location.Y += (y - GridSize/2) * GridSpacing;
            
            float SurfaceHeight = TargetTerrain->GetHeightAtPosition(WaterPoint.Location);
            WaterPoint.WaterLevel = SurfaceHeight - AverageWaterTableDepth;
            WaterPoint.FlowVelocity = GroundwaterFlowRate;
            WaterPoint.FlowDirection = FVector(1, 0, 0); // Default eastward flow
            
            WaterTableGrid.Add(WaterPoint);
        }
    }
    
    // Generate initial mineral deposits if enabled
    if (bGenerateMineralDeposits)
    {
        for (int32 i = 0; i < 10; i++) // Generate 10 random deposits
        {
            FVector RandomLocation = TargetTerrain->GetActorLocation() +
                FVector(FMath::RandRange(-1000.0f, 1000.0f), FMath::RandRange(-1000.0f, 1000.0f), 0);
            ERockType HostRock = GetRockTypeAtLocation(RandomLocation, FMath::RandRange(20.0f, 100.0f));
            GenerateMineralDeposit(RandomLocation, HostRock);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Successfully initialized with %d geological columns and %d water table points"),
           GeologicalColumns.Num(), WaterTableGrid.Num());
}

// ===== STRATIGRAPHY =====

void AGeologyController::GenerateStratigraphicColumn(FVector Location)
{
    FGeologicalColumn NewColumn;
    NewColumn.Location = Location;
    
    // Generate layers based on geological history
    // This is a simplified model - real stratigraphy would consider regional geology
    
    float CurrentDepth = 0.0f;
    
    // Recent deposits
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        FRockLayer AlluviumLayer;
        AlluviumLayer.RockType = ERockType::Gravel;
        AlluviumLayer.Thickness = FMath::RandRange(1.0f, 5.0f);
        AlluviumLayer.Depth = CurrentDepth;
        AlluviumLayer.Hardness = 0.3f;
        AlluviumLayer.Porosity = 0.35f;
        AlluviumLayer.Permeability = 0.8f;
        AlluviumLayer.ErosionResistance = 0.3f;
        AlluviumLayer.Age = 0.01f;
        AlluviumLayer.LayerColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
        NewColumn.Layers.Add(AlluviumLayer);
        CurrentDepth += AlluviumLayer.Thickness;
    }
    
    // Sedimentary sequence
    int32 NumSedimentaryLayers = FMath::RandRange(3, 6);
    for (int32 i = 0; i < NumSedimentaryLayers; i++)
    {
        FRockLayer SedLayer;
        
        // Alternate between different sedimentary rocks
        float RockChoice = FMath::RandRange(0.0f, 1.0f);
        if (RockChoice < 0.33f)
        {
            SedLayer.RockType = ERockType::Sandstone;
            SedLayer.Hardness = 0.6f;
            SedLayer.Porosity = 0.2f;
            SedLayer.Permeability = 0.4f;
            SedLayer.LayerColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
        }
        else if (RockChoice < 0.66f)
        {
            SedLayer.RockType = ERockType::Limestone;
            SedLayer.Hardness = 0.7f;
            SedLayer.Porosity = 0.15f;
            SedLayer.Permeability = 0.3f;
            SedLayer.LayerColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
            SedLayer.bContainsFossils = FMath::RandRange(0.0f, 1.0f) < 0.4f;
        }
        else
        {
            SedLayer.RockType = ERockType::Shale;
            SedLayer.Hardness = 0.5f;
            SedLayer.Porosity = 0.1f;
            SedLayer.Permeability = 0.05f;
            SedLayer.LayerColor = FLinearColor(0.4f, 0.4f, 0.35f, 1.0f);
        }
        
        SedLayer.Thickness = FMath::RandRange(10.0f, 40.0f);
        SedLayer.Depth = CurrentDepth;
        SedLayer.ErosionResistance = SedLayer.Hardness * 0.8f;
        SedLayer.Age = 50.0f + (i * 50.0f); // Older with depth
        SedLayer.bHasMineralDeposits = FMath::RandRange(0.0f, 1.0f) < MineralFormationProbability;
        
        NewColumn.Layers.Add(SedLayer);
        CurrentDepth += SedLayer.Thickness;
    }
    
    // Basement rock
    FRockLayer BasementLayer;
    float BasementChoice = FMath::RandRange(0.0f, 1.0f);
    if (BasementChoice < 0.6f)
    {
        BasementLayer.RockType = ERockType::Granite;
        BasementLayer.Hardness = 0.9f;
        BasementLayer.LayerColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else if (BasementChoice < 0.8f)
    {
        BasementLayer.RockType = ERockType::Basalt;
        BasementLayer.Hardness = 0.85f;
        BasementLayer.LayerColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
    }
    else
    {
        BasementLayer.RockType = ERockType::Quartzite;
        BasementLayer.Hardness = 0.95f;
        BasementLayer.LayerColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.0f);
    }
    
    BasementLayer.Thickness = 1000.0f;
    BasementLayer.Depth = CurrentDepth;
    BasementLayer.Porosity = 0.02f;
    BasementLayer.Permeability = 0.01f;
    BasementLayer.ErosionResistance = 0.95f;
    BasementLayer.Age = 1000.0f;
    BasementLayer.bHasMineralDeposits = true;
    NewColumn.Layers.Add(BasementLayer);
    
    NewColumn.TotalDepth = CurrentDepth + BasementLayer.Thickness;
    
    // Chance of fault line
    NewColumn.bHasFaultLine = FMath::RandRange(0.0f, 1.0f) < 0.1f;
    if (NewColumn.bHasFaultLine)
    {
        NewColumn.FaultDisplacement = FMath::RandRange(-20.0f, 20.0f);
    }
    
    GeologicalColumns.Add(NewColumn);
    
}

FGeologicalColumn* AGeologyController::GetColumnAtLocation(FVector Location)
{
    // Find nearest geological column
    float MinDistance = FLT_MAX;
    int32 NearestIndex = 0;
    
    for (int32 i = 0; i < GeologicalColumns.Num(); i++)
    {
        float Distance = FVector::Dist(Location, GeologicalColumns[i].Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestIndex = i;
        }
    }
    
    if (GeologicalColumns.IsValidIndex(NearestIndex))
    {
        return &GeologicalColumns[NearestIndex];
    }
    
    // Return nullptr if none found
    return nullptr;
}

const FGeologicalColumn* AGeologyController::GetColumnAtLocation(FVector Location) const
{
    // Find nearest geological column
    float MinDistance = FLT_MAX;
    int32 NearestIndex = 0;
    
    for (int32 i = 0; i < GeologicalColumns.Num(); i++)
    {
        float Distance = FVector::Dist(Location, GeologicalColumns[i].Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestIndex = i;
        }
    }
    
    if (GeologicalColumns.IsValidIndex(NearestIndex))
    {
        return &GeologicalColumns[NearestIndex];
    }
    
    return nullptr;
}

FGeologicalColumn AGeologyController::GetColumnAtLocationCopy(FVector Location) const
{
    const FGeologicalColumn* Column = GetColumnAtLocation(Location);
    if (Column)
    {
        return *Column;
    }
    
    // Return default column if none found
    return FGeologicalColumn();
}

void AGeologyController::AddGeologicalLayer(FVector Location, const FRockLayer& NewLayer)
{
    FGeologicalColumn* Column = GetColumnAtLocation(Location);
    if (!Column)
    {
        // Create new column if none exists
        FGeologicalColumn NewColumn;
        NewColumn.Location = Location;
        GeologicalColumns.Add(NewColumn);
        Column = &GeologicalColumns.Last();
    }
    
    // Insert layer at appropriate depth
    int32 InsertIndex = 0;
    for (int32 i = 0; i < Column->Layers.Num(); i++)
    {
        if (NewLayer.Depth < Column->Layers[i].Depth)
        {
            InsertIndex = i;
            break;
        }
    }
    
    Column->Layers.Insert(NewLayer, InsertIndex);
    
    // Adjust depths of lower layers
    for (int32 i = InsertIndex + 1; i < Column->Layers.Num(); i++)
    {
        Column->Layers[i].Depth += NewLayer.Thickness;
    }
}

// ===== CORE FUNCTIONS =====

void AGeologyController::UpdateGeologicalProcesses(float DeltaTime)
{
    if (!TargetTerrain || !WaterSystem) return;
    
    // Process different geological phenomena
    ProcessSedimentation(DeltaTime);
    ProcessWeathering(DeltaTime);
    ProcessErosion(DeltaTime);
    ProcessGroundwaterFlow(DeltaTime);
    UpdateAquiferPressure(DeltaTime);
}

void AGeologyController::UpdateWaterTable(float DeltaTime)
{
    if (!WaterSystem) return;
    
    for (FWaterTableData& WaterPoint : WaterTableGrid)
    {
        FVector SurfaceLocation = WaterPoint.Location;
        SurfaceLocation.Z = TargetTerrain->GetHeightAtPosition(WaterPoint.Location);
        
        float SurfaceWaterDepth = WaterSystem->GetWaterDepthAtPosition(SurfaceLocation);
        
        // Recharge from surface water
        if (SurfaceWaterDepth > 0.1f)
        {
            float Recharge = WaterTableRechargeRate * DeltaTime * SurfaceWaterDepth;
            WaterPoint.WaterLevel += Recharge * 0.1f;
        }
        
        // Natural drainage and flow
        WaterPoint.WaterLevel -= 0.01f * DeltaTime;
        
        // Calculate flow based on hydraulic gradient
        FVector FlowGradient = FVector::ZeroVector;
        for (const FWaterTableData& OtherPoint : WaterTableGrid)
        {
            if (&OtherPoint != &WaterPoint)
            {
                FVector ToOther = OtherPoint.Location - WaterPoint.Location;
                float Distance = ToOther.Size();
                if (Distance < 500.0f && Distance > 0.1f) // Consider nearby points
                {
                    float HeadDifference = OtherPoint.WaterLevel - WaterPoint.WaterLevel;
                    FlowGradient += (ToOther / Distance) * HeadDifference;
                }
            }
        }
        
        WaterPoint.FlowDirection = FlowGradient.GetSafeNormal();
        WaterPoint.FlowVelocity = FlowGradient.Size() * GroundwaterFlowRate;
        
        // Check for artesian conditions
        float PressureHead = WaterPoint.WaterLevel + ArtesianPressureThreshold;
        WaterPoint.bIsArtesian = PressureHead > SurfaceLocation.Z;
        
        // Create springs where water table intersects surface
        if (WaterPoint.WaterLevel > SurfaceLocation.Z - SpringFormationThreshold)
        {
            CreateSpring(SurfaceLocation);
        }
    }
}

void AGeologyController::FormSedimentLayer(FVector Location, float Thickness, ERockType RockType)
{
    FRockLayer NewLayer = CreateDefaultRockLayer(RockType);
    NewLayer.Thickness = Thickness;
    NewLayer.Depth = 0.0f; // New sediment forms at surface
    NewLayer.Age = 0.0f; // Brand new
    
    AddGeologicalLayer(Location, NewLayer);
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Formed %s sediment layer (%.1fm thick) at %s"),
           *UEnum::GetDisplayValueAsText(RockType).ToString(), Thickness, *Location.ToString());
}

void AGeologyController::ApplyMetamorphism(FVector Location, float Pressure, float Temperature)
{
    FGeologicalColumn* Column = GetColumnAtLocation(Location);
    if (!Column) return;
    
    for (FRockLayer& Layer : Column->Layers)
    {
        // Check if conditions are sufficient for metamorphism
        float MetamorphicThreshold = 200.0f; // Simplified
        if (Temperature > MetamorphicThreshold || Pressure > MetamorphicThreshold)
        {
            ERockType NewType = GetMetamorphicEquivalent(Layer.RockType);
            if (NewType != Layer.RockType)
            {
                UE_LOG(LogTemp, Log, TEXT("GeologyController: Metamorphism transformed %s to %s at %s"),
                       *UEnum::GetDisplayValueAsText(Layer.RockType).ToString(),
                       *UEnum::GetDisplayValueAsText(NewType).ToString(),
                       *Location.ToString());
                
                Layer.RockType = NewType;
                Layer.Hardness = FMath::Min(Layer.Hardness * 1.5f, 0.95f);
                Layer.Porosity *= 0.5f;
                Layer.Permeability *= 0.3f;
                Layer.ErosionResistance = Layer.Hardness;
            }
        }
    }
}

void AGeologyController::CreateFaultLine(FVector Start, FVector End, float Displacement)
{
    // Find all geological columns along the fault line
    FVector FaultDirection = (End - Start).GetSafeNormal();
    float FaultLength = FVector::Dist(Start, End);
    
    for (FGeologicalColumn& Column : GeologicalColumns)
    {
        // Check if column is near fault line
        FVector ToColumn = Column.Location - Start;
        float AlongFault = FVector::DotProduct(ToColumn, FaultDirection);
        
        if (AlongFault >= 0 && AlongFault <= FaultLength)
        {
            FVector NearestPointOnFault = Start + FaultDirection * AlongFault;
            float DistanceToFault = FVector::Dist(Column.Location, NearestPointOnFault);
            
            if (DistanceToFault < 100.0f) // Within fault influence zone
            {
                Column.bHasFaultLine = true;
                Column.FaultDisplacement = Displacement * (1.0f - DistanceToFault / 100.0f);
                
                // Offset layers based on fault displacement
                for (FRockLayer& Layer : Column.Layers)
                {
                    Layer.Depth += Column.FaultDisplacement;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Created fault line from %s to %s with %.1fm displacement"),
           *Start.ToString(), *End.ToString(), Displacement);
}

// ===== GEOLOGICAL QUERIES =====

ERockType AGeologyController::GetRockTypeAtLocation(FVector WorldLocation, float Depth) const
{
    const FGeologicalColumn* Column = GetColumnAtLocation(WorldLocation);
    
    if (Column)
    {
        // Find the layer at the specified depth
        for (const FRockLayer& Layer : Column->Layers)
        {
            if (Depth >= Layer.Depth && Depth < Layer.Depth + Layer.Thickness)
            {
                return Layer.RockType;
            }
        }
    }
    
    // If we're deeper than all layers, return bedrock
    return ERockType::Bedrock;
}

float AGeologyController::GetRockHardnessAtLocation(FVector WorldLocation, float Depth) const
{
    const FGeologicalColumn* Column = GetColumnAtLocation(WorldLocation);
    
    if (Column)
    {
        for (const FRockLayer& Layer : Column->Layers)
        {
            if (Depth >= Layer.Depth && Depth < Layer.Depth + Layer.Thickness)
            {
                return Layer.Hardness;
            }
        }
    }
    
    return 0.9f; // Default bedrock hardness
}

float AGeologyController::GetWaterTableDepthAtLocation(FVector WorldLocation) const
{
    // Find nearest water table point
    float MinDistance = FLT_MAX;
    float WaterLevel = 0.0f;
    
    for (const FWaterTableData& WaterPoint : WaterTableGrid)
    {
        float Distance = FVector::Dist2D(WorldLocation, WaterPoint.Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            WaterLevel = WaterPoint.WaterLevel;
        }
    }
    
    float SurfaceHeight = TargetTerrain ? TargetTerrain->GetHeightAtPosition(WorldLocation) : WorldLocation.Z;
    return SurfaceHeight - WaterLevel;
}

float AGeologyController::GetGroundwaterFlowAtLocation(FVector WorldLocation) const
{
    // Find nearest water table point
    float MinDistance = FLT_MAX;
    float FlowVelocity = 0.0f;
    
    for (const FWaterTableData& WaterPoint : WaterTableGrid)
    {
        float Distance = FVector::Dist2D(WorldLocation, WaterPoint.Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            FlowVelocity = WaterPoint.FlowVelocity;
        }
    }
    
    // Modify flow based on rock permeability at water table depth
    float WaterTableDepth = GetWaterTableDepthAtLocation(WorldLocation);
    ERockType RockType = GetRockTypeAtLocation(WorldLocation, WaterTableDepth);
    
    float PermeabilityFactor = 1.0f;
    switch (RockType)
    {
    case ERockType::Sand: PermeabilityFactor = 2.0f; break;
    case ERockType::Gravel: PermeabilityFactor = 3.0f; break;
    case ERockType::Clay: PermeabilityFactor = 0.1f; break;
    case ERockType::Granite: PermeabilityFactor = 0.05f; break;
    case ERockType::Limestone: PermeabilityFactor = 1.5f; break;
    case ERockType::Sandstone: PermeabilityFactor = 1.2f; break;
    case ERockType::Shale: PermeabilityFactor = 0.2f; break;
    default: break;
    }
    
    return FlowVelocity * PermeabilityFactor;
}

bool AGeologyController::IsLocationAboveWaterTable(FVector WorldLocation) const
{
    float WaterTableDepth = GetWaterTableDepthAtLocation(WorldLocation);
    float DepthBelowSurface = 0.0f; // Assuming we're checking at surface
    
    return DepthBelowSurface < WaterTableDepth;
}

TArray<FMineralDeposit> AGeologyController::GetNearbyMineralDeposits(FVector Location, float Radius) const
{
    TArray<FMineralDeposit> NearbyDeposits;
    
    for (const FMineralDeposit& Deposit : MineralDeposits)
    {
        if (FVector::Dist(Location, Deposit.Location) <= Radius)
        {
            NearbyDeposits.Add(Deposit);
        }
    }
    
    return NearbyDeposits;
}

// ===== SYSTEM COORDINATION =====

void AGeologyController::OnWaterFlowChanged(FVector Location, float FlowRate)
{
    // High flow rates increase erosion
    if (FlowRate > 1.0f)
    {
        float SurfaceDepth = 0.0f;
        ERockType SurfaceRockType = GetRockTypeAtLocation(Location, SurfaceDepth);
        float Hardness = GetRockHardnessAtLocation(Location, SurfaceDepth);
        float ErosionAmount = FlowRate * 0.1f * (1.0f - Hardness);
        
        // Trigger erosion event
        OnErosionOccurred(Location, ErosionAmount, SurfaceRockType);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Water erosion at %s, amount %.3f"),
               *Location.ToString(), ErosionAmount);
    }
}

void AGeologyController::OnErosionOccurred(FVector Location, float ErosionAmount, ERockType ErodedType)
{
    // Don't create sediment for tiny erosion amounts
    if (ErosionAmount < 0.01f) return;
    
    // Create sediment based on eroded rock type
    ERockType SedimentType = ERockType::Sand; // Default
    
    switch (ErodedType)
    {
    case ERockType::Granite:
        SedimentType = ERockType::Sand; // Granite weathers to sand
        break;
    case ERockType::Limestone:
        SedimentType = ERockType::Silt; // Limestone dissolves to fine particles
        break;
    case ERockType::Shale:
        SedimentType = ERockType::Clay; // Shale breaks down to clay
        break;
    case ERockType::Basalt:
        SedimentType = ERockType::Clay; // Basalt weathers to clay minerals
        break;
    default:
        SedimentType = ERockType::Sand;
        break;
    }
    
    // Find downstream location for deposition
    FVector FlowDirection = FVector(1, 0, -0.1f).GetSafeNormal(); // Simplified
    FVector DepositionLocation = Location + FlowDirection * 100.0f;
    
    // PERFORMANCE FIX: Accumulate sediment instead of creating layers immediately
    FIntVector GridKey = FIntVector(
        FMath::RoundToInt(DepositionLocation.X / 100.0f),
        FMath::RoundToInt(DepositionLocation.Y / 100.0f),
        0
    );
    
    float& AccumulatedAmount = AccumulatedSediment.FindOrAdd(GridKey);
    AccumulatedAmount += ErosionAmount * 0.1f;
    
    // Log accumulation instead of formation
    UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Accumulated %.3fm of %s sediment at grid %s"),
           ErosionAmount * 0.1f, *UEnum::GetDisplayValueAsText(SedimentType).ToString(), *GridKey.ToString());
}

void AGeologyController::OnTemperatureChanged(float NewTemperature)
{
    CurrentTemperature = NewTemperature;
    
    // Temperature affects weathering rates
    if (bEnableChemicalWeathering)
    {
        // Chemical weathering increases with temperature
        ChemicalWeatheringRate = 0.5f * (1.0f + (CurrentTemperature - 20.0f) / 40.0f);
        ChemicalWeatheringRate = FMath::Clamp(ChemicalWeatheringRate, 0.1f, 2.0f);
    }
    
    if (bEnableMechanicalWeathering)
    {
        // Frost wedging is most effective near freezing
        float FrostEffectiveness = 1.0f - FMath::Abs(CurrentTemperature) / 20.0f;
        FrostWedgingEffect = FMath::Max(0.0f, FrostEffectiveness) * 1.5f;
    }
}

void AGeologyController::SetSandDuneController(ASandDuneController* Controller)
{
    SandDuneController = Controller;
    
    if (SandDuneController)
    {
        // Initialize sand dune controller with our terrain reference
        SandDuneController->Initialize(TargetTerrain, this);
        
        UE_LOG(LogTemp, Warning, TEXT("GeologyController: Sand dune controller connected"));
    }
}

// ===== VISUALIZATION =====

void AGeologyController::ShowWaterTable(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing water table"));
    
    for (const FWaterTableData& WaterPoint : WaterTableGrid)
    {
        if (TargetTerrain)
        {
            FVector SurfacePoint = WaterPoint.Location;
            SurfacePoint.Z = TargetTerrain->GetHeightAtPosition(WaterPoint.Location);
            
            FVector WaterLevelPoint = WaterPoint.Location;
            WaterLevelPoint.Z = WaterPoint.WaterLevel;
            
            // Draw line from surface to water table
            FColor LineColor = WaterPoint.bIsArtesian ? FColor::Cyan : FColor::Blue;
            DrawDebugLine(GetWorld(), SurfacePoint, WaterLevelPoint, LineColor, false, 5.0f, 0, 2.0f);
            
            // Draw water level indicator
            DrawDebugSphere(GetWorld(), WaterLevelPoint, 25.0f, 8, LineColor, false, 5.0f);
            
            // Show flow direction
            if (WaterPoint.FlowVelocity > 0.01f)
            {
                FVector FlowEnd = WaterLevelPoint + WaterPoint.FlowDirection * 100.0f * WaterPoint.FlowVelocity;
                DrawDebugDirectionalArrow(GetWorld(), WaterLevelPoint, FlowEnd,
                                          20.0f, FColor::Yellow, false, 5.0f, 0, 2.0f);
            }
            
            // Mark artesian points
            if (WaterPoint.bIsArtesian)
            {
                DrawDebugString(GetWorld(), SurfacePoint + FVector(0, 0, 50),
                                TEXT("ARTESIAN"), nullptr, FColor::Cyan, 5.0f);
            }
        }
    }
}

void AGeologyController::ShowRockLayers(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing rock layers"));
    
    // Show geological columns as vertical cross-sections
    for (const FGeologicalColumn& Column : GeologicalColumns)
    {
        FVector ColumnBase = Column.Location;
        ColumnBase.Z = TargetTerrain ? TargetTerrain->GetHeightAtPosition(Column.Location) : Column.Location.Z;
        
        float CurrentZ = ColumnBase.Z;
        
        for (const FRockLayer& Layer : Column.Layers)
        {
            FVector LayerTop = ColumnBase;
            LayerTop.Z = CurrentZ;
            
            FVector LayerBottom = ColumnBase;
            LayerBottom.Z = CurrentZ - Layer.Thickness;
            
            // Draw layer boundaries
            FColor LayerColor = Layer.LayerColor.ToFColor(true);
            DrawDebugLine(GetWorld(), LayerTop, LayerBottom, LayerColor, false, 5.0f, 0, 5.0f);
            
            // Draw layer info
            FVector LabelPos = (LayerTop + LayerBottom) * 0.5f + FVector(50, 0, 0);
            FString LayerInfo = FString::Printf(TEXT("%s (%.1fm)"),
                *UEnum::GetDisplayValueAsText(Layer.RockType).ToString(), Layer.Thickness);
            DrawDebugString(GetWorld(), LabelPos, LayerInfo, nullptr, LayerColor, 5.0f);
            
            // Mark special features
            if (Layer.bContainsFossils)
            {
                DrawDebugSphere(GetWorld(), LabelPos - FVector(30, 0, 0), 10.0f, 6, FColor::White, false, 5.0f);
            }
            
            if (Layer.bHasMineralDeposits)
            {
                DrawDebugBox(GetWorld(), LabelPos - FVector(30, 30, 0), FVector(5), ColorGold, false, 5.0f);
            }
            
            CurrentZ = LayerBottom.Z;
            
            // Stop at bedrock depth
            if (CurrentZ < ColumnBase.Z - BedrockDepth)
                break;
        }
        
        // Show fault lines
        if (Column.bHasFaultLine)
        {
            FVector FaultTop = ColumnBase + FVector(0, 0, 100);
            FVector FaultBottom = ColumnBase - FVector(0, 0, Column.TotalDepth);
            DrawDebugLine(GetWorld(), FaultTop, FaultBottom, FColor::Red, false, 5.0f, 0, 3.0f);
            
            FString FaultInfo = FString::Printf(TEXT("FAULT: %.1fm"), Column.FaultDisplacement);
            DrawDebugString(GetWorld(), FaultTop + FVector(0, 0, 20), FaultInfo, nullptr, FColor::Red, 5.0f);
        }
    }
}

void AGeologyController::ShowMineralDeposits(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing mineral deposits"));
    
    for (const FMineralDeposit& Deposit : MineralDeposits)
    {
        FVector DepositLocation = Deposit.Location;
        if (TargetTerrain)
        {
            DepositLocation.Z = TargetTerrain->GetHeightAtPosition(Deposit.Location) - Deposit.Depth;
        }
        
        // Color based on mineral type
        FColor MineralColor = ColorGold; // Default
        if (Deposit.MineralType == "Iron") MineralColor = FColor::Red;
        else if (Deposit.MineralType == "Copper") MineralColor = ColorOrange;
        else if (Deposit.MineralType == "Silver") MineralColor = FColor::Silver;
        else if (Deposit.MineralType == "Coal") MineralColor = FColor::Black;
        
        // Draw deposit as a box
        float DepositSize = FMath::Pow(Deposit.Volume, 0.33f) * 2.0f;
        DrawDebugBox(GetWorld(), DepositLocation, FVector(DepositSize), MineralColor, false, 5.0f);
        
        // Show deposit info
        FString DepositInfo = FString::Printf(TEXT("%s\nVol: %.0fm³\nConc: %.0f%%"),
            *Deposit.MineralType, Deposit.Volume, Deposit.Concentration * 100.0f);
        DrawDebugString(GetWorld(), DepositLocation + FVector(0, 0, DepositSize + 20),
                        DepositInfo, nullptr, MineralColor, 5.0f);
    }
}

void AGeologyController::ShowGeologicalColumns(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GeologyController: Showing geological column locations"));
    
    for (const FGeologicalColumn& Column : GeologicalColumns)
    {
        FVector ColumnLocation = Column.Location;
        if (TargetTerrain)
        {
            ColumnLocation.Z = TargetTerrain->GetHeightAtPosition(Column.Location);
        }
        
        // Draw column marker
        DrawDebugCylinder(GetWorld(), ColumnLocation, ColumnLocation + FVector(0, 0, 100),
                          20.0f, 8, FColor::Green, false, 5.0f);
        
        // Show total depth
        FString ColumnInfo = FString::Printf(TEXT("Depth: %.1fm\nLayers: %d"),
            Column.TotalDepth, Column.Layers.Num());
        DrawDebugString(GetWorld(), ColumnLocation + FVector(0, 0, 120),
                        ColumnInfo, nullptr, FColor::Green, 5.0f);
    }
}

void AGeologyController::PrintGeologicalStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== GEOLOGICAL STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Geological Columns: %d"), GeologicalColumns.Num());
    UE_LOG(LogTemp, Warning, TEXT("Water Table Points: %d"), WaterTableGrid.Num());
    UE_LOG(LogTemp, Warning, TEXT("Mineral Deposits: %d"), MineralDeposits.Num());
    UE_LOG(LogTemp, Warning, TEXT("Average Water Table Depth: %.1fm"), AverageWaterTableDepth);
    UE_LOG(LogTemp, Warning, TEXT("Current Temperature: %.1f°C"), CurrentTemperature);
    UE_LOG(LogTemp, Warning, TEXT("Bedrock Depth: %.1fm"), BedrockDepth);
    
    // Count rock types
    TMap<ERockType, int32> RockTypeCounts;
    for (const FGeologicalColumn& Column : GeologicalColumns)
    {
        for (const FRockLayer& Layer : Column.Layers)
        {
            RockTypeCounts.FindOrAdd(Layer.RockType)++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Rock Type Distribution:"));
    for (const auto& Pair : RockTypeCounts)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %d layers"),
               *UEnum::GetDisplayValueAsText(Pair.Key).ToString(), Pair.Value);
    }
}

// ===== ISCALABLESYSTEM INTERFACE =====

void AGeologyController::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuring from master controller"));
    
    CurrentWorldConfig = Config;
    
    // Scale geological features based on world size
    float WorldScale = Config.TerrainScale;
    
    // Adjust water table grid
    WaterTableGrid.Empty();
    
    int32 GridSize = FMath::Max(10, Config.GeologyConfig.ErosionGridWidth / 4);
    float GridSpacing = Config.GeologyConfig.ErosionCellSize * 4.0f;
    
    FVector GridOrigin = CurrentCoordinateSystem.WorldOrigin;
    
    for (int32 x = 0; x < GridSize; x++)
    {
        for (int32 y = 0; y < GridSize; y++)
        {
            FWaterTableData WaterPoint;
            WaterPoint.Location = GridOrigin + FVector(
                (x - GridSize/2) * GridSpacing,
                (y - GridSize/2) * GridSpacing,
                0.0f
            );
            
            if (TargetTerrain)
            {
                float SurfaceHeight = TargetTerrain->GetHeightAtPosition(WaterPoint.Location);
                WaterPoint.WaterLevel = SurfaceHeight - AverageWaterTableDepth;
            }
            else
            {
                WaterPoint.WaterLevel = CurrentCoordinateSystem.WorldOrigin.Z - AverageWaterTableDepth;
            }
            
            WaterTableGrid.Add(WaterPoint);
        }
    }
    
    // Generate geological columns across the scaled world
    int32 NumColumns = FMath::Max(4, GridSize / 2);
    GeologicalColumns.Empty();
    
    for (int32 i = 0; i < NumColumns; i++)
    {
        for (int32 j = 0; j < NumColumns; j++)
        {
            FVector ColumnLocation = GridOrigin + FVector(
                (i - NumColumns/2) * GridSpacing * 2,
                (j - NumColumns/2) * GridSpacing * 2,
                0.0f
            );
            
            GenerateStratigraphicColumn(ColumnLocation);
        }
    }
    
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[GEOLOGY SCALING] Configuration complete - Water table: %d points, Columns: %d"),
           WaterTableGrid.Num(), GeologicalColumns.Num());
}

void AGeologyController::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[GEOLOGY SCALING] Synchronizing coordinates"));
    
    CurrentCoordinateSystem = Coords;
    
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
    DebugInfo += FString::Printf(TEXT("  - Water Table Grid: %d points\n"), WaterTableGrid.Num());
    DebugInfo += FString::Printf(TEXT("  - Geological Columns: %d\n"), GeologicalColumns.Num());
    DebugInfo += FString::Printf(TEXT("  - Mineral Deposits: %d\n"), MineralDeposits.Num());
    return DebugInfo;
}

// ===== INTERNAL FUNCTIONS =====

void AGeologyController::InitializeGeologicalColumns()
{
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Initializing geological columns"));
    
    // Columns are generated during ConfigureFromMaster or Initialize
}

void AGeologyController::ProcessSedimentation(float DeltaTime)
{
    if (!WaterSystem) return;
    
    // Sedimentation occurs where water velocity is low
    float SedimentAmount = SedimentationRate * DeltaTime;
    
    if (SedimentAmount > 0.001f)
    {
        // Check water velocities across terrain
        // In areas with low flow, deposit sediment
        // This is simplified - real implementation would check flow field
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Processing sedimentation (%.3f m)"),
               SedimentAmount);
    }
}

void AGeologyController::ProcessWeathering(float DeltaTime)
{
    if (bEnableChemicalWeathering)
    {
        float ChemicalAmount = ChemicalWeatheringRate * DeltaTime;
        
        // Chemical weathering is enhanced by:
        // - Higher temperatures
        // - Lower pH (acid rain)
        // - Presence of water
        
        float AcidityFactor = 1.0f + (7.0f - AcidRainPH) * 0.2f;
        ChemicalAmount *= AcidityFactor;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Chemical weathering active (rate: %.3f)"),
               ChemicalAmount);
    }
    
    if (bEnableMechanicalWeathering)
    {
        float MechanicalAmount = MechanicalWeatheringRate * DeltaTime;
        
        // Mechanical weathering enhanced by:
        // - Freeze-thaw cycles
        // - Wind abrasion
        // - Root growth (not implemented yet)
        
        if (CurrentTemperature < 5.0f && CurrentTemperature > -5.0f)
        {
            MechanicalAmount *= FrostWedgingEffect;
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("GeologyController: Mechanical weathering active (rate: %.3f)"),
               MechanicalAmount);
    }
}

void AGeologyController::ProcessErosion(float DeltaTime)
{
    // Erosion is handled by external systems (water, wind)
    // This function processes the geological consequences
    
    // Update rock layers based on cumulative erosion
    // This would modify the terrain heightmap in a full implementation
}

void AGeologyController::ProcessGroundwaterFlow(float DeltaTime)
{
    if (!bEnableWaterTable) return;
    
    // Darcy's law for groundwater flow
    for (FWaterTableData& WaterPoint : WaterTableGrid)
    {
        // Flow is driven by hydraulic gradient
        // Modified by rock permeability
        
        float LocalPermeability = 1.0f;
        float WaterDepth = GetWaterTableDepthAtLocation(WaterPoint.Location);
        ERockType LocalRock = GetRockTypeAtLocation(WaterPoint.Location, WaterDepth);
        
        // Get permeability from rock type
        const FGeologicalColumn* Column = GetColumnAtLocation(WaterPoint.Location);
        if (Column)
        {
            for (const FRockLayer& Layer : Column->Layers)
            {
                if (Layer.RockType == LocalRock)
                {
                    LocalPermeability = Layer.Permeability;
                    break;
                }
            }
        }
        
        WaterPoint.FlowVelocity *= LocalPermeability;
    }
}

void AGeologyController::UpdateAquiferPressure(float DeltaTime)
{
    // Check for confined aquifers and artesian conditions
    for (FWaterTableData& WaterPoint : WaterTableGrid)
    {
        float WaterDepth = GetWaterTableDepthAtLocation(WaterPoint.Location);
        
        // Check if there's an impermeable layer above the water table
        bool bIsConfined = false;
        const FGeologicalColumn* Column = GetColumnAtLocation(WaterPoint.Location);
        
        if (Column)
        {
            for (const FRockLayer& Layer : Column->Layers)
            {
                if (Layer.Depth < WaterDepth && Layer.Permeability < 0.1f)
                {
                    bIsConfined = true;
                    break;
                }
            }
        }
        
        if (bIsConfined)
        {
            // Calculate pressure head
            float RechargeArea = WaterPoint.Location.Z + 100.0f; // Simplified
            float PressureHead = RechargeArea - WaterPoint.WaterLevel;
            
            if (PressureHead > ArtesianPressureThreshold)
            {
                WaterPoint.bIsArtesian = true;
            }
        }
    }
}

void AGeologyController::CreateSpring(FVector Location)
{
    // Springs form where water table intersects surface
    // This would create a water source in the water system
    
    if (WaterSystem)
    {
        // Add a small water source at this location
        // WaterSystem would handle the actual water spawning
        
        UE_LOG(LogTemp, Log, TEXT("GeologyController: Spring formed at %s"), *Location.ToString());
    }
}

void AGeologyController::GenerateMineralDeposit(FVector Location, ERockType HostRock)
{
    FMineralDeposit NewDeposit;
    NewDeposit.Location = Location;
    
    // Mineral type depends on host rock and geological processes
    switch (HostRock)
    {
    case ERockType::Granite:
        NewDeposit.MineralType = FMath::RandBool() ? "Tin" : "Tungsten";
        NewDeposit.Concentration = FMath::RandRange(0.3f, 0.7f);
        break;
        
    case ERockType::Basalt:
        NewDeposit.MineralType = FMath::RandBool() ? "Copper" : "Nickel";
        NewDeposit.Concentration = FMath::RandRange(0.4f, 0.8f);
        break;
        
    case ERockType::Limestone:
        NewDeposit.MineralType = FMath::RandBool() ? "Lead" : "Zinc";
        NewDeposit.Concentration = FMath::RandRange(0.2f, 0.6f);
        break;
        
    case ERockType::Sandstone:
        NewDeposit.MineralType = "Uranium"; // Roll-front deposits
        NewDeposit.Concentration = FMath::RandRange(0.1f, 0.3f);
        break;
        
    case ERockType::Shale:
        NewDeposit.MineralType = FMath::RandBool() ? "Oil" : "Natural Gas";
        NewDeposit.Concentration = FMath::RandRange(0.5f, 0.9f);
        break;
        
    default:
        NewDeposit.MineralType = "Iron";
        NewDeposit.Concentration = FMath::RandRange(0.3f, 0.6f);
        break;
    }
    
    NewDeposit.Volume = FMath::RandRange(500.0f, 5000.0f);
    NewDeposit.Depth = FMath::RandRange(20.0f, 200.0f);
    
    MineralDeposits.Add(NewDeposit);
    
    UE_LOG(LogTemp, Log, TEXT("GeologyController: Generated %s deposit at %s (%.0f%% concentration)"),
           *NewDeposit.MineralType, *Location.ToString(), NewDeposit.Concentration * 100.0f);
}

FRockLayer AGeologyController::CreateDefaultRockLayer(ERockType Type) const
{
    FRockLayer Layer;
    Layer.RockType = Type;
    
    // Set properties based on rock type
    switch (Type)
    {
    case ERockType::Granite:
        Layer.Hardness = 0.9f;
        Layer.Porosity = 0.02f;
        Layer.Permeability = 0.01f;
        Layer.ErosionResistance = 0.95f;
        Layer.LayerColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
        break;
        
    case ERockType::Sandstone:
        Layer.Hardness = 0.6f;
        Layer.Porosity = 0.2f;
        Layer.Permeability = 0.4f;
        Layer.ErosionResistance = 0.5f;
        Layer.LayerColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
        break;
        
    case ERockType::Limestone:
        Layer.Hardness = 0.7f;
        Layer.Porosity = 0.15f;
        Layer.Permeability = 0.3f;
        Layer.ErosionResistance = 0.4f; // Susceptible to chemical weathering
        Layer.LayerColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
        break;
        
    case ERockType::Shale:
        Layer.Hardness = 0.5f;
        Layer.Porosity = 0.1f;
        Layer.Permeability = 0.05f;
        Layer.ErosionResistance = 0.3f;
        Layer.LayerColor = FLinearColor(0.4f, 0.4f, 0.35f, 1.0f);
        break;
        
    case ERockType::Basalt:
        Layer.Hardness = 0.85f;
        Layer.Porosity = 0.1f;
        Layer.Permeability = 0.15f;
        Layer.ErosionResistance = 0.8f;
        Layer.LayerColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
        break;
        
    case ERockType::Marble:
        Layer.Hardness = 0.75f;
        Layer.Porosity = 0.05f;
        Layer.Permeability = 0.02f;
        Layer.ErosionResistance = 0.6f;
        Layer.LayerColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.0f);
        break;
        
    case ERockType::Quartzite:
        Layer.Hardness = 0.95f;
        Layer.Porosity = 0.02f;
        Layer.Permeability = 0.01f;
        Layer.ErosionResistance = 0.98f;
        Layer.LayerColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
        break;
        
    case ERockType::Clay:
        Layer.Hardness = 0.3f;
        Layer.Porosity = 0.4f;
        Layer.Permeability = 0.01f; // High porosity but low permeability
        Layer.ErosionResistance = 0.2f;
        Layer.LayerColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
        break;
        
    case ERockType::Sand:
        Layer.Hardness = 0.2f;
        Layer.Porosity = 0.4f;
        Layer.Permeability = 0.9f;
        Layer.ErosionResistance = 0.1f;
        Layer.LayerColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        break;
        
    case ERockType::Silt:
        Layer.Hardness = 0.25f;
        Layer.Porosity = 0.35f;
        Layer.Permeability = 0.3f;
        Layer.ErosionResistance = 0.15f;
        Layer.LayerColor = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
        break;
        
    case ERockType::Gravel:
        Layer.Hardness = 0.4f;
        Layer.Porosity = 0.3f;
        Layer.Permeability = 0.95f;
        Layer.ErosionResistance = 0.3f;
        Layer.LayerColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
        break;
        
    default:
        Layer.Hardness = 0.9f;
        Layer.Porosity = 0.05f;
        Layer.Permeability = 0.05f;
        Layer.ErosionResistance = 0.9f;
        Layer.LayerColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
        break;
    }
    
    Layer.Thickness = 10.0f; // Default
    Layer.Age = 100.0f; // Default 100 million years
    
    return Layer;
}

float AGeologyController::CalculateErosionResistance(const FRockLayer& Layer) const
{
    float Resistance = Layer.ErosionResistance;
    
    // Modify based on environmental factors
    if (bEnableChemicalWeathering && Layer.RockType == ERockType::Limestone)
    {
        // Limestone is vulnerable to acid rain
        Resistance *= (AcidRainPH / 7.0f);
    }
    
    if (bEnableMechanicalWeathering)
    {
        // Fractured rock erodes faster
        Resistance *= (1.0f - Layer.Porosity * 0.5f);
    }
    
    return FMath::Clamp(Resistance, 0.1f, 1.0f);
}

ERockType AGeologyController::GetMetamorphicEquivalent(ERockType Original) const
{
    // Metamorphic rock transformations
    switch (Original)
    {
    case ERockType::Limestone:
        return ERockType::Marble;
        
    case ERockType::Sandstone:
        return ERockType::Quartzite;
        
    case ERockType::Shale:
        return ERockType::Granite; // Simplified - would be slate/schist/gneiss
        
    case ERockType::Clay:
        return ERockType::Shale; // Low-grade metamorphism
        
    default:
        return Original; // No change
    }
}

void AGeologyController::ProcessAccumulatedSediment()
{
    // Only process if we have accumulated sediment
    if (AccumulatedSediment.Num() == 0) return;
    
    int32 LayersFormed = 0;
    TArray<FIntVector> KeysToRemove;
    
    for (auto& Pair : AccumulatedSediment)
    {
        if (Pair.Value >= MinimumSedimentThickness)
        {
            // Convert grid key back to world location
            FVector WorldLocation = FVector(
                Pair.Key.X * 100.0f,
                Pair.Key.Y * 100.0f,
                0.0f
            );
            
            // Adjust Z to terrain height
            if (TargetTerrain)
            {
                WorldLocation.Z = TargetTerrain->GetHeightAtPosition(WorldLocation);
            }
            
            // Form the sediment layer
            FormSedimentLayer(WorldLocation, Pair.Value, ERockType::Sand);
            LayersFormed++;
            
            // Mark for removal
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    // Remove formed sediment
    for (const FIntVector& Key : KeysToRemove)
    {
        AccumulatedSediment.Remove(Key);
    }
    
    if (LayersFormed > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GeologyController: Formed %d sediment layers from accumulated material"), LayersFormed);
    }
}
