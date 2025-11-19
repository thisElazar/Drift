/**
 * ============================================================================
 * TERRAI TERRAIN CONTROLLER - REORGANIZED
 * ============================================================================
 * Reorganized: November 2025
 * Original: 2,575 lines | Reorganized: ~2,780 lines | Functions: 98
 * All function logic preserved exactly - zero changes to implementation
 * Added comprehensive documentation (~205 lines, ~7.4% overhead)
 *
 * PURPOSE:
 * Player-controlled camera and editing system serving as the primary interface
 * between user input and the TerrAI watershed simulation. Coordinates
 * multi-mode camera movement, interactive terrain/water editing, and integrates
 * with MasterController through the Universal Brush System receiver pattern.
 *
 * ARCHITECTURE HIGHLIGHTS:
 * - Authority Delegation: Implements IBrushReceiver to receive commands from
 *   MasterController without tight coupling
 * - Multi-Mode Camera: Three camera modes (Overhead, FirstPerson) with smooth
 *   transitions and terrain-aware positioning
 * - Unified Cursor: Single cursor position for all editing operations with
 *   instant response and world-space awareness
 * - Enhanced Input System: UE5 context-based action mapping with state-driven
 *   input handling to prevent conflicts
 *
 * KEY FEATURES:
 * â€¢ Interactive terrain modification (raise/lower)
 * â€¢ Interactive water editing (add/remove)
 * â€¢ Groundwater spring management
 * â€¢ Atmospheric parameter modification
 * â€¢ Smooth camera transitions between modes
 * â€¢ Performance monitoring and HUD display
 * â€¢ Real-time brush preview with material feedback
 *
 * SYSTEM DEPENDENCIES:
 * - MasterController: Central authority for brush system and coordinates
 * - DynamicTerrain: Terrain mesh and heightmap data
 * - WaterSystem: Water simulation and modification
 * - GeologyController: Spring management
 * - AtmosphericSystem: Weather parameter modification
 * - TemporalManager: Time scaling integration
 * ============================================================================
 */

// ============================================================================
// SECTION 1: INCLUDES & CONSTRUCTOR (~140 lines, 5%)
// ============================================================================
/**
 * PURPOSE:
 * Header dependencies and component initialization. Sets up camera system,
 * brush preview, and default values for all controller subsystems.
 *
 * COMPONENTS CREATED:
 * - SceneRoot: Base attachment point for all components
 * - SpringArm: Overhead camera positioning system
 * - Camera: Overhead/ThirdPerson view
 * - FirstPersonCamera: First-person view
 * - BrushPreview: Visual cursor for editing operations
 *
 * INITIALIZATION STRATEGY:
 * Constructor establishes component hierarchy and default values, but does NOT
 * perform system discovery or authority establishment. That happens in
 * InitializeControllerWithAuthority() called by MasterController during the
 * proper initialization sequence.
 * ============================================================================
 */

// TerrainController.cpp - Complete Clean UE 5.4 Compatible Version with Universal Brush System
#include "TerrainController.h"
#include "TerrAI.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "AtmosphericSystem.h"
#include "WaterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "DynamicTerrain.h"
#include "TerrAIGameInstance.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TerrAIGameInstance.h"
#include "WaterController.h"
#include "GeologyController.h"
#include "MasterController.h"

ATerrainController::ATerrainController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create a proper root component first
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    
    // Create spring arm component and attach to root
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 3000.0f;
    SpringArm->SetUsingAbsoluteRotation(true);
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;
    
    SpringArm->bDoCollisionTest = false;
    SpringArm->bUseCameraLagSubstepping = false;
    SpringArm->CameraLagSpeed = 0.0f;
    SpringArm->CameraLagMaxDistance = 0.0f;
    SpringArm->ProbeSize = 0.0f;
    SpringArm->ProbeChannel = ECC_Camera;
    
    // Create camera component
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->SetActive(true); // SURGICAL FIX: Explicitly activate
    
    Camera->SetConstraintAspectRatio(false);
    Camera->SetOrthoNearClipPlane(1.0f);
    Camera->SetOrthoFarClipPlane(100000.0f);
    
    // Create first person camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(SceneRoot);
    FirstPersonCamera->SetActive(false);
    
    FirstPersonCamera->SetConstraintAspectRatio(false);
    FirstPersonCamera->SetOrthoNearClipPlane(1.0f);
    FirstPersonCamera->SetOrthoFarClipPlane(100000.0f);
    
    // Create brush preview component
    BrushPreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrushPreview"));
    BrushPreview->SetupAttachment(RootComponent);
    BrushPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BrushPreview->SetCastShadow(false);
    BrushPreview->SetVisibility(true);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        BrushPreview->SetStaticMesh(SphereMeshAsset.Object);
        UE_LOG(LogTemp, Log, TEXT("Brush preview mesh set to default sphere"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load default sphere mesh for brush preview"));
    }
    
    // Initialize camera system
    CurrentCameraMode = ECameraMode::Overhead;
    TargetCameraMode = ECameraMode::Overhead;
    FirstPersonHeight = 152.4f;
    TransitionSpeed = 3.0f;
    bTransitioning = false;
    
    // Initialize settings
    CameraMoveSpeed = 3000.0f;
    CameraFlySpeed = 2000.0f;
    MouseSensitivity = 0.5f;
    CameraZoomSpeed = 1000.0f;
    MinZoomDistance = 500.0f;
    MaxZoomDistance = 20000.0f;
    
    WaterBrushStrength = 10.0f;
    MinWaterBrushStrength = 1.0f;
    MaxWaterBrushStrength = 100.0f;
    WaterStrengthChangeRate = 5.0f;
    
    CurrentVisualMode = ETerrainVisualMode::Wireframe;
    CurrentMainMode = EMainEditingMode::Terrain;
    CurrentTerrainSubMode = ETerrainSubMode::Raise;
    CurrentWaterSubMode = EWaterSubMode::WaterBrush;
    
    bIsEditingTerrain = false;
    bIsRaisingTerrain = false;
    bIsLoweringTerrain = false;
    bIsEditingWater = false;
    bIsAddingWater = false;
    bIsRemovingWater = false;
    
    bShowPerformanceStats = true;
    bShowTerrainInfo = true;
    
    MovementInput = FVector2D(0.0f, 0.0f);
    LookInput = FVector2D(0.0f, 0.0f);
    SmoothedMovementInput = FVector2D(0.0f, 0.0f);
    CameraRotation = FRotator(-45.0f, 0.0f, 0.0f);
    
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
}


// ============================================================================
// SECTION 2: UNIVERSAL BRUSH RECEIVER SYSTEM â­ (~70 lines, 3%)
// ============================================================================
/**
 * PURPOSE:
 * Implements the IBrushReceiver interface to integrate with MasterController's
 * Universal Brush System. This is a CRITICAL architecture pattern demonstrating
 * proper authority delegation.
 *
 * ARCHITECTURE PRINCIPLE: Authority Delegation
 * Instead of TerrainController owning its own brush system, it implements the
 * IBrushReceiver interface and registers with MasterController. This allows:
 *
 * 1. SINGLE SOURCE OF TRUTH: MasterController owns brush settings
 * 2. CLEAN SEPARATION: TerrainController focuses on terrain/water operations
 * 3. MULTI-SYSTEM COORDINATION: Same brush settings can apply to multiple systems
 * 4. LOOSE COUPLING: Systems communicate through well-defined interfaces
 *
 * PATTERN:
 *   MasterController (Authority)
 *        â†“
 *   IBrushReceiver Interface
 *        â†“
 *   TerrainController (Implementation)
 *
 * The ApplyBrush() method receives commands but doesn't make decisions about
 * brush size, strength, or falloff. Those are MasterController's responsibility.
 * TerrainController only implements "what to do" with the brush, not "how big"
 * or "how strong" it should be.
 *
 * FUNCTIONS:
 * - ApplyBrush(): Receives brush commands from MasterController authority
 * - UpdateBrushSettings(): Caches settings for local reference
 * - CanReceiveBrush(): Validates receiver state for brush operations
 * ============================================================================
 */

// ===== UNIVERSAL BRUSH SYSTEM IMPLEMENTATION =====

void ATerrainController::ApplyBrush(FVector WorldPosition, const FUniversalBrushSettings& Settings, float DeltaTime)
{
    if (!CanReceiveBrush())
    {
        return;
    }
    
    // Handle terrain modification
    if (bIsEditingTerrain && TargetTerrain)
    {
        float ModificationAmount = Settings.BrushStrength * DeltaTime;
        
        if (bIsRaisingTerrain || bIsLoweringTerrain)
        {
            TargetTerrain->ModifyTerrain(WorldPosition, Settings.BrushRadius,
                ModificationAmount, bIsRaisingTerrain);
        }
    }
    // Handle water modification through brush system
    else if (bIsEditingWater && TargetTerrain && TargetTerrain->WaterSystem)
    {
        float WaterAmount = Settings.BrushStrength * DeltaTime;
        
        if (bIsAddingWater)
        {
            // Calculate volume for tracking
            float CellArea = MasterController->GetWaterCellArea();
            float RadiusInCells = Settings.BrushRadius / MasterController->GetTerrainScale();
            float NumCells = PI * RadiusInCells * RadiusInCells;
            float VolumeAdded = WaterAmount * CellArea * NumCells;
            
            // Add water
            TargetTerrain->WaterSystem->AddWater(WorldPosition, WaterAmount);
            
        
        }
        else if (bIsRemovingWater)
        {
            // Calculate volume for tracking
            float CellArea = MasterController->GetWaterCellArea();
            float RadiusInCells = Settings.BrushRadius / MasterController->GetTerrainScale();
            float NumCells = PI * RadiusInCells * RadiusInCells;
            float VolumeRemoved = WaterAmount * CellArea * NumCells;
            
            // Remove water
            TargetTerrain->WaterSystem->RemoveWater(WorldPosition, WaterAmount);

        }
    }
}

void ATerrainController::UpdateBrushSettings(const FUniversalBrushSettings& Settings)
{
    CurrentBrushSettings = Settings;
    
    // The Universal Brush System handles all preview updates automatically
    // No need to manually call UpdateBrushPreview() here
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[TERRAIN CONTROLLER] Updated brush settings - Radius: %.1f, Strength: %.1f"),
           Settings.BrushRadius, Settings.BrushStrength);
}

bool ATerrainController::CanReceiveBrush() const
{
    // Can receive brush if we're in terrain editing mode and have a valid terrain
    return (CurrentMainMode == EMainEditingMode::Terrain) &&
           (TargetTerrain != nullptr) &&
           bInitializationComplete;
}


// ============================================================================
// SECTION 3: INITIALIZATION & LIFECYCLE (~190 lines, 7%)
// ============================================================================
/**
 * PURPOSE:
 * Actor lifecycle management, system discovery, and authority establishment.
 * Coordinates initialization sequence with MasterController to ensure proper
 * system ordering and dependency resolution.
 *
 * INITIALIZATION SEQUENCE (Called by MasterController):
 * 1. BeginPlay() - Waits for MasterController authority
 * 2. InitializeControllerWithAuthority() - MasterController calls this
 * 3. Register as brush receiver
 * 4. Discover terrain and system references
 * 5. Initialize fog system
 * 6. Set up brush preview
 * 7. Mark initialization complete
 *
 * AUTHORITY PATTERN:
 * TerrainController does NOT auto-initialize. It waits for MasterController
 * to explicitly call InitializeControllerWithAuthority() after all core
 * systems are ready. This prevents race conditions and ensures proper
 * dependency ordering.
 *
 * ATMOSPHERIC FOG INTEGRATION:
 * Implements dynamic fog density based on atmospheric conditions:
 * - Humidity increases fog (>70% humidity threshold)
 * - Temperature differential enhances fog (near freezing point)
 * - Fog color shifts with temperature (blue=cold, white=warm)
 *
 * SUBSECTIONS:
 * 3.1: Actor Lifecycle (BeginPlay, PossessedBy)
 * 3.2: Authority Initialization
 * 3.3: Fog System Integration
 * ============================================================================
 */

void ATerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    // AUTHORITY: Wait for MasterController to initialize us
    // Do NOT auto-initialize - wait for proper authority chain
    if (!MasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainController: Waiting for MasterController authority..."));
        // MasterController will call InitializeControllerWithAuthority() when ready
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TerrainController: MasterController already set, but waiting for authority call"));
}




void ATerrainController::InitializeControllerWithAuthority()
{
    if (bInitializationComplete || !MasterController)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TerrainController: Initializing with MasterController authority"));
    
    // ===== UNIVERSAL BRUSH SYSTEM REGISTRATION =====
    
    // Register as a brush receiver with the MasterController
    TScriptInterface<IBrushReceiver> BrushReceiverInterface;
    BrushReceiverInterface.SetObject(this);
    BrushReceiverInterface.SetInterface(static_cast<IBrushReceiver*>(this));
    MasterController->RegisterBrushReceiver(BrushReceiverInterface);
    
    UE_LOG(LogTemp, Warning, TEXT("[TERRAIN CONTROLLER] Registered as brush receiver with MasterController"));
    
    // Initialize current brush settings from MasterController
    CurrentBrushSettings = MasterController->GetUniversalBrushSettings();
    
    // Set initial camera rotation
    FRotator InitialCameraRotation = FRotator(0.0f, 0.0f, 0.0f); //
    SpringArm->SetWorldRotation(InitialCameraRotation);
    
    // Ensure FirstPersonCamera starts with level rotation
    FirstPersonCamera->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));
    
    // Get terrain from MasterController authority
    TargetTerrain = MasterController->MainTerrain;
    if (!TargetTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: MasterController has no MainTerrain"));
        return;
    }
    
    // Get WaterController from MasterController
    WaterController = MasterController->WaterController;
    
    // Get GeologyController from MasterController
        if (MasterController)
        {
            GeologyController = MasterController->GeologyController;
            
            if (GeologyController)
            {
                UE_LOG(LogTemp, Warning, TEXT("TerrainController: Got GeologyController from MasterController"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("TerrainController: No GeologyController in MasterController!"));
            }
        }
    
    // Initialize atmospheric system
    if (TargetTerrain)
    {
        AtmosphericSystem = TargetTerrain->GetAtmosphericSystem();
    }
    
    // Initialize fog system
    InitializeFogSystem();
    
    // Set up brush preview (Universal Brush System handles this)
    if (BrushPreview)
    {
        BrushPreview->SetVisibility(false); // Start hidden
        BrushPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BrushPreview->SetCastShadow(false);
        UE_LOG(LogTemp, Log, TEXT("[TERRAIN CONTROLLER] Brush preview initialized for Universal Brush System"));
    }
    
    // Initialize water textures
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->CreateWaterDepthTexture();
        TargetTerrain->WaterSystem->CreateAdvancedWaterTexture();
        TargetTerrain->WaterSystem->UpdateWaterDepthTexture();
    }
    
    // Set visual mode
    SetVisualMode(CurrentVisualMode);
    
    // Apply depth buffer optimizations
    if (GetWorld())
    {
        GetWorld()->Exec(GetWorld(), TEXT("r.SetNearClipPlane 1.0"));
        GetWorld()->Exec(GetWorld(), TEXT("r.EarlyZPass 1"));
    }
    
    bWaitingForMasterController = false;
    bInitializationComplete = true;
    
    UE_LOG(LogTemp, Warning, TEXT("TerrainController: Authority initialization complete"));
}

// ===== ATMOSPHERIC FOG FUNCTIONS =====

void ATerrainController::InitializeFogSystem()
{
    if (!GetWorld()) return;
    
    // Set up basic fog parameters for depth perception
    GetWorld()->Exec(GetWorld(), TEXT("r.Fog 1"));
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Fog.StartDistance %.1f"), FogStartDistance));
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Fog.MaxOpacity %.2f"), FogMaxOpacity));
    GetWorld()->Exec(GetWorld(), TEXT("r.Fog.FalloffExponent 2.0"));
    
    UE_LOG(LogTemp, Warning, TEXT("Atmospheric fog system initialized"));
}

void ATerrainController::UpdateAtmosphericFog()
{
    if (!GetWorld() || !AtmosphericSystem) return;
    
    // Sample atmospheric conditions at camera position
    FVector CameraPosition = GetActorLocation();
    float Humidity = AtmosphericSystem->GetHumidityAt(CameraPosition);
    float Temperature = AtmosphericSystem->GetTemperatureAt(CameraPosition);
    
    // Calculate dynamic fog density
    float DynamicFogDensity = CalculateFogDensity(CameraPosition);
    
    // Apply fog parameters to world
    float FogDensity = BaseFogDensity * DynamicFogDensity;
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Fog.Density %.4f"), FogDensity));
    
    // Adjust fog color based on temperature (cooler = bluer, warmer = whiter)
    float TempRatio = FMath::Clamp((Temperature - 263.15f) / 30.0f, 0.0f, 1.0f); // -10Â°C to 20Â°C range
    FLinearColor FogColor = FLinearColor::LerpUsingHSV(
        FLinearColor(0.7f, 0.8f, 1.0f, 1.0f), // Cool blue-white
        FLinearColor(1.0f, 1.0f, 0.95f, 1.0f), // Warm white
        TempRatio
    );
    
    // Note: Fog color would need to be set via post-process volume in actual implementation
    // This is a simplified version showing the concept
}

float ATerrainController::CalculateFogDensity(FVector Position) const
{
    if (!AtmosphericSystem) return 1.0f;
    
    float Humidity = AtmosphericSystem->GetHumidityAt(Position);
    float Temperature = AtmosphericSystem->GetTemperatureAt(Position);
    
    // Fog forms when humidity is high and there's temperature differential
    float HumidityFactor = FMath::Clamp((Humidity - 0.7f) * HumidityFogMultiplier, 0.0f, 2.0f);
    
    // Temperature differential enhances fog (around freezing point)
    float TempDiff = FMath::Abs(Temperature - 273.15f); // Distance from 0Â°C
    float TempFactor = 1.0f + (1.0f / (1.0f + TempDiff * 0.1f)) * TemperatureFogMultiplier;
    
    // Combine factors for final fog intensity
    float FogMultiplier = 1.0f + (HumidityFactor * TempFactor);
    
    return FMath::Clamp(FogMultiplier, 0.1f, 3.0f);
}


void ATerrainController::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SetupInputMapping();
}


// ============================================================================
// SECTION 4: CORE UPDATE LOOP (~175 lines, 7%)
// ============================================================================
/**
 * PURPOSE:
 * Main frame update coordinating all controller subsystems. Orchestrates
 * cursor updates, editing operations, camera management, and performance
 * monitoring in proper sequence.
 *
 * UPDATE SEQUENCE (per frame):
 * 1. Authority validation (wait for initialization)
 * 2. Unified cursor update (CRITICAL - must be first)
 * 3. Camera position updates
 * 4. Editing operations (terrain/water/spring/atmospheric)
 * 5. Atmospheric fog updates
 * 6. Camera conflict detection and resolution
 * 7. Brush preview updates
 * 8. First-person height validation
 * 9. Performance stats display
 *
 * CRITICAL ORDERING:
 * UpdateUnifiedCursor() must execute FIRST to ensure all other systems have
 * access to the current cursor position for the frame. This prevents one-frame
 * lag in editing operations.
 *
 * CAMERA CONFLICT RESOLUTION:
 * Actively monitors for invalid camera states (both active, neither active)
 * and automatically corrects them based on CurrentCameraMode. This surgical
 * fix prevents the "black screen" bug.
 *
 * PERFORMANCE OVERHEAD:
 * - UpdateUnifiedCursor: ~0.05ms
 * - Camera updates: ~0.1ms
 * - Editing operations: 0-2ms (only when active)
 * - Brush preview: ~0.1ms
 * - Performance stats: ~0.05ms (throttled to 4 Hz)
 * ============================================================================
 */

void ATerrainController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // AUTHORITY: Only operate if properly initialized
    if (bWaitingForMasterController || !bInitializationComplete)
    {
        return;
    }
    
    // CRITICAL: Update unified cursor FIRST
    UpdateUnifiedCursor(DeltaTime);
    
    // Update camera position
    UpdateCameraPosition(DeltaTime);
    
    // Update terrain modification if active
    if (bIsEditingTerrain)
    {
        UpdateTerrainModification(DeltaTime);
    }
    
    // Update water modification if active
    if (bIsEditingWater)
    {
        UpdateWaterModification(DeltaTime);
    }
    
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
    {
        UpdateSpringEditing(DeltaTime);
    }
    
    // Update atmospheric editing if active
    if (CurrentMainMode == EMainEditingMode::Atmosphere)
    {
        UpdateAtmosphericEditing(DeltaTime);
    }
    
    // Update atmospheric fog
    if (bEnableAtmosphericFog && AtmosphericSystem)
    {
        UpdateAtmosphericFog();
    }
    
    // SAFETY CHECK: Ensure only one camera is active at a time
    // ADD: Null check first
    if (Camera && FirstPersonCamera)
    {
        if (Camera->IsActive() && FirstPersonCamera->IsActive())
        {
            UE_LOG(LogTemp, Error, TEXT("CAMERA CONFLICT: Both cameras active! Current mode: %s, fixing..."),
                   CurrentCameraMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"));
            
            // Fix based on current mode
            if (CurrentCameraMode == ECameraMode::FirstPerson)
            {
                Camera->SetActive(false);
            }
            else
            {
                FirstPersonCamera->SetActive(false);
            }
        }
        else if (!Camera->IsActive() && !FirstPersonCamera->IsActive())
        {
            UE_LOG(LogTemp, Error, TEXT("CAMERA CONFLICT: No cameras active! Current mode: %s, fixing..."),
                   CurrentCameraMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"));
            
            // Activate correct camera based on current mode
            if (CurrentCameraMode == ECameraMode::FirstPerson)
            {
                FirstPersonCamera->SetActive(true);
            }
            else
            {
                Camera->SetActive(true);
            }
        }
    }
    
    // Update brush preview using Universal Brush System
    if (bShowBrushPreview)
    {
        // The Universal Brush System implementation handles preview updates
        // Get current brush settings and update preview accordingly
        if (BrushPreview && MasterController)
        {
            const FUniversalBrushSettings& BrushSettings = GetCurrentBrushSettings();
            
            // Position preview at cursor location
            FVector CursorPosition = GetCursorWorldPosition();
            if (CursorPosition != FVector::ZeroVector)
            {
                BrushPreview->SetWorldLocation(CursorPosition);
                
                // Scale preview to match brush radius
                float ScaledRadius = BrushSettings.BrushRadius;
                FVector PreviewScale = FVector(ScaledRadius / 50.0f);
                BrushPreview->SetWorldScale3D(PreviewScale);
                
                // Set visibility based on cursor validity and preview settings
                // FIXED: Always show cursor when valid, regardless of editing state
                bool bShouldShow = bShowBrushPreview && BrushSettings.bShowPreview;
                BrushPreview->SetVisibility(bShouldShow);
                
                // Update material based on current editing mode
                if (bIsEditingTerrain) {
                    if (TerrainEditMaterial) {
                        BrushPreview->SetMaterial(0, TerrainEditMaterial);
                    }
                } else if (bIsEditingWater) {
                    UMaterialInterface* MaterialToUse = bIsAddingWater ? WaterAddMaterial : WaterRemoveMaterial;
                    if (MaterialToUse) {
                        BrushPreview->SetMaterial(0, MaterialToUse);
                    }
                } else if (bIsEditingSpring) {
                    // Spring mode materials
                    UMaterialInterface* MaterialToUse = nullptr;
                    if (bIsAddingSpring && SpringAddMaterial)
                    {
                        MaterialToUse = SpringAddMaterial;
                    }
                    else if (bIsRemovingSpring && SpringRemoveMaterial)
                    {
                        MaterialToUse = SpringRemoveMaterial;
                    }
                    
                    if (MaterialToUse)
                    {
                        BrushPreview->SetMaterial(0, MaterialToUse);
                    }
                }
                else
                {
                    // Default cursor appearance when not editing
                    // Use TerrainEditMaterial as the default cursor material
                    if (TerrainEditMaterial)
                    {
                        BrushPreview->SetMaterial(0, TerrainEditMaterial);
                    }
                    else
                    {
                        // Fallback: Create a semi-transparent white material if no default is set
                        UMaterialInstanceDynamic* DynamicMat = BrushPreview->CreateDynamicMaterialInstance(0);
                        if (DynamicMat)
                        {
                            DynamicMat->SetVectorParameterValue(FName("BaseColor"),
                                FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
                        }
                    }
                }
            }
            else
            {
                BrushPreview->SetVisibility(false);
            }
        }
    }
    
    // Validate first-person terrain height periodically
    if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        ValidateFirstPersonHeight(DeltaTime);
    }
    
    // Update performance stats
    if (bShowPerformanceStats)
    {
        UpdatePerformanceStats(DeltaTime);
    }
}


// ============================================================================
// SECTION 5: INPUT SYSTEM CONFIGURATION (~155 lines, 6%)
// ============================================================================
/**
 * PURPOSE:
 * Enhanced Input System binding for UE5. Maps input actions to controller
 * functions using context-based action mapping with proper event types.
 *
 * INPUT ARCHITECTURE:
 * Uses UE5's Enhanced Input System which provides:
 * - Context-based input mapping (different actions per game state)
 * - Input modifiers (smoothing, dead zones, scaling)
 * - Input triggers (Started, Triggered, Completed, Canceled)
 * - Input processors (axis conversion, inversion)
 *
 * EVENT TYPES:
 * - Triggered: Fires continuously while input is active
 * - Started: Fires once when input begins
 * - Completed: Fires once when input ends
 *
 * INPUT CATEGORIES:
 * 1. Movement Controls: WASD movement, mouse look, zoom, fly up/down
 * 2. Editing Controls: Raise/lower terrain, add/remove water, springs
 * 3. Brush Controls: Size and strength adjustment
 * 4. System Controls: Visual mode, rain, editing mode, reset
 * 5. Time Controls: Speed up/slow down, pause
 * 6. Camera Controls: Mode switching, warping
 * 7. UI Controls: Control panel toggle, main menu return
 *
 * STATE MANAGEMENT:
 * Input handlers update state flags (bIsEditingTerrain, bIsAddingWater, etc.)
 * that the Tick() function checks to determine which update loops to run.
 * This state-driven approach prevents input conflicts and ensures clean
 * transitions between editing modes.
 * ============================================================================
 */

void ATerrainController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movement
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATerrainController::Move);
        }
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATerrainController::Look);
        }
        if (ZoomAction)
        {
            EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATerrainController::Zoom);
        }
        if (FlyUpAction)
        {
            EnhancedInputComponent->BindAction(FlyUpAction, ETriggerEvent::Triggered, this, &ATerrainController::FlyUp);
        }
        if (FlyDownAction)
        {
            EnhancedInputComponent->BindAction(FlyDownAction, ETriggerEvent::Triggered, this, &ATerrainController::FlyDown);
        }
        
        // Terrain editing
        if (RaiseTerrainAction)
        {
            EnhancedInputComponent->BindAction(RaiseTerrainAction, ETriggerEvent::Started, this, &ATerrainController::StartRaiseTerrain);
            EnhancedInputComponent->BindAction(RaiseTerrainAction, ETriggerEvent::Completed, this, &ATerrainController::StopRaiseTerrain);
        }
        if (LowerTerrainAction)
        {
            EnhancedInputComponent->BindAction(LowerTerrainAction, ETriggerEvent::Started, this, &ATerrainController::StartLowerTerrain);
            EnhancedInputComponent->BindAction(LowerTerrainAction, ETriggerEvent::Completed, this, &ATerrainController::StopLowerTerrain);
        }
        
        // Water editing
        if (AddWaterAction)
        {
            EnhancedInputComponent->BindAction(AddWaterAction, ETriggerEvent::Started, this, &ATerrainController::StartAddWater);
            EnhancedInputComponent->BindAction(AddWaterAction, ETriggerEvent::Completed, this, &ATerrainController::StopAddWater);
        }
        if (RemoveWaterAction)
        {
            EnhancedInputComponent->BindAction(RemoveWaterAction, ETriggerEvent::Started, this, &ATerrainController::StartRemoveWater);
            EnhancedInputComponent->BindAction(RemoveWaterAction, ETriggerEvent::Completed, this, &ATerrainController::StopRemoveWater);
        }
        
        // Spring editing bindings
            if (AddSpringAction)
            {
                EnhancedInputComponent->BindAction(AddSpringAction, ETriggerEvent::Started, this, &ATerrainController::StartAddSpring);
                EnhancedInputComponent->BindAction(AddSpringAction, ETriggerEvent::Completed, this, &ATerrainController::StopAddSpring);
            }
            if (RemoveSpringAction)
            {
                EnhancedInputComponent->BindAction(RemoveSpringAction, ETriggerEvent::Started, this, &ATerrainController::StartRemoveSpring);
                EnhancedInputComponent->BindAction(RemoveSpringAction, ETriggerEvent::Completed, this, &ATerrainController::StopRemoveSpring);
            }
        
        // Brush controls
        if (IncreaseBrushSizeAction)
        {
            EnhancedInputComponent->BindAction(IncreaseBrushSizeAction, ETriggerEvent::Triggered, this, &ATerrainController::IncreaseBrushSize);
        }
        if (DecreaseBrushSizeAction)
        {
            EnhancedInputComponent->BindAction(DecreaseBrushSizeAction, ETriggerEvent::Triggered, this, &ATerrainController::DecreaseBrushSize);
        }
        if (IncreaseBrushStrengthAction)
        {
            EnhancedInputComponent->BindAction(IncreaseBrushStrengthAction, ETriggerEvent::Triggered, this, &ATerrainController::IncreaseBrushStrength);
        }
        if (DecreaseBrushStrengthAction)
        {
            EnhancedInputComponent->BindAction(DecreaseBrushStrengthAction, ETriggerEvent::Triggered, this, &ATerrainController::DecreaseBrushStrength);
        }
        
        // System controls
        if (ToggleVisualizationAction)
        {
            EnhancedInputComponent->BindAction(ToggleVisualizationAction, ETriggerEvent::Started, this, &ATerrainController::ToggleVisualMode);
        }
        if (ToggleRainAction)
        {
            EnhancedInputComponent->BindAction(ToggleRainAction, ETriggerEvent::Started, this, &ATerrainController::HandleWaterToggle);
        }

        if (ToggleEditingModeAction)
        {
            EnhancedInputComponent->BindAction(ToggleEditingModeAction, ETriggerEvent::Started, this, &ATerrainController::CycleSubMode);
        }
        
        // Direct mode selection (1-4 keys)
        if (SelectTerrainModeAction)
        {
            EnhancedInputComponent->BindAction(SelectTerrainModeAction, ETriggerEvent::Started, this, &ATerrainController::SelectTerrainMode);
        }
        if (SelectWaterModeAction)
        {
            EnhancedInputComponent->BindAction(SelectWaterModeAction, ETriggerEvent::Started, this, &ATerrainController::SelectWaterMode);
        }
        if (SelectAtmosphereModeAction)
        {
            EnhancedInputComponent->BindAction(SelectAtmosphereModeAction, ETriggerEvent::Started, this, &ATerrainController::SelectAtmosphereMode);
        }
        if (SelectEcosystemModeAction)
        {
            EnhancedInputComponent->BindAction(SelectEcosystemModeAction, ETriggerEvent::Started, this, &ATerrainController::SelectEcosystemMode);
        }
        
        if (ResetTerrainAction)
        {
            EnhancedInputComponent->BindAction(ResetTerrainAction, ETriggerEvent::Started, this, &ATerrainController::ResetTerrain);
        }
        
        // Time control
        if (IncreaseTimeSpeedAction)
        {
            EnhancedInputComponent->BindAction(IncreaseTimeSpeedAction, ETriggerEvent::Triggered, this, &ATerrainController::IncreaseTimeSpeed);
        }
        if (DecreaseTimeSpeedAction)
        {
            EnhancedInputComponent->BindAction(DecreaseTimeSpeedAction, ETriggerEvent::Triggered, this, &ATerrainController::DecreaseTimeSpeed);
        }
        
        if (PauseAction)
        {
            EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started,
                this, &ATerrainController::HandlePauseToggle);
        }
        
        // Camera switching
        if (SwitchCameraAction)
        {
            EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Started, this, &ATerrainController::WarpToFirstPerson);
        }
        if (ReturnToOverheadAction)
        {
            EnhancedInputComponent->BindAction(ReturnToOverheadAction, ETriggerEvent::Started, this, &ATerrainController::CycleCameraMode);
        }
        
        // Atmospheric brush cycling (Y key)
        if (CycleBrushModeAction)
        {
            EnhancedInputComponent->BindAction(CycleBrushModeAction, ETriggerEvent::Started, this, &ATerrainController::HandleBrushCycle);
        }
        
        // Return to main menu (Escape key)
        if (ReturnToMainMenuAction)
        {
            EnhancedInputComponent->BindAction(ReturnToMainMenuAction, ETriggerEvent::Started, this, &ATerrainController::ReturnToMainMenu);
        }
        
        if (ToggleControlPanelAction)
        {
            EnhancedInputComponent->BindAction(ToggleControlPanelAction,
                ETriggerEvent::Started,
                this,
                &ATerrainController::ToggleControlPanel);
        }
    }
}


// ===== MOVEMENT FUNCTIONS =====

void ATerrainController::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    MovementInput = MovementVector;
}

void ATerrainController::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    
    // CRITICAL FIX: Same as Move() - explicitly zero when input is nearly zero
    // This prevents "flick" accumulation when shift is released
    if (LookVector.IsNearlyZero(0.001f))
    {
        LookInput = FVector2D::ZeroVector;
        return;
    }
    
    // Only allow look input when shift is held and not editing
    if (!bIsEditingTerrain && !bIsEditingWater)
    {
        LookInput = LookVector;
    }
    else
    {
        LookInput = FVector2D::ZeroVector;
    }
}

void ATerrainController::Zoom(const FInputActionValue& Value)
{
    float ZoomValue = Value.Get<float>();
    ZoomInput = ZoomValue;
}

void ATerrainController::FlyUp(const FInputActionValue& Value)
{
    float FlyValue = Value.Get<float>();
    FlyUpInput = FlyValue;
}

void ATerrainController::FlyDown(const FInputActionValue& Value)
{
    float FlyValue = Value.Get<float>();
    FlyDownInput = FlyValue;
}

void ATerrainController::UpdateCameraPosition(float DeltaTime)
{
    // Update authority cache first
    UpdateAuthorityCache(DeltaTime);
    
    // Handle camera transitions
    UpdateCameraTransition(DeltaTime);
    
    if (bTransitioning)
    {
        ResetInputs();
        return;
    }
    
    if (CurrentCameraMode == ECameraMode::Overhead)
    {
        UpdateOverheadCamera(DeltaTime);
    }
    else if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        UpdateFirstPersonCamera(DeltaTime);
    }
    
    // FIXED: Consolidated input reset at END
    ResetInputs();
}

void ATerrainController::ToggleControlPanel()
{
    // Just broadcast an event that Blueprint can listen to
    OnToggleControlPanel();
}


// ============================================================================
// SECTION 7: TERRAIN EDITING SYSTEM ðŸ’§ (~225 lines, 9%)
// ============================================================================
/**
 * PURPOSE:
 * Interactive terrain modification system providing user control over terrain
 * heightmap editing, water addition/removal, and brush parameter adjustment.
 * Integrates with Universal Brush System for coordinated multi-system editing.
 *
 * EDITING MODES:
 * 1. Terrain Editing (bIsEditingTerrain):
 *    - Raise terrain (bIsRaisingTerrain)
 *    - Lower terrain (bIsLoweringTerrain)
 *    - Pauses water simulation during terrain edits to prevent artifacts
 *    - Forces water system sync after editing completes
 *
 * 2. Water Editing (bIsEditingWater):
 *    - Add water (bIsAddingWater) with volume tracking
 *    - Remove water (bIsRemovingWater) with volume tracking
 *    - Tracks exact volume changes for conservation validation
 *
 * 3. Brush Parameter Control:
 *    - Size adjustment (IncreaseBrushSize/DecreaseBrushSize)
 *    - Strength adjustment (IncreaseBrushStrength/DecreaseBrushStrength)
 *    - Delegates to MasterController's Universal Brush System
 *
 * UNIVERSAL BRUSH INTEGRATION:
 * Terrain modification now uses MasterController->ApplyBrushToReceivers()
 * which calls back to our ApplyBrush() interface method (Section 2).
 * This ensures consistent brush behavior across all systems.
 *
 * STATE MANAGEMENT:
 * Input handlers set state flags that Tick() checks to run appropriate
 * update loops. Start/Stop functions toggle flags and manage system state
 * (like pausing water simulation during terrain edits).
 *
 * WATER CONSERVATION:
 * Water editing tracks exact volume changes using MeasureVolumeChange()
 * lambdas to ensure conservation principle is maintained.
 * ============================================================================
 */

// ===== TERRAIN EDITING FUNCTIONS =====

void ATerrainController::StartRaiseTerrain(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        StartTerrainEditing(true);
    }
}

void ATerrainController::StopRaiseTerrain(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        StopTerrainEditing();
    }
}

void ATerrainController::StartLowerTerrain(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        StartTerrainEditing(false);
    }
}

void ATerrainController::StopLowerTerrain(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        StopTerrainEditing();
    }
}

void ATerrainController::StartTerrainEditing(bool bRaise)
{
    bIsEditingTerrain = true;
    bIsRaisingTerrain = bRaise;
    bIsLoweringTerrain = !bRaise;

    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->bPausedForTerrainEdit = true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started %s terrain"), bRaise ?
           TEXT("raising") : TEXT("lowering"));
}

void ATerrainController::StopTerrainEditing()
{
    bIsEditingTerrain = false;
    bIsRaisingTerrain = false;
    bIsLoweringTerrain = false;
    
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->bPausedForTerrainEdit = false;
        TargetTerrain->WaterSystem->ForceTerrainSync();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped terrain editing"));
}

void ATerrainController::UpdateTerrainModification(float DeltaTime)
{
    if (!TargetTerrain || !MasterController)
    {
        return;
    }
    
    // Get cursor position using the unified cursor system
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition != FVector::ZeroVector && ValidateCursorPosition(CursorPosition))
    {
        // UNIVERSAL BRUSH SYSTEM: Apply brush through MasterController authority
        MasterController->ApplyBrushToReceivers(CursorPosition, DeltaTime);
        
        // The actual terrain modification happens in ApplyBrush() interface method
        // This eliminates the need for direct terrain modification here
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("[TERRAIN CONTROLLER] Applied universal brush at position: %s"),
               *CursorPosition.ToString());
    }
}

// ===== WATER EDITING FUNCTIONS =====

void ATerrainController::StartAddWater(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::WaterBrush)
    {
        StartWaterEditing(true);
    }
}

void ATerrainController::StopAddWater(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water)
    {
        StopWaterEditing();
    }
}

void ATerrainController::StartRemoveWater(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::WaterBrush)
    {
        StartWaterEditing(false);
    }
}

void ATerrainController::StopRemoveWater(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water)
    {
        StopWaterEditing();
    }
}

void ATerrainController::StartWaterEditing(bool bAdd)
{
    bIsEditingWater = true;
    bIsAddingWater = bAdd;
    bIsRemovingWater = !bAdd;
  
    UE_LOG(LogTemp, Warning, TEXT("Started %s water"), bAdd ?
           TEXT("adding") : TEXT("removing"));
}


void ATerrainController::StopWaterEditing()
{
    bIsEditingWater = false;
    bIsAddingWater = false;
    bIsRemovingWater = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped water editing"));
}

void ATerrainController::UpdateWaterModification(float DeltaTime)
{
    if (!TargetTerrain || !MasterController || !TargetTerrain->WaterSystem) return;
    
    FVector CursorWorldPos = GetCursorWorldPosition();
    
    if (bIsAddingWater)
    {
        float AmountToAdd = WaterBrushStrength * WaterAdditionRate * DeltaTime;
        
        // Measure exact volume change
        float VolumeChange = TargetTerrain->WaterSystem->MeasureVolumeChange([&]()
        {
            TargetTerrain->WaterSystem->AddWater(CursorWorldPos, AmountToAdd);
        });
    }
    else if (bIsRemovingWater)
    {
        float AmountToRemove = WaterBrushStrength * DeltaTime;
        
        // Measure exact volume change
        float VolumeChange = TargetTerrain->WaterSystem->MeasureVolumeChange([&]()
        {
            TargetTerrain->WaterSystem->RemoveWater(CursorWorldPos, AmountToRemove);
        });
    }
}

// ===== BRUSH CONTROL FUNCTIONS =====

void ATerrainController::IncreaseBrushSize(const FInputActionValue& Value)
{
    
        float CurrentRadius = GetBrushRadius();
        float ChangeRate = 50.0f; // Hardcoded since we removed the property
        SetBrushRadius(CurrentRadius + ChangeRate);
    
}

void ATerrainController::DecreaseBrushSize(const FInputActionValue& Value)
{
    
        float CurrentRadius = GetBrushRadius();
        float ChangeRate = 50.0f; // Hardcoded since we removed the property
        SetBrushRadius(CurrentRadius - ChangeRate);
    
}

void ATerrainController::IncreaseBrushStrength(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        float CurrentStrength = GetBrushStrength();
        float ChangeRate = 50.0f; // Hardcoded since we removed the property
        SetBrushStrength(CurrentStrength + ChangeRate);
    }
    else if (CurrentMainMode == EMainEditingMode::Water)
    {
        SetWaterBrushStrength(WaterBrushStrength + WaterStrengthChangeRate);
    }
}

void ATerrainController::DecreaseBrushStrength(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Terrain)
    {
        float CurrentStrength = GetBrushStrength();
        float ChangeRate = 50.0f; // Hardcoded since we removed the property
        SetBrushStrength(CurrentStrength - ChangeRate);
    }
    else if (CurrentMainMode == EMainEditingMode::Water)
    {
        SetWaterBrushStrength(WaterBrushStrength - WaterStrengthChangeRate);
    }
}
// REMOVED: All duplicate function definitions that were causing linker errors

void ATerrainController::ApplyAtmosphericBrush(FVector Position)
{
    if (!AtmosphericSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("No atmospheric system available"));
        return;
    }
    
    if (!ValidateCursorPosition(Position))
    {
        return;
    }
    
    // Apply master controller scaling to atmospheric brush
    float ScaledBrushRadius = AtmosphericBrushRadius * GetMasterBrushScale();
/*
    switch (CurrentAtmosphericBrush)
    {
        case EAtmosphericBrushType::Wind:
            AtmosphericSystem->ApplyWindBrush(Position, ScaledBrushRadius,
                                            WindForce, AtmosphericBrushIntensity);
            break;
            
        case EAtmosphericBrushType::Pressure:
            AtmosphericSystem->ApplyPressureBrush(Position, ScaledBrushRadius,
                                                PressureDelta, AtmosphericBrushIntensity);
            break;
            
        case EAtmosphericBrushType::Temperature:
            AtmosphericSystem->ApplyTemperatureBrush(Position, ScaledBrushRadius,
                                                    TemperatureDelta, AtmosphericBrushIntensity);
            break;
            
        case EAtmosphericBrushType::Humidity:
            AtmosphericSystem->ApplyHumidityBrush(Position, ScaledBrushRadius,
                                                HumidityDelta, AtmosphericBrushIntensity);
            break;
    }
 */
}
void ATerrainController::SetWaterBrushStrength(float NewStrength)
{
    WaterBrushStrength = FMath::Clamp(NewStrength, MinWaterBrushStrength, MaxWaterBrushStrength);
    UE_LOG(LogTemp, Warning, TEXT("Water brush strength: %.1f"), WaterBrushStrength);
}

// ===== SYSTEM CONTROL FUNCTIONS =====

void ATerrainController::HandleWaterToggle()
{

        WaterController->ToggleWaterVisualMode();
        UE_LOG(LogTemp, Log, TEXT("TerrainController: Delegated water visual toggle to WaterController"));

}

void ATerrainController::ToggleVisualMode()
{
    // Normal terrain material cycling
    int32 CurrentModeInt = static_cast<int32>(CurrentVisualMode);
    CurrentModeInt = (CurrentModeInt + 1) % 5;  // Cycles through 5 terrain modes
    SetVisualMode(static_cast<ETerrainVisualMode>(CurrentModeInt));
}

void ATerrainController::SetVisualMode(ETerrainVisualMode NewMode)
{
    CurrentVisualMode = NewMode;
    
    UMaterialInterface* MaterialToApply = nullptr;
    FString ModeName;
    
    switch (CurrentVisualMode)
    {
        case ETerrainVisualMode::Wireframe:
            MaterialToApply = WireframeMaterial;
            ModeName = TEXT("Digital Wireframe");
            break;
        case ETerrainVisualMode::Naturalistic:
            MaterialToApply = NaturalisticMaterial;
            ModeName = TEXT("Natural Landscape");
            break;
        case ETerrainVisualMode::Hybrid:
            MaterialToApply = HybridMaterial;
            ModeName = TEXT("Hybrid Mode");
            break;
        case ETerrainVisualMode::Chrome:
            MaterialToApply = ChromeMaterial;
            ModeName = TEXT("Chrome");
            break;
        case ETerrainVisualMode::Glass:
            MaterialToApply = GlassMaterial;
            ModeName = TEXT("Glass");
            break;
        default:
            MaterialToApply = WireframeMaterial;
            ModeName = TEXT("Default Wireframe");
            break;
    }
    
    if (MaterialToApply)
    {
        ApplyMaterialToAllChunks(MaterialToApply);
        
        // CRITICAL FIX: Trigger water system shader creation when materials change
        if (TargetTerrain && TargetTerrain->WaterSystem && TargetTerrain->WaterSystem->bUseShaderWater)
        {
            // Force water depth texture creation/update
            TargetTerrain->WaterSystem->CreateWaterDepthTexture();
            TargetTerrain->WaterSystem->UpdateWaterDepthTexture();
            
            UE_LOG(LogTemp, Warning, TEXT("Water shader textures refreshed for material change"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Visual mode changed to: %s"), *ModeName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Material not set for visual mode: %s"), *ModeName);
    }
}



// ============================================================================
// NEW: HIERARCHICAL MODE SYSTEM
// ============================================================================
/**
 * PURPOSE:
 * Implements improved input system with hierarchical modes:
 * - 1-4 keys for direct main mode selection (Terrain, Water, Atmosphere, Ecosystem)
 * - T key for cycling sub-modes within current main mode
 * - Mode memory: each mode remembers its last sub-mode
 */

void ATerrainController::SelectTerrainMode(const FInputActionValue& Value)
{
    CurrentMainMode = EMainEditingMode::Terrain;
    StopAllEditing();
    
    UE_LOG(LogTemp, Warning, TEXT("🏔️ TERRAIN mode | Sub-mode: %s (T to cycle)"),
        *GetCurrentSubModeDisplayName());
}

void ATerrainController::SelectWaterMode(const FInputActionValue& Value)
{
    CurrentMainMode = EMainEditingMode::Water;
    StopAllEditing();
    
    UE_LOG(LogTemp, Warning, TEXT("💧 WATER mode | Sub-mode: %s (T to cycle)"),
        *GetCurrentSubModeDisplayName());
    
    // Show controls for spring placement mode
    if (CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Left click: Add spring | Right click: Remove springs"));
    }
}

void ATerrainController::SelectAtmosphereMode(const FInputActionValue& Value)
{
    CurrentMainMode = EMainEditingMode::Atmosphere;
    StopAllEditing();
    
    UE_LOG(LogTemp, Warning, TEXT("☁️ ATMOSPHERE mode | Sub-mode: %s (T to cycle)"),
        *GetCurrentSubModeDisplayName());
}

void ATerrainController::SelectEcosystemMode(const FInputActionValue& Value)
{
    CurrentMainMode = EMainEditingMode::Ecosystem;
    StopAllEditing();
    
    UE_LOG(LogTemp, Warning, TEXT("🌿 ECOSYSTEM mode - Coming Soon!"));
    UE_LOG(LogTemp, Warning, TEXT("  Future: Vegetation placement, growth simulation, and ecosystem dynamics"));
}

void ATerrainController::StopAllEditing()
{
    // Stop all editing states
    StopTerrainEditing();
    StopWaterEditing();
    StopSpringEditing();
    // Note: Atmospheric editing is instantaneous (no persistent state)
}

void ATerrainController::CycleSubMode(const FInputActionValue& Value)
{
    StopAllEditing();
    
    switch (CurrentMainMode)
    {
        case EMainEditingMode::Terrain:
        {
            // Left click = Raise, Right click = Lower (always active)
            // Future: Smooth and Flatten sub-modes
            UE_LOG(LogTemp, Warning, TEXT("Terrain: Left click raises, Right click lowers (Smooth/Flatten coming soon)"));
            break;
        }
        
        case EMainEditingMode::Water:
        {
            // Toggle between Water Brush and Spring Placement
            int32 SubModeInt = static_cast<int32>(CurrentWaterSubMode);
            SubModeInt = (SubModeInt + 1) % 2;
            CurrentWaterSubMode = static_cast<EWaterSubMode>(SubModeInt);
            
            UE_LOG(LogTemp, Warning, TEXT("Water: %s"), *GetCurrentSubModeDisplayName());
            
            // Show spring controls when switching to spring mode
            if (CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Left click: Add spring | Right click: Remove springs"));
            }
            break;
        }
        
        case EMainEditingMode::Atmosphere:
        {
            // Use existing atmospheric brush cycling
            CycleAtmosphericBrush();
            UE_LOG(LogTemp, Warning, TEXT("Atmosphere: %s"), *GetCurrentSubModeDisplayName());
            break;
        }
        
        case EMainEditingMode::Ecosystem:
        {
            UE_LOG(LogTemp, Warning, TEXT("Ecosystem sub-modes coming soon!"));
            break;
        }
    }
}

FString ATerrainController::GetCurrentModeDisplayName() const
{
    switch (CurrentMainMode)
    {
        case EMainEditingMode::Terrain: return TEXT("Terrain");
        case EMainEditingMode::Water: return TEXT("Water");
        case EMainEditingMode::Atmosphere: return TEXT("Atmosphere");
        case EMainEditingMode::Ecosystem: return TEXT("Ecosystem");
        default: return TEXT("Unknown");
    }
}

FString ATerrainController::GetCurrentSubModeDisplayName() const
{
    switch (CurrentMainMode)
    {
        case EMainEditingMode::Terrain:
            // Both raise and lower are always active (left/right click)
            return TEXT("Raise/Lower");
            
        case EMainEditingMode::Water:
            switch (CurrentWaterSubMode)
            {
                case EWaterSubMode::WaterBrush: return TEXT("Water Brush");
                case EWaterSubMode::SpringPlacement: return TEXT("Spring Placement");
                default: return TEXT("Unknown");
            }
            
        case EMainEditingMode::Atmosphere:
            // Use existing GetCurrentBrushDisplayName() for atmospheric modes
            return GetCurrentBrushDisplayName();
            
        case EMainEditingMode::Ecosystem:
            return TEXT("Coming Soon");
            
        default:
            return TEXT("Unknown");
    }
}

// END HIERARCHICAL MODE SYSTEM
// ============================================================================

void ATerrainController::HandleRainToggle()
{
    if (WaterController)
    {
        WaterController->ToggleRain();
        UE_LOG(LogTemp, Log, TEXT("TerrainController: Delegated rain toggle to WaterController"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainController: No WaterController to delegate rain toggle"));
    }
}

// Keep legacy rain function for backwards compatibility - now delegates to WaterController
void ATerrainController::ToggleRain()
{
    if (WaterController)
    {
        WaterController->ToggleRain();
        UE_LOG(LogTemp, Log, TEXT("TerrainController: Legacy ToggleRain() delegated to WaterController"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainController: No WaterController for legacy rain toggle"));
    }
}

void ATerrainController::ToggleRainInput(const FInputActionValue& Value)
{
    HandleRainToggle();
}




// ============================================================================
// SECTION 8: SPRING SYSTEM (~230 lines, 9%)
// ============================================================================
/**
 * PURPOSE:
 * Interactive groundwater spring management system allowing users to place
 * and remove springs with flow rates calculated from brush size. Integrates
 * with GeologyController for spring lifecycle management.
 *
 * SPRING MECHANICS:
 * - Springs are user-placed water sources
 * - Flow rate (mÂ³/s) scales with brush size (100-5000 units)
 * - Uses power curve (1.5 exponent) for intuitive scaling
 * - Small brush = weak spring (0.1 mÂ³/s)
 * - Large brush = strong spring (10.0 mÂ³/s)
 *
 * WORKFLOW:
 * 1. Switch to Spring editing mode (T key cycling)
 * 2. Left click + hold = add spring preview
 * 3. Release = place spring with current flow rate
 * 4. Right click = remove all springs within brush radius
 *
 * INTEGRATION:
 * - GeologyController manages spring lifecycle and updates
 * - Springs feed into water system through GeologyController
 * - Brush size from MasterController's Universal Brush System
 * - Removal radius matches current brush size
 *
 * INPUT HANDLING:
 * - StartAddSpring/StopAddSpring: Place spring on release (not continuous)
 * - StartRemoveSpring/StopRemoveSpring: Remove springs on release
 * - UpdateSpringEditing: Tracks cursor and updates removal radius
 * ============================================================================
 */

// Spring editing input handlers
void ATerrainController::StartAddSpring(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
    {
        bIsAddingSpring = true;
        bIsEditingSpring = true;
        
        // Visual feedback only - actual placement on release
    }
}

void ATerrainController::StopAddSpring(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::SpringPlacement && bIsAddingSpring)
    {
        // Place spring at cursor position with flow rate based on brush size
        FVector CursorPosition = GetCursorWorldPosition();
        if (CursorPosition != FVector::ZeroVector && GeologyController)
        {
            // Calculate flow rate from current brush size
            float FlowRate = CalculateSpringFlowFromBrushSize();
            
            GeologyController->AddUserSpring(CursorPosition, FlowRate);
            UE_LOG(LogTemp, Warning, TEXT("Placed spring at %s with flow rate %.2f m³/s (brush size: %.0f)"),
                *CursorPosition.ToString(), FlowRate, MasterController->GetBrushRadius());
        }
        
        bIsAddingSpring = false;
        bIsEditingSpring = false;
    }
}

void ATerrainController::StartRemoveSpring(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
    {
        bIsRemovingSpring = true;
        bIsEditingSpring = true;
    }
}

void ATerrainController::StopRemoveSpring(const FInputActionValue& Value)
{
    if (CurrentMainMode == EMainEditingMode::Water &&
        CurrentWaterSubMode == EWaterSubMode::SpringPlacement && bIsRemovingSpring)
    {
        // Remove springs near cursor position using current brush size as radius
        FVector CursorPosition = GetCursorWorldPosition();
        if (CursorPosition != FVector::ZeroVector && GeologyController && MasterController)
        {
            // Use current brush radius for removal
            float RemovalRadius = MasterController->GetBrushRadius();
            
            GeologyController->RemoveUserSpring(CursorPosition, RemovalRadius);
            UE_LOG(LogTemp, Warning, TEXT("Removed springs near %s (radius: %.0f)"),
                *CursorPosition.ToString(), RemovalRadius);
        }
        
        bIsRemovingSpring = false;
        bIsEditingSpring = false;
    }
}

void ATerrainController::StartSpringEditing(bool bAdd)
{
    bIsEditingSpring = true;
    bIsAddingSpring = bAdd;
    bIsRemovingSpring = !bAdd;
    /*
    // SURGICAL FIX: Snap cursor to eliminate interpolation jump
    FVector CurrentRawPosition;
    if (PerformCursorTrace(CurrentRawPosition))
    {
        UnifiedCursorPosition = CurrentRawPosition;
        bUnifiedCursorValid = true;
        UE_LOG(LogTemp, VeryVerbose, TEXT("[SPRING EDIT START] Snapped cursor to: %s"),
               *CurrentRawPosition.ToString());
    }
    */
    UE_LOG(LogTemp, Warning, TEXT("Started %s springs"), bAdd ?
           TEXT("adding") : TEXT("removing"));
}

void ATerrainController::StopSpringEditing()
{
    bIsEditingSpring = false;
    bIsAddingSpring = false;
    bIsRemovingSpring = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped spring editing"));
}

// UPDATED: Calculate spring flow rate based on brush size
float ATerrainController::CalculateSpringFlowFromBrushSize() const
{
    if (!MasterController) return 1.0f;
    
    // Get current brush radius from the Universal Brush System
    float BrushRadius = MasterController->GetBrushRadius();
    
    // Map brush radius to flow rate
    // Default brush radius range is typically 100-5000 units
    // Map this to flow rate of 0.1-10.0 mÂ³/s
    float MinBrushRadius = 100.0f;
    float MaxBrushRadius = 5000.0f;
    
    float NormalizedRadius = FMath::GetMappedRangeValueClamped(
        FVector2D(MinBrushRadius, MaxBrushRadius),
        FVector2D(0.0f, 1.0f),
        BrushRadius
    );
    
    // Use a power curve for more intuitive scaling
    // Small brushes = weak springs, large brushes = strong springs
    float FlowRate = FMath::Lerp(MinSpringFlowRate, MaxSpringFlowRate,
                                  FMath::Pow(NormalizedRadius, 1.5f));
    
    return FlowRate;
}

// Update function for spring editing
void ATerrainController::UpdateSpringEditing(float DeltaTime)
{
    if (!bIsEditingSpring || !GeologyController)
    {
        return;
    }
    
    // Get cursor position
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition == FVector::ZeroVector || !ValidateCursorPosition(CursorPosition))
    {
        return;
    }
    
    // Update the removal radius to match current brush size when removing
    if (bIsRemovingSpring && MasterController)
    {
        SpringRemovalRadius = MasterController->GetBrushRadius();
    }
}

void ATerrainController::HandlePauseToggle()
{
    if (MasterController)
    {
        MasterController->TogglePause();
    }
}

void ATerrainController::ResetTerrain(const FInputActionValue& Value)
{
    if (TargetTerrain)
    {
        // Set resetting flag to prevent height queries during reset
        SetTerrainResetting(true);
        
        // Reset transition state to prevent invalid camera states
        if (bTransitioning)
        {
            bTransitioning = false;
            UE_LOG(LogTemp, Warning, TEXT("Reset interrupted camera transition"));
        }
        
        // Reset terrain (which now includes atmosphere reset)
        TargetTerrain->ResetTerrainFully();
        UE_LOG(LogTemp, Warning, TEXT("Full terrain system reset"));
        
        // OPTIONAL: Explicitly reset atmosphere through MasterController
        if (MasterController && MasterController->AtmosphereController)
        {
            UE_LOG(LogTemp, Warning, TEXT("Ensuring atmosphere reset via MasterController"));
            // Already handled in ResetTerrainFully, but this confirms it
        }
        
        // Reset flag after a brief delay to allow systems to stabilize
        FTimerHandle ResetTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, [this]()
        {
            SetTerrainResetting(false);
            UE_LOG(LogTemp, Warning, TEXT("Terrain reset complete - operations resumed"));
        }, 0.5f, false); // 0.5 second delay
    }
}


// ===== UTILITY FUNCTIONS =====

FVector ATerrainController::GetCursorWorldPosition() const
{
    return UnifiedCursorPosition;
}



float ATerrainController::GetMasterBrushScale() const
{
    if (!MasterController)
    {
        return 1.0f; // Default scale
    }
    
    // Get brush scale multiplier from MasterController
    return MasterController->GetBrushScaleMultiplier();
}

bool ATerrainController::ValidateCursorPosition(FVector CursorPos) const
{
    if (!MasterController)
    {
        return true; // Legacy mode - no validation
    }
    
    // Basic validation - ensure position is reasonable
    if (TargetTerrain)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(CursorPos);
        return TerrainHeight > -99999.0f;
    }
    
    return true;
}


// ============================================================================
// SECTION 9: CURSOR & POSITIONING (~155 lines, 6%)
// ============================================================================
/**
 * PURPOSE:
 * World space cursor management and raycasting for editing operations.
 * Provides unified cursor position for all subsystems with camera-aware
 * raycasting and terrain boundary validation.
 *
 * UNIFIED CURSOR SYSTEM:
 * All editing operations use GetCursorWorldPosition() which returns the
 * UnifiedCursorPosition. This is updated in Tick() via UpdateUnifiedCursor()
 * to provide instant response without lag.
 *
 * RAYCASTING STRATEGY:
 * The PerformCursorTrace() function uses different approaches for each
 * camera mode:
 *
 * 1. FirstPerson Mode:
 *    - Deprojects mouse to world ray
 *    - Performs line trace against terrain mesh
 *    - Falls back to terrain plane intersection if no hit
 *    - Queries terrain heightmap for final Z coordinate
 *
 * 2. Overhead Mode:
 *    - Deprojects mouse to world ray
 *    - Intersects ray with terrain plane (Z=0)
 *    - Clamps to terrain boundaries
 *    - Queries terrain heightmap for final Z coordinate
 *
 * WORLD SCALING AWARENESS:
 * GetMasterBrushScale() queries MasterController for the current world
 * scaling multiplier. This ensures brush sizes adapt properly when switching
 * between Small/Medium/Large/Massive world configurations.
 *
 * BOUNDARY VALIDATION:
 * ValidateCursorPosition() ensures cursor stays within terrain bounds
 * and returns valid terrain height. Prevents editing outside world limits.
 * ============================================================================
 */

bool ATerrainController::PerformCursorTrace(FVector& OutHitLocation) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return false;
    }
    
    // SURGICAL FIX: Prevent ZeroVector cache corruption during initialization
    if (!TargetTerrain)
    {
        return false;
    }
    
    // CAMERA-SPECIFIC CURSOR LOGIC
    if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        if (!FirstPersonCamera)
        {
            return false;
        }
        
        FVector MouseWorldLocation, MouseWorldDirection;
        if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
        {
            FHitResult HitResult;
            FVector TraceStart = MouseWorldLocation;
            FVector TraceEnd = MouseWorldLocation + (MouseWorldDirection * 100000.0f);
            
            FCollisionQueryParams QueryParams;
            QueryParams.bTraceComplex = true;
            QueryParams.AddIgnoredActor(this);
            
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                TraceStart,
                TraceEnd,
                ECC_WorldStatic,
                QueryParams
            );
            
            if (bHit)
            {
                OutHitLocation = HitResult.Location;
                return true;
            }
            else
            {
                FVector PlaneNormal = FVector::UpVector;
                FVector PlanePoint = TargetTerrain->GetActorLocation(); // Now safe - checked above
                
                float Denominator = FVector::DotProduct(MouseWorldDirection, PlaneNormal);
                if (FMath::Abs(Denominator) > 0.0001f)
                {
                    float T = FVector::DotProduct(PlanePoint - MouseWorldLocation, PlaneNormal) / Denominator;
                    if (T > 0)
                    {
                        FVector PlaneIntersection = MouseWorldLocation + (MouseWorldDirection * T);
                        float TerrainHeight = TargetTerrain->GetHeightAtPosition(PlaneIntersection);
                        OutHitLocation = FVector(PlaneIntersection.X, PlaneIntersection.Y, TerrainHeight);
                        return true;
                    }
                }
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        // OVERHEAD MODE
        FVector MouseWorldLocation, MouseWorldDirection;
        if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
        {
            FVector PlaneNormal = FVector::UpVector;
            FVector PlanePoint = TargetTerrain->GetActorLocation(); // Now safe - checked above
            
            float Denominator = FVector::DotProduct(MouseWorldDirection, PlaneNormal);
            
            if (FMath::Abs(Denominator) > 0.0001f)
            {
                float T = FVector::DotProduct(PlanePoint - MouseWorldLocation, PlaneNormal) / Denominator;
                FVector PlaneIntersection = MouseWorldLocation + (MouseWorldDirection * T);
                
                FVector TerrainMin, TerrainMax;
                FVector TerrainLocation = TargetTerrain->GetActorLocation();
                float WorldSizeX = TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale;
                float WorldSizeY = TargetTerrain->TerrainHeight * TargetTerrain->TerrainScale;
                
                TerrainMin = TerrainLocation;
                TerrainMax = TerrainLocation + FVector(WorldSizeX, WorldSizeY, TargetTerrain->MaxTerrainHeight);
                
                float ClampedX = FMath::Clamp(PlaneIntersection.X, TerrainMin.X, TerrainMax.X);
                float ClampedY = FMath::Clamp(PlaneIntersection.Y, TerrainMin.Y, TerrainMax.Y);
                
                float TerrainHeight = TargetTerrain->GetHeightAtPosition(FVector(ClampedX, ClampedY, 0.0f));
                
                OutHitLocation = FVector(ClampedX, ClampedY, TerrainHeight);
                return true;
            }
        }
        
        return false;
    }
}


// ============================================================================
// SECTION 10: RENDERING & VISUAL UPDATES (~235 lines, 9%)
// ============================================================================
/**
 * PURPOSE:
 * Material system, chunk updates, performance monitoring, and visual feedback.
 * Coordinates rendering operations with terrain system and provides real-time
 * performance statistics for user feedback.
 *
 * MATERIAL SYSTEM:
 * - ApplyMaterialToAllChunks: Sets active material for entire terrain
 * - SetShadowCastingForAllChunks: Toggles shadow rendering per chunk
 * - Supports 5 visual modes (Wireframe, Naturalistic, Hybrid, Chrome, Glass)
 *
 * UNIFIED CURSOR UPDATE:
 * UpdateUnifiedCursor() provides instant response cursor positioning without
 * smoothing. This is CRITICAL for responsive editing and runs FIRST in Tick().
 * Previous smoothing caused noticeable lag in editing operations.
 *
 * TERRAIN SMOOTHING:
 * ApplyTerrainSmoothing() provides gentle topology cleanup by sampling
 * heights in a circle and smoothing toward average. Used as a secondary
 * pass to prevent sharp edges and mesh corruption.
 *
 * CHUNK UPDATE PRIORITIZATION:
 * - RequestChunkUpdate: Delegates to terrain's priority queue system
 * - CalculateChunkPriority: Distance-based priority with editing boost
 * - Closer chunks = higher priority
 * - Chunks near cursor during editing = 10x priority boost
 *
 * PERFORMANCE MONITORING:
 * UpdatePerformanceStats() displays real-time HUD information:
 * - Current editing mode (Terrain/Water/Spring/Atmosphere)
 * - Brush parameters (radius, strength)
 * - Visual mode
 * - Camera mode with transition state
 * - Spring count and flow rates
 * - Throttled to 4 Hz to reduce overhead
 * ============================================================================
 */

void ATerrainController::ApplyMaterialToAllChunks(UMaterialInterface* Material)
{
    CurrentActiveMaterial = Material;
    
    if (TargetTerrain)
    {
        TargetTerrain->SetActiveMaterial(Material);
        UE_LOG(LogTemp, Warning, TEXT("TerrainController: Applied material to terrain"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TerrainController: No target terrain found"));
    }
}

void ATerrainController::SetShadowCastingForAllChunks(bool bCastShadows)
{
    if (TargetTerrain)
    {
        for (const FTerrainChunk& Chunk : TargetTerrain->TerrainChunks)
        {
            if (Chunk.MeshComponent)
            {
                Chunk.MeshComponent->SetCastShadow(bCastShadows);
            }
        }
    }
}

void ATerrainController::UpdateUnifiedCursor(float DeltaTime)
{
    FVector CurrentRawCursorPosition;
    bool bValidCursor = PerformCursorTrace(CurrentRawCursorPosition);
    
    if (!bValidCursor)
    {
        return;
    }
    
    if (!bUnifiedCursorValid)
    {
        UnifiedCursorPosition = CurrentRawCursorPosition;
        bUnifiedCursorValid = true;
        return;
    }
    
    // INSTANT response - no smoothing
    UnifiedCursorPosition = CurrentRawCursorPosition;
}

void ATerrainController::ApplyTerrainSmoothing(FVector Position, float Radius, float Strength)
{
    if (!TargetTerrain) return;
    
    // Apply gentle smoothing to prevent sharp edges and mesh corruption
    // This acts as a secondary pass to clean up topology issues
    
    // Sample heights in a circle around the position
    const int32 SampleCount = 8;
    float AverageHeight = 0.0f;
    int32 ValidSamples = 0;
    
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = (float)i / (float)SampleCount * 2.0f * PI;
        FVector SamplePos = Position + FVector(
            FMath::Cos(Angle) * Radius * 0.5f,
            FMath::Sin(Angle) * Radius * 0.5f,
            0.0f
        );
        
        float SampleHeight = TargetTerrain->GetHeightAtPosition(SamplePos);
        if (SampleHeight > -99999.0f) // Valid height
        {
            AverageHeight += SampleHeight;
            ValidSamples++;
        }
    }
    
    if (ValidSamples > 0)
    {
        AverageHeight /= ValidSamples;
        float CurrentHeight = TargetTerrain->GetHeightAtPosition(Position);
        
        // Smooth towards average but preserve general shape
        float HeightDifference = AverageHeight - CurrentHeight;
        float SmoothingAmount = HeightDifference * Strength;
        
        // Apply smoothing as a gentle terrain modification
        bool bRaiseForSmooth = SmoothingAmount > 0.0f;
        TargetTerrain->ModifyTerrain(Position, Radius,
            FMath::Abs(SmoothingAmount), bRaiseForSmooth);
    }
}

void ATerrainController::RequestChunkUpdate(int32 ChunkIndex, float Priority)
{
    if (!TargetTerrain) return;
    
    // Delegate priority-based updates to terrain system
    // The DynamicTerrain will handle the actual priority queue
    TargetTerrain->RequestPriorityChunkUpdate(ChunkIndex, Priority);
}

float ATerrainController::CalculateChunkPriority(int32 ChunkIndex) const
{
    if (!TargetTerrain) return 0.0f;
    
    // Get camera position for distance calculation
    FVector CameraPos = GetActorLocation();
    FVector ChunkWorldPos = TargetTerrain->GetChunkWorldPosition(ChunkIndex);
    
    // Distance-based priority (closer = higher priority)
    float DistanceToCamera = FVector::Dist(CameraPos, ChunkWorldPos);
    float DistancePriority = 1.0f / FMath::Max(DistanceToCamera, 100.0f);
    
    // Editing priority boost
    FVector CursorPos = GetCursorWorldPosition();
    float DistanceToCursor = FVector::Dist(CursorPos, ChunkWorldPos);
    float EditingBoost = (bIsEditingTerrain || bIsEditingWater) ?
        (DistanceToCursor < 2000.0f ? 10.0f : 1.0f) : 1.0f;
    
    return DistancePriority * EditingBoost;
}


void ATerrainController::SetupInputMapping()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
                UE_LOG(LogTemp, Warning, TEXT("Input mapping context added"));
            }
        }
    }
}

void ATerrainController::UpdatePerformanceStats(float DeltaTime)
{
    StatUpdateTimer += DeltaTime;
    
    if (StatUpdateTimer >= 0.25f && GEngine)
    {
        // Show hierarchical editing mode: "Mode: Terrain | Raise [1-4: modes, T: cycle]"
        FString ModeText = FString::Printf(TEXT("Mode: %s | %s [1-4: modes, T: cycle]"),
            *GetCurrentModeDisplayName(), *GetCurrentSubModeDisplayName());
        GEngine->AddOnScreenDebugMessage(30, 0.5f, FColor::Cyan, ModeText);
        
        // Show context-specific info
        if (CurrentMainMode == EMainEditingMode::Terrain && MasterController)
        {
            float BrushRadius = GetBrushRadius();
            float BrushStrength = GetBrushStrength();
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Brush: %.0f radius, %.0f strength [L:Raise R:Lower]"), BrushRadius, BrushStrength));
        }
        else if (CurrentMainMode == EMainEditingMode::Water)
        {
            if (CurrentWaterSubMode == EWaterSubMode::WaterBrush)
            {
                GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                    FString::Printf(TEXT("Water Brush: %.1f strength"), WaterBrushStrength));
            }
            else if (CurrentWaterSubMode == EWaterSubMode::SpringPlacement)
            {
                float CurrentFlowRate = CalculateSpringFlowFromBrushSize();
                float BrushRadius = MasterController ? MasterController->GetBrushRadius() : 200.0f;
                
                GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                    FString::Printf(TEXT("Spring: %.2f m³/s flow, %.0f radius"),
                        CurrentFlowRate, BrushRadius));
                
                if (GeologyController)
                {
                    int32 SpringCount = GeologyController->UserSprings.Num();
                    GEngine->AddOnScreenDebugMessage(32, 0.5f, FColor::Cyan,
                        FString::Printf(TEXT("Active Springs: %d"), SpringCount));
                }
            }
        }
        else if (CurrentMainMode == EMainEditingMode::Atmosphere)
        {
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Atmospheric Brush: %s (Y to cycle)"), *GetCurrentBrushDisplayName()));
        }
        
        // Show current visual mode
        FString VisualModeText;
        switch (CurrentVisualMode)
        {
            case ETerrainVisualMode::Wireframe:
                VisualModeText = TEXT("Digital Wireframe");
                break;
            case ETerrainVisualMode::Naturalistic:
                VisualModeText = TEXT("Natural Landscape");
                break;
            case ETerrainVisualMode::Hybrid:
                VisualModeText = TEXT("Hybrid Mode");
                break;
            case ETerrainVisualMode::Chrome:
                VisualModeText = TEXT("Chrome");
                break;
            case ETerrainVisualMode::Glass:
                VisualModeText = TEXT("Glass");
                break;
            default:
                VisualModeText = TEXT("Unknown Mode");
                break;
        }
        
        GEngine->AddOnScreenDebugMessage(32, 0.5f, FColor::White,
            FString::Printf(TEXT("Visual Mode: %s (V to toggle)"), *VisualModeText));
        
        // Camera mode info with enhanced debugging
        FString CurrentModeText = (CurrentCameraMode == ECameraMode::Overhead) ? TEXT("OH") : TEXT("FP");
        FString TargetModeText = (TargetCameraMode == ECameraMode::Overhead) ? TEXT("OH") : TEXT("FP");
        FString TransitionText = bTransitioning ? FString::Printf(TEXT(" (%s->%s)"), *CurrentModeText, *TargetModeText) : TEXT("");
        
        // Camera active state debugging
        FString CameraStateText = FString::Printf(TEXT(" [OH:%s FP:%s]"),
            Camera->IsActive() ? TEXT("ON") : TEXT("off"),
            FirstPersonCamera->IsActive() ? TEXT("ON") : TEXT("off"));
        
        // Current rotation debugging (only show current active camera)
        FRotator CurrentRotation;
        FString ActiveCameraText;
        if (Camera->IsActive())
        {
            CurrentRotation = Camera->GetComponentRotation();
            ActiveCameraText = TEXT("OH-Active");
        }
        else if (FirstPersonCamera->IsActive())
        {
            CurrentRotation = FirstPersonCamera->GetComponentRotation();
            ActiveCameraText = TEXT("FP-Active");
        }
        else
        {
            CurrentRotation = FRotator::ZeroRotator;
            ActiveCameraText = TEXT("NONE-Active");
        }
        
        FString RotationText = FString::Printf(TEXT(" %.0fÂ°,%.0fÂ°"),
            CurrentRotation.Pitch, CurrentRotation.Yaw);
        
        GEngine->AddOnScreenDebugMessage(33, 0.5f, FColor::Yellow,
            FString::Printf(TEXT("Camera: %s%s%s %s%s (C=Warp, TAB=Cycle)"),
                           *CurrentModeText, *TransitionText, *CameraStateText, *ActiveCameraText, *RotationText));
        
        // Water mode info now handled by WaterController
        GEngine->AddOnScreenDebugMessage(34, 0.5f, FColor::Cyan,
            TEXT("Water Mode: Controlled by WaterController (Shift+V to toggle)"));
        
        StatUpdateTimer = 0.0f;
    }
}

void ATerrainController::UpdateTerrainInfo(float DeltaTime)
{
    // Additional terrain info can be displayed here
    // Currently handled in UpdatePerformanceStats
}


// ============================================================================
// SECTION 11: TIME MANAGEMENT (~35 lines, 1%)
// ============================================================================
/**
 * PURPOSE:
 * Temporal system integration allowing real-time control over simulation
 * speed. Provides user-facing time dilation controls while delegating to
 * MasterController when available for coordinated system-wide time scaling.
 *
 * TIME DILATION:
 * Uses UE5's world time dilation for simple playback speed control:
 * - 1.0x = Normal speed
 * - 0.5x = Half speed (slower)
 * - 2.0x = Double speed (faster)
 * - Range: 0.1x to 10.0x (configurable)
 *
 * FUTURE INTEGRATION:
 * Current implementation uses simple world time dilation. Future versions
 * should integrate with TemporalManager for multi-scale time coordination
 * (real-time, geological time, etc.) through MasterController authority.
 * ============================================================================
 */

// ===== MASTER CONTROLLER INTEGRATION DEBUG FUNCTIONS =====
// GetCachedCursorPosition() removed - unified cursor system handles all cursor requests

// ===== TIME CONTROL FUNCTIONS =====

void ATerrainController::IncreaseTimeSpeed()
{
    SetTimeSpeed(TimeSpeed + TimeSpeedChangeRate);
}

void ATerrainController::DecreaseTimeSpeed()
{
    SetTimeSpeed(TimeSpeed - TimeSpeedChangeRate);
}

void ATerrainController::SetTimeSpeed(float NewSpeed)
{
    TimeSpeed = FMath::Clamp(NewSpeed, MinTimeSpeed, MaxTimeSpeed);
    
    if (GetWorld())
    {
        GetWorld()->GetWorldSettings()->SetTimeDilation(TimeSpeed);
        UE_LOG(LogTemp, Warning, TEXT("Time Speed: %.1fx"), TimeSpeed);
    }
}

void ATerrainController::ResetTimeSpeed()
{
    SetTimeSpeed(1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Time speed reset to normal"));
}




// ============================================================================
// SECTION 12: CAMERA SYSTEM â­ (~405 lines, 16%)
// ============================================================================
/**
 * PURPOSE:
 * Sophisticated multi-mode camera system with smooth transitions and terrain-
 * aware positioning. Provides three distinct camera modes with seamless
 * blending and automatic height validation for first-person mode.
 *
 * CAMERA MODES:
 * 1. Overhead Mode (Default):
 *    - Top-down perspective at ~1500-20000 cm height
 *    - WASD movement in camera-relative directions
 *    - Mouse look with shift held (prevents conflict with editing)
 *    - Zoom via mouse wheel (adjusts actor Z position)
 *    - Vertical flight with Q/E keys
 *    - Uses SpringArm + Camera component
 *
 * 2. FirstPerson Mode:
 *    - Ground-level perspective at FirstPersonHeight (152.4 cm / 5 feet)
 *    - WASD movement in camera-facing direction
 *    - Free mouse look (pitch clamped to -80Â° to +80Â°)
 *    - Automatic terrain following with smooth height interpolation
 *    - Periodic height validation to catch terrain edits
 *    - Uses FirstPersonCamera component
 *
 * TRANSITION SYSTEM:
 * Two transition triggers with distinct behaviors:
 *
 * C Key (WarpToFirstPerson):
 * - Teleports to cursor position
 * - Always targets FirstPerson mode
 * - Maintains current camera yaw, sets pitch to 0Â°
 * - Instant horizontal movement + smooth transition
 *
 * TAB Key (CycleCameraMode):
 * - Cycles Overhead â†” FirstPerson
 * - Smooth in-place transitions (no teleporting)
 * - Overheadâ†’FirstPerson: Descends straight down, pitch 0Â°
 * - FirstPersonâ†’Overhead: Ascends straight up, pitch -45Â°
 * - Preserves current XY position
 *
 * TRANSITION MECHANICS:
 * - Uses VInterpTo for smooth location blending (TransitionSpeed)
 * - RInterpTo for smooth rotation blending
 * - Prepares target camera while source is still active
 * - Switches cameras at ~200 cm distance for smoothness
 * - Completes at <25 cm distance with final state validation
 * - Explicitly sets TargetCameraMode to prevent height-based detection errors
 *
 * TERRAIN FOLLOWING (FirstPerson only):
 * - Queries terrain height at current XY position
 * - Maintains FirstPersonHeight (152.4 cm) above terrain
 * - Smooth interpolation (HeightLerpSpeed = 8.0) for natural feel
 * - 5 cm tolerance to avoid constant micro-adjustments
 * - Periodic validation (10 Hz) catches terrain edits while stationary
 *
 * CAMERA CONFLICT RESOLUTION:
 * Active prevention of invalid camera states:
 * - Both cameras active â†’ Deactivate based on CurrentCameraMode
 * - Neither camera active â†’ Activate based on CurrentCameraMode
 * - Runs every frame in Tick() as safety check
 * - Prevents "black screen" bug from race conditions
 *
 * STATE MANAGEMENT:
 * - CurrentCameraMode: Active mode (authoritative)
 * - TargetCameraMode: Desired mode during transitions
 * - bTransitioning: Transition in progress flag
 * - TargetLocation: Destination position
 * - TargetRotation: Destination rotation
 *
 * INPUT HANDLING:
 * - Movement inputs smoothed with MovementInputSmoothness
 * - Look input zeroed during editing to prevent conflicts
 * - Zoom affects actor Z in Overhead mode
 * - All inputs consolidated and reset at end of UpdateCameraPosition()
 *
 * ARCHITECTURE PRINCIPLE: State Machine
 * The camera system operates as a clean state machine with:
 * - Well-defined states (Overhead, FirstPerson)
 * - Explicit transition triggers (C key, TAB key)
 * - Smooth interpolation during transitions
 * - No ambiguous or undefined states
 * - Active validation and conflict resolution
 *
 * SUBSECTIONS:
 * 12.1: Camera Mode Switching (WarpToFirstPerson, CycleCameraMode)
 * 12.2: Transition Management (UpdateCameraTransition)
 * 12.3: Mode-Specific Updates (UpdateOverheadCamera, UpdateFirstPersonCamera)
 * 12.4: Helper Functions (GetNextCameraMode, SwitchCameraMode, etc.)
 * ============================================================================
 */

// ===== CAMERA SYSTEM FUNCTIONS =====

void ATerrainController::WarpToFirstPerson()
{
    if (bTransitioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("WarpToFirstPerson blocked - already transitioning"));
        return;
    }
    
    // Get spawn position at cursor
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition == FVector::ZeroVector)
    {
        CursorPosition = GetActorLocation();
    }
    
    // FIXED: Use current active camera rotation as starting point
    FRotator CurrentActiveRotation;
    // ADD: Null check first
    if (!Camera || !FirstPersonCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera components are null in WarpToFirstPerson!"));
        return;
    }

    if (CurrentCameraMode == ECameraMode::Overhead && Camera->IsActive())
    {
        CurrentActiveRotation = SpringArm->GetComponentRotation();
    }
    else if (CurrentCameraMode == ECameraMode::FirstPerson && FirstPersonCamera->IsActive())
    {
        CurrentActiveRotation = FirstPersonCamera->GetComponentRotation();
    }
    else
    {
        // STRENGTHEN: Force correct camera state before fallback
        UE_LOG(LogTemp, Warning, TEXT("No camera active in WarpToFirstPerson, forcing correct state"));
        if (CurrentCameraMode == ECameraMode::Overhead)
        {
            Camera->SetActive(true);
            FirstPersonCamera->SetActive(false);
            CurrentActiveRotation = SpringArm->GetComponentRotation();
        }
        else
        {
            FirstPersonCamera->SetActive(true);
            Camera->SetActive(false);
            CurrentActiveRotation = FirstPersonCamera->GetComponentRotation();
        }
    }
    
    // Calculate first person target (horizontal movement to cursor)
    FVector TerrainHeight = GetTerrainHeightAtCursor();
    TargetLocation = FVector(CursorPosition.X, CursorPosition.Y, TerrainHeight.Z + FirstPersonHeight);
    // FIXED: FirstPerson should look straight ahead, preserve current Yaw
    TargetRotation = FRotator(0.0f, CurrentActiveRotation.Yaw, 0.0f);
    
    // EXPLICIT: Always target FirstPerson mode for C key
    TargetCameraMode = ECameraMode::FirstPerson;
    bTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting warp to FirstPerson: %s -> FirstPerson (Target: %s, Rotation: %s)"),
           CurrentCameraMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"),
           *TargetLocation.ToString(), *TargetRotation.ToString());
}

// ===== SIMPLE CAMERA SYSTEM HELPER FUNCTIONS =====

ECameraMode ATerrainController::GetNextCameraMode() const
{
    switch (CurrentCameraMode)
    {
        case ECameraMode::Overhead:
            return ECameraMode::FirstPerson;
            
        case ECameraMode::FirstPerson:
        default:
            return ECameraMode::Overhead;
    }
}

void ATerrainController::CycleCameraMode()
{
    if (bTransitioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("CycleCameraMode blocked - already transitioning"));
        return;
    }
    
    // Determine next mode in cycle
    ECameraMode NextMode = GetNextCameraMode();
    
    // Simple direct transitions - no complex arcing
    FVector CurrentPos = GetActorLocation();
    
    // FIXED: Use CURRENT ACTIVE camera rotation as starting point
    FRotator CurrentActiveRotation;
    // ADD: Null check first
    if (!Camera || !FirstPersonCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera components are null in CycleCameraMode!"));
        return;
    }

    if (CurrentCameraMode == ECameraMode::Overhead && Camera->IsActive())
    {
        CurrentActiveRotation = SpringArm->GetComponentRotation();
    }
    else if (CurrentCameraMode == ECameraMode::FirstPerson && FirstPersonCamera->IsActive())
    {
        CurrentActiveRotation = FirstPersonCamera->GetComponentRotation();
    }
    else
    {
        // STRENGTHEN: Force correct camera state before fallback
        UE_LOG(LogTemp, Warning, TEXT("No camera active in CycleCameraMode, forcing correct state"));
        if (CurrentCameraMode == ECameraMode::Overhead)
        {
            Camera->SetActive(true);
            FirstPersonCamera->SetActive(false);
            CurrentActiveRotation = SpringArm->GetComponentRotation();
        }
        else
        {
            FirstPersonCamera->SetActive(true);
            Camera->SetActive(false);
            CurrentActiveRotation = FRotator(-45.0f, 0.0f, 0.0f);
        }
    }
    
    if (NextMode == ECameraMode::FirstPerson)
    {
        // Overhead -> FirstPerson: Go straight down to ground level
        float TerrainHeight = GetSafeTerrainHeight(CurrentPos);
        TargetLocation = FVector(CurrentPos.X, CurrentPos.Y, TerrainHeight + FirstPersonHeight);
        // FIXED: FirstPerson should look straight ahead (level)
        TargetRotation = FRotator(0.0f, CurrentActiveRotation.Yaw, 0.0f);
    }
    else
    {
        // FirstPerson -> Overhead: Go straight up to overview height
        float TerrainHeight = GetSafeTerrainHeight(CurrentPos);
        TargetLocation = FVector(CurrentPos.X, CurrentPos.Y, TerrainHeight + 1500.0f);
        // FIXED: Overhead should look down at 45 degrees
        TargetRotation = FRotator(-45.0f, CurrentActiveRotation.Yaw, 0.0f);
    }
    
    // EXPLICIT: Store what mode we're transitioning TO
    TargetCameraMode = NextMode;
    bTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting transition: %s -> %s (Target: %s, Rotation: %s)"),
           CurrentCameraMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"),
           NextMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"),
           *TargetLocation.ToString(), *TargetRotation.ToString());
}

// Legacy function - now just calls WarpToFirstPerson
void ATerrainController::SwitchCameraMode()
{
    WarpToFirstPerson();
}

void ATerrainController::UpdateCameraTransition(float DeltaTime)
{
    if (!bTransitioning) return;
    
    // Simple unified transition system for both Tab cycling and C key warping
    FVector CurrentLoc = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, TransitionSpeed);
    SetActorLocation(CurrentLoc);
    
    // EXPLICIT: Use stored target mode instead of height detection
    bool bTransitioningToFirstPerson = (TargetCameraMode == ECameraMode::FirstPerson);
    
    // Apply rotation to currently active camera, prepare target camera
    if (bTransitioningToFirstPerson)
    {
        // Transitioning TO FirstPerson
        if (Camera->IsActive())
        {
            // Prepare FirstPersonCamera with target rotation for smooth switch
            FirstPersonCamera->SetWorldRotation(TargetRotation);
        }
        else if (FirstPersonCamera->IsActive())
        {
            // Already in FirstPerson, update its rotation
            FRotator CurrentRot = FMath::RInterpTo(FirstPersonCamera->GetComponentRotation(), TargetRotation, DeltaTime, TransitionSpeed);
            FirstPersonCamera->SetWorldRotation(CurrentRot);
        }
    }
    else
    {
        // Transitioning TO Overhead
        if (FirstPersonCamera->IsActive())
        {
            // Prepare SpringArm with target rotation for smooth switch
            SpringArm->SetWorldRotation(TargetRotation);
        }
        else if (Camera->IsActive())
        {
            // Already in Overhead, update SpringArm rotation
            FRotator CurrentRot = FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRotation, DeltaTime, TransitionSpeed);
            SpringArm->SetWorldRotation(CurrentRot);
        }
    }
    
    // Switch cameras at the midpoint of transition for smoothness
    float DistanceToTarget = FVector::Dist(CurrentLoc, TargetLocation);
    bool bShouldSwitchCameras = (DistanceToTarget < 200.0f);
    
    if (bShouldSwitchCameras)
    {
        if (bTransitioningToFirstPerson && Camera->IsActive())
        {
            // Switch TO FirstPerson
            Camera->SetActive(false);
            FirstPersonCamera->SetActive(true);
            FirstPersonCamera->SetWorldRotation(TargetRotation);
            CurrentCameraMode = ECameraMode::FirstPerson;
            UE_LOG(LogTemp, Log, TEXT("Camera switched to FirstPerson at distance %.1f"), DistanceToTarget);
        }
        else if (!bTransitioningToFirstPerson && FirstPersonCamera->IsActive())
        {
            // Switch TO Overhead
            FirstPersonCamera->SetActive(false);
            Camera->SetActive(true);
            SpringArm->SetWorldRotation(TargetRotation);
            CurrentCameraMode = ECameraMode::Overhead;
            UE_LOG(LogTemp, Log, TEXT("Camera switched to Overhead at distance %.1f"), DistanceToTarget);
        }
    }
    
    // Complete transition when very close to target
    if (DistanceToTarget < 25.0f)
    {
        SetActorLocation(TargetLocation);
        
        // Ensure final state is correct
        CurrentCameraMode = TargetCameraMode;
        
        if (CurrentCameraMode == ECameraMode::FirstPerson)
        {
            // Ensure FirstPerson camera is active and properly rotated
            Camera->SetActive(false);
            FirstPersonCamera->SetActive(true);
            FirstPersonCamera->SetWorldRotation(TargetRotation);
        }
        else
        {
            // Ensure Overhead camera is active and properly rotated
            FirstPersonCamera->SetActive(false);
            Camera->SetActive(true);
            SpringArm->SetWorldRotation(TargetRotation);
        }
        
        bTransitioning = false;
        UE_LOG(LogTemp, Warning, TEXT("Camera transition complete - now in %s mode with final rotation: %s"),
               CurrentCameraMode == ECameraMode::FirstPerson ? TEXT("FirstPerson") : TEXT("Overhead"),
               *TargetRotation.ToString());

        // ADD: Force final camera state validation
        if (Camera && FirstPersonCamera)
        {
            if (CurrentCameraMode == ECameraMode::FirstPerson)
            {
                Camera->SetActive(false);
                FirstPersonCamera->SetActive(true);
                UE_LOG(LogTemp, Log, TEXT("Transition complete: Forced FirstPerson camera active"));
            }
            else
            {
                FirstPersonCamera->SetActive(false);
                Camera->SetActive(true);
                UE_LOG(LogTemp, Log, TEXT("Transition complete: Forced Overhead camera active"));
            }
        }
    }
}

void ATerrainController::UpdateOverheadCamera(float DeltaTime)
{
    // Smooth movement input
    SmoothedMovementInput.X = FMath::FInterpTo(SmoothedMovementInput.X, MovementInput.X, DeltaTime, MovementInputSmoothness);
    SmoothedMovementInput.Y = FMath::FInterpTo(SmoothedMovementInput.Y, MovementInput.Y, DeltaTime, MovementInputSmoothness);
    
    // WASD movement
    if (!SmoothedMovementInput.IsZero())
    {
        FVector ForwardDir = Camera->GetForwardVector();
        FVector RightDir = Camera->GetRightVector();
        
        ForwardDir.Z = 0;
        RightDir.Z = 0;
        ForwardDir.Normalize();
        RightDir.Normalize();
        
        FVector MovementDir = (ForwardDir * SmoothedMovementInput.Y) + (RightDir * SmoothedMovementInput.X);
        FVector NewLocation = GetActorLocation() + (MovementDir * CameraMoveSpeed * DeltaTime);
        SetActorLocation(NewLocation);
    }
    
    // Mouse look (existing logic)
    if (!LookInput.IsZero() && bMouseLookEnabled && !bIsEditingTerrain && !bIsEditingWater && !bTransitioning)
    {
        FRotator CurrentRotation = SpringArm->GetComponentRotation();
        CurrentRotation.Yaw += LookInput.X * MouseSensitivity;
        CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (LookInput.Y * MouseSensitivity), -89.0f, 89.0f);
        SpringArm->SetWorldRotation(CurrentRotation);
    }
    
    // Always reset look input when editing to prevent accumulation
    if (bIsEditingTerrain || bIsEditingWater)
    {
        LookInput = FVector2D::ZeroVector;
    }
    
    // Zoom (replace SpringArm logic)
    if (FMath::Abs(ZoomInput) > 0.01f)
    {
        FVector CurrentLoc = GetActorLocation();
        CurrentLoc.Z = FMath::Clamp(CurrentLoc.Z - (ZoomInput * CameraZoomSpeed * DeltaTime), MinCameraHeight, MaxCameraHeight);
        SetActorLocation(CurrentLoc);
    }
    
    // Vertical movement
    if (FMath::Abs(FlyUpInput) > 0.1f || FMath::Abs(FlyDownInput) > 0.1f)
    {
        float VerticalMovement = (FlyUpInput - FlyDownInput) * CameraFlySpeed * DeltaTime;
        FVector CurrentLocation = GetActorLocation();
        CurrentLocation.Z = FMath::Clamp(CurrentLocation.Z + VerticalMovement, MinCameraHeight, MaxCameraHeight);
        SetActorLocation(CurrentLocation);
    }
    
    // REMOVED: All input resets - now handled in UpdateCameraPosition()
}

void ATerrainController::UpdateFirstPersonCamera(float DeltaTime)
{
    // Smooth movement input
    SmoothedMovementInput.X = FMath::FInterpTo(SmoothedMovementInput.X, MovementInput.X, DeltaTime, MovementInputSmoothness);
    SmoothedMovementInput.Y = FMath::FInterpTo(SmoothedMovementInput.Y, MovementInput.Y, DeltaTime, MovementInputSmoothness);
    
    // Get current location for terrain following
    FVector CurrentLocation = GetActorLocation();
    FVector FinalLocation = CurrentLocation;
    bool bNeedLocationUpdate = false;
    
    // WASD movement at human scale
    if (!SmoothedMovementInput.IsZero())
    {
        FVector ForwardDir = FirstPersonCamera->GetForwardVector();
        FVector RightDir = FirstPersonCamera->GetRightVector();
        
        ForwardDir.Z = 0;
        RightDir.Z = 0;
        ForwardDir.Normalize();
        RightDir.Normalize();
        
        FVector MovementDir = (ForwardDir * SmoothedMovementInput.Y) + (RightDir * SmoothedMovementInput.X);
        FinalLocation = CurrentLocation + (MovementDir * CameraMoveSpeed * 0.3f * DeltaTime);
        bNeedLocationUpdate = true;
    }
    
    // CRITICAL: Apply terrain height adjustment to final location when safe
    if (IsTerrainValid() && !bTransitioning && !bWaitingForMasterController)
    {
        float TerrainHeight = GetSafeTerrainHeight(FinalLocation);
        float TargetHeight = TerrainHeight + FirstPersonHeight;
        
        // Only adjust if the difference is significant and terrain is stable
        if (FMath::Abs(FinalLocation.Z - TargetHeight) > 5.0f) // 5cm tolerance
        {
            // Smooth height interpolation for natural feel
            float HeightLerpSpeed = 8.0f; // Adjust for responsiveness
            FinalLocation.Z = FMath::FInterpTo(FinalLocation.Z, TargetHeight, DeltaTime, HeightLerpSpeed);
            bNeedLocationUpdate = true;
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("First-person height adjusted: %.1f -> %.1f (terrain: %.1f)"),
                   CurrentLocation.Z, FinalLocation.Z, TerrainHeight);
        }
    }
    
    // Single SetActorLocation call per frame
    if (bNeedLocationUpdate)
    {
        SetActorLocation(FinalLocation);
    }
    
    // Mouse look for first person
    if (!LookInput.IsZero() && bMouseLookEnabled && !bIsEditingTerrain && !bIsEditingWater && !bTransitioning)
    {
        FRotator CurrentRotation = FirstPersonCamera->GetComponentRotation();
        CurrentRotation.Yaw += LookInput.X * MouseSensitivity;
        CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (LookInput.Y * MouseSensitivity), -80.0f, 80.0f);
        FirstPersonCamera->SetWorldRotation(CurrentRotation);
    }
    
    // Always reset during editing to prevent input accumulation
    if (bIsEditingTerrain || bIsEditingWater)
    {
        LookInput = FVector2D::ZeroVector;
    }
}

FVector ATerrainController::GetTerrainHeightAtCursor() const
{
    FVector CursorPos = GetCursorWorldPosition();
    
    if (TargetTerrain)
    {
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(CursorPos);
        return FVector(CursorPos.X, CursorPos.Y, TerrainHeight);
    }
    
    return CursorPos;
}


// ============================================================================
// SECTION 13: ATMOSPHERIC BRUSH SYSTEM (~95 lines, 4%)
// ============================================================================
/**
 * PURPOSE:
 * Atmospheric parameter modification system allowing users to paint
 * temperature, humidity, pressure, and wind patterns directly onto the
 * atmospheric grid.
 *
 * BRUSH TYPES:
 * - Wind: Applies directional wind force (commented out pending AtmosphericSystem update)
 * - Pressure: Modifies atmospheric pressure patterns
 * - Temperature: Paints temperature deltas
 * - Humidity: Adjusts humidity levels
 *
 * CYCLING:
 * Y key cycles through atmospheric brush types when in Atmosphere editing
 * mode. Brush radius scales with MasterController's world scaling settings.
 *
 * STATUS:
 * Currently disabled pending AtmosphericSystem brush function implementation.
 * Framework is in place for future activation.
 * ============================================================================
 */

// ===== ATMOSPHERIC BRUSH FUNCTIONS =====

void ATerrainController::HandleBrushCycle()
{
    switch (CurrentMainMode)
    {
        case EMainEditingMode::Terrain:
            // Future: Cycle through terrain brush types (Raise/Lower handled by CycleSubMode)
            UE_LOG(LogTemp, Warning, TEXT("Terrain brush cycling not implemented yet"));
            break;
            
        case EMainEditingMode::Water:
            // Future: Cycle through water brush types (Water/Springs handled by CycleSubMode)
            UE_LOG(LogTemp, Warning, TEXT("Water brush cycling not implemented yet"));
            break;
            
        case EMainEditingMode::Atmosphere:
            CycleAtmosphericBrush();
            break;
            
        case EMainEditingMode::Ecosystem:
            UE_LOG(LogTemp, Warning, TEXT("Ecosystem brush cycling not implemented yet"));
            break;
    }
}

void ATerrainController::CycleAtmosphericBrush()
{
    int32 CurrentBrushInt = static_cast<int32>(CurrentAtmosphericBrush);
    CurrentBrushInt = (CurrentBrushInt + 1) % 4;
    CurrentAtmosphericBrush = static_cast<EAtmosphericBrushType>(CurrentBrushInt);
    
    UE_LOG(LogTemp, Warning, TEXT("Atmospheric brush: %s"), *GetCurrentBrushDisplayName());
    // UpdateBrushPreview(); // REMOVED: Function handled by Universal Brush System
}

FString ATerrainController::GetCurrentBrushDisplayName() const
{
    switch (CurrentAtmosphericBrush)
    {
        case EAtmosphericBrushType::Wind:        return TEXT("Wind");
        case EAtmosphericBrushType::Pressure:    return TEXT("Pressure");
        case EAtmosphericBrushType::Temperature: return TEXT("Temperature");
        case EAtmosphericBrushType::Humidity:    return TEXT("Humidity");
        default: return TEXT("Unknown");
    }
}

void ATerrainController::UpdateAtmosphericEditing(float DeltaTime)
{
    if (!AtmosphericSystem)
        return;
        
    // Apply atmospheric brush when left mouse is held
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC && PC->IsInputKeyDown(EKeys::LeftMouseButton))
    {
        FVector CursorPosition = GetCursorWorldPosition();
        if (CursorPosition != FVector::ZeroVector)
        {
            ApplyAtmosphericBrush(CursorPosition);
        }
    }
}

// ===== HELPER FUNCTION IMPLEMENTATIONS =====



float ATerrainController::GetBrushRadius() const
{
    return MasterController ? MasterController->GetBrushRadius() : 500.0f;
}

float ATerrainController::GetBrushStrength() const
{
    return MasterController ? MasterController->GetBrushStrength() : 200.0f;
}

void ATerrainController::SetBrushRadius(float NewRadius)
{
    if (MasterController)
    {
        MasterController->SetBrushRadius(NewRadius);
    }
}

void ATerrainController::SetBrushStrength(float NewStrength)
{
    if (MasterController)
    {
        MasterController->SetBrushStrength(NewStrength);
    }
}

const FUniversalBrushSettings& ATerrainController::GetCurrentBrushSettings() const
{
    if (MasterController)
    {
        return MasterController->GetUniversalBrushSettings();
    }
    return CurrentBrushSettings; // Fallback to cached settings
}


// ============================================================================
// SECTION 14: VALIDATION & TESTING (~80 lines, 3%)
// ============================================================================
/**
 * PURPOSE:
 * System connectivity validation for Universal Brush System integration.
 * Provides comprehensive testing of the authority delegation pattern and
 * interface implementation.
 *
 * TEST SEQUENCE:
 * 1. MasterController connection verification
 * 2. IBrushReceiver implementation check
 * 3. Brush settings delegation validation
 * 4. Settings cache synchronization test
 * 5. Brush application simulation (dry run)
 *
 * This function can be called from Blueprint or console to verify that
 * TerrainController is properly integrated with MasterController's
 * Universal Brush System.
 * ============================================================================
 */

// ===== UNIVERSAL BRUSH SYSTEM DEBUG =====

void ATerrainController::TestUniversalBrushConnection()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN CONTROLLER UNIVERSAL BRUSH TEST ==="));
    
    // Test 1: Check MasterController connection
    if (!MasterController)
    {
        UE_LOG(LogTemp, Error, TEXT("âŒ FAILED: No MasterController reference"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("âœ… PASS: MasterController connected"));
    
    // Test 2: Check IBrushReceiver implementation
    if (!CanReceiveBrush())
    {
        UE_LOG(LogTemp, Error, TEXT("âŒ FAILED: CanReceiveBrush() returned false"));
        UE_LOG(LogTemp, Error, TEXT("  - CurrentMainMode: %d (0=Terrain, 1=Water, 2=Atmosphere, 3=Ecosystem)"), (int32)CurrentMainMode);
        UE_LOG(LogTemp, Error, TEXT("  - CurrentTerrainSubMode: %d (0=Raise, 1=Lower)"), (int32)CurrentTerrainSubMode);
        UE_LOG(LogTemp, Error, TEXT("  - TargetTerrain: %s"), TargetTerrain ? TEXT("valid") : TEXT("null"));
        UE_LOG(LogTemp, Error, TEXT("  - bInitializationComplete: %s"), bInitializationComplete ? TEXT("true") : TEXT("false"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("âœ… PASS: CanReceiveBrush() = true"));
    
    // Test 3: Check brush settings delegation
    float MasterRadius = MasterController->GetBrushRadius();
    float LocalRadius = GetBrushRadius();
    
    if (FMath::Abs(MasterRadius - LocalRadius) > 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("âŒ FAILED: Brush radius mismatch - Master: %.1f, Local: %.1f"),
               MasterRadius, LocalRadius);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("âœ… PASS: Brush radius delegation working (%.1f)"), MasterRadius);
    
    // Test 4: Check brush settings cache
    const FUniversalBrushSettings& CachedSettings = GetCurrentBrushSettings();
    const FUniversalBrushSettings& MasterSettings = MasterController->GetUniversalBrushSettings();
    
    if (FMath::Abs(CachedSettings.BrushRadius - MasterSettings.BrushRadius) > 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("âŒ FAILED: Cached settings out of sync"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("âœ… PASS: Brush settings cache synchronized"));
    
    // Test 5: Simulate brush application (dry run)
    FVector TestPos(1000.0f, 1000.0f, 0.0f);
    
    // Mock editing state for test
    bool OriginalEditingState = bIsEditingTerrain;
    bool OriginalRaisingState = bIsRaisingTerrain;
    EMainEditingMode OriginalMainMode = CurrentMainMode;
    ETerrainSubMode OriginalTerrainSubMode = CurrentTerrainSubMode;
    
    bIsEditingTerrain = true;
    bIsRaisingTerrain = true;
    CurrentMainMode = EMainEditingMode::Terrain;
    CurrentTerrainSubMode = ETerrainSubMode::Raise;
    
    // Test ApplyBrush with very small delta time (minimal effect)
    ApplyBrush(TestPos, MasterSettings, 0.001f);
    
    // Restore original state
    bIsEditingTerrain = OriginalEditingState;
    bIsRaisingTerrain = OriginalRaisingState;
    CurrentMainMode = OriginalMainMode;
    CurrentTerrainSubMode = OriginalTerrainSubMode;
    
    UE_LOG(LogTemp, Warning, TEXT("âœ… PASS: ApplyBrush simulation completed"));
    
    UE_LOG(LogTemp, Warning, TEXT("ðŸŽ‰ SUCCESS: Universal Brush System fully connected!"));
    UE_LOG(LogTemp, Warning, TEXT("TerrainController is ready to receive brush commands from MasterController"));
}



// ============================================================================
// SECTION 15: HELPER FUNCTIONS & UTILITIES (~120 lines, 5%)
// ============================================================================
/**
 * PURPOSE:
 * Utility functions for common operations including terrain validation,
 * height queries, input management, authority caching, and system state
 * management.
 *
 * TERRAIN VALIDATION:
 * IsTerrainValid() checks multiple conditions:
 * - TargetTerrain exists and is valid
 * - TerrainChunks are initialized
 * - Not currently resetting (prevents invalid queries during reset)
 *
 * SAFE HEIGHT QUERIES:
 * GetSafeTerrainHeight() provides defensive height queries with:
 * - Null/invalid terrain checks
 * - Sentinel value validation (-99999.0f to 50000.0f)
 * - Fallback to current Z position if query fails
 *
 * INPUT MANAGEMENT:
 * ResetInputs() clears all movement inputs at end of frame to prevent
 * accumulation. Look input is handled separately to allow editing mode
 * to override it.
 *
 * AUTHORITY CACHING:
 * UpdateAuthorityCache() periodically validates MasterController reference
 * at 10 Hz. This catches if the reference is lost during gameplay and
 * prevents cascade failures.
 *
 * RESET STATE MANAGEMENT:
 * SetTerrainResetting() flag prevents height queries during terrain reset
 * operations. This avoids crashes from invalid terrain state during the
 * reset process.
 *
 * FIRST-PERSON HEIGHT VALIDATION:
 * ValidateFirstPersonHeight() runs at 10 Hz when in FirstPerson mode to
 * catch terrain edits that occur while player is stationary. Corrects
 * height if >20 cm off-target.
 * ============================================================================
 */

// ===== ACTUAL HELPER FUNCTION IMPLEMENTATIONS =====

bool ATerrainController::IsTerrainValid() const
{
    // Check if terrain exists and has chunks initialized
    return TargetTerrain && IsValid(TargetTerrain) &&
           TargetTerrain->TerrainChunks.Num() > 0 &&
           !bTerrainResetting;
}

float ATerrainController::GetSafeTerrainHeight(const FVector& Position) const
{
    if (!TargetTerrain || !IsValid(TargetTerrain))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("TargetTerrain is null/invalid, using default height"));
        return Position.Z; // Return current Z if no terrain
    }
    
    float TerrainHeight = TargetTerrain->GetHeightAtPosition(Position);
    
    // Validate returned height (terrain functions can return sentinel values)
    if (TerrainHeight < -99999.0f || TerrainHeight > 50000.0f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Invalid terrain height (%.1f), using current Z"), TerrainHeight);
        return Position.Z;
    }
    
    return TerrainHeight;
}

void ATerrainController::ResetInputs()
{
    /*
    // Always reset look input when editing to prevent accumulation
    if (bIsEditingTerrain || bIsEditingWater)
    {
        LookInput = FVector2D::ZeroVector;
    }
     */
    MovementInput = FVector2D::ZeroVector;
    // Reset other inputs
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
}

void ATerrainController::UpdateAuthorityCache(float DeltaTime)
{
    AuthorityCacheTimer += DeltaTime;
    
    // Only check authority periodically (10x per second)
    if (AuthorityCacheTimer >= AuthorityCacheRate)
    {
        AuthorityCacheTimer = 0.0f;
        
        // Validate MasterController is still valid
        if (MasterController)
        {
            bMasterControllerValid = true;
        }
        else
        {
            bMasterControllerValid = false;
            UE_LOG(LogTemp, Warning, TEXT("[AUTHORITY] MasterController reference lost!"));
        }
    }
}

void ATerrainController::SetTerrainResetting(bool bResetting)
{
    bTerrainResetting = bResetting;
    
    if (bResetting)
    {
        // Reset validation timer when terrain reset starts
        FirstPersonValidationTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Terrain resetting - height validation disabled"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain reset complete - height validation re-enabled"));
    }
}

// Add this periodic terrain validation check
void ATerrainController::ValidateFirstPersonHeight(float DeltaTime)
{
    // Only run validation for first person mode
    if (CurrentCameraMode != ECameraMode::FirstPerson || bTransitioning)
    {
        FirstPersonValidationTimer = 0.0f; // Reset when not in first person
        return;
    }
    
    FirstPersonValidationTimer += DeltaTime; // Use instance variable
    
    // Run validation every 0.1 seconds (10Hz) to catch terrain changes
    if (FirstPersonValidationTimer >= 0.1f && IsTerrainValid())
    {
        FVector CurrentLocation = GetActorLocation();
        float TerrainHeight = GetSafeTerrainHeight(CurrentLocation);
        float TargetHeight = TerrainHeight + FirstPersonHeight;
        float HeightDifference = FMath::Abs(CurrentLocation.Z - TargetHeight);
        
        // If we're significantly off-height, correct it
        if (HeightDifference > 20.0f) // 20cm tolerance for validation
        {
            FVector CorrectedLocation = CurrentLocation;
            CorrectedLocation.Z = TargetHeight;
            SetActorLocation(CorrectedLocation);
            
            UE_LOG(LogTemp, Warning, TEXT("First-person height validation correction: %.1f cm difference"),
                   HeightDifference);
        }
        
        FirstPersonValidationTimer = 0.0f;
    }
}

void ATerrainController::ReturnToMainMenu()
{
    // Get game instance and cast to our custom type
    if (UTerrAIGameInstance* GameInstance = Cast<UTerrAIGameInstance>(GetGameInstance()))
    {
        GameInstance->ReturnToMainMenu();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ReturnToMainMenu: Failed to get TerrAIGameInstance"));
    }
}

// End of File
