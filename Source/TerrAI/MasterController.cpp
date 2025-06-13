/**
 * ============================================
 * TERRAI MASTER WORLD CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "MasterController.h"
#include "WaterController.h"
 #include "AtmosphereController.h"  
#include "EcosystemController.h"
#include "GeologyController.h"
#include "DynamicTerrain.h"
#include "TerrainController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AMasterWorldController::AMasterWorldController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MasterRoot"));
}

void AMasterWorldController::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoInitializeOnBeginPlay)
    {
        InitializeWorld();
    }
}

void AMasterWorldController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CurrentWorldState == EWorldState::Running)
    {
        UpdatePerformanceMetrics(DeltaTime);
        
        if (bAdaptiveQuality)
        {
            OptimizeForCurrentPerformance();
        }
    }
}

// ===== WORLD MANAGEMENT =====

void AMasterWorldController::InitializeWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Initializing world systems..."));
    
    CurrentWorldState = EWorldState::Initializing;
    SystemErrors.Empty();
    
    // Auto-discover systems if not manually assigned
    AutoDiscoverSystems();
    
    // Validate all system references
    ValidateSystemReferences();
    
    // Initialize controllers in dependency order
    bool bAllSystemsReady = true;
    
    if (MainTerrain)
    {
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Main terrain found"));
    }
    else
    {
        SystemErrors.Add("Main terrain not found");
        bAllSystemsReady = false;
    }
    
    // Initialize Water Controller
    if (WaterController && MainTerrain)
    {
        WaterController->Initialize(MainTerrain);
        bAllSystemsReady &= InitializeController(WaterController, "WaterController");
    }
    
    // Initialize Ecosystem Controller
    if (EcosystemController && MainTerrain && WaterController)
    {
        EcosystemController->Initialize(MainTerrain, WaterController->GetWaterSystem());
        bAllSystemsReady &= InitializeController(EcosystemController, "EcosystemController");
    }
    
    // Initialize Geology Controller
    if (GeologyController && MainTerrain && WaterController)
    {
        GeologyController->Initialize(MainTerrain, WaterController->GetWaterSystem());
        bAllSystemsReady &= InitializeController(GeologyController, "GeologyController");
    }
    
    // Initialize Atmosphere Controller
     if (AtmosphereController)
     {
         bAllSystemsReady &= InitializeController(AtmosphereController, "AtmosphereController");
     }
    
    // Set final state
    if (bAllSystemsReady)
    {
        CurrentWorldState = EWorldState::Running;
        bSystemsInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: World initialization complete!"));
    }
    else
    {
        CurrentWorldState = EWorldState::Error;
        HandleSystemErrors();
    }
}

void AMasterWorldController::ShutdownWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Shutting down world systems..."));
    
    CurrentWorldState = EWorldState::Uninitialized;
    bSystemsInitialized = false;
    
    // Controllers handle their own cleanup in EndPlay
}

bool AMasterWorldController::SaveWorldState(const FString& SaveName)
{
    if (CurrentWorldState != EWorldState::Running)
    {
        UE_LOG(LogTemp, Error, TEXT("MasterWorldController: Cannot save - world not running"));
        return false;
    }
    
    CurrentWorldState = EWorldState::Saving;
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Saving world state: %s"), *SaveName);
    
    // TODO: Implement actual save/load system
    // For now, just log the attempt
    
    CurrentWorldState = EWorldState::Running;
    return true;
}

bool AMasterWorldController::LoadWorldState(const FString& SaveName)
{
    CurrentWorldState = EWorldState::Loading;
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Loading world state: %s"), *SaveName);
    
    // TODO: Implement actual save/load system
    
    CurrentWorldState = EWorldState::Running;
    return true;
}

// ===== SYSTEM COORDINATION =====

void AMasterWorldController::SynchronizeAllSystems()
{
    if (!bSystemsInitialized) return;
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Synchronizing all systems"));
    
    // Broadcast current world state to all systems
    if (WaterController && EcosystemController)
    {
        // Example: Sync water state with ecosystem
        float TotalWater = WaterController->GetTotalWaterInSystem();
        // EcosystemController could use this for vegetation growth
    }
}

void AMasterWorldController::PauseAllSystems()
{
    if (CurrentWorldState != EWorldState::Running) return;
    
    CurrentWorldState = EWorldState::Paused;
    SetWorldTimeScale(0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: All systems paused"));
}

void AMasterWorldController::ResumeAllSystems()
{
    if (CurrentWorldState != EWorldState::Paused) return;
    
    CurrentWorldState = EWorldState::Running;
    SetWorldTimeScale(WorldTimeScale);
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: All systems resumed"));
}

void AMasterWorldController::SetWorldTimeScale(float NewTimeScale)
{
    WorldTimeScale = NewTimeScale;
    
    if (UWorld* World = GetWorld())
    {
        World->GetWorldSettings()->SetTimeDilation(WorldTimeScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Time scale set to %.2f"), WorldTimeScale);
}

// ===== PERFORMANCE MANAGEMENT =====

void AMasterWorldController::OptimizeForCurrentPerformance()
{
    if (CurrentFrameRate < TargetFrameRate * 0.8f) // 80% threshold
    {
        // Reduce quality automatically
        if (WaterController)
        {
            // Example: Reduce water update frequency
            UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Reducing quality for performance"));
        }
    }
    else if (CurrentFrameRate > TargetFrameRate * 1.1f) // 110% threshold
    {
        // Increase quality if we have headroom
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Increasing quality - good performance"));
    }
}

void AMasterWorldController::SetQualityPreset(int32 QualityLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Setting quality preset: %d"), QualityLevel);
    
    // Apply quality settings to all systems
    if (WaterController)
    {
        // Example: Set water quality based on preset
        switch (QualityLevel)
        {
        case 0: // Low
            // WaterController->SetUpdateRate(10.0f);
            break;
        case 1: // Medium
            // WaterController->SetUpdateRate(20.0f);
            break;
        case 2: // High
            // WaterController->SetUpdateRate(30.0f);
            break;
        }
    }
}

// ===== CROSS-SYSTEM EVENTS =====

void AMasterWorldController::OnSeasonChanged(float SeasonValue)
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Season changed to %.2f"), SeasonValue);
    
    if (EcosystemController)
    {
        EcosystemController->OnSeasonChanged(SeasonValue);
    }
    
     if (AtmosphereController)
     {
         // AtmosphereController->SetSeasonalWeather(SeasonValue);
     }
}

void AMasterWorldController::OnWeatherChanged(float Temperature, float Humidity, float WindSpeed)
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Weather changed - Temp:%.1f Humidity:%.2f Wind:%.1f"), 
           Temperature, Humidity, WindSpeed);
    
    if (EcosystemController)
    {
        EcosystemController->OnWeatherChanged(Temperature, Humidity);
    }
    
    if (GeologyController)
    {
        FVector WindDirection = FVector(1, 0, 0); // Default east
        GeologyController->OnWindChanged(WindDirection, WindSpeed);
    }
}

void AMasterWorldController::OnTerrainModified(FVector Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Terrain modified at %s, radius %.1f"), 
           *Location.ToString(), Radius);
    
    if (EcosystemController)
    {
        EcosystemController->OnTerrainModified(Location, Radius);
    }
    
    if (GeologyController)
    {
        GeologyController->OnErosionOccurred(Location, Radius * 0.1f); // Convert radius to erosion amount
    }
}

// ===== DEBUG & MONITORING =====

void AMasterWorldController::PrintSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== MASTER WORLD CONTROLLER STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("World State: %d"), (int32)CurrentWorldState);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f / %.1f"), CurrentFrameRate, TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Systems Initialized: %s"), bSystemsInitialized ? TEXT("Yes") : TEXT("No"));
    
    UE_LOG(LogTemp, Warning, TEXT("Controllers:"));
    UE_LOG(LogTemp, Warning, TEXT("  Water: %s"), WaterController ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("  Ecosystem: %s"), EcosystemController ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("  Geology: %s"), GeologyController ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("  Atmosphere: %s"), AtmosphereController ? TEXT("✓") : TEXT("✗"));
    
    if (SystemErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("System Errors:"));
        for (const FString& Error : SystemErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
}

void AMasterWorldController::EnableDebugVisualization(bool bEnable)
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Debug visualization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
    
    if (WaterController)
    {
        WaterController->DrawDebugInfo();
    }
    
    if (EcosystemController)
    {
        EcosystemController->DrawBiomeMap(bEnable);
    }
    
    if (GeologyController)
    {
        GeologyController->ShowRockLayers(bEnable);
        GeologyController->ShowWaterTable(bEnable);
    }
}

FString AMasterWorldController::GetDetailedSystemReport() const
{
    FString Report = TEXT("=== TERRAI SYSTEM REPORT ===\n");
    Report += FString::Printf(TEXT("World State: %d\n"), (int32)CurrentWorldState);
    Report += FString::Printf(TEXT("Frame Rate: %.1f FPS\n"), CurrentFrameRate);
    Report += FString::Printf(TEXT("Time Scale: %.2fx\n"), WorldTimeScale);
    
    if (WaterController)
    {
        Report += FString::Printf(TEXT("Water System: %.0f total water, %d cells\n"), 
                                WaterController->GetTotalWaterInSystem(),
                                WaterController->GetWaterCellCount());
    }
    
    return Report;
}

// ===== PRIVATE FUNCTIONS =====

void AMasterWorldController::AutoDiscoverSystems()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find terrain if not assigned
    if (!MainTerrain)
    {
        MainTerrain = Cast<ADynamicTerrain>(UGameplayStatics::GetActorOfClass(World, ADynamicTerrain::StaticClass()));
        if (MainTerrain)
        {
            UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Auto-discovered main terrain"));
        }
    }
    
    // Find controllers if not assigned
    if (!WaterController)
    {
        WaterController = Cast<AWaterController>(UGameplayStatics::GetActorOfClass(World, AWaterController::StaticClass()));
    }
    
    if (!EcosystemController)
    {
        EcosystemController = Cast<AEcosystemController>(UGameplayStatics::GetActorOfClass(World, AEcosystemController::StaticClass()));
    }
    
    if (!GeologyController)
    {
        GeologyController = Cast<AGeologyController>(UGameplayStatics::GetActorOfClass(World, AGeologyController::StaticClass()));
    }
}

void AMasterWorldController::ValidateSystemReferences()
{
    SystemErrors.Empty();
    
    if (!MainTerrain)
    {
        SystemErrors.Add("Main terrain reference missing");
    }
    
    if (!WaterController)
    {
        SystemErrors.Add("Water controller reference missing");
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: System validation complete - %d errors"), SystemErrors.Num());
}

bool AMasterWorldController::InitializeController(AActor* Controller, const FString& ControllerName)
{
    if (!Controller)
    {
        SystemErrors.Add(FString::Printf(TEXT("%s is null"), *ControllerName));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: %s initialized successfully"), *ControllerName);
    return true;
}

void AMasterWorldController::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    // Update FPS every second
    if (FrameTimeAccumulator >= 1.0f)
    {
        CurrentFrameRate = FrameCount / FrameTimeAccumulator;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
}

void AMasterWorldController::HandleSystemErrors()
{
    UE_LOG(LogTemp, Error, TEXT("MasterWorldController: System initialization failed!"));
    
    for (const FString& Error : SystemErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("  Error: %s"), *Error);
    }
    
    // Could implement fallback behavior here
}
