/**
 * ============================================
 * DRIFT GEOLOGY CONTROLLER - IMPLEMENTATION
 * ============================================
 * Reorganized: December 2025
 * Lines: ~1,100 | Functions: ~40
 *
 * PURPOSE:
 * Simplified geology system managing groundwater, water table,
 * rock types, and user-placeable springs for Drift watershed simulator.
 *
 * KEY CAPABILITIES:
 * - Global water table elevation tracking
 * - Groundwater volume conservation (MasterController authority)
 * - User-placeable springs with flow rate control
 * - Rock type classification for infiltration rates
 * - Hydraulic conductivity calculations
 *
 * ARCHITECTURE:
 * - AActor implementing IScalableSystem interface
 * - Tick disabled - updates via MasterController/TemporalManager
 * - Authority pattern: MasterController owns groundwater budget
 *
 * TABLE OF CONTENTS:
 *
 * SECTION 1: INCLUDES & CONSTRUCTOR (~20 lines)
 * SECTION 2: ACTOR LIFECYCLE (~60 lines)
 * SECTION 3: INITIALIZATION (~200 lines)
 * SECTION 4: WATER TABLE MANAGEMENT (~150 lines)
 * SECTION 5: CORE UPDATE (~20 lines)
 * SECTION 6: QUERIES (~120 lines)
 * SECTION 7: SOIL & INFILTRATION (~80 lines)
 * SECTION 8: USER SPRING SYSTEM (~100 lines)
 * SECTION 9: ISCALABLESYSTEM INTERFACE (~40 lines)
 * SECTION 10: MASTERCONTROLLER INTEGRATION (~40 lines)
 * SECTION 11: VISUALIZATION & DEBUG (~100 lines)
 * SECTION 12: GRID UTILITIES (~80 lines)
 *
 * DEPENDENCIES:
 * - ADynamicTerrain: Height queries, coordinate transforms
 * - UWaterSystem: Surface water for spring output
 * - AMasterWorldController: Groundwater budget authority
 */

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


// ============================================================================
// SECTION 1: INCLUDES & CONSTRUCTOR
// ============================================================================

AGeologyController::AGeologyController()
{
    PrimaryActorTick.bCanEverTick = false; // from true to false for MasterController Authority
    // PrimaryActorTick.TickInterval = 1.0f; // Update every second commented out for MasterController Authority
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GeologyRoot"));
}


// ============================================================================
// SECTION 2: ACTOR LIFECYCLE
// ============================================================================

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

void AGeologyController::ResetGeologySystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RESETTING GEOLOGY SYSTEM ==="));
    
    // 1. Clear all user-placed springs (CRITICAL - this was missing!)
    ClearAllUserSprings();
    
    // 2. Reset water table initialization flag
    bWaterTableInitialized = false;
    
    // 3. Reset geology grid hydraulic heads
    if (GeologyGrid.Num() > 0)
    {
        for (FSimplifiedGeology& Cell : GeologyGrid)
        {
            Cell.HydraulicHead = GlobalWaterTableElevation;
            // Keep LastTerrainHeight - it will update naturally
        }
        UE_LOG(LogTemp, Warning, TEXT("  → Reset %d geology cells"), GeologyGrid.Num());
    }
    
    // 4. Clear debug visualization if active
    if (WaterTableDebugMesh && WaterTableDebugMesh->IsVisible())
    {
        WaterTableDebugMesh->ClearAllMeshSections();
        WaterTableDebugMesh->SetVisibility(false);
    }
    
    // 5. Reinitialize water table with fresh terrain
    // This recalculates GlobalWaterTableElevation based on new terrain heights
    InitializeWaterTable();
    
    // 6. Reset groundwater reservoir through MasterController
    if (MasterController)
    {
        // MasterController handles its own groundwater reset,
        // but we can trigger a volume update here if needed
        MasterController->ResetWaterBudget();
        UE_LOG(LogTemp, Warning, TEXT("  → Groundwater reservoir reset via MasterController"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✓ Geology system reset complete"));
}

// ============================================================================
// SECTION 3: INITIALIZATION
// ============================================================================

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
            WorldArea = 1000000.0f; // 1kmÂ² default
        }
    }
    
    float AverageDepth = 100.0f; // Assume 100m average saturated thickness
    GlobalWaterTableVolume = WorldArea * AverageDepth * GlobalPorosity;
    
    // Validate the calculated volume
    if (!FMath::IsFinite(GlobalWaterTableVolume) || GlobalWaterTableVolume <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid water table volume calculated: %.2f - using default"), GlobalWaterTableVolume);
        GlobalWaterTableVolume = 1000000.0f; // Default 1 million mÂ³
    }
    
    // Register initial groundwater with master controller
    if (MasterController)
    {
        MasterController->SetInitialGroundwater(GlobalWaterTableVolume);
    }
    
    // CRITICAL: Initialize seepage points immediately
   /* if (TargetTerrain && WaterSystem)
    {
        UpdateActiveSeepagePoints();
        bWaterTableInitialized = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Water table initialized at %.1fm with %d seepage points"),
               GlobalWaterTableElevation, ActiveSeepagePoints.Num());
    }*/
}

void AGeologyController::InitializeWaterTableFill()
{
    if (bWaterTableInitialized || !TargetTerrain || !WaterSystem)
    {
        return;
    }
    
    // Initialize the seepage system
    //UpdateActiveSeepagePoints();
    bWaterTableInitialized = true;
    
   /* UE_LOG(LogTemp, Warning, TEXT("Water table initialized with %d seepage points"),
           ActiveSeepagePoints.Num());*/
}


// ============================================================================
// SECTION 4: WATER TABLE MANAGEMENT
// ============================================================================

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
   /* if (VolumeM3 > 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Added %.1f mÂ³ to water table, new elevation: %.2f m"),
            VolumeM3, GlobalWaterTableElevation);
    }  HELPFUL LOGSPAM */
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

void AGeologyController::CheckInitialWaterTable()
{
    if (!TargetTerrain || !WaterSystem) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Initializing water table at elevation %.1fm"),
           GlobalWaterTableElevation);
    
    // Simply update the active seepage points
    //UpdateActiveSeepagePoints();
    
    // The hydraulic seepage system will handle filling naturally
    bWaterTableInitialized = true;
}

void AGeologyController::OnWaterTableElevationChanged()
{
    CheckInitialWaterTable();
}

void AGeologyController::SetWaterTableDepth(FVector Location, float Depth)
{
    // This function now sets the global water table elevation based on the location's terrain height
    if (!TargetTerrain) return;
    
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(Location);
    SetGlobalWaterTableElevation(TerrainHeight - Depth);
}


// ============================================================================
// SECTION 5: CORE UPDATE
// ============================================================================

void AGeologyController::UpdateGeologySystem(float DeltaTime)
{
    // SAFETY CHECK: Prevent updates when paused or invalid time
    if (DeltaTime <= 0.0f || !TargetTerrain || !WaterSystem)
    {
        return;
    }
    
    // Process soil moisture drainage and evapotranspiration
    // This is the critical intermediate layer between surface water and groundwater
    ProcessSoilMoistureTick(DeltaTime);

    // Process user-created springs
    ProcessUserSprings(DeltaTime);

    // Update debug visualization
    UpdateWaterTableDebugVisualization();
}


// ============================================================================
// SECTION 5.5: SOIL MOISTURE LAYER
// ============================================================================
// The critical intermediate buffer between surface water and groundwater.
// Water infiltrates into soil, sits there for plants to use, then slowly
// drains to the water table. This creates realistic moisture persistence.

void AGeologyController::ProcessSoilMoistureTick(float DeltaTime)
{
    if (GeologyGrid.Num() == 0 || !MasterController) return;

    float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
    float TotalDrainageVolume = 0.0f;
    float TotalEvapVolume = 0.0f;

    // Process each cell's soil moisture
    for (int32 i = 0; i < GeologyGrid.Num(); i++)
    {
        FSimplifiedGeology& Cell = GeologyGrid[i];

        if (Cell.SoilMoisture <= 0.0f) continue;

        // Get soil capacity for this rock type
        float SoilCapacity = GetSoilCapacity(Cell.SurfaceRock);
        float CurrentWaterVolume = Cell.SoilMoisture * SoilCapacity * CellArea;

        // 1. DRAINAGE TO WATER TABLE (slow, permeability-dependent)
        // More permeable soils drain faster
        float EffectiveDrainageRate = SoilDrainageRate * Cell.Permeability;
        float DrainageAmount = Cell.SoilMoisture * EffectiveDrainageRate * DeltaTime;
        DrainageAmount = FMath::Min(DrainageAmount, Cell.SoilMoisture);

        // 2. EVAPOTRANSPIRATION (moisture returns to atmosphere)
        // TODO: Modulate by temperature from AtmosphericSystem
        // TODO: Modulate by vegetation density from EcosystemManager
        float EvapAmount = Cell.SoilMoisture * SoilEvapotranspirationRate * DeltaTime;
        EvapAmount = FMath::Min(EvapAmount, Cell.SoilMoisture - DrainageAmount);

        // Apply changes
        Cell.SoilMoisture -= (DrainageAmount + EvapAmount);
        Cell.SoilMoisture = FMath::Max(0.0f, Cell.SoilMoisture);

        // Track volumes for water budget
        float DrainageVolume = DrainageAmount * SoilCapacity * CellArea;
        float EvapVolume = EvapAmount * SoilCapacity * CellArea;

        TotalDrainageVolume += DrainageVolume;
        TotalEvapVolume += EvapVolume;
    }

    // Transfer drained water to water table (conserved)
    if (TotalDrainageVolume > 0.0f)
    {
        AddWaterToWaterTable(TotalDrainageVolume);
        TotalSoilMoistureVolume -= TotalDrainageVolume;
    }

    // Transfer evaporated water to atmosphere (conserved)
    if (TotalEvapVolume > 0.0f && MasterController)
    {
        FVector CenterLocation = TargetTerrain ? TargetTerrain->GetActorLocation() : FVector::ZeroVector;
        MasterController->TransferSurfaceToAtmosphere(CenterLocation, TotalEvapVolume);
        TotalSoilMoistureVolume -= TotalEvapVolume;
    }

    // Ensure tracking stays non-negative
    TotalSoilMoistureVolume = FMath::Max(0.0f, TotalSoilMoistureVolume);
}

void AGeologyController::AddWaterToSoilMoisture(FVector Location, float VolumeM3)
{
    if (VolumeM3 <= 0.0f || !MasterController) return;

    FVector2D Coords = WorldToGridCoordinates(Location);
    int32 X = FMath::FloorToInt(Coords.X);
    int32 Y = FMath::FloorToInt(Coords.Y);

    if (!IsValidGridCoordinate(X, Y)) return;

    int32 Index = GetGridIndex(X, Y);
    FSimplifiedGeology& Cell = GeologyGrid[Index];

    // Calculate how much this cell can absorb
    float SoilCapacity = GetSoilCapacity(Cell.SurfaceRock);
    float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
    float MaxCellVolume = Cell.StorageCoefficient * SoilCapacity * CellArea;
    float CurrentVolume = Cell.SoilMoisture * SoilCapacity * CellArea;
    float AvailableSpace = MaxCellVolume - CurrentVolume;

    // Absorb what we can
    float AbsorbedVolume = FMath::Min(VolumeM3, AvailableSpace);
    if (AbsorbedVolume > 0.0f)
    {
        Cell.SoilMoisture += AbsorbedVolume / (SoilCapacity * CellArea);
        Cell.SoilMoisture = FMath::Clamp(Cell.SoilMoisture, 0.0f, 1.0f);
        TotalSoilMoistureVolume += AbsorbedVolume;
    }

    // IMPORTANT: Excess stays as surface water - it does NOT go directly to groundwater!
    // The caller (WaterSystem) should handle excess by keeping it on the surface
    // This prevents the "water disappearing too fast" problem

    // Log for debugging water budget
    if (VolumeM3 > AbsorbedVolume)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[SOIL MOISTURE] Absorbed %.4f of %.4f m³ - soil saturated at %s"),
            AbsorbedVolume, VolumeM3, *Location.ToString());
    }
}


// ============================================================================
// SECTION 6: QUERIES
// ============================================================================

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

float AGeologyController::GetHydraulicConductivityAt(FVector WorldPos) const
{
    ERockType RockType = GetRockTypeAtLocation(WorldPos, 0.0f);
    
    switch (RockType)
    {
        case ERockType::Clay:      return 0.000001f;   // Very slow
        case ERockType::Silt:      return 0.00001f;
        case ERockType::Sand:      return 0.001f;      // Moderate
        case ERockType::Gravel:    return 0.01f;       // Fast
        case ERockType::Limestone: return 0.0001f;
        case ERockType::Granite:   return 0.0000001f;  // Nearly impermeable
        case ERockType::Sandstone: return 0.00001f;
        case ERockType::Shale:     return 0.00000001f; // Extremely slow
        default:                   return BaseHydraulicConductivity;
    }
}

float AGeologyController::GetWaterTableDepthAtLocation(FVector Location) const
{
    // Simple: depth below surface at this location
    if (!TargetTerrain) return 0.0f;
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(Location);
    return TerrainHeight - GlobalWaterTableElevation;
}

bool AGeologyController::IsLocationAboveWaterTable(FVector Location) const
{
    float WaterTableDepth = GetWaterTableDepthAtLocation(Location);
    return WaterTableDepth > 0.0f;
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


// ============================================================================
// SECTION 7: SOIL & INFILTRATION
// ============================================================================

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

void AGeologyController::ApplyInfiltration(FVector Location, float WaterAmount)
{
    // REDESIGNED: Now uses the soil moisture layer properly
    // Water goes into soil moisture buffer, NOT directly to groundwater
    // The soil moisture tick handles slow drainage to water table over time
    // This creates the "residence time" that plants need!

    AddWaterToSoilMoisture(Location, WaterAmount);

    // NOTE: If soil is saturated, excess water stays on surface (handled by caller)
    // This is intentional - prevents water from "disappearing" too fast
}


// ============================================================================
// SECTION 8: USER SPRING SYSTEM
// ============================================================================

void AGeologyController::AddUserSpring(FVector WorldLocation, float FlowRate)
{
    FUserSpring NewSpring;
    NewSpring.Location = WorldLocation;
    NewSpring.FlowRate = (FlowRate > 0.0f) ? FlowRate : DefaultSpringFlowRate;
    NewSpring.bActive = true;
    
    UserSprings.Add(NewSpring);
    
    UE_LOG(LogTemp, Warning, TEXT("Added user spring at %s with flow rate %.2f mÂ³/s"),
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
    for (FUserSpring& Spring : UserSprings)
    {
        if (!Spring.bActive) continue;
        
        float VolumeToAdd = Spring.FlowRate * DeltaTime;
        
        if (MasterController->CanGroundwaterEmerge(VolumeToAdd))
        {
            // CHANGED: Much tighter fountain radius - springs should be focused
            // Small spring (0.1 m³/s) = 2 cell radius
            // Large spring (10 m³/s) = 8 cell radius (was 25!)
            float FountainRadius = FMath::Lerp(2.0f, 8.0f,
                FMath::Clamp(Spring.FlowRate / 10.0f, 0.0f, 1.0f));
            
            // Convert to terrain coordinates
            FVector2D TerrainCoords = MasterController->WorldToTerrainCoordinates(Spring.Location);
            int32 X = FMath::FloorToInt(TerrainCoords.X);
            int32 Y = FMath::FloorToInt(TerrainCoords.Y);
            
            // Add water as focused fountain
            if (WaterSystem && WaterSystem->IsSystemReady())
            {
                float CellArea = MasterController->GetTerrainScale() * MasterController->GetTerrainScale();
                float DepthToAdd = VolumeToAdd / CellArea;
                
                WaterSystem->AddWaterInRadius(X, Y, FountainRadius, DepthToAdd);
            }
            
            // Deduct from groundwater
            MasterController->TotalGroundwater -= VolumeToAdd;
            RemoveWaterFromWaterTable(VolumeToAdd);
        }
    }
}


// ============================================================================
// SECTION 9: ISCALABLESYSTEM INTERFACE
// ============================================================================

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


// ============================================================================
// SECTION 10: MASTERCONTROLLER INTEGRATION
// ============================================================================

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


// ============================================================================
// SECTION 11: VISUALIZATION & DEBUG
// ============================================================================

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


// ============================================================================
// SECTION 12: GRID UTILITIES
// ============================================================================

FVector2D AGeologyController::WorldToGridCoordinates(const FVector& WorldPosition) const
{
    // âœ… Use SAME coordinate authority as everyone else
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

bool AGeologyController::IsEdgeCell(int32 X, int32 Y) const
{
    return X == 0 || X == GeologyGridWidth - 1 || Y == 0 || Y == GeologyGridHeight - 1;
}

float AGeologyController::GetTotalWorldArea() const
{
    if (!MasterController) return 1000000.0f;  // Default 1 kmÂ²
    
    FVector2D WorldDims = MasterController->GetWorldDimensions();
    float TerrainScale = MasterController->GetTerrainScale();
    
    return WorldDims.X * WorldDims.Y * TerrainScale * TerrainScale;
}


// ============================================================================
// END OF GEOLOGYCONTROLLER.CPP
// ============================================================================
/**
 * REORGANIZATION SUMMARY:
 * - Lines: ~1,100
 * - Functions: ~40
 * - Sections: 12
 *
 * CRITICAL SECTIONS:
 * - Section 3: Initialization (SYSTEM STARTUP)
 * - Section 4: Water Table Management (GROUNDWATER AUTHORITY)
 * - Section 8: User Spring System (PLAYER INTERACTION)
 *
 * AUTHORITY INTEGRATION:
 * - MasterController: Groundwater volume tracking
 * - DynamicTerrain: Height queries
 * - WaterSystem: Spring water delivery
 */

// End of file
