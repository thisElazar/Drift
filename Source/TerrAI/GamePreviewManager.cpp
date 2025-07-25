// GamePreviewManager.cpp - Procedural Menu World Generation
#include "GamePreviewManager.h"
#include "DynamicTerrain.h"
#include "AtmosphericSystem.h"
#include "WaterSystem.h"
#include "TerrAIGameInstance.h"
#include "MasterController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"



// FGameSettings constructor implementation
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
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Component initialized"));
}

void UGamePreviewManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize preview systems
    InitializePreviewSystems();
    
    // Generate initial procedural world
    GenerateProceduralTerrain();
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Menu world generated"));
}

void UGamePreviewManager::TickComponent(float DeltaTime, ELevelTick TickType, 
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateVisualFeedback();
}

void UGamePreviewManager::InitializePreviewSystems()
{
    // Instead of searching for terrain, create our own dedicated preview terrain
    if (!PreviewTerrain)
    {
        PreviewTerrain = GetWorld()->SpawnActor<ADynamicTerrain>();
        bOwnsTerrain = true;  // Mark that we own this terrain
        
        if (PreviewTerrain)
        {
            // Connect to MasterController for proper authority
            CachedMasterController = Cast<AMasterWorldController>(
                UGameplayStatics::GetActorOfClass(GetWorld(), AMasterWorldController::StaticClass()));
            
            if (!CachedMasterController)
            {
                CachedMasterController = GetWorld()->SpawnActor<AMasterWorldController>();
                UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Created MasterController for menu"));
            }
            
            // Initialize with authority - this sets all dimensions properly
            PreviewTerrain->InitializeWithMasterController(CachedMasterController);
            
            // Menu-specific optimizations using configuration
            PreviewTerrain->MaxUpdatesPerFrame = PreviewConfig.MenuMaxUpdatesPerFrame;
            
            // Set preview position using configuration
            PreviewTerrain->SetActorLocation(PreviewConfig.PreviewOffset);
            
            // WATER SYSTEM INITIALIZATION: Basic setup without material complexity
            if (PreviewTerrain->WaterSystem)
            {
                // Force water system settings for main menu preview
                PreviewTerrain->WaterSystem->bEnableWaterVolumes = true;  // Enable surface generation
                PreviewTerrain->WaterSystem->MinVolumeDepth = 0.1f;       // Lower threshold
                PreviewTerrain->WaterSystem->MaxVolumeChunks = 32;        // Allow more chunks
                
                // Disable water drainage for stable menu preview
                PreviewTerrain->WaterSystem->WaterEvaporationRate = 0.0f;
                PreviewTerrain->WaterSystem->WaterAbsorptionRate = 0.0f;
                PreviewTerrain->WaterSystem->bEnableEdgeDrainage = false;
                
                // Apply time scale to water simulation
                PreviewTerrain->WaterSystem->WaterFlowSpeed *= TimeScale;
                
                UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Applied time scale %.2f to water simulation"), TimeScale);
            }
            
            // Apply material with configurable delay to prevent teal flash
            FTimerHandle TerrainMaterialTimer;
            GetWorld()->GetTimerManager().SetTimer(TerrainMaterialTimer, [this]()
            {
                if (PreviewTerrain)
                {
                    ApplySettingsToTerrain(); // Use our new function
                }
            }, PreviewConfig.MaterialLoadDelay, false);
            
            UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Created dedicated preview terrain (%dx%d)"), 
                   PreviewTerrain->TerrainWidth, PreviewTerrain->TerrainHeight);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GamePreviewManager: Failed to spawn preview terrain"));
            return;
        }
    }
    
    if (PreviewTerrain)
    {
        PreviewAtmosphere = PreviewTerrain->AtmosphericSystem;
        
        if (PreviewAtmosphere)
        {
            UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Atmospheric system initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Atmospheric system not yet available"));
        }
    }
}

void UGamePreviewManager::GenerateProceduralTerrain()
{
    if (!PreviewTerrain) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Starting procedural generation"));
    
    PreviewTerrain->ResetTerrainFully();
    PreviewTerrain->GenerateProceduralTerrain(); // Use DynamicTerrain's working generation
    AddWaterFeatures();
    SeedAtmosphericPatterns();
    ApplySettingsToTerrain();
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Procedural generation complete"));
}



void UGamePreviewManager::AddWaterFeatures()
{
    if (!PreviewTerrain || !PreviewTerrain->WaterSystem) return;
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Adding water features"));
    
    // Use configurable water feature delay
    FTimerHandle WaterDelay;
    GetWorld()->GetTimerManager().SetTimer(WaterDelay, [this]()
    {
        if (PreviewTerrain && PreviewTerrain->WaterSystem)
        {
            // Use MasterController for coordinate authority
            FVector TerrainCenter = FVector::ZeroVector;
            if (CachedMasterController)
            {
                // Calculate center relative to terrain's actual position
                float CenterX = PreviewTerrain->TerrainWidth * PreviewTerrain->TerrainScale * 0.5f;
                float CenterY = PreviewTerrain->TerrainHeight * PreviewTerrain->TerrainScale * 0.5f;
                TerrainCenter = PreviewTerrain->GetActorLocation() + FVector(CenterX, CenterY, 0.0f);
            }
            else
            {
                // Fallback to local calculation
                float TerrainCenterX = (PreviewTerrain->TerrainWidth * PreviewTerrain->TerrainScale) * 0.5f;
                float TerrainCenterY = (PreviewTerrain->TerrainHeight * PreviewTerrain->TerrainScale) * 0.5f;
                TerrainCenter = PreviewTerrain->GetActorLocation() + FVector(TerrainCenterX, TerrainCenterY, 0.0f);
            }
            
            // Clamp water positions to valid terrain bounds
            float MaxOffset = FMath::Min(PreviewTerrain->TerrainWidth, PreviewTerrain->TerrainHeight) * PreviewTerrain->TerrainScale * 0.3f;
            
            // Add central water feature
            PreviewTerrain->WaterSystem->AddWater(TerrainCenter, 8000000.0f);
            
            // Add smaller water features with bounds checking
            for (int32 i = 0; i < 6; i++)
            {
                FVector StreamStart = TerrainCenter + FVector(
                    FMath::RandRange(-MaxOffset, MaxOffset),
                    FMath::RandRange(-MaxOffset, MaxOffset),
                    0.0f);
                PreviewTerrain->WaterSystem->AddWater(StreamStart, 300000.0f);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Water features added"));
        }
    }, PreviewConfig.WaterFeatureDelay, false);
}

void UGamePreviewManager::SeedAtmosphericPatterns()
{
    if (!PreviewAtmosphere) return;
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Seeding atmospheric patterns"));
}

void UGamePreviewManager::ApplySettingsToTerrain()
{
    if (!PreviewTerrain) return;
    
    UMaterialInterface* TargetMaterial = nullptr;
    
    switch (PreviewTexture)
    {
        case EDefaultTexture::Wireframe:
            TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EDefaultTexture::Natural:
            TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TerrainNatural"));
            break;
        case EDefaultTexture::Hybrid:
            // Use wireframe as fallback for hybrid mode
            TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        default:
            TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TerrainNatural"));
            break;
    }
    
    if (TargetMaterial)
    {
        PreviewTerrain->SetActiveMaterial(TargetMaterial);
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Applied material for texture mode %d"), (int32)PreviewTexture);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Failed to load material for texture mode %d"), (int32)PreviewTexture);
    }
}

void UGamePreviewManager::UpdateVisualFeedback()
{
    TimeModePulseTimer += GetWorld()->GetDeltaSeconds();
    WeatherEffectTimer += GetWorld()->GetDeltaSeconds();
    
    // Rotate terrain if enabled - around camera view point for seamless rotation
    if (bEnableRotation && PreviewTerrain)
    {
        // Get camera location (or use fixed point if no camera)
        FVector CameraLocation = FVector::ZeroVector;
        if (GetWorld())
        {
            if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
            {
                if (PC->PlayerCameraManager)
                {
                    CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
                }
            }
        }
        
        // If no camera found, use a fixed point in front of terrain
        if (CameraLocation.IsZero())
        {
            CameraLocation = PreviewTerrain->GetActorLocation() + FVector(5000.0f, 0.0f, 1000.0f);
        }
        
        // Rotate around the camera position
        FVector TerrainLocation = PreviewTerrain->GetActorLocation();
        FVector OffsetFromCamera = TerrainLocation - CameraLocation;
        
        // Apply rotation to the offset vector
        float RotationDelta = RotationSpeed * GetWorld()->GetDeltaSeconds();
        FVector RotatedOffset = FRotator(0, RotationDelta, 0).RotateVector(OffsetFromCamera);
        
        // Set new position and rotation
        PreviewTerrain->SetActorLocation(CameraLocation + RotatedOffset);
        
        FRotator CurrentRotation = PreviewTerrain->GetActorRotation();
        CurrentRotation.Yaw += RotationDelta;
        PreviewTerrain->SetActorRotation(CurrentRotation);
    }
}

void UGamePreviewManager::UpdateWorldSize(EWorldSize NewSize)
{
    PreviewWorldSize = NewSize;
    
    // Set in GameInstance for persistence to game level
    if (UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance()))
    {
        GameInstance->SetWorldSize(NewSize);
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Set world size in GameInstance"));
    }
    
    // Apply to menu MasterController and regenerate preview
    if (CachedMasterController)
    {
        CachedMasterController->SetWorldSizeFromUI(NewSize);
        
        // Reinitialize preview terrain with new dimensions
        if (PreviewTerrain)
        {
            PreviewTerrain->InitializeWithMasterController(CachedMasterController);
            
            // Regenerate terrain with new world size
            GenerateProceduralTerrain();
            
            UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Regenerated preview with new world size"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: World size changed"));
    ShowWorldSizeOverlay(NewSize);
}

void UGamePreviewManager::UpdateVisualMode(EDefaultTexture NewTexture)
{
    PreviewTexture = NewTexture;
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Visual mode changed"));
    ApplySettingsToTerrain();
}

void UGamePreviewManager::UpdateTimeMode(EDriftTimeMode NewMode)
{
    PreviewTimeMode = NewMode;
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Time mode changed"));
    TriggerTimeModeTransition(NewMode);
}

void UGamePreviewManager::UpdateWeatherSettings(bool bEnableRain)
{
    bRainEnabled = bEnableRain;
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Rain %s"), 
           bEnableRain ? TEXT("enabled") : TEXT("disabled"));
}

void UGamePreviewManager::GenerateNewPreviewWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Generating new preview world"));
    
    HeightVariation = FMath::RandRange(200.0f, 800.0f);
    NoiseScale = FMath::RandRange(0.005f, 0.02f);
    
    GenerateProceduralTerrain();
}

void UGamePreviewManager::ResetPreviewWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Resetting preview world"));
    
    HeightVariation = 500.0f;
    NoiseScale = 0.01f;
    
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

// Cleanup function for owned terrain
void UGamePreviewManager::BeginDestroy()
{
    // Clean up our preview terrain when component is destroyed
    if (PreviewTerrain && bOwnsTerrain && IsValid(PreviewTerrain))
    {
        UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: Cleaning up owned preview terrain"));
        PreviewTerrain->Destroy();
        PreviewTerrain = nullptr;
        bOwnsTerrain = false;
    }
    
    Super::BeginDestroy();
}

// Add new function for immediate UI-triggered regeneration
void UGamePreviewManager::RegenerateWithCurrentSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("GamePreviewManager: UI-triggered regeneration with current settings"));
    GenerateProceduralTerrain();
}

// Visual feedback function stubs
void UGamePreviewManager::ShowWorldSizeOverlay(EWorldSize Size) {}
void UGamePreviewManager::TriggerTimeModeTransition(EDriftTimeMode Mode) {}
void UGamePreviewManager::EnableFlowVisualization(bool bEnable) {}
void UGamePreviewManager::EnableErosionPreview(bool bEnable) {}
