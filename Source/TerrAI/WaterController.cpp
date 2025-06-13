/**
 * WaterController.cpp - Complete Implementation with Niagara FX
 */
#include "WaterController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
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
}

void AWaterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (WaterSystem)
    {
        UpdateWaterSystemFromController();
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

void AWaterController::AddWater(FVector WorldPosition, float Amount)
{
    if (WaterSystem)
    {
        WaterSystem->AddWater(WorldPosition, Amount);
    }
}

void AWaterController::RemoveWater(FVector WorldPosition, float Amount)
{
    if (WaterSystem)
    {
        WaterSystem->RemoveWater(WorldPosition, Amount);
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
