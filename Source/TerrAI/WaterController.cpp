/**
 * WaterController.cpp - Complete Implementation 
 */
#include "WaterController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "MasterController.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
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
    
    // Initialize GPU water if enabled
       if (bUseGPUVertexDisplacement && WaterSystem)
       {
           InitializeGPUWaterSystem();
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
    
    // Update GPU water if enabled
        if (bUseGPUVertexDisplacement && WaterSystem)
        {
            WaterSystem->UpdateGPUWaveParameters(DeltaTime);
            
            // Show stats if requested
            if (bShowGPUStats && GEngine)
            {
                static float StatsTimer = 0.0f;
                StatsTimer += DeltaTime;
                if (StatsTimer > 1.0f) // Update every second
                {
                    StatsTimer = 0.0f;
                    FString StatsMessage = FString::Printf(
                        TEXT("GPU Water: Scale=%.2f Speed=%.2f Wind=(%.2f,%.2f) Strength=%.2f"),
                        GPUWaveScale, GPUWaveSpeed,
                        WindDirection.X, WindDirection.Y, WindStrength
                    );
                    GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Cyan, StatsMessage);
                }
            }
        }
    
    // Keep GPU water material in sync with visual mode
       if (bUseGPUVertexDisplacement && WaterSystem)
       {
           // Check if material needs updating
           UMaterialInterface* CurrentMaterial = GetCurrentWaterMaterial();
           if (CurrentMaterial && WaterSystem->WaterMaterialWithDisplacement != CurrentMaterial)
           {
               // Material changed, update GPU water
               WaterSystem->WaterMaterialWithDisplacement = CurrentMaterial;
               
               // Update all chunks
               for (FWaterSurfaceChunk& Chunk : WaterSystem->WaterSurfaceChunks)
               {
                   if (Chunk.SurfaceMesh && Chunk.bHasWater)
                   {
                       UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(
                           CurrentMaterial, WaterSystem);
                       
                       if (DynMat)
                       {
                           SetupDynamicMaterialParameters(DynMat);
                           Chunk.SurfaceMesh->SetMaterial(0, DynMat);
                       }
                   }
               }
           }
           
           // Update GPU wave parameters
           WaterSystem->UpdateGPUWaveParameters(DeltaTime);
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
    if (CurrentVisualMode == NewMode) return;
    
    CurrentVisualMode = NewMode;
    
    // Get the material for this mode
    UMaterialInterface* NewMaterial = GetCurrentWaterMaterial();
    
    if (!NewMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("No material set for mode: %d"), (int32)NewMode);
        return;
    }
    
    // Update the water system's material
    if (WaterSystem)
    {
        // Update both CPU and GPU materials
        WaterSystem->VolumeMaterial = NewMaterial;
        
        // If GPU mode is active, update GPU material too
        if (bUseGPUVertexDisplacement)
        {
            WaterSystem->WaterMaterialWithDisplacement = NewMaterial;
            
            // Force update all GPU chunks with new material
            for (FWaterSurfaceChunk& Chunk : WaterSystem->WaterSurfaceChunks)
            {
                if (Chunk.SurfaceMesh && Chunk.bHasWater)
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(
                        NewMaterial, WaterSystem);
                    
                    if (DynMat)
                    {
                        // Set GPU parameters on the new material
                        SetupDynamicMaterialParameters(DynMat);
                        Chunk.SurfaceMesh->SetMaterial(0, DynMat);
                    }
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Updated GPU water material to: %s"), *NewMaterial->GetName());
        }
    }
    
    // Update terrain material if needed
    if (TargetTerrain)
    {
        TargetTerrain->SetWaterVolumeMaterial(NewMaterial);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Water visual mode changed to: %s"),
           NewMode == EWaterVisualMode::Water ? TEXT("Water") :
           NewMode == EWaterVisualMode::Milk ? TEXT("Milk") : TEXT("Debug"));
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

void AWaterController::EnableGPUWater()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AWaterController::EnableGPUWater() START ==="));
    
    // Step 1: Validate WaterSystem
    if (!WaterSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: No WaterSystem available"));
        return;
    }
    
    // Step 2: Validate Terrain
    if (!TargetTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: No TargetTerrain set"));
        return;
    }
    
    // Step 3: Ensure terrain is fully initialized (use HeightMap member directly)
    if (TargetTerrain->HeightMap.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: Terrain HeightMap not initialized"));
        bPendingGPUInit = true;
        return;
    }
    
    // Step 4: Ensure terrain chunks exist
    if (TargetTerrain->TerrainChunks.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: No terrain chunks, attempting generation..."));
        
        // Try to initialize terrain properly
        TargetTerrain->InitializeTerrainData();
        
        // Check if generation succeeded
        if (TargetTerrain->TerrainChunks.Num() == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: Failed to generate terrain chunks"));
            bPendingGPUInit = true;
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: Terrain ready with %d chunks"),
           TargetTerrain->TerrainChunks.Num());
    
    // Step 5: Ensure water simulation is initialized
    if (!WaterSystem->SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: Water simulation not initialized, initializing..."));
        
        // Initialize simulation data properly
        WaterSystem->SimulationData.Initialize(
            TargetTerrain->TerrainWidth,
            TargetTerrain->TerrainHeight
        );
        
        if (!WaterSystem->SimulationData.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: Failed to initialize water simulation"));
            return;
        }
    }
    
    // Step 6: Set the flag
        bUseGPUVertexDisplacement = true;
        
        // Step 7: Initialize GPU system
        WaterSystem->InitializeGPUDisplacement();
        
        // Step 8: USE THE CURRENT WATER MATERIAL FROM SWITCHER
        UMaterialInterface* CurrentMaterial = GetCurrentWaterMaterial();
        if (CurrentMaterial)
        {
            WaterSystem->WaterMaterialWithDisplacement = CurrentMaterial;
            UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: Using current material mode: %s"),
                   *CurrentMaterial->GetName());
        }
        else if (WaterMaterial)
        {
            // Fallback to default water material
            WaterSystem->WaterMaterialWithDisplacement = WaterMaterial;
            UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: Using default water material"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("EnableGPUWater: No water material available!"));
        }
        
        // Step 9: Enable GPU vertex displacement
        WaterSystem->ToggleVertexDisplacement(true);
        
        // Step 10: Update initial parameters
        UpdateGPUWaveParameters();
        
        // Step 11: Force immediate mesh update for visible chunks
        if (WaterSystem->WaterSurfaceChunks.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnableGPUWater: Forcing mesh update for %d chunks"),
                   WaterSystem->WaterSurfaceChunks.Num());
            
            for (FWaterSurfaceChunk& Chunk : WaterSystem->WaterSurfaceChunks)
            {
                if (Chunk.bHasWater)
                {
                    WaterSystem->GenerateFlatBaseMesh(Chunk);
                }
            }
        }
        
        bPendingGPUInit = false;
        GPUInitRetryCount = 0;
        
        UE_LOG(LogTemp, Warning, TEXT("=== AWaterController::EnableGPUWater() SUCCESS ==="));
    }

void AWaterController::DisableGPUWater()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AWaterController::DisableGPUWater() ==="));
    
    if (!WaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("DisableGPUWater: No WaterSystem"));
        return;
    }
    
    bUseGPUVertexDisplacement = false;
    bPendingGPUInit = false;
    GPUInitRetryCount = 0;
    
    // Disable GPU vertex displacement
    WaterSystem->ToggleVertexDisplacement(false);
    
    UE_LOG(LogTemp, Warning, TEXT("DisableGPUWater: GPU Water DISABLED"));
}

void AWaterController::ToggleGPUWater()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AWaterController::ToggleGPUWater() ==="));
    
    // Validate prerequisites
    if (!TargetTerrain || !WaterSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ToggleGPUWater: Missing required components"));
        return;
    }
    
    if (TargetTerrain->TerrainChunks.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ToggleGPUWater: Terrain not ready (0 chunks)"));
        bPendingGPUInit = true;  // Try again later
        return;
    }
    
    // Toggle state
    if (bUseGPUVertexDisplacement)
    {
        DisableGPUWater();
    }
    else
    {
        EnableGPUWater();
    }
}

void AWaterController::UpdateGPUWaveParameters()
{
    if (!WaterSystem)
    {
        return;
    }
    
    // Update basic parameters
    WaterSystem->GPUWaveScale = GPUWaveScale;
    WaterSystem->GPUWaveSpeed = GPUWaveSpeed;
    
    // Update material parameters directly (removed IsValid check)
    WaterSystem->MaterialParams.WaveScale = GPUWaveScale;
    WaterSystem->MaterialParams.WaveSpeed = GPUWaveSpeed;
    WaterSystem->MaterialParams.WindDirection = WindDirection;
    WaterSystem->MaterialParams.WindStrength = WindStrength;
    
    // If GPU mode is active, update the wave parameters
    if (bUseGPUVertexDisplacement && WaterSystem->bUseVertexDisplacement)
    {
        WaterSystem->UpdateGPUWaveParameters(0.0f); // Pass 0 for immediate update
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("UpdateGPUWaveParameters: Scale=%.2f Speed=%.2f Wind=(%.2f,%.2f) Strength=%.2f"),
           GPUWaveScale, GPUWaveSpeed, WindDirection.X, WindDirection.Y, WindStrength);
}

void AWaterController::InitializeGPUWaterSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AWaterController::InitializeGPUWaterSystem() ==="));
    
    // Validate WaterSystem
    if (!WaterSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeGPUWaterSystem: No WaterSystem"));
        return;
    }
    
    // Check if terrain is ready
    if (!TargetTerrain || TargetTerrain->TerrainChunks.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeGPUWaterSystem: Deferring - terrain not ready"));
        bPendingGPUInit = true;
        return;
    }
    
    // Initialize the GPU displacement system
    WaterSystem->InitializeGPUDisplacement();
    
    // Set initial parameters
    UpdateGPUWaveParameters();
    
    // Enable if requested
    if (bUseGPUVertexDisplacement)
    {
        WaterSystem->ToggleVertexDisplacement(true);
        UE_LOG(LogTemp, Warning, TEXT("InitializeGPUWaterSystem: System initialized and enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeGPUWaterSystem: System initialized (not enabled)"));
    }
    
    bPendingGPUInit = false;
    GPUInitRetryCount = 0;
}

// Console commands
void AWaterController::SetGPUWaves(bool bEnable)
{
    if (bEnable)
        EnableGPUWater();
    else
        DisableGPUWater();
}

void AWaterController::SetWaveScale(float Scale)
{
    GPUWaveScale = Scale;
    UpdateGPUWaveParameters();
}

void AWaterController::SetWaveSpeed(float Speed)
{
    GPUWaveSpeed = Speed;
    UpdateGPUWaveParameters();
}

void AWaterController::SetWind(float X, float Y, float Strength)
{
    WindDirection = FVector2D(X, Y).GetSafeNormal();
    WindStrength = Strength;
    UpdateGPUWaveParameters();
}


void AWaterController::DebugGPUWater()
{
    if (!WaterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No WaterSystem"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GPU WATER DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("GPU Mode Enabled: %s"),
           bUseGPUVertexDisplacement ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Water Chunks: %d"),
           WaterSystem->WaterSurfaceChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("GPU Material: %s"),
           GPUWaterMaterial ? *GPUWaterMaterial->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Wave Scale: %.2f"), GPUWaveScale);
    UE_LOG(LogTemp, Warning, TEXT("Wave Speed: %.2f"), GPUWaveSpeed);
    
    if (TargetTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain Chunks: %d"),
               TargetTerrain->TerrainChunks.Num());
        UE_LOG(LogTemp, Warning, TEXT("Terrain Scale: %.1f"),
               TargetTerrain->TerrainScale);
        UE_LOG(LogTemp, Warning, TEXT("Chunk Size: %d"),
               TargetTerrain->ChunkSize);
        UE_LOG(LogTemp, Warning, TEXT("Chunk Overlap: %d"),
               TargetTerrain->ChunkOverlap);
        UE_LOG(LogTemp, Warning, TEXT("HeightMap Size: %d"),
               TargetTerrain->HeightMap.Num());
    }
    
    // Debug individual water chunks
    for (int32 i = 0; i < FMath::Min(5, WaterSystem->WaterSurfaceChunks.Num()); i++)
    {
        const FWaterSurfaceChunk& Chunk = WaterSystem->WaterSurfaceChunks[i];
        UE_LOG(LogTemp, Warning, TEXT("  Chunk %d: TerrainIdx=%d, Pos=(%d,%d), HasWater=%s"),
               i, Chunk.ChunkIndex, Chunk.ChunkX, Chunk.ChunkY,
               Chunk.bHasWater ? TEXT("YES") : TEXT("NO"));
    }
}

void AWaterController::SetupDynamicMaterialParameters(UMaterialInstanceDynamic* DynMat)
{
    if (!DynMat || !WaterSystem) return;
    
    // Set textures if available
    if (WaterSystem->WaveOutputTexture)
    {
        DynMat->SetTextureParameterValue(FName("WaveHeightTexture"), WaterSystem->WaveOutputTexture);
        DynMat->SetTextureParameterValue(FName("WaveOutputTexture"), WaterSystem->WaveOutputTexture);
        DynMat->SetTextureParameterValue(FName("DisplacementTexture"), WaterSystem->WaveOutputTexture);
    }
    
    if (WaterSystem->WaterDepthTexture)
    {
        DynMat->SetTextureParameterValue(FName("WaterDepthTexture"), WaterSystem->WaterDepthTexture);
        DynMat->SetTextureParameterValue(FName("DepthTexture"), WaterSystem->WaterDepthTexture);
    }
    
    // Set scalar parameters
    DynMat->SetScalarParameterValue(FName("Time"), WaterSystem->AccumulatedScaledTime);
    DynMat->SetScalarParameterValue(FName("WaveScale"), GPUWaveScale);
    DynMat->SetScalarParameterValue(FName("WaveSpeed"), GPUWaveSpeed);
    DynMat->SetScalarParameterValue(FName("WaveAmplitude"), GPUWaveScale * 10.0f);
    
    // Set wind parameters
    FLinearColor WindVec(WindDirection.X, WindDirection.Y, 0, WindStrength);
    DynMat->SetVectorParameterValue(FName("WindDirection"), WindVec);
    
    // Set any mode-specific parameters
    if (CurrentVisualMode == EWaterVisualMode::Debug)
    {
        // Debug mode might want different parameters
        DynMat->SetScalarParameterValue(FName("DebugMode"), 1.0f);
    }
}

#if WITH_EDITOR
void AWaterController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (!PropertyChangedEvent.Property)
        return;
    
    FName PropertyName = PropertyChangedEvent.Property->GetFName();
    
    // Handle GPU water property changes
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, bUseGPUVertexDisplacement))
    {
        if (bUseGPUVertexDisplacement)
            EnableGPUWater();
        else
            DisableGPUWater();
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, GPUWaterMaterial) ||
             PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, GPUWaveScale) ||
             PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, GPUWaveSpeed) ||
             PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, WindDirection) ||
             PropertyName == GET_MEMBER_NAME_CHECKED(AWaterController, WindStrength))
    {
        UpdateGPUWaveParameters();
    }
}
#endif

