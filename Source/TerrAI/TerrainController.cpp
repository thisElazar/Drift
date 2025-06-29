 // TerrainController.cpp - Complete Clean UE 5.4 Compatible Version with Universal Brush System
#include "TerrainController.h"
#include "TerrAI.h"  // Include for validation macros and constants
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMaterialLibrary.h"  // For material parameter updates
#include "NiagaraComponent.h"  // For Niagara effects
#include "NiagaraFunctionLibrary.h"  // For Niagara spawning
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"  // Added missing include
#include "AtmosphericSystem.h"  // Added missing include
#include "WaterSystem.h"  // CRITICAL FIX: Add WaterSystem include
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "DynamicTerrain.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TerrAIGameInstance.h"
#include "WaterController.h"
#include "MasterController.h"  // CRITICAL: Include for Universal Brush System

ATerrainController::ATerrainController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create a proper root component first
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    
    // Create spring arm component and attach to root
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);  // Attach to root, not BE the root
    SpringArm->TargetArmLength = 3000.0f;
    SpringArm->SetUsingAbsoluteRotation(true);
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;
    
    // *** OPTIMIZED COLLISION SETTINGS FOR STABILITY ***
    SpringArm->bDoCollisionTest = false;             // Disable collision for large terrain
    SpringArm->bUseCameraLagSubstepping = false;     // Disable lag when no collision
    SpringArm->CameraLagSpeed = 0.0f;                // No lag compensation needed
    SpringArm->CameraLagMaxDistance = 0.0f;          // No lag distance
    SpringArm->ProbeSize = 0.0f;                     // No collision probe
    SpringArm->ProbeChannel = ECC_Camera;            // Keep channel for future use
    
    // Optional: More aggressive collision settings
    //SpringArm->bUseSubstepping = true;               // Smooth collision movement
    //SpringArm->TargetOffset = FVector(0, 0, 100);    // Offset camera slightly above target
    
    // Create camera component with depth optimization for large terrain
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    
    // CRITICAL: Optimize camera depth precision for 51.3km terrain
    Camera->SetConstraintAspectRatio(false);
    Camera->SetOrthoNearClipPlane(1.0f);      // Closer near plane for better precision
    Camera->SetOrthoFarClipPlane(100000.0f);  // 100km far plane
    
    // Create first person camera with same depth optimization
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(SceneRoot);
    FirstPersonCamera->SetActive(false);
    
    // Apply same depth precision settings
    FirstPersonCamera->SetConstraintAspectRatio(false);
    FirstPersonCamera->SetOrthoNearClipPlane(1.0f);
    FirstPersonCamera->SetOrthoFarClipPlane(100000.0f);
    
    // Create brush preview component - attach to root, not spring arm
    BrushPreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrushPreview"));
    BrushPreview->SetupAttachment(RootComponent);  // Attach to root scene component
    BrushPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BrushPreview->SetCastShadow(false);
    BrushPreview->SetVisibility(true);
    
    // Initialize camera system
    CurrentCameraMode = ECameraMode::Overhead;
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
    
    // Terrain editing settings - get from MasterController for authority
    // BrushRadius and BrushStrength now come from MasterController
    MinBrushRadius = 100.0f;
    MaxBrushRadius = 2000.0f;
    MinBrushStrength = 10.0f;
    MaxBrushStrength = 1000.0f;
    BrushSizeChangeRate = 50.0f;
    BrushStrengthChangeRate = 20.0f;
    
    // Water editing settings
    WaterBrushStrength = 10.0f;
    MinWaterBrushStrength = 1.0f;
    MaxWaterBrushStrength = 100.0f;
    WaterStrengthChangeRate = 5.0f;
    
    // Visual mode system
    CurrentVisualMode = ETerrainVisualMode::Wireframe;
    CurrentEditingMode = EEditingMode::Terrain;
    
    // Initialize states
    bIsEditingTerrain = false;
    bIsRaisingTerrain = false;
    bIsLoweringTerrain = false;
    bIsEditingWater = false;
    bIsAddingWater = false;
    bIsRemovingWater = false;
    
    // Performance settings
    bShowPerformanceStats = true;
    bShowTerrainInfo = true;
    
    // Brush preview settings
    bShowBrushPreview = true;
    BrushPreviewOpacity = 0.3f;
    
    // Initialize vectors properly
    MovementInput = FVector2D(0.0f, 0.0f);
    LookInput = FVector2D(0.0f, 0.0f);
    SmoothedMovementInput = FVector2D(0.0f, 0.0f);
    LastCursorPosition = FVector(0.0f, 0.0f, 0.0f);
    CameraRotation = FRotator(-45.0f, 0.0f, 0.0f);
    
    // Initialize input values
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
    MovementInputSmoothness = 10.0f;
    bMouseLookEnabled = true;
    
    // Performance tracking
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    StatUpdateTimer = 0.0f;
    
    // AUTHORITY: Defer all initialization until MasterController is ready
    bWaitingForMasterController = true;
}

// ===== UNIVERSAL BRUSH SYSTEM IMPLEMENTATION =====

void ATerrainController::ApplyBrush(FVector WorldPosition, const FUniversalBrushSettings& Settings, float DeltaTime)
{
    if (!TargetTerrain || !CanReceiveBrush())
    {
        return;
    }
    
    // Apply terrain modification using universal brush settings
    if (bIsEditingTerrain)
    {
        // Use the settings provided by the MasterController (already scaled)
        float ModificationAmount = Settings.BrushStrength * DeltaTime;
        
        // Apply modification based on current terrain editing mode
        if (bIsRaisingTerrain || bIsLoweringTerrain)
        {
            TargetTerrain->ModifyTerrain(WorldPosition, Settings.BrushRadius, 
                ModificationAmount, bIsRaisingTerrain);
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
    return (CurrentEditingMode == EEditingMode::Terrain) && 
           (TargetTerrain != nullptr) && 
           bInitializationComplete;
}

// ===== BRUSH AUTHORITY DELEGATION =====

float ATerrainController::GetBrushRadius() const
{
    if (MasterController)
    {
        return MasterController->GetBrushRadius();
    }
    
    // Fallback: use current brush settings
    return CurrentBrushSettings.BrushRadius;
}

float ATerrainController::GetBrushStrength() const
{
    if (MasterController)
    {
        return MasterController->GetBrushStrength();
    }
    
    // Fallback: use current brush settings
    return CurrentBrushSettings.BrushStrength;
}

void ATerrainController::SetBrushRadius(float NewRadius)
{
    if (MasterController)
    {
        MasterController->SetBrushRadius(NewRadius);
        UE_LOG(LogTemp, Log, TEXT("[TERRAIN CONTROLLER] Delegated brush radius change to MasterController: %.1f"), NewRadius);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[TERRAIN CONTROLLER] No MasterController - cannot set brush radius"));
    }
}

void ATerrainController::SetBrushStrength(float NewStrength)
{
    if (MasterController)
    {
        MasterController->SetBrushStrength(NewStrength);
        UE_LOG(LogTemp, Log, TEXT("[TERRAIN CONTROLLER] Delegated brush strength change to MasterController: %.1f"), NewStrength);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[TERRAIN CONTROLLER] No MasterController - cannot set brush strength"));
    }
}

const FUniversalBrushSettings& ATerrainController::GetCurrentBrushSettings() const
{
    if (MasterController)
    {
        return MasterController->GetUniversalBrushSettings();
    }
    
    // Fallback: return local copy
    return CurrentBrushSettings;
}

void ATerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    // AUTHORITY: Wait for MasterController to initialize us
    // Do NOT auto-find or create anything - wait for proper authority
    if (!MasterController)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainController: Waiting for MasterController authority..."));
        // MasterController will call InitializeWithAuthority() when ready
        return;
    }
    
    // If MasterController is already set, initialize immediately
    InitializeWithAuthority();
}

void ATerrainController::InitializeWithAuthority()
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
    SpringArm->SetWorldRotation(CameraRotation);
    
    // Get terrain from MasterController authority
    TargetTerrain = MasterController->MainTerrain;
    if (!TargetTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: MasterController has no MainTerrain"));
        return;
    }
    
    // Get WaterController from MasterController
    WaterController = MasterController->WaterController;
    
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
    float TempRatio = FMath::Clamp((Temperature - 263.15f) / 30.0f, 0.0f, 1.0f); // -10°C to 20°C range
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
    float TempDiff = FMath::Abs(Temperature - 273.15f); // Distance from 0°C
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
    
    // Update atmospheric editing if active
    if (CurrentEditingMode == EEditingMode::Atmosphere)
    {
        UpdateAtmosphericEditing(DeltaTime);
    }
    
    // Update atmospheric fog
    if (bEnableAtmosphericFog && AtmosphericSystem)
    {
        UpdateAtmosphericFog();
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
                
                // Set visibility based on editing mode
                bool bShouldShow = bShowBrushPreview && 
                                  (bIsEditingTerrain || bIsEditingWater) &&
                                  BrushSettings.bShowPreview;
                BrushPreview->SetVisibility(bShouldShow);
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
            EnhancedInputComponent->BindAction(ToggleVisualizationAction, ETriggerEvent::Started, this, &ATerrainController::HandleWaterToggle);
        }
        if (ToggleRainAction)
        {
            EnhancedInputComponent->BindAction(ToggleRainAction, ETriggerEvent::Started, this, &ATerrainController::HandleRainToggle);
        }
        
        // Shift key tracking for delegation
        if (LeftShiftAction)
        {
            EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Started, this, &ATerrainController::OnLeftShiftPressed);
            EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Completed, this, &ATerrainController::OnLeftShiftReleased);
        }
        if (RightShiftAction)
        {
            EnhancedInputComponent->BindAction(RightShiftAction, ETriggerEvent::Started, this, &ATerrainController::OnRightShiftPressed);
            EnhancedInputComponent->BindAction(RightShiftAction, ETriggerEvent::Completed, this, &ATerrainController::OnRightShiftReleased);
        }
        if (ToggleEditingModeAction)
        {
            EnhancedInputComponent->BindAction(ToggleEditingModeAction, ETriggerEvent::Started, this, &ATerrainController::ToggleEditingModeInput);
        }
        if (ResetTerrainAction)
        {
            EnhancedInputComponent->BindAction(ResetTerrainAction, ETriggerEvent::Started, this, &ATerrainController::ResetTerrain);
        }
        
        // Time control
        if (IncreaseTimeSpeedAction)
        {
            EnhancedInputComponent->BindAction(IncreaseTimeSpeedAction, ETriggerEvent::Started, this, &ATerrainController::IncreaseTimeSpeed);
        }
        if (DecreaseTimeSpeedAction)
        {
            EnhancedInputComponent->BindAction(DecreaseTimeSpeedAction, ETriggerEvent::Started, this, &ATerrainController::DecreaseTimeSpeed);
        }
        
        // Camera switching
        if (SwitchCameraAction)
        {
            EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Started, this, &ATerrainController::WarpToFirstPerson);
        }
        if (ReturnToOverheadAction)
        {
            EnhancedInputComponent->BindAction(ReturnToOverheadAction, ETriggerEvent::Started, this, &ATerrainController::ReturnToOverhead);
        }
        
        // Atmospheric brush cycling (Y key)
        if (CycleBrushModeAction)
        {
            EnhancedInputComponent->BindAction(CycleBrushModeAction, ETriggerEvent::Started, this, &ATerrainController::HandleBrushCycle);
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
    LookInput = LookVector;
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

// ===== TERRAIN EDITING FUNCTIONS =====

void ATerrainController::StartRaiseTerrain(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        StartTerrainEditing(true);
    }
}

void ATerrainController::StopRaiseTerrain(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        StopTerrainEditing();
    }
}

void ATerrainController::StartLowerTerrain(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        StartTerrainEditing(false);
    }
}

void ATerrainController::StopLowerTerrain(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        StopTerrainEditing();
    }
}

void ATerrainController::StartTerrainEditing(bool bRaise)
{
    bIsEditingTerrain = true;
    bIsRaisingTerrain = bRaise;
    bIsLoweringTerrain = !bRaise;
    
    // FIXED: Don't reset old cursor system - unified cursor handles this
    // bCursorInitialized = false;  // Removed - causes snap-back
    
    UE_LOG(LogTemp, Warning, TEXT("Started %s terrain"), bRaise ? TEXT("raising") : TEXT("lowering"));
}

void ATerrainController::StopTerrainEditing()
{
    bIsEditingTerrain = false;
    bIsRaisingTerrain = false;
    bIsLoweringTerrain = false;
    
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
    if (CurrentEditingMode == EEditingMode::Water)
    {
        StartWaterEditing(true);
    }
}

void ATerrainController::StopAddWater(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Water)
    {
        StopWaterEditing();
    }
}

void ATerrainController::StartRemoveWater(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Water)
    {
        StartWaterEditing(false);
    }
}

void ATerrainController::StopRemoveWater(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Water)
    {
        StopWaterEditing();
    }
}

void ATerrainController::StartWaterEditing(bool bAdd)
{
    bIsEditingWater = true;
    bIsAddingWater = bAdd;
    bIsRemovingWater = !bAdd;
    
    // FIXED: Don't reset old cursor system - unified cursor handles this
    // bCursorInitialized = false;  // Removed - causes snap-back
    
    UE_LOG(LogTemp, Warning, TEXT("Started %s water"), bAdd ? TEXT("adding") : TEXT("removing"));
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
    if (!TargetTerrain) return;
    
    // Use the same cursor position that works for terrain editing
    FVector CursorWorldPos = GetCursorWorldPosition();
    
    if (bIsAddingWater)
    {
        // CRITICAL: Use direct WaterSystem call, not through WaterController transform
        if (TargetTerrain->WaterSystem)
        {
            TargetTerrain->WaterSystem->AddWater(CursorWorldPos, WaterBrushStrength * WaterAdditionRate * DeltaTime);
        }
    }
    else if (bIsRemovingWater)
    {
        if (TargetTerrain->WaterSystem)
        {
            TargetTerrain->WaterSystem->RemoveWater(CursorWorldPos, WaterBrushStrength * DeltaTime);
        }
    }
}

// ===== BRUSH CONTROL FUNCTIONS =====

void ATerrainController::IncreaseBrushSize(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        float CurrentRadius = GetBrushRadius();
        SetBrushRadius(CurrentRadius + BrushSizeChangeRate);
    }
}

void ATerrainController::DecreaseBrushSize(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        float CurrentRadius = GetBrushRadius();
        SetBrushRadius(CurrentRadius - BrushSizeChangeRate);
    }
}

void ATerrainController::IncreaseBrushStrength(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        float CurrentStrength = GetBrushStrength();
        SetBrushStrength(CurrentStrength + BrushStrengthChangeRate);
    }
    else if (CurrentEditingMode == EEditingMode::Water)
    {
        SetWaterBrushStrength(WaterBrushStrength + WaterStrengthChangeRate);
    }
}

void ATerrainController::DecreaseBrushStrength(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        float CurrentStrength = GetBrushStrength();
        SetBrushStrength(CurrentStrength - BrushStrengthChangeRate);
    }
    else if (CurrentEditingMode == EEditingMode::Water)
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
}
void ATerrainController::SetWaterBrushStrength(float NewStrength)
{
    WaterBrushStrength = FMath::Clamp(NewStrength, MinWaterBrushStrength, MaxWaterBrushStrength);
    UE_LOG(LogTemp, Warning, TEXT("Water brush strength: %.1f"), WaterBrushStrength);
}

// ===== SYSTEM CONTROL FUNCTIONS =====

void ATerrainController::HandleWaterToggle()
{
    // Use Enhanced Input shift state instead of legacy input
    bool bShiftHeld = bLeftShiftHeld || bRightShiftHeld;
    
    if (bShiftHeld && WaterController)
    {
        // Shift+V: Toggle water materials via WaterController
        WaterController->ToggleWaterVisualMode();
        UE_LOG(LogTemp, Log, TEXT("TerrainController: Delegated water visual toggle to WaterController"));
    }
    else
    {
        // V only: Toggle terrain materials (existing functionality)
        ToggleVisualMode();
    }
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



void ATerrainController::ToggleEditingMode()
{
    int32 CurrentModeInt = static_cast<int32>(CurrentEditingMode);
    CurrentModeInt = (CurrentModeInt + 1) % 3; // Cycle through 3 modes
    CurrentEditingMode = static_cast<EEditingMode>(CurrentModeInt);
    
    switch (CurrentEditingMode)
    {
        case EEditingMode::Terrain:
            UE_LOG(LogTemp, Warning, TEXT("Switched to TERRAIN editing mode"));
            break;
        case EEditingMode::Water:
            UE_LOG(LogTemp, Warning, TEXT("Switched to WATER editing mode"));
            break;
        case EEditingMode::Atmosphere:
            UE_LOG(LogTemp, Warning, TEXT("Switched to ATMOSPHERE editing mode - Brush: %s"), 
                   *GetCurrentBrushDisplayName());
            break;
    }
    
    // Stop any current editing
    StopTerrainEditing();
    StopWaterEditing();
    // UpdateBrushPreview(); // REMOVED: Function handled by Universal Brush System
}

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

void ATerrainController::ToggleEditingModeInput(const FInputActionValue& Value)
{
    ToggleEditingMode();
}

void ATerrainController::ResetTerrain(const FInputActionValue& Value)
{
    if (TargetTerrain)
    {
        TargetTerrain->ResetTerrainFully();
        UE_LOG(LogTemp, Warning, TEXT("Full terrain system reset"));
    }
}

// ===== UTILITY FUNCTIONS =====

FVector ATerrainController::GetCursorWorldPosition() const
{
    if (MasterController && bUnifiedCursorValid)
    {
        // Use master controller coordinate transforms
        return GetMasterCursorWorldPosition();
    }
    
    // Fallback to legacy behavior
    return bUnifiedCursorValid ? UnifiedCursorPosition : FVector::ZeroVector;
}

FVector ATerrainController::GetMasterCursorWorldPosition() const
{
    if (!MasterController || !bUnifiedCursorValid)
    {
        return FVector::ZeroVector;
    }
    
    // Fallback: just return unified cursor position
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
    
    FVector MouseWorldLocation, MouseWorldDirection;
    if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
    {
        // Ensure trace starts high above terrain
        FVector Start = MouseWorldLocation;
        if (TargetTerrain)
        {
            FVector TerrainCenter = TargetTerrain->GetActorLocation();
            float MaxTerrainHeight = TerrainCenter.Z + 5000.0f;
            Start.Z = FMath::Max(Start.Z, MaxTerrainHeight);
        }
        
        FVector End = Start + (MouseWorldDirection * 50000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.bTraceComplex = false;
        CollisionParams.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, CollisionParams))
        {
            // FIXED: More permissive surface normal check
            if (HitResult.Normal.Z > 0.5f) // Was 0.7f - too restrictive
            {
                OutHitLocation = HitResult.Location;
                return true;
            }
        }
        
        // FIXED: Use actual terrain bounds instead of calculated size
        if (TargetTerrain)
        {
            FVector TerrainMin, TerrainMax;
            TargetTerrain->GetActorBounds(false, TerrainMin, TerrainMax);
            
            // Project ray onto terrain plane
            FVector TerrainCenter = TargetTerrain->GetActorLocation();
            FVector PlaneNormal = FVector::UpVector;
            FVector PlanePoint = TerrainCenter;
            
            float Denominator = FVector::DotProduct(MouseWorldDirection, PlaneNormal);
            if (FMath::Abs(Denominator) > 0.0001f)
            {
                float T = FVector::DotProduct(PlanePoint - MouseWorldLocation, PlaneNormal) / Denominator;
                FVector PlaneIntersection = MouseWorldLocation + (MouseWorldDirection * T);
                
                // FIXED: Clamp to actual terrain bounds
                PlaneIntersection.X = FMath::Clamp(PlaneIntersection.X, TerrainMin.X, TerrainMax.X);
                PlaneIntersection.Y = FMath::Clamp(PlaneIntersection.Y, TerrainMin.Y, TerrainMax.Y);
                
                // Get actual terrain height at this position
                if (TargetTerrain)
                {
                    float TerrainHeight = TargetTerrain->GetHeightAtPosition(PlaneIntersection);
                    OutHitLocation = FVector(PlaneIntersection.X, PlaneIntersection.Y, TerrainHeight);
                    return true;
                }
            }
        }
    }
    
    return false;
}

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
    CursorUpdateTimer += DeltaTime;
    
    // Throttle cursor updates to reduce CPU load and improve stability
    if (CursorUpdateTimer >= CursorUpdateRate)
    {
        FVector RawCursorPosition;
        if (PerformCursorTrace(RawCursorPosition))
        {
            if (!bUnifiedCursorValid)
            {
                // First valid position - no smoothing
                UnifiedCursorPosition = RawCursorPosition;
                bUnifiedCursorValid = true;
            }
            else
            {
                // Simple distance-based smoothing
                float Distance = FVector::Dist(RawCursorPosition, UnifiedCursorPosition);
                float SmoothingSpeed = Distance > 1000.0f ? 8.0f : 15.0f;
                
                UnifiedCursorPosition = FMath::VInterpTo(
                    UnifiedCursorPosition, 
                    RawCursorPosition, 
                    CursorUpdateTimer, 
                    SmoothingSpeed
                );
            }
            
            PreviousCursorPosition = RawCursorPosition;
        }
        // If trace fails, keep using last valid position
        
        CursorUpdateTimer = 0.0f;
    }
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

// Old CreateBrushPreview removed - replaced by Universal Brush System implementation

// Old UpdateBrushPreview removed - replaced by Universal Brush System implementation

// Old UpdateBrushPreviewMaterial removed - replaced by Universal Brush System implementation

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
        // Show editing mode and brush info
        FString EditingModeText;
        switch (CurrentEditingMode)
        {
            case EEditingMode::Terrain: EditingModeText = TEXT("Terrain"); break;
            case EEditingMode::Water: EditingModeText = TEXT("Water"); break;
            case EEditingMode::Atmosphere: EditingModeText = TEXT("Atmosphere"); break;
        }
        GEngine->AddOnScreenDebugMessage(30, 0.5f, FColor::White,
            FString::Printf(TEXT("Editing Mode: %s (T to cycle)"), *EditingModeText));
        
        if (CurrentEditingMode == EEditingMode::Terrain && MasterController)
        {
            float BrushRadius = GetBrushRadius();
            float BrushStrength = GetBrushStrength();
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Brush: %.0f radius, %.0f strength"), BrushRadius, BrushStrength));
        }
        else if (CurrentEditingMode == EEditingMode::Water)
        {
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Water Brush: %.1f strength"), WaterBrushStrength));
        }
        else if (CurrentEditingMode == EEditingMode::Atmosphere)
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
        
        // Camera mode info
        FString CameraModeText = (CurrentCameraMode == ECameraMode::Overhead) ? TEXT("Overhead") : TEXT("First Person");
        GEngine->AddOnScreenDebugMessage(33, 0.5f, FColor::Yellow,
            FString::Printf(TEXT("Camera: %s (C=Warp to cursor, TAB=Overhead)"), *CameraModeText));
        
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

// ===== ENHANCED INPUT SHIFT KEY FUNCTIONS =====

void ATerrainController::OnLeftShiftPressed(const FInputActionValue& Value)
{
    bLeftShiftHeld = true;
    UE_LOG(LogTemp, VeryVerbose, TEXT("Left Shift pressed"));
}

void ATerrainController::OnLeftShiftReleased(const FInputActionValue& Value)
{
    bLeftShiftHeld = false;
    UE_LOG(LogTemp, VeryVerbose, TEXT("Left Shift released"));
}

void ATerrainController::OnRightShiftPressed(const FInputActionValue& Value)
{
    bRightShiftHeld = true;
    UE_LOG(LogTemp, VeryVerbose, TEXT("Right Shift pressed"));
}

void ATerrainController::OnRightShiftReleased(const FInputActionValue& Value)
{
    bRightShiftHeld = false;
    UE_LOG(LogTemp, VeryVerbose, TEXT("Right Shift released"));
}

// ===== ATMOSPHERIC CONTROL FUNCTIONS - MOVED TO ATMOSPHERECONTROLLER =====
// All atmospheric functions moved to AtmosphereController for clean separation

// ===== CAMERA SYSTEM FUNCTIONS =====

void ATerrainController::WarpToFirstPerson()
{
    if (bTransitioning) return;
    
    // Store overhead state if we're coming from overhead
    if (CurrentCameraMode == ECameraMode::Overhead)
    {
        StoredOverheadLocation = GetActorLocation();
        StoredOverheadRotation = SpringArm->GetComponentRotation();
        StoredCameraHeight = GetActorLocation().Z;
    }
    
    // Get spawn position at cursor
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition == FVector::ZeroVector)
    {
        CursorPosition = GetActorLocation();
    }
    
    // Calculate first person target
    FVector TerrainHeight = GetTerrainHeightAtCursor();
    TargetLocation = FVector(CursorPosition.X, CursorPosition.Y, TerrainHeight.Z + FirstPersonHeight);
    TargetRotation = FRotator(0.0f, CurrentCameraMode == ECameraMode::Overhead ? StoredOverheadRotation.Yaw : FirstPersonCamera->GetComponentRotation().Yaw, 0.0f);
    
    CurrentCameraMode = ECameraMode::FirstPerson;
    bTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Warping to First Person at cursor location"));
}

void ATerrainController::ReturnToOverhead()
{
    if (bTransitioning) return;
    if (CurrentCameraMode == ECameraMode::Overhead) return; // Already overhead
    
    // Return to stored overhead position
    TargetLocation = StoredOverheadLocation;
    TargetRotation = StoredOverheadRotation;
    
    CurrentCameraMode = ECameraMode::Overhead;
    bTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Returning to Overhead view"));
}

// Legacy function - now just calls WarpToFirstPerson
void ATerrainController::SwitchCameraMode()
{
    WarpToFirstPerson();
}

void ATerrainController::UpdateCameraTransition(float DeltaTime)
{
    if (!bTransitioning) return;
    
    if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        // Transition to first person
        FVector CurrentLoc = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, TransitionSpeed);
        FRotator CurrentRot = FMath::RInterpTo(FirstPersonCamera->GetComponentRotation(), TargetRotation, DeltaTime, TransitionSpeed);
        
        SetActorLocation(CurrentLoc);
        FirstPersonCamera->SetWorldRotation(CurrentRot);
        
        // Switch cameras early in transition to avoid jump
        if (FVector::Dist(CurrentLoc, TargetLocation) < 200.0f && Camera->IsActive())
        {
            Camera->SetActive(false);
            FirstPersonCamera->SetActive(true);
        }
        
        if (FVector::Dist(CurrentLoc, TargetLocation) < 10.0f)
        {
            SetActorLocation(TargetLocation);
            FirstPersonCamera->SetWorldRotation(TargetRotation);
            bTransitioning = false;
            UE_LOG(LogTemp, Warning, TEXT("First person transition complete"));
        }
    }
    else
    {
        // Transition to overhead
        FVector CurrentLoc = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, TransitionSpeed);
        FRotator CurrentRot = FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRotation, DeltaTime, TransitionSpeed);
        
        SetActorLocation(CurrentLoc);
        SpringArm->SetWorldRotation(CurrentRot);
        
        // Switch cameras early and match SpringArm position for smooth return
        if (FVector::Dist(CurrentLoc, TargetLocation) < 200.0f && FirstPersonCamera->IsActive())
        {
            FirstPersonCamera->SetActive(false);
            Camera->SetActive(true);
        }
        
        if (FVector::Dist(CurrentLoc, TargetLocation) < 50.0f)
        {
            SetActorLocation(TargetLocation);
            SpringArm->SetWorldRotation(TargetRotation);
            bTransitioning = false;
            UE_LOG(LogTemp, Warning, TEXT("Overhead transition complete"));
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
        FRotator CurrentRotation = Camera->GetComponentRotation();
        CurrentRotation.Yaw += LookInput.X * MouseSensitivity;
        CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (LookInput.Y * MouseSensitivity), -89.0f, 89.0f);
        Camera->SetWorldRotation(CurrentRotation);
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
    FVector NewLocation = CurrentLocation;
    bool bLocationChanged = false;
    
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
        NewLocation = CurrentLocation + (MovementDir * CameraMoveSpeed * 0.3f * DeltaTime);
        bLocationChanged = true;
    }
    
    // CRITICAL: ALWAYS follow terrain height, regardless of movement
    if (IsTerrainValid())
    {
        float TerrainHeight = GetSafeTerrainHeight(NewLocation);
        float TargetHeight = TerrainHeight + FirstPersonHeight;
        
        // Always adjust height to follow terrain
        if (FMath::Abs(NewLocation.Z - TargetHeight) > 5.0f) // 5cm tolerance
        {
            // Smooth height interpolation for natural feel
            float HeightLerpSpeed = 8.0f; // Adjust for responsiveness
            NewLocation.Z = FMath::FInterpTo(NewLocation.Z, TargetHeight, DeltaTime, HeightLerpSpeed);
            bLocationChanged = true;
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("First-person height adjusted: %.1f -> %.1f (terrain: %.1f)"), 
                   CurrentLocation.Z, NewLocation.Z, TerrainHeight);
        }
    }
    
    // Apply location changes if any occurred
    if (bLocationChanged)
    {
        SetActorLocation(NewLocation);
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

// ===== ATMOSPHERIC BRUSH FUNCTIONS =====

void ATerrainController::HandleBrushCycle()
{
    switch (CurrentEditingMode)
    {
        case EEditingMode::Terrain:
            // Future: Cycle through terrain brush types
            UE_LOG(LogTemp, Warning, TEXT("Terrain brush cycling not implemented yet"));
            break;
            
        case EEditingMode::Water:
            // Future: Cycle through water brush types
            UE_LOG(LogTemp, Warning, TEXT("Water brush cycling not implemented yet"));
            break;
            
        case EEditingMode::Atmosphere:
            CycleAtmosphericBrush();
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

// ===== OLD BRUSH FUNCTIONS REMOVED =====
// All brush functions now delegate to Universal Brush System in MasterController
// No local brush implementations to avoid redefinition conflicts

// ===== HELPER FUNCTION IMPLEMENTATIONS =====

bool ATerrainController::IsTerrainValid() const
{
    // Check if terrain exists and has chunks initialized
    return TargetTerrain && IsValid(TargetTerrain) && TargetTerrain->TerrainChunks.Num() > 0;
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
    // Always reset look input when editing to prevent accumulation
    if (bIsEditingTerrain || bIsEditingWater)
    {
        LookInput = FVector2D::ZeroVector;
    }
    
    // Reset other inputs
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
}

void ATerrainController::UpdateAuthorityCache(float DeltaTime)
{
    AuthorityCacheTimer += DeltaTime;
    
    if (AuthorityCacheTimer >= AuthorityCacheRate)
    {
        bMasterControllerValid = (MasterController != nullptr && IsValid(MasterController));
        AuthorityCacheTimer = 0.0f;
    }
}

// Add this periodic terrain validation check
void ATerrainController::ValidateFirstPersonHeight(float DeltaTime)
{
    // Only run validation for first person mode
    if (CurrentCameraMode != ECameraMode::FirstPerson || bTransitioning)
    {
        return;
    }
    
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    
    // Run validation every 0.1 seconds (10Hz) to catch terrain changes
    if (ValidationTimer >= 0.1f && IsTerrainValid())
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
        
        ValidationTimer = 0.0f;
    }
}




