/**
 * WaterController.cpp - Complete Implementation with Niagara FX
 */
#include "WaterController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "MasterController.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AWaterController::AWaterController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AWaterController::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find terrain if not set
    if (!TargetTerrain)
    {
        TargetTerrain = Cast<ADynamicTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(), ADynamicTerrain::StaticClass()));
        if (TargetTerrain)
        {
            UE_LOG(LogTemp, Warning, TEXT("WaterController: Auto-found terrain"));
        }
    }
    
    if (TargetTerrain)
    {
        Initialize(TargetTerrain);
        UE_LOG(LogTemp, Warning, TEXT("WaterController: Initialized with terrain"));
        
        // Apply Blueprint settings that may have been set before BeginPlay
        if (bIsRaining)
        {
            UE_LOG(LogTemp, Warning, TEXT("WaterController: Blueprint rain setting detected - starting rain"));
            StartRain(RainIntensity);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterController: No terrain assigned - set TargetTerrain in Blueprint"));
    }
    
    // Register with MasterController for coordinate authority only
    RegisterWithMasterController();
}

void AWaterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (WaterSystem)
    {
        UpdateWaterSystemFromController();
    }
}

void AWaterController::RegisterWithMasterController()
{
    if (!MasterController)
    {
        MasterController = Cast<AMasterWorldController>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AMasterWorldController::StaticClass())
        );
    }
    
    if (MasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterController: Connected to MasterController for coordinate authority"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WaterController: Failed to find MasterController"));
    }
}

void AWaterController::Initialize(ADynamicTerrain* Terrain)
{
    TargetTerrain = Terrain;
    if (TargetTerrain)
    {
        WaterSystem = TargetTerrain->WaterSystem;  // Direct access to public property
        
        if (WaterSystem)
        {
            ApplySettingsToWaterSystem();
            UE_LOG(LogTemp, Warning, TEXT("WaterController: Successfully connected to WaterSystem"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("WaterController: Failed to get WaterSystem from terrain"));
        }
    }
}

void AWaterController::ResetWaterSystem()
{
    if (WaterSystem)
    {
        WaterSystem->ResetWaterSystem();
        UE_LOG(LogTemp, Warning, TEXT("WaterController: Water system reset"));
    }
}

// ===== VISUAL CONTROLS =====

void AWaterController::ToggleWaterVisualMode()
{
    int32 ModeIndex = (int32)CurrentVisualMode;
    ModeIndex = (ModeIndex + 1) % 3;
    SetWaterVisualMode((EWaterVisualMode)ModeIndex);
}

void AWaterController::SetWaterVisualMode(EWaterVisualMode NewMode)
{
    CurrentVisualMode = NewMode;
    
    if (TargetTerrain && WaterSystem)
    {
        UMaterialInterface* NewMaterial = GetCurrentWaterMaterial();
        if (NewMaterial)
        {
            TargetTerrain->SetWaterVolumeMaterial(NewMaterial);
            UE_LOG(LogTemp, Log, TEXT("WaterController: Changed visual mode to %d"), (int32)NewMode);
        }
    }
}

// ===== WEATHER CONTROLS =====

void AWaterController::StartRain(float Intensity)
{
    if (WaterSystem)
    {
        bIsRaining = true;
        RainIntensity = Intensity;
        WaterSystem->StartRain(Intensity);
        UE_LOG(LogTemp, Warning, TEXT("WaterController: Rain started with intensity %.2f"), Intensity);
    }
}

void AWaterController::StopRain()
{
    if (WaterSystem)
    {
        bIsRaining = false;
        WaterSystem->StopRain();
        UE_LOG(LogTemp, Warning, TEXT("WaterController: Rain stopped"));
    }
}

void AWaterController::ToggleRain()
{
    if (bIsRaining)
    {
        StopRain();
    }
    else
    {
        StartRain(RainIntensity);
    }
}

// ===== INTERACTION =====

// ===== WATER-SPECIFIC BRUSH IMPLEMENTATION =====

void AWaterController::ApplyWaterBrush(FVector WorldPosition, float Amount, float Radius)
{
    if (!CanReceiveBrush())
    {
        return;
    }
    
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterController: Cannot add water - no MasterController"));
        return;
    }
    
    // ✅ FIXED: Use cached reference for performance
    AMasterWorldController* CachedMaster = MasterController;
    
    if (!CachedMaster->ValidateWorldPosition(WorldPosition))
    {
        return;
    }
    
    // Create brush settings from parameters
    FUniversalBrushSettings BrushSettings;
    BrushSettings.BrushRadius = Radius;
    BrushSettings.BrushStrength = Amount;
    BrushSettings.FalloffType = EBrushFalloffType::Smooth;
    BrushSettings.InnerRadius = 0.3f;
    BrushSettings.OuterRadius = 1.0f;
    
    // Apply radius-based water placement
    FVector2D TerrainCoords = CachedMaster->WorldToTerrainCoordinates(WorldPosition);
    float TerrainRadius = Radius / CachedMaster->GetTerrainScale();
    
    int32 CellRadius = FMath::CeilToInt(TerrainRadius);
    int32 CellsProcessed = 0;
    
    for (int32 Y = -CellRadius; Y <= CellRadius; ++Y)
    {
        for (int32 X = -CellRadius; X <= CellRadius; ++X)
        {
            FVector2D CellCoord(TerrainCoords.X + X, TerrainCoords.Y + Y);
            
            FVector2D WorldDims = CachedMaster->GetWorldDimensions();
            if (CellCoord.X < 0 || CellCoord.X >= WorldDims.X || 
                CellCoord.Y < 0 || CellCoord.Y >= WorldDims.Y)
            {
                continue;
            }
            
            FVector CellWorldPos = CachedMaster->TerrainToWorldPosition(CellCoord);
            float Distance = FVector::Dist2D(WorldPosition, CellWorldPos);
            
            if (Distance <= Radius)
            {
                float Falloff = CachedMaster->CalculateBrushFalloff(Distance, BrushSettings);
                if (Falloff > 0.0f)
                {
                    float CellAmount = Amount * Falloff;
                    WaterSystem->AddWater(CellWorldPos, CellAmount);
                    CellsProcessed++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WaterController: Added water at %d cells with custom radius %.1f"), CellsProcessed, Radius);
}

void AWaterController::ApplyWaterRemovalBrush(FVector WorldPosition, float Amount, float Radius)
{
    if (!CanReceiveBrush())
    {
        return;
    }
    
    if (!MasterController)
    {
        WaterSystem->RemoveWater(WorldPosition, Amount);
        return;
    }
    
    // ✅ FIXED: Use cached reference for performance
    AMasterWorldController* CachedMaster = MasterController;
    
    // Create brush settings from parameters
    FUniversalBrushSettings BrushSettings;
    BrushSettings.BrushRadius = Radius;
    BrushSettings.BrushStrength = Amount;
    BrushSettings.FalloffType = EBrushFalloffType::Smooth;
    BrushSettings.InnerRadius = 0.3f;
    BrushSettings.OuterRadius = 1.0f;
    
    // Apply radius-based water removal
    FVector2D TerrainCoords = CachedMaster->WorldToTerrainCoordinates(WorldPosition);
    float TerrainRadius = Radius / CachedMaster->GetTerrainScale();
    
    int32 CellRadius = FMath::CeilToInt(TerrainRadius);
    int32 CellsProcessed = 0;
    
    for (int32 Y = -CellRadius; Y <= CellRadius; ++Y)
    {
        for (int32 X = -CellRadius; X <= CellRadius; ++X)
        {
            FVector2D CellCoord(TerrainCoords.X + X, TerrainCoords.Y + Y);
            
            FVector2D WorldDims = CachedMaster->GetWorldDimensions();
            if (CellCoord.X < 0 || CellCoord.X >= WorldDims.X || 
                CellCoord.Y < 0 || CellCoord.Y >= WorldDims.Y)
            {
                continue;
            }
            
            FVector CellWorldPos = CachedMaster->TerrainToWorldPosition(CellCoord);
            float Distance = FVector::Dist2D(WorldPosition, CellWorldPos);
            
            if (Distance <= Radius)
            {
                float Falloff = CachedMaster->CalculateBrushFalloff(Distance, BrushSettings);
                if (Falloff > 0.0f)
                {
                    float CellAmount = Amount * Falloff;
                    WaterSystem->RemoveWater(CellWorldPos, CellAmount);
                    CellsProcessed++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WaterController: Removed water at %d cells with custom radius %.1f"), CellsProcessed, Radius);
}

bool AWaterController::CanReceiveBrush() const
{
    return WaterSystem && 
           WaterSystem->IsSystemReady() && 
           bEnableWaterSimulation;
}

// ===== UPDATED WATER INTERACTION FUNCTIONS =====

void AWaterController::AddWater(FVector WorldPosition, float Amount, float Radius)
{
    // Use the new radius-based brush system
    ApplyWaterBrush(WorldPosition, Amount, Radius);
}

void AWaterController::RemoveWater(FVector WorldPosition, float Amount, float Radius)
{
    // Use the new radius-based brush system
    ApplyWaterRemovalBrush(WorldPosition, Amount, Radius);
}

void AWaterController::NotifyTerrainChanged()
{
    if (WaterSystem)
    {
        WaterSystem->NotifyTerrainChanged();
    }
}

float AWaterController::GetWaterDepthAtPosition(FVector WorldPosition) const
{
    if (WaterSystem)
    {
        return WaterSystem->GetWaterDepthAtPosition(WorldPosition);
    }
    return 0.0f;
}

// ===== COORDINATE TRANSFORMATION - REMOVED =====
// GetTransformedWaterPosition() function removed to fix scaling conflicts
// Water placement now uses direct world coordinates for consistent behavior

// ===== DEBUG =====

void AWaterController::DrawDebugInfo() const
{
    if (WaterSystem)
    {
        WaterSystem->DrawDebugInfo();
    }
}

// ===== UTILITIES =====

float AWaterController::GetTotalWaterInSystem() const
{
    if (WaterSystem)
    {
        return WaterSystem->GetTotalWaterInSystem();
    }
    return 0.0f;
}

int32 AWaterController::GetWaterCellCount() const
{
    if (WaterSystem)
    {
        return WaterSystem->GetWaterCellCount();
    }
    return 0;
}

float AWaterController::GetMaxFlowSpeed() const
{
    if (WaterSystem)
    {
        return WaterSystem->GetMaxFlowSpeed();
    }
    return 0.0f;
}

// ===== PRIVATE FUNCTIONS =====

void AWaterController::ApplySettingsToWaterSystem()
{
    if (!WaterSystem) return;
    
    UE_LOG(LogTemp, Warning, TEXT("WaterController: Applying all settings to WaterSystem..."));
    
    // Apply physics settings
    WaterSystem->bEnableWaterSimulation = bEnableWaterSimulation;
    WaterSystem->WaterFlowSpeed = WaterFlowSpeed;
    WaterSystem->WaterEvaporationRate = WaterEvaporationRate;
    WaterSystem->WaterAbsorptionRate = WaterAbsorptionRate;
    WaterSystem->WaterDamping = WaterDamping;
    WaterSystem->MaxWaterVelocity = MaxWaterVelocity;
    WaterSystem->MinWaterDepth = MinWaterDepth;
    
    // Erosion settings removed - now handled by GeologyController
    
    // Apply volumetric settings
    WaterSystem->bEnableWaterVolumes = bEnableVolumetricWater;
    WaterSystem->MinVolumeDepth = MinVolumeDepth;
    WaterSystem->VolumeUpdateDistance = VolumeUpdateDistance;
    WaterSystem->MaxVolumeChunks = MaxVolumeChunks;
    WaterSystem->BaseSurfaceResolution = BaseSurfaceResolution;
    
    // Apply material
    if (VolumeMaterial)
    {
        WaterSystem->VolumeMaterial = VolumeMaterial;
    }
    
    // Apply shader settings
    WaterSystem->bUseShaderWater = bUseShaderWater;
    WaterSystem->WaterShaderUpdateInterval = WaterShaderUpdateInterval;
    WaterSystem->WaterDepthScale = WaterDepthScale;
    
    // Apply parameter collection
    if (WaterParameterCollection)
    {
        WaterSystem->WaterParameterCollection = WaterParameterCollection;
    }
    
    // Apply weather settings
    WaterSystem->bIsRaining = bIsRaining;
    WaterSystem->RainIntensity = RainIntensity;
    WaterSystem->WeatherChangeInterval = WeatherChangeInterval;
    WaterSystem->bAutoWeather = bAutoWeather;
    
    // Apply debug settings
    WaterSystem->bShowWaterStats = bShowWaterStats;
    WaterSystem->bShowWaterDebugTexture = bShowWaterDebugTexture;
    
    // ===== TRANSFER NIAGARA FX SETTINGS =====
    // TODO: Add these properties to WaterSystem.h first
    /*
    WaterSystem->bEnableNiagaraFX = bEnableNiagaraFX;
    WaterSystem->RiverFlowEmitterTemplate = RiverFlowEmitterTemplate;
    WaterSystem->FoamEmitterTemplate = FoamEmitterTemplate;
    WaterSystem->LakeMistEmitterTemplate = LakeMistEmitterTemplate;
    WaterSystem->RainImpactEmitterTemplate = RainImpactEmitterTemplate;
    WaterSystem->NiagaraUpdateRate = NiagaraUpdateRate;
    WaterSystem->MaxNiagaraDistance = MaxNiagaraDistance;
    WaterSystem->MaxActiveNiagaraComponents = MaxActiveNiagaraComponents;
    WaterSystem->FoamIntensityScale = FoamIntensityScale;
    WaterSystem->MistDensityScale = MistDensityScale;
    WaterSystem->MinFlowSpeedForFX = MinFlowSpeedForFX;
    */
    
    // Set water quality resolution
    switch (WaterQuality)
    {
        case EWaterQuality::Low:    BaseSurfaceResolution = 16; break;
        case EWaterQuality::Medium: BaseSurfaceResolution = 32; break;
        case EWaterQuality::High:   BaseSurfaceResolution = 64; break;
        case EWaterQuality::Ultra:  BaseSurfaceResolution = 128; break;
    }
    WaterSystem->BaseSurfaceResolution = BaseSurfaceResolution;
    
    UE_LOG(LogTemp, Warning, TEXT("WaterController: All settings applied successfully"));
}

void AWaterController::UpdateWaterSystemFromController()
{
    // Sync any runtime changes - called every tick
    if (!WaterSystem) return;
    
    // Update material if changed
    UMaterialInterface* CurrentMaterial = GetCurrentWaterMaterial();
    if (CurrentMaterial && WaterSystem->VolumeMaterial != CurrentMaterial)
    {
        WaterSystem->VolumeMaterial = CurrentMaterial;
        if (TargetTerrain)
        {
            TargetTerrain->SetWaterVolumeMaterial(CurrentMaterial);
        }
    }
    
    // Sync critical runtime parameters (lightweight updates only)
    // TODO: Re-enable when Niagara properties added to WaterSystem
    /*
    if (WaterSystem->bEnableNiagaraFX != bEnableNiagaraFX)
    {
        WaterSystem->bEnableNiagaraFX = bEnableNiagaraFX;
    }
    
    if (WaterSystem->FoamIntensityScale != FoamIntensityScale)
    {
        WaterSystem->FoamIntensityScale = FoamIntensityScale;
    }
    
    if (WaterSystem->MistDensityScale != MistDensityScale)
    {
        WaterSystem->MistDensityScale = MistDensityScale;
    }
    */
}

UMaterialInterface* AWaterController::GetCurrentWaterMaterial() const
{
    switch (CurrentVisualMode)
    {
        case EWaterVisualMode::Water: return WaterMaterial;
        case EWaterVisualMode::Milk:  return MilkMaterial;
        case EWaterVisualMode::Debug: return DebugMaterial;
        default: return WaterMaterial;
    }
}
