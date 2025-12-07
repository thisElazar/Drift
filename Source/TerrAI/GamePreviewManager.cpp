// GamePreviewManager.cpp - Menu World Preview Implementation
// CLEANED: November 2025 - Authority Pattern, Spring System Integration
#include "GamePreviewManager.h"
#include "DynamicTerrain.h"
#include "AtmosphericSystem.h"
#include "WaterSystem.h"
#include "GeologyController.h"
#include "TerrAIGameInstance.h"
#include "MasterController.h"
#include "TerrainController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "ProceduralMeshComponent.h"

// ============================================================================
// SECTION 1: CONSTRUCTION & INITIALIZATION
// ============================================================================

FGameSettings::FGameSettings()
{
    WorldSize = EWorldSize::Medium;
    DefaultTexture = EDefaultTexture::Natural;
    StartingTimeMode = EDriftTimeMode::Pulse;
    bEnableWeather = true;
    TimeAcceleration = 1.0f;
    bAdvancedAtmosphere = true;
}

UGamePreviewManager::UGamePreviewManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default settings
    PreviewWorldSize = EWorldSize::Medium;
    PreviewTexture = EDefaultTexture::Natural;
    PreviewTimeMode = EDriftTimeMode::Pulse;
    bRainEnabled = true;
    
    // Procedural generation defaults
    PreviewChunkSize = 8;
    HeightVariation = 500.0f;
    NoiseScale = 0.01f;
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Component constructed"));
}

void UGamePreviewManager::BeginPlay()
{
    Super::BeginPlay();
    
    // ⭐ AUTHORITY PATTERN: Wait for MasterController instead of spawning
    // The MainMenu level should have a MasterController placed in it
    
    if (!CachedMasterController)
    {
        CachedMasterController = Cast<AMasterWorldController>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AMasterWorldController::StaticClass()));
    }
    
    if (!CachedMasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Waiting for MasterController authority..."));
        UE_LOG(LogTemp, Warning, TEXT("  -> Place a MasterController in the MainMenu level"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: MasterController found, checking initialization..."));
    
    // Wait for MasterController to complete its 8-phase initialization
    if (CachedMasterController->IsInitializationComplete())
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: MasterController ready, initializing with authority"));
        
        ADynamicTerrain* Terrain = CachedMasterController->MainTerrain;
        
        if (Terrain)
        {
            InitializeWithAuthority(CachedMasterController, Terrain);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GamePreviewManager: MasterController has no terrain!"));
        }
    }
    else
    {
        // MasterController still initializing - wait for it via timer
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: MasterController still initializing, will retry..."));
        
        FTimerHandle RetryTimer;
        GetWorld()->GetTimerManager().SetTimer(
            RetryTimer,
            FTimerDelegate::CreateLambda([this]()
            {
                if (CachedMasterController && CachedMasterController->IsInitializationComplete())
                {
                    ADynamicTerrain* Terrain = CachedMasterController->MainTerrain;
                    
                    if (Terrain)
                    {
                        InitializeWithAuthority(CachedMasterController, Terrain);
                    }
                }
            }),
            0.1f,  // Check every 100ms
            true   // Loop until initialized
        );
    }
}

void UGamePreviewManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up references - terrain lifecycle is managed by MasterController
    PreviewTerrain = nullptr;
    CachedMasterController = nullptr;
    PreviewAtmosphere = nullptr;
    bInitializedWithAuthority = false;
    
    Super::EndPlay(EndPlayReason);
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cleanup complete"));
}

void UGamePreviewManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bInitializedWithAuthority)
    {
        UpdateVisualFeedback();
    }
}

// ============================================================================
// SECTION 2: AUTHORITY-BASED INITIALIZATION ⭐
// ============================================================================

void UGamePreviewManager::InitializeWithAuthority(AMasterWorldController* Master, ADynamicTerrain* Terrain)
{
    if (bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Already initialized with authority, skipping"));
        return;
    }
    
    if (!Master || !Terrain)
    {
        UE_LOG(LogTemp, Error, TEXT("GamePreviewManager: Cannot initialize - null authority references!"));
        return;
    }
    
    CachedMasterController = Master;
    PreviewTerrain = Terrain;
    
    UE_LOG(LogTemp, Warning, TEXT("=== GamePreviewManager: Initializing with MasterController Authority ==="));
    
    // Get system references from authority's terrain
    if (PreviewTerrain)
    {
        PreviewAtmosphere = PreviewTerrain->AtmosphericSystem;
        
        // Configure terrain for menu preview
        PreviewTerrain->MaxUpdatesPerFrame = PreviewConfig.MenuMaxUpdatesPerFrame;
        PreviewTerrain->SetActorLocation(PreviewConfig.PreviewOffset);
        
        UE_LOG(LogTemp, Warning, TEXT("  -> Configured terrain: MaxUpdates=%d, Offset=(%.1f,%.1f,%.1f)"),
               PreviewConfig.MenuMaxUpdatesPerFrame,
               PreviewConfig.PreviewOffset.X,
               PreviewConfig.PreviewOffset.Y,
               PreviewConfig.PreviewOffset.Z);
        
        // Configure water system for stable menu preview
        if (PreviewTerrain->WaterSystem)
        {
            // Enable surface generation
            PreviewTerrain->WaterSystem->bEnableWaterVolumes = true;
            PreviewTerrain->WaterSystem->MinVolumeDepth = 0.1f;
            PreviewTerrain->WaterSystem->MaxVolumeChunks = 32;
            
            // Disable drainage for stable preview
            PreviewTerrain->WaterSystem->WaterEvaporationRate = 0.0f;
            PreviewTerrain->WaterSystem->WaterAbsorptionRate = 0.0f;
            PreviewTerrain->WaterSystem->bEnableEdgeDrainage = false;
            
            // Apply time scale
            PreviewTerrain->WaterSystem->WaterFlowSpeed *= TimeScale;
            
            UE_LOG(LogTemp, Warning, TEXT("  -> Configured water: Stable mode, TimeScale=%.2f"), TimeScale);
        }
        
        // Log atmospheric connection
        if (PreviewAtmosphere)
        {
            UE_LOG(LogTemp, Warning, TEXT("  -> Connected to AtmosphericSystem"));
        }
    }
    
    bInitializedWithAuthority = true;
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Authority initialization complete"));
    
    // Generate initial preview terrain with next-frame timing
    // This ensures all systems are fully synchronized
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        GenerateProceduralTerrain();
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Initial preview generation complete"));
    });
}

// ============================================================================
// SECTION 3: PROCEDURAL GENERATION (THROUGH AUTHORITY)
// ============================================================================

void UGamePreviewManager::GenerateProceduralTerrain()
{
    if (!PreviewTerrain || !bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cannot generate - not initialized with authority"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Starting procedural generation through authority"));
    
    // Reset terrain through its own systems
    PreviewTerrain->ResetTerrainFully();
    
    // Generate using DynamicTerrain's procedural generation
    PreviewTerrain->GenerateProceduralTerrain();
    
    // Apply visual settings with delay for material loading
    // NOTE: Delay is necessary for material system stability
    FTimerHandle MaterialTimer;
    GetWorld()->GetTimerManager().SetTimer(
        MaterialTimer,
        [this]()
        {
            ApplySettingsToTerrain();
        },
        PreviewConfig.MaterialLoadDelay,
        false
    );
    
    // Add water features with delay for terrain stability
    // NOTE: Delay ensures heightfield data is fully propagated
    FTimerHandle WaterTimer;
    GetWorld()->GetTimerManager().SetTimer(
        WaterTimer,
        [this]()
        {
            AddWaterFeatures();
        },
        PreviewConfig.WaterFeatureDelay,
        false
    );
    
    // Seed atmospheric patterns
    SeedAtmosphericPatterns();
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Procedural generation sequence initiated"));
}

void UGamePreviewManager::AddWaterFeatures()
{
    if (!PreviewTerrain || !CachedMasterController || !bInitializedWithAuthority)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Setting up water features with spring system"));
    
    // Get GeologyController reference from MasterController
    AGeologyController* GeologyController = CachedMasterController->GeologyController;
    if (!GeologyController)
    {
        UE_LOG(LogTemp, Warning, TEXT("  -> No GeologyController found, skipping spring setup"));
        return;
    }
    
    // Clear any existing springs from previous generation
    GeologyController->ClearAllUserSprings();
    
    // Calculate terrain dimensions
    FVector2D WorldDims = CachedMasterController->GetWorldDimensions();
    float TerrainScale = CachedMasterController->GetTerrainScale();
    float WorldWidth = WorldDims.X * TerrainScale;
    float WorldHeight = WorldDims.Y * TerrainScale;
    
    // Initialize groundwater reservoir from config
    CachedMasterController->SetInitialGroundwater(PreviewConfig.InitialGroundwaterVolume);
    UE_LOG(LogTemp, Warning, TEXT("  -> Initialized groundwater: %.0f mÂ³"),
           PreviewConfig.InitialGroundwaterVolume);
    
    // Generate spring locations and flow rates
    // Distribution pattern: Strategic placement for interesting watershed patterns
    TArray<FVector> SpringLocations;
    TArray<float> SpringFlowRates;
    
    int32 NumSprings = FMath::Clamp(PreviewConfig.NumberOfSprings, 1, 10);
    
    if (NumSprings >= 1)
    {
        // Spring 1: Center (strongest - creates main watershed)
        SpringLocations.Add(FVector(WorldWidth * 0.5f, WorldHeight * 0.5f, 0.0f));
        SpringFlowRates.Add(PreviewConfig.MaxSpringFlow);
    }
    
    if (NumSprings >= 2)
    {
        // Spring 2: Upper-left quadrant
        SpringLocations.Add(FVector(WorldWidth * 0.3f, WorldHeight * 0.3f, 0.0f));
        SpringFlowRates.Add(FMath::Lerp(PreviewConfig.MinSpringFlow, PreviewConfig.MaxSpringFlow, 0.6f));
    }
    
    if (NumSprings >= 3)
    {
        // Spring 3: Upper-right quadrant
        SpringLocations.Add(FVector(WorldWidth * 0.7f, WorldHeight * 0.3f, 0.0f));
        SpringFlowRates.Add(PreviewConfig.MinSpringFlow);
    }
    
    if (NumSprings >= 4)
    {
        // Spring 4: Lower-left
        SpringLocations.Add(FVector(WorldWidth * 0.25f, WorldHeight * 0.7f, 0.0f));
        SpringFlowRates.Add(FMath::Lerp(PreviewConfig.MinSpringFlow, PreviewConfig.MaxSpringFlow, 0.5f));
    }
    
    if (NumSprings >= 5)
    {
        // Spring 5: Lower-right
        SpringLocations.Add(FVector(WorldWidth * 0.75f, WorldHeight * 0.75f, 0.0f));
        SpringFlowRates.Add(PreviewConfig.MinSpringFlow);
    }
    
    // Add additional springs if configured (random placement)
    for (int32 i = 5; i < NumSprings; i++)
    {
        float RandomX = FMath::FRandRange(0.2f, 0.8f) * WorldWidth;
        float RandomY = FMath::FRandRange(0.2f, 0.8f) * WorldHeight;
        float RandomFlow = FMath::FRandRange(PreviewConfig.MinSpringFlow, PreviewConfig.MaxSpringFlow * 0.7f);
        
        SpringLocations.Add(FVector(RandomX, RandomY, 0.0f));
        SpringFlowRates.Add(RandomFlow);
    }
    
    // Add all springs to geology controller
    float TotalFlow = 0.0f;
    for (int32 i = 0; i < SpringLocations.Num(); i++)
    {
        GeologyController->AddUserSpring(SpringLocations[i], SpringFlowRates[i]);
        TotalFlow += SpringFlowRates[i];
        
        UE_LOG(LogTemp, Warning, TEXT("  -> Spring %d: (%.0f, %.0f) @ %.1f mÂ³/s"),
               i + 1, SpringLocations[i].X, SpringLocations[i].Y, SpringFlowRates[i]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Spring setup complete - %d springs, total flow %.1f mÂ³/s"),
           SpringLocations.Num(), TotalFlow);
}

void UGamePreviewManager::SeedAtmosphericPatterns()
{
    if (!PreviewAtmosphere || !bInitializedWithAuthority)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Seeding atmospheric patterns"));
    
    // Initialize atmospheric system with preview settings
    if (bRainEnabled)
    {
        // Seed some initial atmospheric moisture and temperature variation
        // This would normally be done through AtmosphericSystem's initialization
        UE_LOG(LogTemp, Warning, TEXT("  -> Weather enabled for preview"));
    }
}

void UGamePreviewManager::ApplySettingsToTerrain()
{
    if (!PreviewTerrain || !bInitializedWithAuthority)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Applying visual settings to terrain"));
    
    // Apply texture/material settings through DynamicTerrain's systems
    // This would interact with the terrain's material system
    
    const TCHAR* TextureName = TEXT("Unknown");
    switch(PreviewTexture)
    {
        case EDefaultTexture::Wireframe: TextureName = TEXT("Wireframe"); break;
        case EDefaultTexture::Natural: TextureName = TEXT("Natural"); break;
        case EDefaultTexture::Hybrid: TextureName = TEXT("Hybrid"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("  -> Applied texture mode: %s"), TextureName);
}

// ============================================================================
// SECTION 4: UI-TRIGGERED UPDATES
// ============================================================================

void UGamePreviewManager::UpdateWorldSize(EWorldSize NewSize)
{
    if (!bInitializedWithAuthority)
    {
        return;
    }
    
    PreviewWorldSize = NewSize;
    
    const TCHAR* SizeName = TEXT("Unknown");
    switch(NewSize)
    {
        case EWorldSize::Small: SizeName = TEXT("Small"); break;
        case EWorldSize::Medium: SizeName = TEXT("Medium"); break;
        case EWorldSize::Large: SizeName = TEXT("Large"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: World size changed to %s"), SizeName);
    
    // Store in GameInstance for game transition
    UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance)
    {
        GameInstance->SetWorldSize(NewSize);
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Set world size in GameInstance"));
    }
    
    // Apply to MasterController authority
    CachedMasterController->SetWorldSizeFromUI(NewSize);
    
    // Regenerate preview with new dimensions
    if (PreviewTerrain)
    {
        // Reinitialize terrain with new dimensions from authority
        PreviewTerrain->InitializeWithMasterController(CachedMasterController);
        
        // Regenerate terrain
        GenerateProceduralTerrain();
        
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Regenerated preview with new world size"));
    }
}

void UGamePreviewManager::UpdateVisualMode(EDefaultTexture NewTexture)
{
    if (!bInitializedWithAuthority)
    {
        return;
    }
    
    PreviewTexture = NewTexture;
    
    const TCHAR* TextureName = TEXT("Unknown");
    switch(NewTexture)
    {
        case EDefaultTexture::Wireframe: TextureName = TEXT("Wireframe"); break;
        case EDefaultTexture::Natural: TextureName = TEXT("Natural"); break;
        case EDefaultTexture::Hybrid: TextureName = TEXT("Hybrid"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Visual mode changed to %s"), TextureName);
    
    ApplySettingsToTerrain();
}

void UGamePreviewManager::UpdateTimeMode(EDriftTimeMode NewMode)
{
    if (!bInitializedWithAuthority)
    {
        return;
    }
    
    PreviewTimeMode = NewMode;
    
    const TCHAR* ModeName = TEXT("Unknown");
    switch(NewMode)
    {
        case EDriftTimeMode::Pulse: ModeName = TEXT("Pulse"); break;
        case EDriftTimeMode::Trace: ModeName = TEXT("Trace"); break;
        case EDriftTimeMode::Drift: ModeName = TEXT("Drift"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Time mode changed to %s"), ModeName);
}

void UGamePreviewManager::UpdateWeatherSettings(bool bEnableRain)
{
    if (!bInitializedWithAuthority)
    {
        return;
    }
    
    bRainEnabled = bEnableRain;
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Rain %s"),
           bEnableRain ? TEXT("enabled") : TEXT("disabled"));
}

void UGamePreviewManager::GenerateNewPreviewWorld()
{
    if (!bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cannot generate - not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Generating new random preview world"));
    
    // Randomize procedural parameters
    HeightVariation = FMath::RandRange(200.0f, 800.0f);
    NoiseScale = FMath::RandRange(0.005f, 0.02f);
    
    GenerateProceduralTerrain();
}

void UGamePreviewManager::ResetPreviewWorld()
{
    if (!bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cannot reset - not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Resetting preview world to defaults"));
    
    // Reset to default parameters
    HeightVariation = 500.0f;
    NoiseScale = 0.01f;
    
    GenerateProceduralTerrain();
}

void UGamePreviewManager::RegenerateWithCurrentSettings()
{
    if (!bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cannot regenerate - not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: UI-triggered regeneration with current settings"));
    GenerateProceduralTerrain();
}

FGameSettings UGamePreviewManager::GetCurrentSettings() const
{
    FGameSettings Settings;
    Settings.WorldSize = PreviewWorldSize;
    Settings.DefaultTexture = PreviewTexture;
    Settings.StartingTimeMode = PreviewTimeMode;
    Settings.bEnableWeather = bRainEnabled;
    Settings.TimeAcceleration = 1.0f;
    Settings.bAdvancedAtmosphere = true;
    
    return Settings;
}

// ============================================================================
// SECTION 5: VISUAL FEEDBACK
// ============================================================================

void UGamePreviewManager::UpdateVisualFeedback()
{
    if (!PreviewTerrain || !bInitializedWithAuthority)
    {
        return;
    }
    
    // Handle preview terrain rotation
    if (bEnableRotation)
    {
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        float RotationDelta = RotationSpeed * DeltaTime;
        
        // Get current camera location for center point calculation
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!PC || !PC->PlayerCameraManager)
        {
            return;
        }
        
        FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        FVector OffsetFromCamera = PreviewTerrain->GetActorLocation() - CameraLocation;
        
        // Rotate offset and apply
        FVector RotatedOffset = FRotator(0, RotationDelta, 0).RotateVector(OffsetFromCamera);
        PreviewTerrain->SetActorLocation(CameraLocation + RotatedOffset);
        
        // Rotate terrain itself
        FRotator CurrentRotation = PreviewTerrain->GetActorRotation();
        CurrentRotation.Yaw += RotationDelta;
        PreviewTerrain->SetActorRotation(CurrentRotation);
    }
    
    // Update time mode pulse timer
    TimeModePulseTimer += GetWorld()->GetDeltaSeconds();
    
    // Update weather effect timer
    if (bRainEnabled)
    {
        WeatherEffectTimer += GetWorld()->GetDeltaSeconds();
    }
}
