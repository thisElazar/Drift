// TerrainController.cpp - Complete Clean UE 5.4 Compatible Version with Water System
#include "TerrainController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
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
    
    // *** CRITICAL COLLISION SETTINGS FOR TERRAIN CLIPPING FIX ***
    SpringArm->bDoCollisionTest = false;             // Disable collision to prevent camera jumping on hills
    SpringArm->bUseCameraLagSubstepping = true;      // Smooth collision response
    SpringArm->CameraLagSpeed = 10.0f;               // Fast lag recovery
    SpringArm->CameraLagMaxDistance = 50.0f;         // Max lag distance
    SpringArm->ProbeSize = 12.0f;                    // Collision sphere radius (default is 0)
    SpringArm->ProbeChannel = ECC_Camera;            // Use camera collision channel
    
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
    
    // Terrain editing settings
    BrushRadius = 500.0f;
    BrushStrength = 200.0f;
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
}

void ATerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial camera rotation
    SpringArm->SetWorldRotation(CameraRotation);
    
    // Find terrain in world if not set
    if (!TargetTerrain)
    {
        // Try UGameplayStatics first
        TargetTerrain = Cast<ADynamicTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(), ADynamicTerrain::StaticClass()));
        
        // Fallback to TActorIterator if needed
        if (!TargetTerrain)
        {
            for (TActorIterator<ADynamicTerrain> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                TargetTerrain = *ActorItr;
                break;
            }
        }
        
        // Auto-create terrain if none exists
        if (!TargetTerrain)
        {
            TargetTerrain = GetWorld()->SpawnActor<ADynamicTerrain>();
            UE_LOG(LogTemp, Warning, TEXT("Auto-created DynamicTerrain"));
        }
        
        // Apply game settings from GameInstance
        UTerrAIGameInstance* GameInst = Cast<UTerrAIGameInstance>(GetGameInstance());
        if (GameInst && TargetTerrain)
        {
            // Apply world size setting
            int32 NewSize = GameInst->GetTerrainSize();
            TargetTerrain->TerrainWidth = NewSize;
            TargetTerrain->TerrainHeight = NewSize;
            
            // Apply default texture setting
            switch (GameInst->DefaultTexture)
            {
                case EDefaultTexture::Wireframe:
                    CurrentVisualMode = ETerrainVisualMode::Wireframe;
                    break;
                case EDefaultTexture::Natural:
                    CurrentVisualMode = ETerrainVisualMode::Naturalistic;
                    break;
                case EDefaultTexture::Hybrid:
                    CurrentVisualMode = ETerrainVisualMode::Hybrid;
                    break;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Applied game settings: Size=%d, Texture=%d"), NewSize, (int32)GameInst->DefaultTexture);
        }
        
        // Auto-find WaterController if not set (backup method)
        if (!WaterController)
        {
            WaterController = Cast<AWaterController>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaterController::StaticClass()));
            if (WaterController)
            {
                UE_LOG(LogTemp, Warning, TEXT("Auto-found WaterController in world"));
            }
        }
        
        // Initialize WaterController if available - it handles all water parameters
        if (WaterController)
        {
            WaterController->Initialize(TargetTerrain);
            UE_LOG(LogTemp, Warning, TEXT("TerrainController: WaterController initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("TerrainController: No WaterController assigned"));
        }
        
        // ADD ATMOSPHERECONTROLLER DELEGATION (Phase 4)
        if (TargetTerrain && TargetTerrain->AtmosphericSystem)
        {
            // Delegate atmospheric control to AtmosphericSystem instead of handling directly
            UE_LOG(LogTemp, Warning, TEXT("TerrainController: AtmosphericSystem available for delegation"));
        }
        
        // Create brush preview
        CreateBrushPreview();
        
        // *** SET VISUAL MODE AFTER WATER SYSTEM IS READY ***
        SetVisualMode(CurrentVisualMode);
        
        // Apply depth buffer optimizations for z-fighting fix
        if (GetWorld())
        {
            GetWorld()->Exec(GetWorld(), TEXT("r.SetNearClipPlane 1.0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.EarlyZPass 1"));
            UE_LOG(LogTemp, Warning, TEXT("Applied z-fighting fixes for large terrain"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainController initialized with water system support"));
    }
}


void ATerrainController::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SetupInputMapping();
}

void ATerrainController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
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
    
    // Update brush preview
    if (bShowBrushPreview)
    {
        UpdateBrushPreview();
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
    // Handle camera transitions first
    UpdateCameraTransition(DeltaTime);
    
    if (bTransitioning) return;
    
    if (CurrentCameraMode == ECameraMode::Overhead)
    {
        UpdateOverheadCamera(DeltaTime);
    }
    else if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        UpdateFirstPersonCamera(DeltaTime);
    }
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
    
    // Reset cursor smoothing
    bCursorInitialized = false;
    
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
    if (!TargetTerrain)
    {
        return;
    }
    
    FVector RawCursorPosition = GetCursorWorldPosition();
    if (RawCursorPosition != FVector::ZeroVector)
    {
        // Initialize cursor position on first valid trace
        if (!bCursorInitialized)
        {
            SmoothedCursorPosition = RawCursorPosition;
            LastValidCursorPosition = RawCursorPosition;
            bCursorInitialized = true;
        }
        
        // Smooth cursor movement to prevent jumping
        SmoothedCursorPosition = FMath::VInterpTo(SmoothedCursorPosition, 
            RawCursorPosition, DeltaTime, CursorSmoothingSpeed);
        
        // Apply modification at smoothed position
        TargetTerrain->ModifyTerrain(SmoothedCursorPosition, BrushRadius, 
            BrushStrength * DeltaTime, bIsRaisingTerrain);
        
        LastValidCursorPosition = SmoothedCursorPosition;
    }
    else if (bCursorInitialized)
    {
        // Use last valid position if trace fails
        TargetTerrain->ModifyTerrain(LastValidCursorPosition, BrushRadius, 
            BrushStrength * DeltaTime, bIsRaisingTerrain);
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
    
    // Reset cursor smoothing
    bCursorInitialized = false;
    
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
    // PHASE 1: Delegate to WaterController, fallback to direct terrain access
    if (WaterController)
    {
        FVector RawCursorPosition = GetCursorWorldPosition();
        if (RawCursorPosition != FVector::ZeroVector)
        {
            // Apply same smoothing logic
            if (!bCursorInitialized)
            {
                SmoothedCursorPosition = RawCursorPosition;
                LastValidCursorPosition = RawCursorPosition;
                bCursorInitialized = true;
            }
            
            SmoothedCursorPosition = FMath::VInterpTo(SmoothedCursorPosition, 
                RawCursorPosition, DeltaTime, CursorSmoothingSpeed);
            
            float WaterAmount = bIsAddingWater ?
                WaterBrushStrength * WaterAdditionRate * DeltaTime :
                WaterBrushStrength * DeltaTime;
                
            if (bIsAddingWater)
            {
                WaterController->AddWater(SmoothedCursorPosition, WaterAmount);
            }
            else if (bIsRemovingWater)
            {
                WaterController->RemoveWater(SmoothedCursorPosition, WaterAmount);
            }
            LastValidCursorPosition = SmoothedCursorPosition;
        }
        else if (bCursorInitialized)
        {
            // Use last valid position if trace fails
            float WaterAmount = bIsAddingWater ?
                WaterBrushStrength * WaterAdditionRate * DeltaTime :
                WaterBrushStrength * DeltaTime;
                
            if (bIsAddingWater)
            {
                WaterController->AddWater(LastValidCursorPosition, WaterAmount);
            }
            else if (bIsRemovingWater)
            {
                WaterController->RemoveWater(LastValidCursorPosition, WaterAmount);
            }
        }
    }
    else if (TargetTerrain)
    {
        // Fallback: direct terrain access until WaterController is fixed
        UE_LOG(LogTemp, Warning, TEXT("No WaterController - using fallback terrain access"));
        FVector RawCursorPosition = GetCursorWorldPosition();
        if (RawCursorPosition != FVector::ZeroVector)
        {
            // Apply smoothing to fallback too
            if (!bCursorInitialized)
            {
                SmoothedCursorPosition = RawCursorPosition;
                LastValidCursorPosition = RawCursorPosition;
                bCursorInitialized = true;
            }
            
            SmoothedCursorPosition = FMath::VInterpTo(SmoothedCursorPosition, 
                RawCursorPosition, DeltaTime, CursorSmoothingSpeed);
            
            float WaterAmount = bIsAddingWater ?
                WaterBrushStrength * WaterAdditionRate * DeltaTime :
                WaterBrushStrength * DeltaTime;
                
            if (bIsAddingWater)
            {
                WaterController->AddWater(SmoothedCursorPosition, WaterAmount);
            }
            else if (bIsRemovingWater)
            {
                WaterController->RemoveWater(SmoothedCursorPosition, WaterAmount);
            }
            LastValidCursorPosition = SmoothedCursorPosition;
        }
    }
}

// ===== BRUSH CONTROL FUNCTIONS =====

void ATerrainController::IncreaseBrushSize(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        SetBrushRadius(BrushRadius + BrushSizeChangeRate);
    }
}

void ATerrainController::DecreaseBrushSize(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        SetBrushRadius(BrushRadius - BrushSizeChangeRate);
    }
}

void ATerrainController::IncreaseBrushStrength(const FInputActionValue& Value)
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        SetBrushStrength(BrushStrength + BrushStrengthChangeRate);
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
        SetBrushStrength(BrushStrength - BrushStrengthChangeRate);
    }
    else if (CurrentEditingMode == EEditingMode::Water)
    {
        SetWaterBrushStrength(WaterBrushStrength - WaterStrengthChangeRate);
    }
}

void ATerrainController::SetBrushRadius(float NewRadius)
{
    BrushRadius = FMath::Clamp(NewRadius, MinBrushRadius, MaxBrushRadius);
    UE_LOG(LogTemp, Warning, TEXT("Brush radius: %.1f"), BrushRadius);
}

void ATerrainController::SetBrushStrength(float NewStrength)
{
    BrushStrength = FMath::Clamp(NewStrength, MinBrushStrength, MaxBrushStrength);
    UE_LOG(LogTemp, Warning, TEXT("Brush strength: %.1f"), BrushStrength);
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
        UE_LOG(LogTemp, Warning, TEXT("Visual mode changed to: %s"), *ModeName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Material not set for visual mode: %s"), *ModeName);
    }
}



void ATerrainController::ToggleEditingMode()
{
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        CurrentEditingMode = EEditingMode::Water;
        UE_LOG(LogTemp, Warning, TEXT("Switched to Water Editing Mode"));
    }
    else
    {
        CurrentEditingMode = EEditingMode::Terrain;
        UE_LOG(LogTemp, Warning, TEXT("Switched to Terrain Editing Mode"));
    }
    
    // Stop any current editing
    StopTerrainEditing();
    StopWaterEditing();
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
        TargetTerrain->GenerateProceduralTerrain();
        UE_LOG(LogTemp, Warning, TEXT("Terrain reset"));
    }
}

// ===== UTILITY FUNCTIONS =====

FVector ATerrainController::GetCursorWorldPosition() const
{
    FVector HitLocation;
    if (PerformCursorTrace(HitLocation))
    {
        return HitLocation;
    }
    return FVector::ZeroVector;
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
        // Ensure trace starts high above terrain to avoid underground hits
        FVector Start = MouseWorldLocation;
        if (TargetTerrain)
        {
            FVector TerrainCenter = TargetTerrain->GetActorLocation();
            float MaxTerrainHeight = TerrainCenter.Z + 5000.0f; // Conservative max height
            Start.Z = FMath::Max(Start.Z, MaxTerrainHeight);
        }
        
        FVector End = Start + (MouseWorldDirection * 50000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.bTraceComplex = false;
        CollisionParams.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, CollisionParams))
        {
            // Only accept hits on upward-facing surfaces (terrain top)
            if (HitResult.Normal.Z > 0.7f) // Surface normal pointing mostly upward
            {
                OutHitLocation = HitResult.Location;
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Verbose, TEXT("Ray hit surface from below, ignoring"));
            }
        }
        
        // Fallback: Find closest terrain edge point when ray misses
        if (TargetTerrain)
        {
            // Project ray onto terrain plane and find closest valid point
            FVector TerrainCenter = TargetTerrain->GetActorLocation();
            float TerrainSize = TargetTerrain->TerrainWidth * TargetTerrain->TerrainScale;
            
            // Intersect ray with terrain's base plane
            FVector PlaneNormal = FVector::UpVector;
            FVector PlanePoint = TerrainCenter;
            
            float Denominator = FVector::DotProduct(MouseWorldDirection, PlaneNormal);
            if (FMath::Abs(Denominator) > 0.0001f)
            {
                float T = FVector::DotProduct(PlanePoint - MouseWorldLocation, PlaneNormal) / Denominator;
                FVector PlaneIntersection = MouseWorldLocation + (MouseWorldDirection * T);
                
                // Clamp to terrain bounds
                float HalfSize = TerrainSize * 0.5f;
                PlaneIntersection.X = FMath::Clamp(PlaneIntersection.X, 
                    TerrainCenter.X - HalfSize, TerrainCenter.X + HalfSize);
                PlaneIntersection.Y = FMath::Clamp(PlaneIntersection.Y, 
                    TerrainCenter.Y - HalfSize, TerrainCenter.Y + HalfSize);
                
                // Get actual terrain height at this position
                float TerrainHeight = TargetTerrain->GetHeightAtPosition(PlaneIntersection);
                OutHitLocation = FVector(PlaneIntersection.X, PlaneIntersection.Y, TerrainHeight);
                return true;
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

void ATerrainController::CreateBrushPreview()
{
    if (BrushPreview)
    {
        BrushPreview->SetVisibility(bShowBrushPreview);
        UE_LOG(LogTemp, Warning, TEXT("Brush preview component ready"));
    }
}

void ATerrainController::UpdateBrushPreview()
{
    if (!BrushPreview || !bShowBrushPreview)
    {
        return;
    }
    
    // Use smoothed position during editing, raw position when not editing
    FVector DisplayPosition;
    if (bIsEditingTerrain || bIsEditingWater)
    {
        // During editing: use smoothed cursor to prevent jumps
        DisplayPosition = bCursorInitialized ? SmoothedCursorPosition : GetCursorWorldPosition();
    }
    else
    {
        // When not editing: use raw cursor for immediate response
        DisplayPosition = GetCursorWorldPosition();
    }
    
    if (DisplayPosition != FVector::ZeroVector)
    {
        BrushPreview->SetWorldLocation(DisplayPosition);
        float PreviewScale = BrushRadius / 100.0f;
        BrushPreview->SetWorldScale3D(FVector(PreviewScale));
        BrushPreview->SetVisibility(true);
        UpdateBrushPreviewMaterial();
    }
    else
    {
        BrushPreview->SetVisibility(false);
    }
}

void ATerrainController::UpdateBrushPreviewMaterial()
{
    if (!BrushPreview)
    {
        return;
    }
    
    UMaterial* BrushMaterial = nullptr;
    
    if (CurrentEditingMode == EEditingMode::Terrain)
    {
        if (bIsRaisingTerrain)
        {
            BrushMaterial = RaiseBrushMaterial;
        }
        else if (bIsLoweringTerrain)
        {
            BrushMaterial = LowerBrushMaterial;
        }
    }
    else if (CurrentEditingMode == EEditingMode::Water)
    {
        if (bIsAddingWater)
        {
            BrushMaterial = AddWaterBrushMaterial;
        }
        else if (bIsRemovingWater)
        {
            BrushMaterial = RemoveWaterBrushMaterial;
        }
    }
    
    if (BrushMaterial)
    {
        BrushPreview->SetMaterial(0, BrushMaterial);
    }
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
        // Show editing mode and brush info
        FString EditingModeText = (CurrentEditingMode == EEditingMode::Terrain) ? TEXT("Terrain") : TEXT("Water");
        GEngine->AddOnScreenDebugMessage(30, 0.5f, FColor::White,
            FString::Printf(TEXT("Editing Mode: %s"), *EditingModeText));
        
        if (CurrentEditingMode == EEditingMode::Terrain)
        {
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Brush: %.0f radius, %.0f strength"), BrushRadius, BrushStrength));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(31, 0.5f, FColor::White,
                FString::Printf(TEXT("Water Brush: %.1f strength"), WaterBrushStrength));
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
    
    // Reset inputs
    LookInput = FVector2D::ZeroVector;
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
}

void ATerrainController::UpdateFirstPersonCamera(float DeltaTime)
{
    // Smooth movement input (was missing)
    SmoothedMovementInput.X = FMath::FInterpTo(SmoothedMovementInput.X, MovementInput.X, DeltaTime, MovementInputSmoothness);
    SmoothedMovementInput.Y = FMath::FInterpTo(SmoothedMovementInput.Y, MovementInput.Y, DeltaTime, MovementInputSmoothness);
    
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
        FVector NewLocation = GetActorLocation() + (MovementDir * CameraMoveSpeed * 0.3f * DeltaTime);
        
        // Maintain height above terrain with validation
        if (TargetTerrain)
        {
            float TerrainHeight = TargetTerrain->GetHeightAtPosition(NewLocation);
            float MinHeight = TerrainHeight + FirstPersonHeight;
            
            // Ensure we never go underground
            if (NewLocation.Z < MinHeight)
            {
                NewLocation.Z = MinHeight;
            }
        }
        
        SetActorLocation(NewLocation);
    }
    
    // CRITICAL: Periodic terrain height check to prevent underground drift
    if (TargetTerrain)
    {
        FVector CurrentLocation = GetActorLocation();
        float TerrainHeight = TargetTerrain->GetHeightAtPosition(CurrentLocation);
        float MinHeight = TerrainHeight + 50.0f; // Minimum 50 units above terrain
        
        if (CurrentLocation.Z < MinHeight)
        {
            CurrentLocation.Z = MinHeight;
            SetActorLocation(CurrentLocation);
            UE_LOG(LogTemp, Warning, TEXT("First-person camera corrected from underground"));
        }
    }
    
    // Mouse look for first person
    if (!LookInput.IsZero() && bMouseLookEnabled && !bIsEditingTerrain && !bIsEditingWater && !bTransitioning)
    {
        FRotator CurrentRotation = FirstPersonCamera->GetComponentRotation();
        CurrentRotation.Yaw += LookInput.X * MouseSensitivity;
        CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (LookInput.Y * MouseSensitivity), -80.0f, 80.0f);
        FirstPersonCamera->SetWorldRotation(CurrentRotation);
    }
    
    // Always reset during editing
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

