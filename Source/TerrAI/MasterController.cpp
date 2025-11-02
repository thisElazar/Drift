#include "MasterController.h"
#include "TerrAIGameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"  // For TActorIterator
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Engine.h"

// Forward includes for your system controllers
#include "WaterController.h"
#include "WaterSystem.h"
#include "AtmosphereController.h"
#include "AtmosphericSystem.h"
#include "EcosystemController.h"
#include "GeologyController.h"
#include "DynamicTerrain.h"
#include "TerrainController.h"

AMasterWorldController::AMasterWorldController()
{
    // CRITICAL: Highest spawn priority for authority establishment
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics; // Tick before other systems
    
    // Initialize performance metrics
    PerformanceMetrics.AverageFrameTime = TARGET_FRAME_TIME;
    PerformanceMetrics.PeakFrameTime = TARGET_FRAME_TIME;
    PerformanceMetrics.FrameSampleCount = 0;
    PerformanceMetrics.LastPerformanceCheck = 0.0f;
}

// ===== ACTOR LIFECYCLE =====

void AMasterWorldController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Beginning initialization sequence"));
    
    // CRITICAL: Set actor priority to ensure we initialize first
    SetActorTickEnabled(false);  // Disable tick until fully initialized
    
    // CRITICAL: Set highest spawn priority
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(false);  // Disable until initialized
    
    // NO DELAY - initialize immediately
    InitializeWorld();
    
    // Enable tick only after complete
    SetActorTickEnabled(true);
}

void AMasterWorldController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Only proceed if initialization is complete
    if (CurrentInitPhase != EInitializationPhase::Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController Tick Initialized"));
        return;
    }
    
    // Update performance tracking
    UpdatePerformanceMetrics(DeltaTime);
    
    // Update water budget periodically
    WaterBudgetTimer += DeltaTime;
    if (WaterBudgetTimer >= WaterBudgetUpdateInterval)
    {
        UpdateSystemWaterBudget();
        WaterBudgetTimer = 0.0f;
        
        // Optional: Display debug info
        if (GEngine && bShowWaterBudgetDebug)
        {
            GEngine->AddOnScreenDebugMessage(100, 5.0f, FColor::Cyan, GetWaterBudgetDebugString());
        }
    }
    
    // Monitor and optimize performance if needed
    if (bAdaptiveQuality)
    {
     //   MonitorAndOptimizePerformance(DeltaTime);
    }
    
    // Update all systems with proper temporal coordination
    if (bEnableUnifiedTiming && TemporalManager)
    {
        UpdateSystemsWithTiming(DeltaTime);
    } else {
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: CallingUpdateSystemsWithTiming NO!"));
        
    }
    
   
    
    // Handle inter-system data exchange
    ProcessSystemDataExchange();
    
    // Debug visualization if enabled
    if (bShowSystemDebugInfo)
    {
     //   DrawSystemDebugInfo();
    }
}

void AMasterWorldController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: EndPlay"));
    
    // Just pause temporal manager
    if (TemporalManager)
    {
        TemporalManager->SetTemporalPause(true);
    }
    
    // Null all pointers - let UE5 handle cleanup
    AtmosphereController = nullptr;
    WaterController = nullptr;
    EcosystemController = nullptr;
    GeologyController = nullptr;
    MainTerrain = nullptr;
    
    Super::EndPlay(EndPlayReason);
}

// ===== WORLD MANAGEMENT =====

void AMasterWorldController::InitializeWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterController: SEQUENTIAL SYSTEM INITIALIZATION"));
    
    // PHASE 0: Load map definition from GameInstance
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (GameInstance)
    {
        UTerrAIGameInstance* TerrAIGameInstance = Cast<UTerrAIGameInstance>(GameInstance);
        if (TerrAIGameInstance && TerrAIGameInstance->HasMapDefinition())
        {
            CurrentMapDefinition = TerrAIGameInstance->GetCurrentMapDefinition();
            bHasMapDefinition = true;
            UE_LOG(LogTemp, Warning, TEXT("Phase 0: Loaded map '%s' with scale %.1f (%.1fkm)"),
                   *CurrentMapDefinition.DisplayName.ToString(),
                   CurrentMapDefinition.TerrainScale,
                   CurrentMapDefinition.GetWorldSizeKm());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Phase 0: No map definition - using default procedural"));
            bHasMapDefinition = false;
        }
    }
    
    // PHASE 1: Self-initialize (no dependencies)
    CurrentInitPhase = EInitializationPhase::MasterController;
    UE_LOG(LogTemp, Warning, TEXT("Phase 1: MasterController self-initialized"));
    
    // PHASE 2: Initialize TemporalManager
    CurrentInitPhase = EInitializationPhase::TemporalManager;
    InitializeTemporalManager();
    if (!TemporalManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: TemporalManager initialization failed"));
        CurrentInitPhase = EInitializationPhase::Failed;
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Phase 2: TemporalManager initialized"));
    
    // PHASE 3: Create/find terrain with authority
    CurrentInitPhase = EInitializationPhase::SystemControllers;
    if (!MainTerrain)
    {
        MainTerrain = FindOrCreateTerrain();
    }
    if (!MainTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Terrain creation/finding failed"));
        CurrentInitPhase = EInitializationPhase::Failed;
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Phase 3: Terrain found/created"));
    
    // PHASE 3.5: Pass map definition to terrain BEFORE initialization
    if (bHasMapDefinition)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        UE_LOG(LogTemp, Warning, TEXT("Phase 3.5: Map definition passed to terrain"));
    }
    
    // PHASE 4: Initialize terrain with authority FIRST
    MainTerrain->InitializeWithMasterController(this);
    UE_LOG(LogTemp, Warning, TEXT("Phase 4: Terrain configured with authority"));
    
    // PHASE 5: Initialize other systems in dependency order
    InitializeSystemControllersSequentially();
    UE_LOG(LogTemp, Warning, TEXT("Phase 5: System controllers initialized"));
    
    // PHASE 5.5: Generate scaling config AFTER terrain is available
    WorldScalingConfig = GenerateScalingConfig();
    WorldCoordinateSystem = GenerateCoordinateSystem();
    UE_LOG(LogTemp, Warning, TEXT("Phase 3.5: Scaling configuration generated"));
    
    // PHASE 6: Apply GameInstance settings
    ApplyGameInstanceSettings();
    UE_LOG(LogTemp, Warning, TEXT("Phase 6: GameInstance settings applied"));
    
    // PHASE 7: Complete
    CurrentInitPhase = EInitializationPhase::Complete;
    UE_LOG(LogTemp, Warning, TEXT("AUTHORITY CHAIN COMPLETE - All systems initialized"));
    
    // PHASE 8: Delayed water budget calculation to ensure groundwater is initialized
    // PHASE 8: Delayed water budget initialization
    if (GetWorld())
    {
        FTimerHandle WaterBudgetTimer;
        GetWorld()->GetTimerManager().SetTimer(
            WaterBudgetTimer,
            [this]()
            {
                // Let geology complete water table emergence first
                if (GeologyController)
                {
                    // Calculate actual groundwater from emerged water
                    float ActualGroundwater = GeologyController->GetGroundwaterVolume();
                    SetInitialGroundwater(ActualGroundwater);
                    
                    UE_LOG(LogTemp, Warning, TEXT("[WATER BUDGET] Initialized with %.0f mÃ‚Â³ groundwater"),
                           ActualGroundwater);
                }
                
                // Force first budget calculation
                UpdateSystemWaterBudget();
            },
            1.0f,  // 1 second delay for emergence to complete
            false
        );
    }
}

// ===== INITIALIZATION PHASES =====

void AMasterWorldController::InitializeTemporalManager()
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Initializing temporal manager"));
    
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (!GameInstance)
    {
        HandleSystemInitializationError(TEXT("TemporalManager"), TEXT("No GameInstance available"));
        return;
    }
    
    TemporalManager = GameInstance->GetSubsystem<UTemporalManager>();
    if (!TemporalManager)
    {
        HandleSystemInitializationError(TEXT("TemporalManager"), TEXT("Failed to get TemporalManager subsystem"));
        return;
    }
    
    // Configure temporal manager with our settings
    TemporalManager->SetGlobalTimeAcceleration(GlobalTimeAcceleration);
    TemporalManager->SetTemporalPause(bPauseSimulation);
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: TemporalManager initialized successfully"));
}

// ===== INTERNAL HELPERS =====

void AMasterWorldController::FindOrCreateSystemControllers()
{
    UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Finding or creating system controllers"));
    
    // Find existing controllers in the world
    if (!WaterController)
    {
        WaterController = Cast<AWaterController>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaterController::StaticClass()));
        if (!WaterController)
        {
            WaterController = GetWorld()->SpawnActor<AWaterController>();
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Created WaterController"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Found existing WaterController"));
        }
    }
    
    if (!AtmosphereController)
    {
        AtmosphereController = Cast<AAtmosphereController>(UGameplayStatics::GetActorOfClass(GetWorld(), AAtmosphereController::StaticClass()));
        if (!AtmosphereController)
        {
            AtmosphereController = GetWorld()->SpawnActor<AAtmosphereController>();
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Created AtmosphereController"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Found existing AtmosphereController"));
        }
    }
    
    if (!EcosystemController)
    {
        EcosystemController = Cast<AEcosystemController>(UGameplayStatics::GetActorOfClass(GetWorld(), AEcosystemController::StaticClass()));
        if (!EcosystemController)
        {
            EcosystemController = GetWorld()->SpawnActor<AEcosystemController>();
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Created EcosystemController"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Found existing EcosystemController"));
        }
    }
    
    // CRITICAL FIX: Add GeologyController logic that was missing
    if (!GeologyController)
    {
        GeologyController = Cast<AGeologyController>(UGameplayStatics::GetActorOfClass(GetWorld(), AGeologyController::StaticClass()));
        if (!GeologyController)
        {
            GeologyController = GetWorld()->SpawnActor<AGeologyController>();
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Created GeologyController"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] Found existing GeologyController"));
        }
    }
    
    // Initialize controllers with terrain and water system
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        if (EcosystemController && !EcosystemController->TargetTerrain)
        {
            EcosystemController->Initialize(MainTerrain, MainTerrain->WaterSystem);
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] EcosystemController initialized"));
        }
        
        if (GeologyController && !GeologyController->TargetTerrain)
        {
            GeologyController->Initialize(MainTerrain, MainTerrain->WaterSystem);
            UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] GeologyController initialized"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SYSTEM INIT] All system controllers found/created successfully"));
}

void AMasterWorldController::ConfigureSystemTemporalSettings()
{
    if (!TemporalManager)
    {
        return;
    }
    
    // Configure update frequencies for each system
    SystemUpdateStates.Add(ESystemType::PlayerInteraction, {true, 0.0f, 60.0f, 1});  // 60 Hz
    SystemUpdateStates.Add(ESystemType::WaterPhysics, {true, 0.0f, 30.0f, 2});       // 30 Hz
    SystemUpdateStates.Add(ESystemType::Atmospheric, {true, 0.0f, 1.0f, 3});         // 1 Hz
    SystemUpdateStates.Add(ESystemType::Ecosystem, {true, 0.0f, 0.1f, 4});           // 0.1 Hz
    SystemUpdateStates.Add(ESystemType::Erosion, {true, 0.0f, 0.01f, 5});            // 0.01 Hz
    SystemUpdateStates.Add(ESystemType::Seasonal, {true, 0.0f, 0.001f, 6});          // 0.001 Hz
}

void AMasterWorldController::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time tracking
    PerformanceMetrics.FrameSampleCount++;
    
    // Calculate running average
    float Alpha = 1.0f / FMath::Min(PerformanceMetrics.FrameSampleCount, PERFORMANCE_SAMPLE_SIZE);
    PerformanceMetrics.AverageFrameTime = FMath::Lerp(PerformanceMetrics.AverageFrameTime, DeltaTime, Alpha);
    
    // Track peak frame time
    PerformanceMetrics.PeakFrameTime = FMath::Max(PerformanceMetrics.PeakFrameTime, DeltaTime);
    
    // Reset peak occasionally to handle temporary spikes
    if (PerformanceMetrics.FrameSampleCount % (PERFORMANCE_SAMPLE_SIZE * 4) == 0)
    {
        PerformanceMetrics.PeakFrameTime = PerformanceMetrics.AverageFrameTime;
    }
}

bool AMasterWorldController::ShouldOptimizePerformance() const
{
    return !IsPerformanceOptimal() && bAdaptiveQuality;
}

void AMasterWorldController::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Applying performance optimizations"));
    
    // Reduce system update frequencies
    if (TemporalManager)
    {
        // Lower atmospheric system update rate
        auto* AtmosphericState = SystemUpdateStates.Find(ESystemType::Atmospheric);
        if (AtmosphericState && AtmosphericState->UpdateFrequency > 0.5f)
        {
            AtmosphericState->UpdateFrequency *= 0.8f;
            UE_LOG(LogTemp, Log, TEXT("Reduced atmospheric update frequency to %.3f Hz"), AtmosphericState->UpdateFrequency);
        }
        
        // Lower ecosystem update rate
        auto* EcosystemState = SystemUpdateStates.Find(ESystemType::Ecosystem);
        if (EcosystemState && EcosystemState->UpdateFrequency > 0.05f)
        {
            EcosystemState->UpdateFrequency *= 0.8f;
            UE_LOG(LogTemp, Log, TEXT("Reduced ecosystem update frequency to %.3f Hz"), EcosystemState->UpdateFrequency);
        }
    }
    
    // Adjust system quality settings
    // TODO: Implement quality reduction for individual systems
    // - Reduce water simulation resolution
    // - Lower atmospheric grid density
    // - Simplify ecosystem calculations
}

bool AMasterWorldController::ValidateSystemReferences() const
{
    bool bAllValid = true;
    
    if (!TemporalManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: TemporalManager reference is null"));
        bAllValid = false;
    }
    
    if (!MainTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: MainTerrain reference is null"));
        bAllValid = false;
    }
    
    // Note: Other controllers are optional and don't fail validation
    
    return bAllValid;
}

void AMasterWorldController::LogSystemStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== MASTER CONTROLLER SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Initialization Phase: %d"), (int32)CurrentInitPhase);
    UE_LOG(LogTemp, Log, TEXT("Temporal Manager: %s"), TemporalManager ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Water Controller: %s"), WaterController ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Atmosphere Controller: %s"), AtmosphereController ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Ecosystem Controller: %s"), EcosystemController ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Geology Controller: %s"), GeologyController ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Main Terrain: %s"), MainTerrain ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("Unified Timing: %s"), bEnableUnifiedTiming ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Log, TEXT("Global Time Scale: %.2fx"), GlobalTimeAcceleration);
}

// ===== ERROR HANDLING =====

void AMasterWorldController::HandleSystemInitializationError(const FString& SystemName, const FString& ErrorMessage)
{
    FString FullError = FString::Printf(TEXT("%s: %s"), *SystemName, *ErrorMessage);
    InitializationErrors.Add(FullError);
    
    UE_LOG(LogTemp, Error, TEXT("MasterWorldController: %s"), *FullError);
    
    CurrentInitPhase = EInitializationPhase::Failed;
    OnSystemError.Broadcast(SystemName, ErrorMessage);
}

bool AMasterWorldController::AttemptSystemRecovery(const FString& SystemName)
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Attempting recovery for %s"), *SystemName);
    
    // Attempt to re-initialize critical systems
    if (SystemName == TEXT("TemporalManager") || SystemName == TEXT("AllSystems"))
    {
        InitializeTemporalManager();
        if (TemporalManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: TemporalManager recovery successful"));
            return true;
        }
    }
    
    // Attempt to find missing controllers
    if (SystemName == TEXT("AllSystems"))
    {
        FindOrCreateSystemControllers();
        if (ValidateSystemReferences())
        {
            UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: System references recovery successful"));
            return true;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("MasterWorldController: Recovery failed for %s"), *SystemName);
    return false;
}

// ===== SAVE/LOAD FUNCTIONALITY =====
// Future implementation planned for world persistence
// ===== ADVANCED SYSTEM CONTROLS =====

void AMasterWorldController::SetSystemEnabled(ESystemType SystemType, bool bEnabled)
{
    auto* SystemState = SystemUpdateStates.Find(SystemType);
    if (SystemState)
    {
        SystemState->bNeedsUpdate = bEnabled;
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: System %d %s"),
               (int32)SystemType, bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    }
}

bool AMasterWorldController::IsSystemEnabled(ESystemType SystemType) const
{
    const auto* SystemState = SystemUpdateStates.Find(SystemType);
    return SystemState ? SystemState->bNeedsUpdate : false;
}

void AMasterWorldController::SetSystemUpdatePriority(ESystemType SystemType, int32 Priority)
{
    auto* SystemState = SystemUpdateStates.Find(SystemType);
    if (SystemState)
    {
        SystemState->UpdatePriority = Priority;
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: System %d priority set to %d"),
               (int32)SystemType, Priority);
    }
}

void AMasterWorldController::ForceSystemReregistration()
{
    UE_LOG(LogTemp, Warning, TEXT("[FORCE REGISTRATION] Re-registering all scalable systems"));
    
    // Clear existing registrations
    RegisteredScalableSystems.Empty();
    
    // Re-run the registration process
    RegisterScalableSystems();
    
    // Apply scaling configuration
    ConfigureWorldScaling();
    
    UE_LOG(LogTemp, Warning, TEXT("[FORCE REGISTRATION] Re-registration complete"));
}

void AMasterWorldController::ForceSynchronization()
{
    SynchronizeAllSystems();
}

// ===== PHASE 1: AUTHORITY ESTABLISHMENT IMPLEMENTATION =====

ADynamicTerrain* AMasterWorldController::FindOrCreateTerrain()
{
    // Find existing terrain
    ADynamicTerrain* Terrain = Cast<ADynamicTerrain>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ADynamicTerrain::StaticClass()));
    
    if (!Terrain)
    {
        // Create new terrain with immediate authority
        Terrain = GetWorld()->SpawnActor<ADynamicTerrain>();
        UE_LOG(LogTemp, Warning, TEXT("Created new terrain"));
    }
    
    return Terrain;
}

void AMasterWorldController::InitializeTerrainController()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterController: Initializing TerrainController with authority"));
    
    // Find TerrainController in the world
    ATerrainController* FoundTerrainController = Cast<ATerrainController>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATerrainController::StaticClass()));
    
    if (FoundTerrainController)
    {
        // Set MasterController reference
        FoundTerrainController->MasterController = this;
        
        // Initialize with authority
        FoundTerrainController->InitializeControllerWithAuthority();
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainController initialized with MasterController authority"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No TerrainController found - this is OK if using different controller"));
    }
}

void AMasterWorldController::InitializeSystemControllersSequentially()
{
    UE_LOG(LogTemp, Warning, TEXT("Sequential system initialization"));
    
    // Step 1: Water system (requires terrain)
    if (MainTerrain)
    {
        if (MainTerrain->WaterSystem && MainTerrain->WaterSystem->IsSystemReady())
        {
            UE_LOG(LogTemp, Warning, TEXT("Water system already initialized - skipping"));
        }
        else
        {
            MainTerrain->InitializeWaterSystem();
            UE_LOG(LogTemp, Warning, TEXT("Water system initialized"));
        }
    }
    
    // Step 2: Atmospheric system initialization with authority
    if (AtmosphereController && MainTerrain)
    {
        // Initialize with authority
        AtmosphereController->InitializeWithAuthority(this, MainTerrain);
        
        // Connect to terrain's atmospheric system if available
        if (MainTerrain->AtmosphericSystem)
        {
            AtmosphereController->AtmosphericSystem = MainTerrain->AtmosphericSystem;
            UE_LOG(LogTemp, Warning, TEXT("Connected AtmosphereController to AtmosphericSystem"));
        }
        
        // Don't directly access protected member WaterSystem
        // The AtmosphereController will get WaterSystem reference internally from the terrain
        
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController initialized with authority"));
    }
    else
    {
        if (!AtmosphereController)
        {
            UE_LOG(LogTemp, Warning, TEXT("No AtmosphereController found"));
        }
        if (!MainTerrain)
        {
            UE_LOG(LogTemp, Error, TEXT("No MainTerrain available for atmosphere init"));
        }
    }
    
    // Step 5: Other controllers (require water system)
    FindOrCreateSystemControllers();
    
    // Step 5.5: Initialize Geology Controller and fill water table
    if (GeologyController && MainTerrain && MainTerrain->WaterSystem)
    {
        // Initialize geology with terrain and water system
        GeologyController->Initialize(MainTerrain, MainTerrain->WaterSystem);
        
        // CRITICAL: Fill initial water table (separate from water budget tracking)
        GeologyController->InitializeWaterTableFill();
        
        UE_LOG(LogTemp, Warning, TEXT("GeologyController initialized and water table filled"));
    }
    
    // Step 6: Establish connections between systems
    EstablishSystemConnections();
    
    // Step 7: Initialize TerrainController
    InitializeTerrainController();
    
    // Step 8: Register with temporal manager
    if (TemporalManager)
    {
        RegisterSystemsWithTemporalManager();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Sequential initialization complete"));
}

void AMasterWorldController::WorldBrushToTextureSpace(
    FVector WorldPosition,
    float WorldRadius,
    FVector2D& OutTextureCoords,
    float& OutRadiusInTexels
) const
{
    if (!MainTerrain)
    {
        OutTextureCoords = FVector2D::ZeroVector;
        OutRadiusInTexels = 0.0f;
        return;
    }
    
    // STEP 1: Transform world position to terrain-local coordinates
    FVector LocalPosition = MainTerrain->GetActorTransform().InverseTransformPosition(WorldPosition);
    
    // STEP 2: Convert to texture space using authoritative scale
    float TerrainScale = GetTerrainScale();
    OutTextureCoords = FVector2D(
        LocalPosition.X / TerrainScale,
        LocalPosition.Y / TerrainScale
    );
    
    // STEP 3: Convert radius to texture space
    OutRadiusInTexels = WorldRadius / TerrainScale;
    
    // STEP 4: Clamp to valid texture bounds
    FVector2D WorldDims = GetWorldDimensions();
    OutTextureCoords.X = FMath::Clamp(OutTextureCoords.X, 0.0f, WorldDims.X - 1.0f);
    OutTextureCoords.Y = FMath::Clamp(OutTextureCoords.Y, 0.0f, WorldDims.Y - 1.0f);
    
    UE_LOG(LogTemp, VeryVerbose,
           TEXT("[COORD AUTH] World(%.1f,%.1f,%.1f) → Texture(%.2f,%.2f), Radius: %.1f → %.2f texels"),
           WorldPosition.X, WorldPosition.Y, WorldPosition.Z,
           OutTextureCoords.X, OutTextureCoords.Y,
           WorldRadius, OutRadiusInTexels);
}

FVector4f AMasterWorldController::GetShaderTerrainParams() const
{
    FVector2D Dims = GetWorldDimensions();
    float Scale = GetTerrainScale();
    float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    return FVector4f(
        Dims.X,     // Texture width
        Dims.Y,     // Texture height
        Scale,      // World units per texel
        Time        // Current time
    );
}

FVector AMasterWorldController::GetTerrainWorldOrigin() const
{
    if (MainTerrain)
    {
        return MainTerrain->GetActorLocation();
    }
    return FVector::ZeroVector;
}

FString AMasterWorldController::CreateWorldSnapshot() const
{
    FString Snapshot = TEXT("{");
    
    // Add temporal state
    if (TemporalManager)
    {
        FString TemporalSnapshot = TemporalManager->CreateTemporalSnapshot();
        Snapshot += FString::Printf(TEXT("\"TemporalState\":%s,"), *TemporalSnapshot);
    }
    
    // Add master controller state
    Snapshot += FString::Printf(TEXT("\"GlobalTimeAcceleration\":%.6f,"), GlobalTimeAcceleration);
    Snapshot += FString::Printf(TEXT("\"bPauseSimulation\":%s,"), bPauseSimulation ? TEXT("true") : TEXT("false"));
    Snapshot += FString::Printf(TEXT("\"bEnableUnifiedTiming\":%s"), bEnableUnifiedTiming ? TEXT("true") : TEXT("false"));
    
    // TODO: Add system-specific state snapshots
    // - Water system state
    // - Atmospheric state
    // - Ecosystem state
    // - Terrain state
    
    Snapshot += TEXT("}");
    return Snapshot;
}

bool AMasterWorldController::RestoreWorldFromSnapshot(const FString& SnapshotData)
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Restoring world from snapshot"));
    
    // TODO: Implement proper JSON parsing and restoration
    // For now, basic implementation that resets systems
    
    if (TemporalManager)
    {
        TemporalManager->RestoreFromSnapshot(SnapshotData);
    }
    
    // Reset to a known good state
  //  ResetWorld();
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: World restoration complete"));
    return true;
("MasterWorldController: Initializing temporal manager");

    // Configure temporal manager with our settings
    TemporalManager->SetGlobalTimeAcceleration(GlobalTimeAcceleration);
    TemporalManager->SetTemporalPause(bPauseSimulation);
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: TemporalManager initialized successfully"));
}

void AMasterWorldController::InitializeSystemControllers()
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Initializing system controllers"));
    
    // Find or create system controllers
    FindOrCreateSystemControllers();
    
    // Initialize each controller
    if (WaterController)
    {
        // Initialize water controller with main terrain
        // WaterController->Initialize(MainTerrain);
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: WaterController initialized"));
    }
    
    if (AtmosphereController)
    {
        // Initialize atmosphere controller
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: AtmosphereController initialized"));
    }
    
    if (EcosystemController)
    {
        // Initialize ecosystem controller
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: EcosystemController initialized"));
    }
    
    if (GeologyController)
    {
        // Initialize geology controller
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: GeologyController initialized"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: System controllers initialized"));
}

void AMasterWorldController::RegisterSystemsWithTemporalManager()
{
    if (!TemporalManager)
    {
        HandleSystemInitializationError(TEXT("SystemRegistration"), TEXT("TemporalManager not available"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Registering systems with TemporalManager"));
    
    // Register each system with appropriate time scales
    TemporalManager->RegisterSystem(ESystemType::PlayerInteraction, 1.0f);     // Real-time
    TemporalManager->RegisterSystem(ESystemType::WaterPhysics, 10.0f);         // 10:1 scale
    TemporalManager->RegisterSystem(ESystemType::Atmospheric, 60.0f);          // 60:1 scale
    TemporalManager->RegisterSystem(ESystemType::Erosion, 3600.0f);            // 3600:1 scale
    TemporalManager->RegisterSystem(ESystemType::Seasonal, 86400.0f);          // 86400:1 scale
    TemporalManager->RegisterSystem(ESystemType::Ecosystem, 3600.0f);          // 3600:1 scale
    
    // Configure system update settings
    ConfigureSystemTemporalSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: All systems registered with TemporalManager"));
}

void AMasterWorldController::EstablishSystemConnections()
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Establishing inter-system connections"));
    
    // Connect AtmosphereController to terrain and water systems
    if (AtmosphereController && MainTerrain && MainTerrain->WaterSystem)
    {
        // Initialize AtmosphereController with terrain and water references
        AtmosphereController->Initialize(MainTerrain, MainTerrain->WaterSystem);
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: AtmosphereController initialized and connected"));
    }
    else
    {
        if (!AtmosphereController)
            UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: No AtmosphereController found"));
        if (!MainTerrain)
            UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: No MainTerrain found"));
        if (!MainTerrain->WaterSystem)
            UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: No WaterSystem found on terrain"));
    }
    
    // Connect water system to atmospheric system for precipitation
    if (WaterController && AtmosphereController)
    {
        // Set up atmospheric precipitation -> water system interface
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Connected Atmosphere -> Water systems"));
    }
    
    // Connect water system to erosion for terrain modification
    if (WaterController && GeologyController && MainTerrain)
    {
        // Set up water erosion -> terrain modification interface
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Connected Water -> Geology systems"));
    }
    
    // Connect ecosystem to water and atmospheric systems
    if (EcosystemController && WaterController && AtmosphereController)
    {
        // Set up ecosystem responses to water availability and weather
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Connected Ecosystem -> Water/Atmosphere systems"));
    }
    
    UpdateSystemConnections();
}

void AMasterWorldController::FinalizeInitialization()
{
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Finalizing initialization"));
    
    // Validate all systems are properly initialized
    if (!ValidateSystemReferences())
    {
        HandleSystemInitializationError(TEXT("Validation"), TEXT("System reference validation failed"));
        return;
    }
    
    // Configure world scaling system
    ConfigureWorldScaling();
    
    // Force initial synchronization
    if (TemporalManager)
    {
        TemporalManager->ForceSynchronization();
    }
    
    // Set initialization as complete
    CurrentInitPhase = EInitializationPhase::Complete;
    
    // Log final status
    LogSystemStatus();
}

// ===== TEMPORAL CONTROL INTERFACE =====

void AMasterWorldController::SetGlobalTimeAcceleration(float Acceleration)
{
    GlobalTimeAcceleration = FMath::Max(0.0f, Acceleration);
    
    if (TemporalManager)
    {
        TemporalManager->SetGlobalTimeAcceleration(GlobalTimeAcceleration);
        OnTemporalStateChanged.Broadcast(GlobalTimeAcceleration);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Global time acceleration set to %.2f"), GlobalTimeAcceleration);
}

void AMasterWorldController::PauseAllSystems(bool bPause)
{
    bPauseSimulation = bPause;
    
    if (TemporalManager)
    {
        TemporalManager->SetTemporalPause(bPause);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MasterWorldController: All systems %s"),
           bPause ? TEXT("PAUSED") : TEXT("RESUMED"));
}

void AMasterWorldController::ResetTemporalState()
{
    if (TemporalManager)
    {
        TemporalManager->ResetAllTemporalSystems();
        UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Temporal state reset"));
    }
}

float AMasterWorldController::GetSystemTime(ESystemType SystemType) const
{
    if (TemporalManager)
    {
        return TemporalManager->GetSystemTime(SystemType);
    }
    return 0.0f;
}

bool AMasterWorldController::IsSystemRegistered(ESystemType SystemType) const
{
    if (TemporalManager)
    {
        return TemporalManager->IsSystemRegistered(SystemType);
    }
    return false;
}

FString AMasterWorldController::GetTemporalStats() const
{
    if (TemporalManager)
    {
        return TemporalManager->GetTemporalSystemStats();
    }
    return TEXT("TemporalManager not available");
}

// ===== TEMPORAL SYSTEM INTEGRATION =====

void AMasterWorldController::UpdateSystemsWithTiming(float DeltaTime)
{
    if (!TemporalManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Ã°Å¸â€Â¥ UpdateSystemsWithTiming: TemporalManager is NULL"));
        return;
    }
    
    TemporalManager->ResetFrameCounter();
    
    // Water: 30 Hz (already working through WaterController tick)
    if (WaterController && TemporalManager->ShouldSystemUpdate(ESystemType::WaterPhysics, 30.0f))
    {
        float WaterDeltaTime = TemporalManager->GetSystemDeltaTime(ESystemType::WaterPhysics, DeltaTime);
        TemporalManager->MarkSystemUpdated(ESystemType::WaterPhysics);
        
        if (bLogSystemUpdates)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("MasterWorldController: Updated WaterSystem (dt=%.4f)"), WaterDeltaTime);
        }
    }
    
    // Atmosphere: 1 Hz
    if (AtmosphereController && TemporalManager->ShouldSystemUpdate(ESystemType::Atmospheric, 1.0f))
    {
        float AtmosphericDeltaTime = TemporalManager->GetSystemDeltaTime(ESystemType::Atmospheric, DeltaTime);
        AtmosphereController->UpdateAtmosphericSystem(AtmosphericDeltaTime);
        TemporalManager->MarkSystemUpdated(ESystemType::Atmospheric);
        
        if (bLogSystemUpdates)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("MasterWorldController: Updated AtmosphericSystem (dt=%.4f)"), AtmosphericDeltaTime);
        }
    }
    
    // Ecosystem: 0.1 Hz
    if (EcosystemController && TemporalManager->ShouldSystemUpdate(ESystemType::Ecosystem, 0.1f))
    {
        float EcosystemDeltaTime = TemporalManager->GetSystemDeltaTime(ESystemType::Ecosystem, DeltaTime);
        EcosystemController->UpdateEcosystemSystem(EcosystemDeltaTime);
        TemporalManager->MarkSystemUpdated(ESystemType::Ecosystem);
        
        if (bLogSystemUpdates)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("MasterWorldController: Updated EcosystemSystem (dt=%.4f)"), EcosystemDeltaTime);
        }
    }
    
    // Geology: 0.01 Hz
    if (GeologyController && TemporalManager->ShouldSystemUpdate(ESystemType::Erosion, 1.0f)) //changed from 0.01
    {
        float ErosionDeltaTime = TemporalManager->GetSystemDeltaTime(ESystemType::Erosion, DeltaTime);
        GeologyController->UpdateGeologySystem(ErosionDeltaTime);
        TemporalManager->MarkSystemUpdated(ESystemType::Erosion);
        
        if (bLogSystemUpdates)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("MasterWorldController: Updated GeologySystem (dt=%.4f)"), ErosionDeltaTime);
        }
    }
}


void AMasterWorldController::ProcessSystemDataExchange()
{
    // Handle data exchange between systems
    // This is where systems communicate their state to each other
    
    // Example: Atmospheric precipitation -> Water system
    if (AtmosphereController && WaterController)
    {
        // Transfer precipitation data from atmosphere to water system
        // float PrecipitationRate = AtmosphereController->GetCurrentPrecipitation();
        // WaterController->ApplyPrecipitation(PrecipitationRate);
    }
    
    // Example: Water flow -> Erosion system
    if (WaterController && GeologyController)
    {
        // Transfer flow data for erosion calculations
        // auto FlowData = WaterController->GetFlowData();
        // GeologyController->ApplyWaterErosion(FlowData);
    }
    
    // Example: Environmental conditions -> Ecosystem
    if (AtmosphereController && WaterController && EcosystemController)
    {
        // auto EnvironmentalData = GatherEnvironmentalData();
        // EcosystemController->UpdateEnvironmentalConditions(EnvironmentalData);
    }
    
    // This is already happening in AtmosphericSystem::UpdateWaterSystemInterface()
    // But we can add monitoring here:
    
    if (AtmosphereController && MainTerrain && MainTerrain->AtmosphericSystem)
    {
        // Monitor precipitation
        float TotalPrecipitation = 0.0f;
        int32 RainingCells = 0;
        
        for (int32 Y = 0; Y < 64; Y++) // Assuming 64x64 grid
        {
            for (int32 X = 0; X < 64; X++)
            {
                FVector WorldPos = MainTerrain->GetActorLocation() +
                                  FVector(X * 1000.0f, Y * 1000.0f, 0);
                float Precip = MainTerrain->AtmosphericSystem->GetPrecipitationAt(WorldPos);
                if (Precip > 0.1f)
                {
                    TotalPrecipitation += Precip;
                    RainingCells++;
                }
            }
        }
        
        // Log every 5 seconds
        static float LastLogTime = 0.0f;
        if (GetWorld()->GetTimeSeconds() - LastLogTime > 5.0f)
        {
            if (RainingCells > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("MasterController: Active precipitation - %d cells, avg %.1f mm/hr"),
                       RainingCells, TotalPrecipitation / FMath::Max(1, RainingCells));
            }
            LastLogTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void AMasterWorldController::SynchronizeAllSystems()
{
    if (TemporalManager)
    {
        TemporalManager->ForceSynchronization();
        UE_LOG(LogTemp, Log, TEXT("MasterWorldController: Forced synchronization of all systems"));
    }
}

void AMasterWorldController::UpdateSystemConnections()
{
    // Refresh connections between systems
    // This can be called when systems are added/removed dynamically
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("MasterWorldController: Updated system connections"));
}


void AMasterWorldController::DrawSystemDebugInfo()
{
    if (!bShowSystemDebugInfo) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    FVector Origin = GetActorLocation();
    DrawDebugString(World, Origin + FVector(0, 0, 500),
                   FString::Printf(TEXT("Systems: %d registered"), RegisteredScalableSystems.Num()),
                   nullptr, FColor::Green, 0.0f);
}

void AMasterWorldController::LogSystemPerformance()
{
    if (!bLogSystemUpdates) return;
    
    UE_LOG(LogTemp, Log, TEXT("=== MASTER CONTROLLER PERFORMANCE ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2fms (Target: %.2fms)"),
           PerformanceMetrics.AverageFrameTime * 1000.0f, TARGET_FRAME_TIME * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredScalableSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("Initialization Phase: %d"), (int32)CurrentInitPhase);
}

bool AMasterWorldController::AreAllSystemsInitialized() const
{
    // Example logic
    return TemporalManager && MainTerrain;
}

bool AMasterWorldController::IsPerformanceOptimal() const
{
    return PerformanceMetrics.AverageFrameTime <= TARGET_FRAME_TIME;
}

// ===== WORLD SCALING IMPLEMENTATION =====

void AMasterWorldController::ConfigureWorldScaling()
{
    UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Configuring world scaling system"));
    
    // Calculate optimal scaling configuration
    CalculateOptimalWorldScaling();
    
    // Register all scalable systems FIRST
    RegisterScalableSystems();
    
    // Then configure them
    for (UObject* Obj : RegisteredScalableSystems)
    {
        IScalableSystem* System = Cast<IScalableSystem>(Obj);
        if (System)
        {
            System->ConfigureFromMaster(WorldScalingConfig);
        }
    }
    
    // Synchronize coordinate systems
    SynchronizeSystemCoordinates();
    
    UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] World scaling configuration complete"));
}

void AMasterWorldController::RegisterScalableSystems()
{
    UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Registering scalable systems"));
    
    RegisteredScalableSystems.Empty();
    
    // Register water system if available
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        UWaterSystem* WaterSystem = MainTerrain->WaterSystem;
        if (WaterSystem->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
        {
            // Use the object directly - it implements the interface
            RegisteredScalableSystems.Add(WaterSystem);
            WaterSystem->RegisterWithMasterController(this);
            UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Water system registered"));
        }
    }
    
    // Register atmospheric system if available
    if (MainTerrain && MainTerrain->AtmosphericSystem)
    {
        UAtmosphericSystem* AtmosphericSystem = MainTerrain->AtmosphericSystem;
        if (AtmosphericSystem->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
        {
            RegisteredScalableSystems.Add(AtmosphericSystem);
            AtmosphericSystem->RegisterWithMasterController(this);
            UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Atmospheric system registered"));
        }
    }
    
    // Register geology controller if available
    if (GeologyController && GeologyController->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
    {
        RegisteredScalableSystems.Add(GeologyController);
        GeologyController->RegisterWithMasterController(this);
        UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Geology controller registered"));
    }
    
    // Register ecosystem controller if available
    if (EcosystemController && EcosystemController->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
    {
        RegisteredScalableSystems.Add(EcosystemController);
        EcosystemController->RegisterWithMasterController(this);
        UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Ecosystem controller registered"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] Registered %d scalable systems"), RegisteredScalableSystems.Num());
}

bool AMasterWorldController::AreAllSystemsScaled() const
{
    if (RegisteredScalableSystems.Num() == 0)
    {
        return false; // No systems registered
    }
    
    for (UObject* Obj : RegisteredScalableSystems)
    {
        IScalableSystem* System = Cast<IScalableSystem>(Obj);
        if (!System || !System->IsSystemScaled())
        {
            return false;
        }
    }
    
    return true;
}

void AMasterWorldController::SynchronizeSystemCoordinates()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WORLD SCALING] Synchronizing system coordinates"));
    
    for (UObject* Obj : RegisteredScalableSystems)
    {
        IScalableSystem* System = Cast<IScalableSystem>(Obj);
        if (System)
        {
            System->SynchronizeCoordinates(WorldCoordinateSystem);
        }
    }
}

FString AMasterWorldController::GetWorldScalingDebugInfo() const
{
    FString DebugInfo = TEXT("=== WORLD SCALING DEBUG INFO ===\n");
    
    DebugInfo += FString::Printf(TEXT("Terrain Size: %dx%d\n"),
                                WorldScalingConfig.TerrainWidth, WorldScalingConfig.TerrainHeight);
    DebugInfo += FString::Printf(TEXT("Terrain Scale: %.2f\n"), WorldScalingConfig.TerrainScale);
    DebugInfo += FString::Printf(TEXT("Optimal Chunk Size: %d\n"), WorldScalingConfig.OptimalChunkSize);
    DebugInfo += FString::Printf(TEXT("World Origin: %s\n"), *WorldCoordinateSystem.WorldOrigin.ToString());
    DebugInfo += FString::Printf(TEXT("World Scale: %.2f\n"), WorldCoordinateSystem.WorldScale);
    DebugInfo += FString::Printf(TEXT("Total Chunks: %d\n"), WorldCoordinateSystem.TotalChunks);
    DebugInfo += FString::Printf(TEXT("Registered Systems: %d\n"), RegisteredScalableSystems.Num());
    DebugInfo += FString::Printf(TEXT("All Systems Scaled: %s\n"), AreAllSystemsScaled() ? TEXT("YES") : TEXT("NO"));
    
    int32 ScaledCount = 0;
    // Add individual system debug info
    for (UObject* Obj : RegisteredScalableSystems)
    {
        IScalableSystem* System = Cast<IScalableSystem>(Obj);
        if (System && System->IsSystemScaled())
        {
            ScaledCount++;
        }
    }
    
    return DebugInfo;
}

// ===== BRUSH SCALING FUNCTIONS =====

float AMasterWorldController::GetBrushScaleForCurrentWorld() const
{
    FVector2D WorldDims = GetWorldDimensions();
    float BaseSize = 513.0f;
    float CurrentSize = FMath::Max(WorldDims.X, WorldDims.Y);
    return BaseSize / CurrentSize;
}

float AMasterWorldController::GetBrushScaleMultiplier() const
{
    float BaseTerrainSize = 513.0f;
    float CurrentTerrainSize = FMath::Max(WorldScalingConfig.TerrainWidth, WorldScalingConfig.TerrainHeight);
    return CurrentTerrainSize / BaseTerrainSize;
}

bool AMasterWorldController::ValidateWorldPosition(FVector WorldPosition) const
{
    FVector2D WorldBounds = GetWorldBounds();
    FVector LocalPos = WorldPosition - WorldCoordinateSystem.WorldOrigin;
    return (LocalPos.X >= 0.0f && LocalPos.X <= WorldBounds.X &&
            LocalPos.Y >= 0.0f && LocalPos.Y <= WorldBounds.Y);
}

FVector2D AMasterWorldController::GetWorldBounds() const
{
    return WorldCoordinateSystem.TerrainBounds;
}


void AMasterWorldController::CalculateOptimalWorldScaling()
{
    UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Calculating optimal world scaling"));
    
    // Generate world scaling configuration based on terrain
    WorldScalingConfig = GenerateScalingConfig();
    
    // Generate coordinate system
    WorldCoordinateSystem = GenerateCoordinateSystem();
    
    UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Optimal scaling calculated - Terrain: %dx%d, Chunks: %d"),
           WorldScalingConfig.TerrainWidth, WorldScalingConfig.TerrainHeight, WorldCoordinateSystem.TotalChunks);
}

void AMasterWorldController::ConfigureSystemBounds()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[WORLD SCALING] Configuring system bounds"));
    
    // Configure bounds for each system type based on scaling configuration
    // This ensures all systems use consistent boundaries
    
    // TODO: Implement system-specific boundary configuration
    // - Water simulation boundaries
    // - Atmospheric grid boundaries
    // - Ecosystem calculation boundaries
}

FWorldScalingConfig AMasterWorldController::GenerateScalingConfig() const
{
    FWorldScalingConfig Config;
    
    if (MainTerrain && MainTerrain->TerrainWidth > 0)
    {
        // Use actual terrain dimensions
        Config.TerrainWidth = MainTerrain->TerrainWidth;
        Config.TerrainHeight = MainTerrain->TerrainHeight;
        Config.TerrainScale = MainTerrain->TerrainScale;
        
        UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Using terrain dimensions: %dx%d"), Config.TerrainWidth, Config.TerrainHeight);
    }
    else
    {
        // Use default configuration when no terrain exists yet
        Config.TerrainWidth = 513;
        Config.TerrainHeight = 513;
        Config.TerrainScale = 100.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] No terrain found, using defaults: %dx%d"), Config.TerrainWidth, Config.TerrainHeight);
    }
    
    // Calculate optimal chunk size
    int32 TargetChunkCount = FMath::Clamp(Config.TerrainWidth / 32, 16, 64);
    Config.OptimalChunkSize = Config.TerrainWidth / TargetChunkCount;
    
    // Configure water system scaling
    Config.WaterConfig.SimulationArrayWidth = Config.TerrainWidth;
    Config.WaterConfig.SimulationArrayHeight = Config.TerrainHeight;
    Config.WaterConfig.WaterCellScale = Config.TerrainScale;
    Config.WaterConfig.OptimalChunkSize = Config.OptimalChunkSize;
    Config.WaterConfig.CoordinateScale = 1.0f;
    
    // Configure atmospheric system scaling
    Config.AtmosphericConfig.GridWidth = FMath::Max(32, Config.TerrainWidth / 8); // Lower resolution
    Config.AtmosphericConfig.GridHeight = FMath::Max(32, Config.TerrainHeight / 8);
    Config.AtmosphericConfig.GridLayers = 12; // Standard atmospheric layers
    Config.AtmosphericConfig.CellSize = (Config.TerrainWidth * Config.TerrainScale) / Config.AtmosphericConfig.GridWidth;
    Config.AtmosphericConfig.CoordinateScale = 1.0f;
    
    // Configure geology system scaling
    Config.GeologyConfig.ErosionGridWidth = FMath::Max(64, Config.TerrainWidth / 4);
    Config.GeologyConfig.ErosionGridHeight = FMath::Max(64, Config.TerrainHeight / 4);
    Config.GeologyConfig.ErosionCellSize = (Config.TerrainWidth * Config.TerrainScale) / Config.GeologyConfig.ErosionGridWidth;
    Config.GeologyConfig.CoordinateScale = 1.0f;
    Config.GeologyConfig.GeologicalTimeScale = 3600.0f; // 1 hour simulated = 1 second real time
    
    // Configure ecosystem system scaling
    Config.EcosystemConfig.BiomeGridWidth = FMath::Max(32, Config.TerrainWidth / 8);
    Config.EcosystemConfig.BiomeGridHeight = FMath::Max(32, Config.TerrainHeight / 8);
    Config.EcosystemConfig.BiomeCellSize = (Config.TerrainWidth * Config.TerrainScale) / Config.EcosystemConfig.BiomeGridWidth;
    Config.EcosystemConfig.MaxVegetationInstances = FMath::RoundToInt(10000.0f * ((Config.TerrainWidth * Config.TerrainHeight) / (513.0f * 513.0f)));
    Config.EcosystemConfig.MaxVegetationInstances = FMath::Clamp(Config.EcosystemConfig.MaxVegetationInstances, 1000, 100000);
    Config.EcosystemConfig.VegetationDensityScale = 1.0f;
    Config.EcosystemConfig.CoordinateScale = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Generated config - %dx%d @ %.2f scale, Atmo: %dx%dx%d"),
           Config.TerrainWidth, Config.TerrainHeight, Config.TerrainScale,
           Config.AtmosphericConfig.GridWidth, Config.AtmosphericConfig.GridHeight, Config.AtmosphericConfig.GridLayers);
    
    return Config;
}

FWorldCoordinateSystem AMasterWorldController::GenerateCoordinateSystem() const
{
    FWorldCoordinateSystem Coords;
    
    if (MainTerrain)
    {
        // Calculate world coordinate system from terrain
        Coords.WorldOrigin = MainTerrain->GetActorLocation();
        Coords.WorldScale = MainTerrain->TerrainScale;
        
        float WorldSizeX = MainTerrain->TerrainWidth * MainTerrain->TerrainScale;
        float WorldSizeY = MainTerrain->TerrainHeight * MainTerrain->TerrainScale;
        Coords.TerrainBounds = FVector2D(WorldSizeX, WorldSizeY);
        
        // Calculate total chunks
        int32 ChunksX = FMath::CeilToInt((float)MainTerrain->TerrainWidth / WorldScalingConfig.OptimalChunkSize);
        int32 ChunksY = FMath::CeilToInt((float)MainTerrain->TerrainHeight / WorldScalingConfig.OptimalChunkSize);
        Coords.TotalChunks = ChunksX * ChunksY;
        
        UE_LOG(LogTemp, Log, TEXT("[WORLD SCALING] Generated coordinates - Origin: %s, Bounds: %.0fx%.0f"),
               *Coords.WorldOrigin.ToString(), Coords.TerrainBounds.X, Coords.TerrainBounds.Y);
    }
    else
    {
        // Use default coordinate system
        Coords.WorldOrigin = FVector::ZeroVector;
        Coords.WorldScale = 100.0f;
        
        float WorldSizeX = 513 * 100.0f;
        float WorldSizeY = 513 * 100.0f;
        Coords.TerrainBounds = FVector2D(WorldSizeX, WorldSizeY);
        
        // Calculate total chunks with default values
        int32 ChunksX = FMath::CeilToInt(513.0f / 32.0f);
        int32 ChunksY = FMath::CeilToInt(513.0f / 32.0f);
        Coords.TotalChunks = ChunksX * ChunksY;
        
        UE_LOG(LogTemp, Warning, TEXT("[WORLD SCALING] No terrain found, using default coordinates"));
    }
    
    return Coords;
}

// ===== SINGLE SOURCE OF TRUTH IMPLEMENTATION =====

FVector2D AMasterWorldController::GetWorldDimensions() const
{
    return FVector2D(WorldScalingConfig.TerrainWidth, WorldScalingConfig.TerrainHeight);
}

int32 AMasterWorldController::GetOptimalChunkSize() const
{
    return WorldScalingConfig.OptimalChunkSize;
}

FVector2D AMasterWorldController::GetChunkDimensions() const
{
    int32 ChunksX = FMath::CeilToInt((float)WorldScalingConfig.TerrainWidth / WorldScalingConfig.OptimalChunkSize);
    int32 ChunksY = FMath::CeilToInt((float)WorldScalingConfig.TerrainHeight / WorldScalingConfig.OptimalChunkSize);
    return FVector2D(ChunksX, ChunksY);
}

int32 AMasterWorldController::GetWaterGridSize() const
{
    return WorldScalingConfig.WaterConfig.SimulationArrayWidth;
}

float AMasterWorldController::GetTerrainScale() const
{
    return WorldScalingConfig.TerrainScale;
}



void AMasterWorldController::SetWorldDimensions(int32 Width, int32 Height)
{
    UE_LOG(LogTemp, Warning, TEXT("[WORLD AUTHORITY] Changing world dimensions from %dx%d to %dx%d"),
           WorldScalingConfig.TerrainWidth, WorldScalingConfig.TerrainHeight, Width, Height);
    
    WorldScalingConfig.TerrainWidth = Width;
    WorldScalingConfig.TerrainHeight = Height;
    
    const float TARGET_WORLD_SIZE = 51300.0f;
    WorldScalingConfig.TerrainScale = TARGET_WORLD_SIZE / FMath::Max(Width, Height);
    
    // Update water system scaling to match
    WorldScalingConfig.WaterConfig.SimulationArrayWidth = Width;
    WorldScalingConfig.WaterConfig.SimulationArrayHeight = Height;
    
    // Recalculate optimal chunk size
    int32 TargetChunkCount = FMath::Clamp(Width / 32, 16, 64);
    WorldScalingConfig.OptimalChunkSize = Width / TargetChunkCount;
    
    // Propagate changes to all systems
    PropagateWorldChanges();
}

void AMasterWorldController::SetWorldSizeFromUI(EWorldSize NewSize)
{
    UE_LOG(LogTemp, Warning, TEXT("UI requested world size change to: %d"), (int32)NewSize);
    
    int32 Width, Height;
    
    switch (NewSize)
    {
        case EWorldSize::Small:
            Width = Height = 257;
            break;
        case EWorldSize::Medium:
            Width = Height = 513;
            break;
        case EWorldSize::Large:
            Width = Height = 1025;
            break;
        default:
            Width = Height = 513;
            break;
    }
    
    CurrentWorldSize = NewSize;
    
    UE_LOG(LogTemp, Warning, TEXT("MasterController: Setting world size to %s (%dx%d)"),
           NewSize == EWorldSize::Small ? TEXT("Small") :
           NewSize == EWorldSize::Medium ? TEXT("Medium") :
           NewSize == EWorldSize::Large ? TEXT("Large") : TEXT("Unknown"),
           Width, Height);
           
    SetWorldDimensions(Width, Height);
    
    // Force validation after size change
    if (GetWorld())
    {
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimer,
            [this]()
            {
                bool bValid = ValidateWorldAuthority();
                UE_LOG(LogTemp, Warning, TEXT("World size validation: %s"), bValid ? TEXT("PASSED") : TEXT("FAILED"));
            },
            1.0f,
            false
        );
    }
}



void AMasterWorldController::PropagateWorldChanges()
{
    UE_LOG(LogTemp, Warning, TEXT("[WORLD AUTHORITY] Propagating world changes to all systems"));
    
    // Recalculate coordinate system
    WorldCoordinateSystem = GenerateCoordinateSystem();
    
    // Update all registered scalable systems
    for (UObject* Obj : RegisteredScalableSystems)
    {
        IScalableSystem* System = Cast<IScalableSystem>(Obj);
        if (System)
        {
            System->ConfigureFromMaster(WorldScalingConfig);
            System->SynchronizeCoordinates(WorldCoordinateSystem);
        }
    }
    
    // Update main terrain if available (force regeneration)
    if (MainTerrain)
    {
        // Update legacy properties for compatibility
        MainTerrain->TerrainWidth = WorldScalingConfig.TerrainWidth;
        MainTerrain->TerrainHeight = WorldScalingConfig.TerrainHeight;
        MainTerrain->ChunkSize = WorldScalingConfig.OptimalChunkSize;
        
        FVector2D ChunkDims = GetChunkDimensions();
        MainTerrain->ChunksX = ChunkDims.X;
        MainTerrain->ChunksY = ChunkDims.Y;
        
        
        MainTerrain->TerrainScale = WorldScalingConfig.TerrainScale;  // Sync scale
        MainTerrain->RefreshAllChunkMaterials();  // Update all materials!
        
        UE_LOG(LogTemp, Warning, TEXT("[WORLD AUTHORITY] Updated terrain: %dx%d, chunks: %dx%d"),
               MainTerrain->TerrainWidth, MainTerrain->TerrainHeight, MainTerrain->ChunksX, MainTerrain->ChunksY);
    }
}



// ===== AUTHORITATIVE COORDINATE TRANSFORMS =====

FVector2D AMasterWorldController::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    if (MainTerrain)
    {
        // Use main terrain's transform but our authoritative scaling
        FVector LocalPosition = MainTerrain->GetActorTransform().InverseTransformPosition(WorldPosition);
        float TerrainX = LocalPosition.X / GetTerrainScale();
        float TerrainY = LocalPosition.Y / GetTerrainScale();
        
        // Clamp to authoritative world dimensions
        FVector2D WorldDims = GetWorldDimensions();
        TerrainX = FMath::Clamp(TerrainX, 0.0f, WorldDims.X - 1.0f);
        TerrainY = FMath::Clamp(TerrainY, 0.0f, WorldDims.Y - 1.0f);
        
        return FVector2D(TerrainX, TerrainY);
    }
    
    // Fallback calculation using coordinate system
    FVector RelativePos = WorldPosition - WorldCoordinateSystem.WorldOrigin;
    float TerrainX = RelativePos.X / WorldCoordinateSystem.WorldScale;
    float TerrainY = RelativePos.Y / WorldCoordinateSystem.WorldScale;
    
    FVector2D WorldDims = GetWorldDimensions();
    TerrainX = FMath::Clamp(TerrainX, 0.0f, WorldDims.X - 1.0f);
    TerrainY = FMath::Clamp(TerrainY, 0.0f, WorldDims.Y - 1.0f);
    
    return FVector2D(TerrainX, TerrainY);
}

FVector AMasterWorldController::TerrainToWorldPosition(FVector2D TerrainPosition) const
{
    if (MainTerrain)
    {
        // Use authoritative scaling with terrain's transform
        FVector LocalPosition = FVector(
            TerrainPosition.X * GetTerrainScale(),
            TerrainPosition.Y * GetTerrainScale(),
            0.0f
        );
        
        return MainTerrain->GetActorTransform().TransformPosition(LocalPosition);
    }
    
    // Fallback using coordinate system
    FVector WorldPosition = WorldCoordinateSystem.WorldOrigin + FVector(
        TerrainPosition.X * WorldCoordinateSystem.WorldScale,
        TerrainPosition.Y * WorldCoordinateSystem.WorldScale,
        0.0f
    );
    
    return WorldPosition;
}

// ===== SINGLE SOURCE OF TRUTH VALIDATION =====

bool AMasterWorldController::ValidateWorldAuthority() const
{
    UE_LOG(LogTemp, Warning, TEXT("[AUTHORITY VALIDATION] Testing single source of truth..."));
    
    bool bValidationPassed = true;
    
    // Test 1: Verify MasterController dimensions match terrain
    if (MainTerrain)
    {
        FVector2D AuthoritativeDims = GetWorldDimensions();
        if (MainTerrain->TerrainWidth != AuthoritativeDims.X || MainTerrain->TerrainHeight != AuthoritativeDims.Y)
        {
            UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VALIDATION] FAILED - Terrain dimensions mismatch: Master(%dx%d) vs Terrain(%d,%d)"),
            (int32)AuthoritativeDims.X, (int32)AuthoritativeDims.Y, MainTerrain->TerrainWidth, MainTerrain->TerrainHeight);
            bValidationPassed = false;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[AUTHORITY VALIDATION] PASS - Terrain dimensions match: %dx%d"),
            (int32)AuthoritativeDims.X, (int32)AuthoritativeDims.Y);
        }
    }
    
    // Test 2: Verify water system uses same dimensions
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        int32 AuthoritativeWaterGrid = GetWaterGridSize();
        UE_LOG(LogTemp, Log, TEXT("[AUTHORITY VALIDATION] Water grid size from authority: %d"), AuthoritativeWaterGrid);
    }
    
    // Test 3: Coordinate roundtrip accuracy test
    FVector TestWorldPos(1000.0f, 2000.0f, 0.0f);
    FVector2D TerrainCoords = WorldToTerrainCoordinates(TestWorldPos);
    FVector BackToWorld = TerrainToWorldPosition(TerrainCoords);
    
    float ErrorDistance = FVector::Dist(TestWorldPos, BackToWorld);
    if (ErrorDistance < 1.0f) // Allow 1cm error
    {
        UE_LOG(LogTemp, Log, TEXT("[AUTHORITY VALIDATION] PASS - Coordinate transform accurate (error: %.2fcm)"), ErrorDistance);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VALIDATION] FAILED - Coordinate transform inaccurate (error: %.2fcm)"), ErrorDistance);
        bValidationPassed = false;
    }
    
    // Test 4: Verify all systems match exactly (from analysis document)
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        FVector2D Dimensions = GetWorldDimensions();
        int32 WaterGridSize = GetWaterGridSize();
        
        UE_LOG(LogTemp, Log, TEXT("[AUTHORITY VALIDATION] Authority dimensions: %dx%d, Water grid: %d"),
               (int32)Dimensions.X, (int32)Dimensions.Y, WaterGridSize);
        
        // This test ensures "AllMaterials->HaveCorrectUVParameters()" equivalent
        if (MainTerrain->TerrainWidth == Dimensions.X && WaterGridSize == Dimensions.X)
        {
            UE_LOG(LogTemp, Log, TEXT("[AUTHORITY VALIDATION] PASS - All systems use consistent dimensions"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VALIDATION] FAILED - Dimension mismatch between systems"));
            bValidationPassed = false;
        }
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTHORITY VALIDATION] SUCCESS - Single source of truth established!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTHORITY VALIDATION] FAILED - Single source of truth violations detected!"));
    }
    
    return bValidationPassed;
}

FString AMasterWorldController::GetAuthorityDebugInfo() const
{
    FString DebugInfo = TEXT("=== SINGLE SOURCE OF TRUTH DEBUG ===\n");
    
    FVector2D AuthoritativeDims = GetWorldDimensions();
    int32 AuthoritativeChunkSize = GetOptimalChunkSize();
    FVector2D ChunkDims = GetChunkDimensions();
    
    DebugInfo += FString::Printf(TEXT("AUTHORITATIVE DIMENSIONS: %dx%d\n"), (int32)AuthoritativeDims.X, (int32)AuthoritativeDims.Y);
    DebugInfo += FString::Printf(TEXT("AUTHORITATIVE CHUNK SIZE: %d\n"), AuthoritativeChunkSize);
    DebugInfo += FString::Printf(TEXT("AUTHORITATIVE CHUNK GRID: %dx%d\n"), (int32)ChunkDims.X, (int32)ChunkDims.Y);
    DebugInfo += FString::Printf(TEXT("AUTHORITATIVE WATER GRID: %d\n"), GetWaterGridSize());
    DebugInfo += FString::Printf(TEXT("AUTHORITATIVE TERRAIN SCALE: %.2f\n"), GetTerrainScale());
    
    if (MainTerrain)
    {
        DebugInfo += TEXT("\nTERRAIN COMPARISON:\n");
        DebugInfo += FString::Printf(TEXT("  Terrain Width: %d (should be %d)\n"), MainTerrain->TerrainWidth, (int32)AuthoritativeDims.X);
        DebugInfo += FString::Printf(TEXT("  Terrain Height: %d (should be %d)\n"), MainTerrain->TerrainHeight, (int32)AuthoritativeDims.Y);
        DebugInfo += FString::Printf(TEXT("  Chunk Size: %d (should be %d)\n"), MainTerrain->ChunkSize, AuthoritativeChunkSize);
        DebugInfo += FString::Printf(TEXT("  ChunksX: %d (should be %d)\n"), MainTerrain->ChunksX, (int32)ChunkDims.X);
        DebugInfo += FString::Printf(TEXT("  ChunksY: %d (should be %d)\n"), MainTerrain->ChunksY, (int32)ChunkDims.Y);
        
        bool bMatches = (MainTerrain->TerrainWidth == AuthoritativeDims.X &&
                        MainTerrain->TerrainHeight == AuthoritativeDims.Y &&
                        MainTerrain->ChunkSize == AuthoritativeChunkSize);
        DebugInfo += FString::Printf(TEXT("  AUTHORITY MATCH: %s\n"), bMatches ? TEXT("YES") : TEXT("NO"));
    }
    
    return DebugInfo;
}


// ===== WORLD SIZE HELPER FUNCTIONS =====

FString AMasterWorldController::GetWorldSizeDisplayName() const
{
    switch (CurrentWorldSize)
    {
        case EWorldSize::Small:   return TEXT("Small (257x257)");
        case EWorldSize::Medium:  return TEXT("Medium (513x513)");
        case EWorldSize::Large:   return TEXT("Large (1025x1025)");
        default: return TEXT("Unknown");
    }
}

void AMasterWorldController::LogAllSystemScalingStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM SCALING STATUS ==="));
    
    UE_LOG(LogTemp, Log, TEXT("Registered scalable systems: %d"), RegisteredScalableSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("All systems scaled: %s"), AreAllSystemsScaled() ? TEXT("YES") : TEXT("NO"));
    
    // Log detailed scaling information
    FString DebugInfo = GetWorldScalingDebugInfo();
    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugInfo);
    
    // Test water system specifically
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Water system status:"));
        UE_LOG(LogTemp, Log, TEXT("  - Registered with master: %s"),
               MainTerrain->WaterSystem->IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - Scaled by master: %s"),
               MainTerrain->WaterSystem->IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *MainTerrain->WaterSystem->GetScalingDebugInfo());
    }
    
    // Test atmospheric system specifically
    if (MainTerrain && MainTerrain->AtmosphericSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Atmospheric system status:"));
        UE_LOG(LogTemp, Log, TEXT("  - Registered with master: %s"),
               MainTerrain->AtmosphericSystem->IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - Scaled by master: %s"),
               MainTerrain->AtmosphericSystem->IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *MainTerrain->AtmosphericSystem->GetScalingDebugInfo());
    }
    
    // Test geology controller specifically
    if (GeologyController)
    {
        UE_LOG(LogTemp, Log, TEXT("Geology controller status:"));
        UE_LOG(LogTemp, Log, TEXT("  - Registered with master: %s"),
               GeologyController->IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - Scaled by master: %s"),
               GeologyController->IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *GeologyController->GetScalingDebugInfo());
    }
    
    // Test ecosystem controller specifically
    if (EcosystemController)
    {
        UE_LOG(LogTemp, Log, TEXT("Ecosystem controller status:"));
        UE_LOG(LogTemp, Log, TEXT("  - Registered with master: %s"),
               EcosystemController->IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - Scaled by master: %s"),
               EcosystemController->IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *EcosystemController->GetScalingDebugInfo());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END SCALING STATUS ==="));
}

// ===== WATER BUDGET CONSOLE COMMAND =====

void AMasterWorldController::CheckWaterBudget()
{
    FWaterDistribution Dist = GetWaterDistribution();
    bool bConserved = IsWaterConserved(1.0f); // 1% tolerance
    
    UE_LOG(LogTemp, Warning, TEXT("========== WATER BUDGET CHECK =========="));
    UE_LOG(LogTemp, Warning, TEXT("Total Water: %.0f mÃ‚Â³"), Dist.TotalWater);
    UE_LOG(LogTemp, Warning, TEXT("  Surface:     %.0f mÃ‚Â³ (%.1f%%)"),
           Dist.SurfaceWater, Dist.SurfacePercent);
    UE_LOG(LogTemp, Warning, TEXT("  Atmospheric: %.0f mÃ‚Â³ (%.1f%%)"),
           Dist.AtmosphericWater, Dist.AtmosphericPercent);
    UE_LOG(LogTemp, Warning, TEXT("  Groundwater: %.0f mÃ‚Â³ (%.1f%%)"),
           Dist.Groundwater, Dist.GroundwaterPercent);
    UE_LOG(LogTemp, Warning, TEXT("Conservation: %s"),
           bConserved ? TEXT("Ã¢Å“â€¦ PASS") : TEXT("Ã¢Å¡Â Ã¯Â¸Â DRIFT DETECTED"));
    UE_LOG(LogTemp, Warning, TEXT("========================================="));
}

// ===== WATER BUDGET IMPLEMENTATION =====
void AMasterWorldController::UpdateSystemWaterBudget()
{
    if (!MainTerrain || !MainTerrain->WaterSystem || !MainTerrain->AtmosphericSystem)
        return;
    
    // Throttle updates for performance
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastBudgetUpdateTime < MinBudgetUpdateInterval)
        return;
    LastBudgetUpdateTime = CurrentTime;
    
    // Get current distribution
    FWaterDistribution Distribution = GetWaterDistribution();
    
    // Simple conservation check
    static float InitialTotalWater = -1.0f;
    if (InitialTotalWater < 0.0f)
    {
        InitialTotalWater = Distribution.TotalWater;
        UE_LOG(LogTemp, Warning, TEXT("[WATER BUDGET] System initialized with %.0f mÃ‚Â³"),
               InitialTotalWater);
    }
    
    // Check conservation periodically
    if (CurrentTime - LastConservationCheckTime > ConservationCheckInterval)
    {
        float Drift = FMath::Abs(Distribution.TotalWater - InitialTotalWater);
        float DriftPercent = InitialTotalWater > 0 ? (Drift / InitialTotalWater) * 100.0f : 0.0f;
        
        if (Drift > 1.0f) // 1 mÃ‚Â³ tolerance
        {
            UE_LOG(LogTemp, Warning, TEXT("[WATER BUDGET] Conservation drift: %.2f mÃ‚Â³ (%.3f%%)"),
                   Drift, DriftPercent);
        }
        
        // Log distribution
        UE_LOG(LogTemp, Log, TEXT("[WATER BUDGET] Total: %.0f mÃ‚Â³ | Surface: %.0f mÃ‚Â³ (%.1f%%) | Atmosphere: %.0f mÃ‚Â³ (%.1f%%) | Groundwater: %.0f mÃ‚Â³ (%.1f%%)"),
               Distribution.TotalWater,
               Distribution.SurfaceWater, Distribution.SurfacePercent,
               Distribution.AtmosphericWater, Distribution.AtmosphericPercent,
               Distribution.Groundwater, Distribution.GroundwaterPercent);
        
        LastConservationCheckTime = CurrentTime;
    }
}

bool AMasterWorldController::IsWaterConserved(float Tolerance) const
{
    // Simple conservation check without authority tracking
    static float InitialTotal = -1.0f;
    float CurrentTotal = GetTotalWaterVolume();
    
    if (InitialTotal < 0.0f)
    {
        InitialTotal = CurrentTotal;
        return true;
    }
    
    float Drift = FMath::Abs(CurrentTotal - InitialTotal);
    float PercentDrift = InitialTotal > 0 ? (Drift / InitialTotal) * 100.0f : 0.0f;
    
    return PercentDrift <= Tolerance;
}


FString AMasterWorldController::GetWaterBudgetDebugString() const
{
    FWaterDistribution Dist = GetWaterDistribution();
    
    return FString::Printf(
        TEXT("Water: %.0f mÃ‚Â³ [Surface: %.1f%% | Atmos: %.1f%% | Ground: %.1f%%]"),
        Dist.TotalWater,
        Dist.SurfacePercent,
        Dist.AtmosphericPercent,
        Dist.GroundwaterPercent
    );
}

void AMasterWorldController::ResetWaterBudget()
{
    // Clear cache to force recalculation
    CachedSurfaceWater = -1.0f;
    CachedAtmosphericWater = -1.0f;
    LastCacheUpdateTime = 0.0f;
    
    // Reset groundwater if geology controller exists
    if (GeologyController)
    {
        TotalGroundwater = GeologyController->GetGroundwaterVolume();
    }
    else
    {
        TotalGroundwater = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[WATER BUDGET] Reset complete"));
}

float AMasterWorldController::GetAtmosphericCellWaterVolume(float MoistureMass) const
{
    // Convert moisture mass (kg/mÃ‚Â²) to volume (mÃ‚Â³)
    // Assuming 1 kg/mÃ‚Â² = 0.001 mÃ‚Â³/mÃ‚Â² for atmospheric moisture
    return MoistureMass * 0.001f;
}

float AMasterWorldController::GetWaterCellVolume(float WaterDepth) const
{
    // Convert water depth (simulation units) to volume (mÃ‚Â³)
    // WaterDepth is in simulation units where 1.0 = 1cm of water
    float ActualDepthMeters = WaterDepth * WATER_DEPTH_SCALE;
    
    // Calculate actual water cell size
    float WaterCellSize = 1.0f; // Water cells are 1:1 with terrain vertices
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        // Water grid matches terrain dimensions, so cell size = world size / grid size
        float WorldSize = GetTerrainScale() * GetWorldDimensions().X;
        float GridSize = MainTerrain->WaterSystem->SimulationData.TerrainWidth;
        if (GridSize > 0)
        {
            WaterCellSize = WorldSize / GridSize;
        }
    }
    
    float CellArea = WaterCellSize * WaterCellSize;
    return ActualDepthMeters * CellArea;
}

float AMasterWorldController::GetGeologyCellWaterVolume(float SaturatedDepth, float Porosity) const
{
    // Convert saturated depth to water volume in geology
    // Use geology cell size from configuration
    float GeologyCellSize = WorldScalingConfig.GeologyConfig.ErosionCellSize;
    if (GeologyCellSize <= 0.0f)
    {
        // Fallback: Calculate from terrain dimensions
        if (GeologyController && GeologyController->GeologyGridWidth > 0)
        {
            float WorldSize = GetTerrainScale() * GetWorldDimensions().X;
            GeologyCellSize = WorldSize / GeologyController->GeologyGridWidth;
        }
        else
        {
            GeologyCellSize = 400.0f; // Default geology cell size
        }
    }
    
    float CellArea = GeologyCellSize * GeologyCellSize;
    return SaturatedDepth * CellArea * Porosity;
}


// ===== GRID CONVERSION FUNCTIONS =====

FVector2D AMasterWorldController::ConvertAtmosphericToWaterGrid(FVector2D AtmosPos) const
{
    // Convert atmospheric grid position to water grid position
    // These should be 1:1 for most cases, but handle scaling if needed
    if (MainTerrain && MainTerrain->AtmosphericSystem && MainTerrain->WaterSystem)
    {
        // Get grid dimensions
        float AtmosGridWidth = 64.0f; // Default atmospheric grid size
        float WaterGridWidth = MainTerrain->WaterSystem->SimulationData.WaterDepthMap.Num();
        
        float ScaleFactor = WaterGridWidth / AtmosGridWidth;
        return AtmosPos * ScaleFactor;
    }
    
    return AtmosPos; // 1:1 fallback
}

FVector2D AMasterWorldController::ConvertWaterToAtmosphericGrid(FVector2D WaterPos) const
{
    // Convert water grid position to atmospheric grid position
    if (MainTerrain && MainTerrain->AtmosphericSystem && MainTerrain->WaterSystem)
    {
        float AtmosGridWidth = 64.0f; // Default atmospheric grid size
        float WaterGridWidth = MainTerrain->WaterSystem->SimulationData.WaterDepthMap.Num();
        
        float ScaleFactor = AtmosGridWidth / WaterGridWidth;
        return WaterPos * ScaleFactor;
    }
    
    return WaterPos; // 1:1 fallback
}

void AMasterWorldController::DelayedInitializeWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterWorldController: Starting delayed initialization"));
    
    // Find all terrain actors FIRST, before they try to find us
    TArray<AActor*> FoundTerrains;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicTerrain::StaticClass(), FoundTerrains);
    
    for (AActor* TerrainActor : FoundTerrains)
    {
        ADynamicTerrain* Terrain = Cast<ADynamicTerrain>(TerrainActor);
        if (Terrain)
        {
            // Force terrain to wait for us
            Terrain->SetActorTickEnabled(false);
            UE_LOG(LogTemp, Warning, TEXT("MasterController: Found terrain, disabling its tick"));
        }
    }
    
    // Now initialize world systems
    InitializeWorld();
    
    // Re-enable all systems after initialization
    SetActorTickEnabled(true);
    
    for (AActor* TerrainActor : FoundTerrains)
    {
        ADynamicTerrain* Terrain = Cast<ADynamicTerrain>(TerrainActor);
        if (Terrain)
        {
            Terrain->SetActorTickEnabled(true);
            UE_LOG(LogTemp, Warning, TEXT("MasterController: Re-enabled terrain tick"));
        }
    }
}

void AMasterWorldController::ApplyGameInstanceSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("MasterController: Applying settings from GameInstance"));
    
    if (UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance()))
    {
        // Apply world size setting
        EWorldSize GameInstanceWorldSize = GameInstance->WorldSize;
        UE_LOG(LogTemp, Warning, TEXT("MasterController: Found GameInstance world size: %d"), (int32)GameInstanceWorldSize);
        
        SetWorldSizeFromUI(GameInstanceWorldSize);
        
        // Log other settings for future implementation
        UE_LOG(LogTemp, Log, TEXT("MasterController: GameInstance DefaultTexture: %d"), (int32)GameInstance->DefaultTexture);
        UE_LOG(LogTemp, Log, TEXT("MasterController: GameInstance WaterPhysics: %s"), GameInstance->bEnableWaterPhysics ? TEXT("Enabled") : TEXT("Disabled"));
        
        UE_LOG(LogTemp, Warning, TEXT("MasterController: GameInstance settings applied successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MasterController: Failed to get TerrAIGameInstance"));
    }
}

// ===== BRUSH SCALING IMPLEMENTATION =====

float AMasterWorldController::GetScaledBrushRadius(float BaseBrushRadius) const
{
   
    float WorldScale = GetTerrainScale();
    return BaseBrushRadius * WorldScale;
}


// ===== UNIVERSAL BRUSH SYSTEM IMPLEMENTATION =====

void AMasterWorldController::UpdateUniversalBrushSettings(const FUniversalBrushSettings& NewSettings)
{
    UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Updating brush settings - Radius: %.1f, Strength: %.1f"),
           NewSettings.BrushRadius, NewSettings.BrushStrength);
    
    UniversalBrushSettings = NewSettings;
    
    // Validate settings
    UniversalBrushSettings.BrushRadius = FMath::Clamp(UniversalBrushSettings.BrushRadius, 10.0f, 5000.0f);
    UniversalBrushSettings.BrushStrength = FMath::Clamp(UniversalBrushSettings.BrushStrength, 1.0f, 10000.0f);
    UniversalBrushSettings.InnerRadius = FMath::Clamp(UniversalBrushSettings.InnerRadius, 0.0f, 0.9f);
    UniversalBrushSettings.OuterRadius = FMath::Clamp(UniversalBrushSettings.OuterRadius, 0.1f, 1.0f);
    
    // Ensure inner radius is less than outer radius
    if (UniversalBrushSettings.InnerRadius >= UniversalBrushSettings.OuterRadius)
    {
        UniversalBrushSettings.InnerRadius = UniversalBrushSettings.OuterRadius - 0.1f;
    }
    
    // Propagate to all registered brush receivers
    for (auto& Receiver : RegisteredBrushReceivers)
    {
        if (Receiver.GetInterface())
        {
            Receiver.GetInterface()->UpdateBrushSettings(UniversalBrushSettings);
        }
    }
    
    // Update material parameters
    SyncBrushToMaterials();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[UNIVERSAL BRUSH] Settings propagated to %d receivers"),
           RegisteredBrushReceivers.Num());
}

void AMasterWorldController::SetBrushRadius(float NewRadius)
{
    float ClampedRadius = FMath::Clamp(NewRadius, 10.0f, 5000.0f);
    
    if (UniversalBrushSettings.BrushRadius != ClampedRadius)
    {
        UniversalBrushSettings.BrushRadius = ClampedRadius;
        
        // Create updated settings and propagate
        FUniversalBrushSettings UpdatedSettings = UniversalBrushSettings;
        UpdateUniversalBrushSettings(UpdatedSettings);
        
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Brush radius set to %.1f"), ClampedRadius);
    }
}

void AMasterWorldController::SetBrushStrength(float NewStrength)
{
    float ClampedStrength = FMath::Clamp(NewStrength, 1.0f, 10000.0f);
    
    if (UniversalBrushSettings.BrushStrength != ClampedStrength)
    {
        UniversalBrushSettings.BrushStrength = ClampedStrength;
        
        // Create updated settings and propagate
        FUniversalBrushSettings UpdatedSettings = UniversalBrushSettings;
        UpdateUniversalBrushSettings(UpdatedSettings);
        
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Brush strength set to %.1f"), ClampedStrength);
    }
}

void AMasterWorldController::SetBrushFalloffType(EBrushFalloffType NewType)
{
    if (UniversalBrushSettings.FalloffType != NewType)
    {
        UniversalBrushSettings.FalloffType = NewType;
        
        // Create updated settings and propagate
        FUniversalBrushSettings UpdatedSettings = UniversalBrushSettings;
        UpdateUniversalBrushSettings(UpdatedSettings);
        
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Falloff type changed to %d"), (int32)NewType);
    }
}

float AMasterWorldController::CalculateBrushFalloff(float Distance, const FUniversalBrushSettings& Settings) const
{
    float BrushRadius = Settings.BrushRadius;
    
    // OPTION 3: Extend falloff beyond the stated radius for softer edges
    // This creates a more natural terrain modification similar to the original system
    const float FalloffExtension = 1.5f; // Extend falloff to 150% of radius
    float ExtendedRadius = BrushRadius * FalloffExtension;
    
    // For backwards compatibility with original behavior,
    // if InnerRadius is 0, use the original quadratic falloff
    if (Settings.InnerRadius <= 0.001f)
    {
        // Original quadratic falloff behavior
        if (Distance >= ExtendedRadius)
        {
            return 0.0f;
        }
        
        // Classic quadratic falloff: (1 - distance/radius)^2
        float NormalizedDistance = Distance / ExtendedRadius;
        return FMath::Pow(1.0f - NormalizedDistance, 2.0f);
    }
    
    // Otherwise use the advanced inner/outer radius system
    float InnerRadius = Settings.InnerRadius * BrushRadius;
    float OuterRadius = Settings.OuterRadius * ExtendedRadius; // Use extended radius
    
    // Early exit if outside extended brush range
    if (Distance >= OuterRadius)
    {
        return 0.0f;
    }
    
    // Full strength within inner radius
    if (Distance <= InnerRadius)
    {
        return 1.0f;
    }
    
    // Calculate falloff between inner and outer radius
    float FalloffRange = OuterRadius - InnerRadius;
    float NormalizedDistance = (Distance - InnerRadius) / FalloffRange;
    
    // Apply different falloff curves based on type
    float FalloffValue = 0.0f;
    
    switch (Settings.FalloffType)
    {
        case EBrushFalloffType::Linear:
            FalloffValue = 1.0f - NormalizedDistance;
            break;
            
        case EBrushFalloffType::Smooth:
            // Smoothstep function for natural falloff
            FalloffValue = 1.0f - (NormalizedDistance * NormalizedDistance * (3.0f - 2.0f * NormalizedDistance));
            break;
            
        case EBrushFalloffType::Gaussian:
            // Gaussian-like falloff for soft edges
            FalloffValue = FMath::Exp(-Settings.FalloffExponent * NormalizedDistance * NormalizedDistance);
            break;
            
        case EBrushFalloffType::Exponential:
            // Exponential falloff for sharp edges
            FalloffValue = FMath::Pow(1.0f - NormalizedDistance, Settings.FalloffExponent);
            break;
            
        case EBrushFalloffType::Custom:
            // Use custom curve if available
            if (CustomBrushCurve)
            {
                FalloffValue = CustomBrushCurve->GetFloatValue(NormalizedDistance);
            }
            else
            {
                // Fallback to smooth if no custom curve
                FalloffValue = 1.0f - (NormalizedDistance * NormalizedDistance * (3.0f - 2.0f * NormalizedDistance));
            }
            break;
            
        default:
            FalloffValue = 1.0f - NormalizedDistance;
            break;
    }
    
    return FMath::Clamp(FalloffValue, 0.0f, 1.0f);
}

void AMasterWorldController::ApplyBrushToReceivers(FVector WorldPosition, float DeltaTime)
{
    if (RegisteredBrushReceivers.Num() == 0)
    {
        return;
    }
    
    // Validate world position
    if (!ValidateWorldPosition(WorldPosition))
    {
        return;
    }
    
    // Get scaled brush settings for current world
    FUniversalBrushSettings ScaledSettings = GetScaledBrushSettings();
    
    // Track performance
    double StartTime = FPlatformTime::Seconds();
    int32 CellsProcessed = 0;
    
    // Apply brush to all registered receivers
    for (auto& Receiver : RegisteredBrushReceivers)
    {
        if (Receiver.GetInterface() && Receiver.GetInterface()->CanReceiveBrush())
        {
            Receiver.GetInterface()->ApplyBrush(WorldPosition, ScaledSettings, DeltaTime);
            CellsProcessed++;
        }
    }
    
    // Update performance metrics
    LastBrushApplicationTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to ms
    LastBrushCellsProcessed = CellsProcessed;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[UNIVERSAL BRUSH] Applied to %d receivers in %.2fms"),
           CellsProcessed, LastBrushApplicationTime);
}

void AMasterWorldController::RegisterBrushReceiver(TScriptInterface<IBrushReceiver> Receiver)
{
    if (Receiver.GetInterface())
    {
        RegisteredBrushReceivers.AddUnique(Receiver);
        
        // Send current brush settings to new receiver
        Receiver.GetInterface()->UpdateBrushSettings(UniversalBrushSettings);
        
        UE_LOG(LogTemp, Warning, TEXT("[UNIVERSAL BRUSH] Registered brush receiver - Total: %d"),
               RegisteredBrushReceivers.Num());
    }
}

void AMasterWorldController::UnregisterBrushReceiver(TScriptInterface<IBrushReceiver> Receiver)
{
    if (Receiver.GetInterface())
    {
        RegisteredBrushReceivers.Remove(Receiver);
        
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Unregistered brush receiver - Total: %d"),
               RegisteredBrushReceivers.Num());
    }
}

void AMasterWorldController::SyncBrushToMaterials()
{
    if (!BrushParameterCollection)
    {
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] No material parameter collection assigned"));
        return;
    }
    
    // Use UE5.4's improved material parameter system
    if (GetWorld())
    {
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushRadius"), UniversalBrushSettings.BrushRadius
        );
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushStrength"), UniversalBrushSettings.BrushStrength
        );
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushInnerRadius"), UniversalBrushSettings.InnerRadius
        );
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushOuterRadius"), UniversalBrushSettings.OuterRadius
        );
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushFalloffExponent"), UniversalBrushSettings.FalloffExponent
        );
        UKismetMaterialLibrary::SetVectorParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushColor"),
            FLinearColor(UniversalBrushSettings.BrushColor.R, UniversalBrushSettings.BrushColor.G, UniversalBrushSettings.BrushColor.B, UniversalBrushSettings.BrushColor.A)
        );
        
        // Falloff type as scalar for material switching
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), BrushParameterCollection,
            FName("BrushFalloffType"), (float)(int32)UniversalBrushSettings.FalloffType
        );
        
        UE_LOG(LogTemp, Log, TEXT("[UNIVERSAL BRUSH] Synced parameters to material collection"));
    }
}

FUniversalBrushSettings AMasterWorldController::GetScaledBrushSettings() const
{
    FUniversalBrushSettings ScaledSettings = UniversalBrushSettings;
    
    // Apply world scaling to brush radius
    float ScaleMultiplier = GetBrushScaleMultiplier();
    ScaledSettings.BrushRadius *= ScaleMultiplier;
    
    return ScaledSettings;
}


// ===== WORLD SCALING TEST FUNCTIONS =====

void AMasterWorldController::TestWorldScalingIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST - Starting integration test"));
    
    // Test 1: Check if we have a terrain
    if (!MainTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST FAILED - No MainTerrain found"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - MainTerrain found"));
    
    // Test 2: Check if we have a water system
    if (!MainTerrain->WaterSystem && MainTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST FAILED - No WaterSystem found"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - WaterSystem found"));
    
    // Test 3: Check if water system implements IScalableSystem
    if (!MainTerrain->WaterSystem->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST FAILED - WaterSystem does not implement IScalableSystem"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - WaterSystem implements IScalableSystem"));
    
    // Test 4: Check if water system is registered
    if (!MainTerrain->WaterSystem->IsRegisteredWithMaster())
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST FAILED - WaterSystem not registered with master"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - WaterSystem registered with master"));
    
    // Test 5: Check if water system is scaled
    if (!MainTerrain->WaterSystem->IsSystemScaled())
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST FAILED - WaterSystem not scaled by master"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - WaterSystem scaled by master"));
    
    // Test 6: Check atmospheric system if available
    if (MainTerrain->AtmosphericSystem)
    {
        if (!MainTerrain->AtmosphericSystem->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - AtmosphericSystem does not implement IScalableSystem"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - AtmosphericSystem implements IScalableSystem"));
        
        if (!MainTerrain->AtmosphericSystem->IsRegisteredWithMaster())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - AtmosphericSystem not registered with master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - AtmosphericSystem registered with master"));
        
        if (!MainTerrain->AtmosphericSystem->IsSystemScaled())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - AtmosphericSystem not scaled by master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - AtmosphericSystem scaled by master"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST WARNING - No AtmosphericSystem found (optional)"));
    }
    
    // Test 7: Check geology controller if available
    if (GeologyController)
    {
        if (!GeologyController->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - GeologyController does not implement IScalableSystem"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - GeologyController implements IScalableSystem"));
        
        if (!GeologyController->IsRegisteredWithMaster())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - GeologyController not registered with master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - GeologyController registered with master"));
        
        if (!GeologyController->IsSystemScaled())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - GeologyController not scaled by master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - GeologyController scaled by master"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST WARNING - No GeologyController found (optional)"));
    }
    
    // Test 8: Check ecosystem controller if available
    if (EcosystemController)
    {
        if (!EcosystemController->GetClass()->ImplementsInterface(UScalableSystem::StaticClass()))
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - EcosystemController does not implement IScalableSystem"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - EcosystemController implements IScalableSystem"));
        
        if (!EcosystemController->IsRegisteredWithMaster())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - EcosystemController not registered with master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - EcosystemController registered with master"));
        
        if (!EcosystemController->IsSystemScaled())
        {
            UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - EcosystemController not scaled by master"));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - EcosystemController scaled by master"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST WARNING - No EcosystemController found (optional)"));
    }
    
    // Test 9: Check scaling configuration - Fixed string handling
    if (MainTerrain->WaterSystem)
    {
        FString WaterScalingInfo = MainTerrain->WaterSystem->GetScalingDebugInfo();
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST Water Info: %s"), *WaterScalingInfo);
    }
    
    if (MainTerrain->AtmosphericSystem)
    {
        FString AtmosphericScalingInfo = MainTerrain->AtmosphericSystem->GetScalingDebugInfo();
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST Atmospheric Info: %s"), *AtmosphericScalingInfo);
    }
    
    // Test 10: Verify coordinate transformation using MasterController authority
    FVector TestWorldPos(1000.0f, 2000.0f, 0.0f);
    FVector2D TerrainCoords = this->WorldToTerrainCoordinates(TestWorldPos);  // Use MasterController authority
    FVector BackToWorld = this->TerrainToWorldPosition(TerrainCoords);
    
    float ErrorDistance = FVector::Dist(TestWorldPos, BackToWorld);
    if (ErrorDistance < 1.0f) // Allow 1cm error
    {
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - Coordinate transformation accurate (error: %.2fcm)"), ErrorDistance);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - Coordinate transformation inaccurate (error: %.2fcm)"), ErrorDistance);
        return;
    }
    
    // Test 11: World size function integration
    EWorldSize OriginalSize = CurrentWorldSize;
    
    // Test small world
    SetWorldSizeFromUI(EWorldSize::Small);
    FVector2D SmallDims = GetWorldDimensions();
    if (SmallDims.X == 257 && SmallDims.Y == 257)
    {
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - Small world dimensions correct"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - Small world dimensions wrong: %.0fx%.0f"), SmallDims.X, SmallDims.Y);
        return;
    }
    
    // Test large world
    SetWorldSizeFromUI(EWorldSize::Large);
    FVector2D LargeDims = GetWorldDimensions();
    if (LargeDims.X == 1025 && LargeDims.Y == 1025)
    {
        UE_LOG(LogTemp, Log, TEXT("WORLD SCALING TEST PASS - Large world dimensions correct"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WORLD SCALING TEST FAILED - Large world dimensions wrong: %.0fx%.0f"), LargeDims.X, LargeDims.Y);
        return;
    }
    
    // Restore original size
    SetWorldSizeFromUI(OriginalSize);
    
    UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST ALL TESTS PASSED - Integration successful!"));
    UE_LOG(LogTemp, Warning, TEXT("\n=== WORLD SIZE SYSTEM READY ===\nUI Buttons can now connect to:\n- SetSmallWorld()\n- SetMediumWorld()\n- SetLargeWorld()\n================================"));
}

void AMasterWorldController::TestWorldScalingIntegrationWithWait()
{
    UE_LOG(LogTemp, Warning, TEXT("ENHANCED WORLD SCALING TEST - Starting with initialization check"));
    
    // STEP 1: Check if initialization is complete
    if (CurrentInitPhase != EInitializationPhase::Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("WORLD SCALING TEST - Initialization incomplete, forcing completion"));
        
        // Force system finding/creation if missing
        FindOrCreateSystemControllers();
        
        // Force re-registration
        ForceSystemReregistration();
        
        // Set phase to complete for testing
        CurrentInitPhase = EInitializationPhase::Complete;
    }
    
    // STEP 2: Force re-registration to ensure all systems are registered
    ForceSystemReregistration();
    
    // STEP 3: Wait a frame then run the original test
    FTimerHandle TestTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TestTimerHandle, [this]()
    {
        TestWorldScalingIntegration();
    }, 0.1f, false);
}

void AMasterWorldController::LogDetailedScalingInfo()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DETAILED SCALING INFO ==="));
    
    UE_LOG(LogTemp, Warning, TEXT("Current World Size: %s"), *GetWorldSizeDisplayName());
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %dx%d @ %.1f scale"),
           MainTerrain ? MainTerrain->TerrainWidth : 0,
           MainTerrain ? MainTerrain->TerrainHeight : 0,
           MainTerrain ? MainTerrain->TerrainScale : 0.0f);
    
    const FWorldScalingConfig& Config = GetWorldScalingConfig();
    UE_LOG(LogTemp, Warning, TEXT("Expected Atmospheric Grid: %dx%dx%d, CellSize: %.1fm"),
           Config.AtmosphericConfig.GridWidth, Config.AtmosphericConfig.GridHeight,
           Config.AtmosphericConfig.GridLayers, Config.AtmosphericConfig.CellSize);
    
    UE_LOG(LogTemp, Warning, TEXT("Expected Geology Grid: %dx%d, CellSize: %.1fm"),
           Config.GeologyConfig.ErosionGridWidth, Config.GeologyConfig.ErosionGridHeight,
           Config.GeologyConfig.ErosionCellSize);
    
    if (MainTerrain && MainTerrain->AtmosphericSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actual Atmospheric: %s"),
               *MainTerrain->AtmosphericSystem->GetScalingDebugInfo());
    }
    
    if (GeologyController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actual Geology: %s"),
               *GeologyController->GetScalingDebugInfo());
    }
}


void AMasterWorldController::DiagnoseTemporalManagerIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Ã°Å¸â€Â TEMPORAL MANAGER INTEGRATION DIAGNOSIS"));
    UE_LOG(LogTemp, Warning, TEXT("================================================"));
    
    // Check critical flags
    UE_LOG(LogTemp, Warning, TEXT("bEnableUnifiedTiming: %s"),
           bEnableUnifiedTiming ? TEXT("Ã¢Å“â€¦ TRUE") : TEXT("Ã¢ÂÅ’ FALSE"));
    
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: %s"),
           TemporalManager ? TEXT("Ã¢Å“â€¦ EXISTS") : TEXT("Ã¢ÂÅ’ NULL"));
    
    UE_LOG(LogTemp, Warning, TEXT("CurrentInitPhase: %d"),
           (int32)CurrentInitPhase);
    
    // Check if UpdateSystemsWithTiming would be called
    bool bWouldUpdate = (CurrentInitPhase == EInitializationPhase::Complete) &&
                        bEnableUnifiedTiming && TemporalManager;
    UE_LOG(LogTemp, Warning, TEXT("Would call UpdateSystemsWithTiming: %s"),
           bWouldUpdate ? TEXT("Ã¢Å“â€¦ YES") : TEXT("Ã¢ÂÅ’ NO"));
    
    if (TemporalManager)
    {
        // Check atmospheric system registration
        bool bAtmosphericRegistered = TemporalManager->IsSystemRegistered(ESystemType::Atmospheric);
        UE_LOG(LogTemp, Warning, TEXT("Atmospheric system registered: %s"),
               bAtmosphericRegistered ? TEXT("Ã¢Å“â€¦ YES") : TEXT("Ã¢ÂÅ’ NO"));
        
        // Check if atmospheric system should update
        bool bShouldUpdate = TemporalManager->ShouldSystemUpdate(ESystemType::Atmospheric, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("Should atmospheric system update: %s"),
               bShouldUpdate ? TEXT("Ã¢Å“â€¦ YES") : TEXT("Ã¢ÂÅ’ NO"));
    }
}






FVector2D AMasterWorldController::ConvertWaterToGeologyGrid(FVector2D WaterPos) const
{
    float ScaleFactor = WorldScalingConfig.WaterConfig.WaterCellScale /
                       WorldScalingConfig.GeologyConfig.ErosionCellSize;
    return WaterPos * ScaleFactor;
}

FVector2D AMasterWorldController::ConvertGeologyToWaterGrid(FVector2D GeologyPos) const
{
    float ScaleFactor = WorldScalingConfig.GeologyConfig.ErosionCellSize /
                       WorldScalingConfig.WaterConfig.WaterCellScale;
    return GeologyPos * ScaleFactor;
}

FVector AMasterWorldController::WaterGridToWorld(FVector2D WaterGridPos) const
{
    FVector WorldPos = FVector(
        WaterGridPos.X * WorldScalingConfig.WaterConfig.WaterCellScale,
        WaterGridPos.Y * WorldScalingConfig.WaterConfig.WaterCellScale,
        0.0f
    );
    
    if (MainTerrain)
    {
        return MainTerrain->GetActorTransform().TransformPosition(WorldPos);
    }
    
    return WorldCoordinateSystem.WorldOrigin + WorldPos;
}

FVector2D AMasterWorldController::WorldToWaterGrid(FVector WorldPos) const
{
    FVector LocalPos;
    
    if (MainTerrain)
    {
        LocalPos = MainTerrain->GetActorTransform().InverseTransformPosition(WorldPos);
    }
    else
    {
        LocalPos = WorldPos - WorldCoordinateSystem.WorldOrigin;
    }
    
    return FVector2D(
        LocalPos.X / WorldScalingConfig.WaterConfig.WaterCellScale,
        LocalPos.Y / WorldScalingConfig.WaterConfig.WaterCellScale
    );
}



void AMasterWorldController::TransferSurfaceToAtmosphere(FVector WorldLocation, float Volume)
{
    if (Volume <= 0.0f || !MainTerrain) return;
    
    // Apply to atmospheric grid
    if (MainTerrain->AtmosphericSystem)
    {
        FVector2D GridPos = FVector2D(
            WorldLocation.X / MainTerrain->AtmosphericSystem->CellSize,
            WorldLocation.Y / MainTerrain->AtmosphericSystem->CellSize
        );
        
        int32 AtmosX = FMath::FloorToInt(GridPos.X);
        int32 AtmosY = FMath::FloorToInt(GridPos.Y);
        
        if (AtmosX >= 0 && AtmosX < MainTerrain->AtmosphericSystem->GridWidth &&
            AtmosY >= 0 && AtmosY < MainTerrain->AtmosphericSystem->GridHeight)
        {
            int32 AtmosIndex = AtmosY * MainTerrain->AtmosphericSystem->GridWidth + AtmosX;
            MainTerrain->AtmosphericSystem->AtmosphericGrid[AtmosIndex].MoistureMass += Volume;
            
            // Clear, semantic logging
            UE_LOG(LogTemp, VeryVerbose, TEXT("[EVAPORATION] %.4f mÃ‚Â³ at %s"),
                   Volume, *WorldLocation.ToString());
        }
    }
}

void AMasterWorldController::TransferAtmosphereToSurface(FVector WorldLocation, float Volume)
{
    if (Volume <= 0.0f || !MainTerrain) return;
    
    // Apply to water grid
    if (MainTerrain->WaterSystem)
    {
        // CRITICAL: Convert from mÂ³ to depth in meters, then to simulation units (1.0 = 1cm)
        float WaterDepthMeters = Volume / GetWaterCellArea();
        float WaterDepthSimUnits = WaterDepthMeters / WATER_DEPTH_SCALE;  // Divide by 0.01 to convert m to cm
        MainTerrain->WaterSystem->AddWater(WorldLocation, WaterDepthSimUnits);
        
        // Clear, semantic logging
        UE_LOG(LogTemp, VeryVerbose, TEXT("[PRECIPITATION] %.4f mÃ‚Â³ at %s"),
               Volume, *WorldLocation.ToString());
    }
}

void AMasterWorldController::TransferSurfaceToGroundwater(FVector WorldLocation, float Volume)
{
    if (Volume <= 0.0f || !GeologyController) return;
    
    // Update groundwater (only tracked value we keep)
    TotalGroundwater += Volume;
    
    // Let geology controller handle its internals
    GeologyController->AddWaterToWaterTable(Volume);
    
    // Clear, semantic logging
    UE_LOG(LogTemp, VeryVerbose, TEXT("[INFILTRATION] %.4f mÃ‚Â³ at %s"),
           Volume, *WorldLocation.ToString());
}

void AMasterWorldController::TransferGroundwaterToSurface(FVector WorldLocation, float Volume)
{
    if (Volume <= 0.0f || !MainTerrain || !MainTerrain->WaterSystem) return;
    
    // Check availability
    if (TotalGroundwater < Volume)
    {
        Volume = TotalGroundwater;
    }
    
    if (Volume <= 0.0f) return;
    
    // Update groundwater
    TotalGroundwater -= Volume;
    
    // Add water to surface
    // CRITICAL: Convert from mÂ³ to depth in meters, then to simulation units (1.0 = 1cm)
    float WaterDepthMeters = Volume / GetWaterCellArea();
    float WaterDepthSimUnits = WaterDepthMeters / WATER_DEPTH_SCALE;  // Divide by 0.01 to convert m to cm
    MainTerrain->WaterSystem->AddWater(WorldLocation, WaterDepthSimUnits);
    
    // Sync geology
    if (GeologyController)
    {
        GeologyController->RemoveWaterFromWaterTable(Volume);
    }
    
    // Clear, semantic logging
    UE_LOG(LogTemp, VeryVerbose, TEXT("[SPRING DISCHARGE] %.4f mÃ‚Â³ at %s"),
           Volume, *WorldLocation.ToString());
}

void AMasterWorldController::TransferSurfaceToAtmosphereBulk(
    const TArray<FVector>& Locations,
    const TArray<float>& Volumes)
{
    if (Locations.Num() != Volumes.Num() || !MainTerrain) return;
    
    float TotalVolume = 0.0f;
    
    // Apply all transfers to atmospheric grid
    if (MainTerrain->AtmosphericSystem)
    {
        for (int32 i = 0; i < Locations.Num(); i++)
        {
            FVector2D GridPos = FVector2D(
                Locations[i].X / MainTerrain->AtmosphericSystem->CellSize,
                Locations[i].Y / MainTerrain->AtmosphericSystem->CellSize
            );
            
            int32 AtmosX = FMath::FloorToInt(GridPos.X);
            int32 AtmosY = FMath::FloorToInt(GridPos.Y);
            
            if (AtmosX >= 0 && AtmosX < MainTerrain->AtmosphericSystem->GridWidth &&
                AtmosY >= 0 && AtmosY < MainTerrain->AtmosphericSystem->GridHeight)
            {
                int32 AtmosIndex = AtmosY * MainTerrain->AtmosphericSystem->GridWidth + AtmosX;
                MainTerrain->AtmosphericSystem->AtmosphericGrid[AtmosIndex].MoistureMass += Volumes[i];
                TotalVolume += Volumes[i];
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[BULK EVAPORATION] %.4f mÃ‚Â³ total (%d transfers)"),
           TotalVolume, Locations.Num());
}

void AMasterWorldController::TransferAtmosphereToSurfaceBulk(
    const TArray<FVector>& Locations,
    const TArray<float>& Volumes)
{
    if (Locations.Num() != Volumes.Num() || !MainTerrain) return;
    
    float TotalVolume = 0.0f;
    
    // Apply all transfers to water system
    if (MainTerrain->WaterSystem)
    {
        for (int32 i = 0; i < Locations.Num(); i++)
        {
            // CRITICAL: Convert from mÂ³ to depth in meters, then to simulation units (1.0 = 1cm)
            float WaterDepthMeters = Volumes[i] / GetWaterCellArea();
            float WaterDepthSimUnits = WaterDepthMeters / WATER_DEPTH_SCALE;  // Divide by 0.01 to convert m to cm
            MainTerrain->WaterSystem->AddWater(Locations[i], WaterDepthSimUnits);
            TotalVolume += Volumes[i];
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[BULK PRECIPITATION] %.4f mÃ‚Â³ total (%d transfers)"),
           TotalVolume, Locations.Num());
}



float AMasterWorldController::GetWaterCellArea() const
{
    float WaterCellSize = 1.0f;
    if (MainTerrain && MainTerrain->WaterSystem)
    {
        float WorldSize = GetTerrainScale() * GetWorldDimensions().X;
        float GridSize = MainTerrain->WaterSystem->SimulationData.TerrainWidth;
        if (GridSize > 0)
        {
            WaterCellSize = WorldSize / GridSize;
        }
    }
    return WaterCellSize * WaterCellSize;
}

void AMasterWorldController::SetInitialGroundwater(float VolumeM3)
{
    TotalGroundwater = VolumeM3;
    UE_LOG(LogTemp, Log, TEXT("[GROUNDWATER] Set to %.0f mÃ‚Â³"), VolumeM3);
}

bool AMasterWorldController::CanGroundwaterEmerge(float RequestedVolume) const
{
    return TotalGroundwater >= RequestedVolume;
}



float AMasterWorldController::CalculateSurfaceWaterFromGrid() const
{
    if (!MainTerrain || !MainTerrain->WaterSystem ||
        !MainTerrain->WaterSystem->SimulationData.IsValid())
        return 0.0f;
    
    float Total = 0.0f;
    const TArray<float>& WaterDepthMap = MainTerrain->WaterSystem->SimulationData.WaterDepthMap;
    
    for (float Depth : WaterDepthMap)
    {
        if (Depth > 0.0f)
        {
            Total += GetWaterCellVolume(Depth);
        }
    }
    
    return Total;
}

float AMasterWorldController::CalculateAtmosphericWaterFromGrid() const
{
    if (!MainTerrain || !MainTerrain->AtmosphericSystem)
        return 0.0f;
    
    float Total = 0.0f;
    const TArray<FSimplifiedAtmosphericCell>& AtmosphericGrid =
        MainTerrain->AtmosphericSystem->AtmosphericGrid;
    
    for (const auto& Cell : AtmosphericGrid)
    {
        if (Cell.MoistureMass > 0.0f)
        {
            Total += Cell.MoistureMass;
        }
    }
    
    return Total;
}

void AMasterWorldController::UpdateWaterCache() const
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Only update if cache is stale
    if (CurrentTime - LastCacheUpdateTime > CacheValidityDuration)
    {
        CachedSurfaceWater = CalculateSurfaceWaterFromGrid();
        CachedAtmosphericWater = CalculateAtmosphericWaterFromGrid();
        LastCacheUpdateTime = CurrentTime;
    }
}

float AMasterWorldController::GetTotalSurfaceWater() const
{
    UpdateWaterCache();
    return CachedSurfaceWater;
}

float AMasterWorldController::GetTotalAtmosphericWater() const
{
    UpdateWaterCache();
    return CachedAtmosphericWater;
}

float AMasterWorldController::GetTotalWaterVolume() const
{
    return GetTotalSurfaceWater() + GetTotalAtmosphericWater() + TotalGroundwater;
}

FWaterDistribution AMasterWorldController::GetWaterDistribution() const
{
    FWaterDistribution Dist;
    
    Dist.SurfaceWater = GetTotalSurfaceWater();
    Dist.AtmosphericWater = GetTotalAtmosphericWater();
    Dist.Groundwater = TotalGroundwater;
    Dist.TotalWater = Dist.SurfaceWater + Dist.AtmosphericWater + Dist.Groundwater;
    
    if (Dist.TotalWater > 0.0f)
    {
        Dist.SurfacePercent = (Dist.SurfaceWater / Dist.TotalWater) * 100.0f;
        Dist.AtmosphericPercent = (Dist.AtmosphericWater / Dist.TotalWater) * 100.0f;
        Dist.GroundwaterPercent = (Dist.Groundwater / Dist.TotalWater) * 100.0f;
    }
    
    return Dist;
}

void AMasterWorldController::TogglePause()
{
    PauseAllSystems(!bPauseSimulation);
    
    UE_LOG(LogTemp, Warning, TEXT("ðŸŽ® Simulation %s"),
           bPauseSimulation ? TEXT("PAUSED") : TEXT("RESUMED"));
    
    // Optional: Show on-screen message
    if (GEngine)
    {
        FString Message = bPauseSimulation ? TEXT("â¸ PAUSED") : TEXT("â–¶ RESUMED");
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message, true, FVector2D(2.0f, 2.0f));
    }
}

// ===== RUNTIME MAP SWITCHING IMPLEMENTATION =====

void AMasterWorldController::SwitchToMap(int32 MapIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("=== SWITCHING TO MAP %d ==="), MapIndex);
    
    // Get GameInstance
    UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance());
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToMap: No GameInstance found!"));
        return;
    }
    
    // Get map definition
    FTerrainMapDefinition NewMapDef;
    
    if (MapIndex < 0)
    {
        // Random map
        NewMapDef = GameInstance->GenerateRandomMapDefinition();
        UE_LOG(LogTemp, Warning, TEXT("Generating random map"));
    }
    else if (MapIndex < GameInstance->GetAvailableMapCount())
    {
        // Specific preset map
        NewMapDef = GameInstance->GetMapDefinitionByIndex(MapIndex);
        UE_LOG(LogTemp, Warning, TEXT("Loading map: %s"), *NewMapDef.DisplayName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid map index %d (max %d)"),
               MapIndex, GameInstance->GetAvailableMapCount() - 1);
        return;
    }
    
    // Store new map definition
    CurrentMapDefinition = NewMapDef;
    bHasMapDefinition = true;
    
    // Pass to terrain BEFORE regeneration
    if (MainTerrain)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        
        // Regenerate terrain
        MainTerrain->ResetTerrainFully();
        
        UE_LOG(LogTemp, Warning, TEXT("=== MAP SWITCH COMPLETE ==="));
        UE_LOG(LogTemp, Warning, TEXT("Map: %s"), *CurrentMapDefinition.DisplayName.ToString());
        UE_LOG(LogTemp, Warning, TEXT("Mode: %s"), *CurrentMapDefinition.GetGenerationModeName());
        UE_LOG(LogTemp, Warning, TEXT("Scale: %.1f (%.1fkm)"),
               CurrentMapDefinition.TerrainScale,
               CurrentMapDefinition.GetWorldSizeKm());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MainTerrain is null - cannot regenerate!"));
    }
}

void AMasterWorldController::ReloadCurrentMap()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RELOADING CURRENT MAP ==="));
    
    if (!bHasMapDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("No current map - using default procedural"));
        SwitchToMap(-1); // Switch to random
        return;
    }
    
    // Regenerate with current definition
    if (MainTerrain)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        MainTerrain->ResetTerrainFully();
        
        UE_LOG(LogTemp, Warning, TEXT("=== RELOAD COMPLETE ==="));
        UE_LOG(LogTemp, Warning, TEXT("Map: %s"), *CurrentMapDefinition.DisplayName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MainTerrain is null - cannot reload!"));
    }
}

// ===== BLUEPRINT-EXPOSED MAP INFO FUNCTIONS =====

TArray<FString> AMasterWorldController::GetAvailableMapNames() const
{
    TArray<FString> MapNames;
    
    UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance)
    {
        TArray<FTerrainMapDefinition> Maps = GameInstance->GetAvailableMapDefinitions();
        for (const FTerrainMapDefinition& Map : Maps)
        {
            MapNames.Add(Map.DisplayName.ToString());
        }
    }
    
    return MapNames;
}

int32 AMasterWorldController::GetCurrentMapIndex() const
{
    if (!bHasMapDefinition)
    {
        return -1;
    }
    
    UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetWorld()->GetGameInstance());
    if (!GameInstance)
    {
        return -1;
    }
    
    // Find matching map in available maps
    TArray<FTerrainMapDefinition> Maps = GameInstance->GetAvailableMapDefinitions();
    for (int32 i = 0; i < Maps.Num(); i++)
    {
        if (Maps[i].DisplayName.ToString() == CurrentMapDefinition.DisplayName.ToString())
        {
            return i;
        }
    }
    
    return -1; // Custom or random map
}

FString AMasterWorldController::GetCurrentMapName() const
{
    if (bHasMapDefinition)
    {
        return CurrentMapDefinition.DisplayName.ToString();
    }
    return TEXT("No Map Loaded");
}

FString AMasterWorldController::GetMapDebugInfo() const
{
    if (!bHasMapDefinition)
    {
        return TEXT("No map definition loaded");
    }
    
    return FString::Printf(TEXT(
        "Map: %s\n"
        "Mode: %s\n"
        "Scale: %.1f (%.1fkm)\n"
        "Height Var: %.1fm\n"
        "Noise Scale: %.4f\n"
        "Octaves: %d\n"
        "Seed: %d\n"
        "Springs: %d\n"
        "Latitude: %.1f°"
    ),
        *CurrentMapDefinition.DisplayName.ToString(),
        *CurrentMapDefinition.GetGenerationModeName(),
        CurrentMapDefinition.TerrainScale,
        CurrentMapDefinition.GetWorldSizeKm(),
        CurrentMapDefinition.HeightVariation,
        CurrentMapDefinition.NoiseScale,
        CurrentMapDefinition.NoiseOctaves,
        CurrentMapDefinition.ProceduralSeed,
        CurrentMapDefinition.NumProceduralSprings,
        CurrentMapDefinition.DefaultLatitude
    );
}

bool AMasterWorldController::IsCurrentMapProcedural() const
{
    if (!bHasMapDefinition)
    {
        return false;
    }
    
    return CurrentMapDefinition.GenerationMode == ETerrainGenerationMode::Procedural ||
           CurrentMapDefinition.GenerationMode == ETerrainGenerationMode::ProceduralSeed;
}

bool AMasterWorldController::IsCurrentMapSeeded() const
{
    if (!bHasMapDefinition)
    {
        return false;
    }
    
    return CurrentMapDefinition.GenerationMode == ETerrainGenerationMode::ProceduralSeed &&
           CurrentMapDefinition.ProceduralSeed >= 0;
}

// ===== RUNTIME PARAMETER UPDATES =====

void AMasterWorldController::UpdateTerrainScale(float NewScale)
{
    if (!bHasMapDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("No map loaded - cannot update scale"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Updating terrain scale: %.1f → %.1f"),
           CurrentMapDefinition.TerrainScale, NewScale);
    
    CurrentMapDefinition.TerrainScale = NewScale;
    
    if (MainTerrain)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        MainTerrain->ResetTerrainFully();
    }
}

void AMasterWorldController::UpdateProceduralParameters(int32 Seed, float HeightVariation,
                                                        float NoiseScale, int32 Octaves)
{
    if (!IsCurrentMapProcedural())
    {
        UE_LOG(LogTemp, Warning, TEXT("Current map is not procedural - cannot update parameters"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Updating procedural parameters: Seed=%d, Height=%.1f, Noise=%.4f, Octaves=%d"),
           Seed, HeightVariation, NoiseScale, Octaves);
    
    CurrentMapDefinition.ProceduralSeed = Seed;
    CurrentMapDefinition.HeightVariation = HeightVariation;
    CurrentMapDefinition.NoiseScale = NoiseScale;
    CurrentMapDefinition.NoiseOctaves = Octaves;
    
    if (Seed >= 0)
    {
        CurrentMapDefinition.GenerationMode = ETerrainGenerationMode::ProceduralSeed;
    }
    else
    {
        CurrentMapDefinition.GenerationMode = ETerrainGenerationMode::Procedural;
    }
    
    if (MainTerrain)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        MainTerrain->ResetTerrainFully();
    }
}

void AMasterWorldController::QuickRegenerate()
{
    UE_LOG(LogTemp, Warning, TEXT("Quick regenerating terrain..."));
    
    if (!bHasMapDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("No map definition - generating random"));
        SwitchToMap(-1);
        return;
    }
    
    if (MainTerrain)
    {
        MainTerrain->SetMapDefinition(CurrentMapDefinition);
        MainTerrain->ResetTerrainFully();
    }
}

// End of file
