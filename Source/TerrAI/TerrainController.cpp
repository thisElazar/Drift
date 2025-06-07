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
    SpringArm->bDoCollisionTest = true;              // Enable collision detection
    SpringArm->bUseCameraLagSubstepping = true;      // Smooth collision response
    SpringArm->CameraLagSpeed = 10.0f;               // Fast lag recovery
    SpringArm->CameraLagMaxDistance = 50.0f;         // Max lag distance
    SpringArm->ProbeSize = 12.0f;                    // Collision sphere radius (default is 0)
    SpringArm->ProbeChannel = ECC_Camera;            // Use camera collision channel
    
    // Optional: More aggressive collision settings
    //SpringArm->bUseSubstepping = true;               // Smooth collision movement
    //SpringArm->TargetOffset = FVector(0, 0, 100);    // Offset camera slightly above target
    
    // Create camera component
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    
    // Create brush preview component - attach to root, not spring arm
    BrushPreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrushPreview"));
    BrushPreview->SetupAttachment(RootComponent);  // Attach to root scene component
    BrushPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BrushPreview->SetCastShadow(false);
    BrushPreview->SetVisibility(true);
    
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
        
        // *** INITIALIZE WATER SYSTEM BEFORE MATERIAL APPLICATION ***
        // Initialize water system parameters FIRST to prevent blue flash
        if (TargetTerrain && TargetTerrain->WaterSystem && WaterParameterCollection)
        {
            TargetTerrain->WaterSystem->WaterParameterCollection = WaterParameterCollection;
            TargetTerrain->WaterSystem->WaterEvaporationRate = WaterEvaporationRate;
            TargetTerrain->WaterSystem->WaterAbsorptionRate = WaterAbsorptionRate;
            TargetTerrain->WaterSystem->WaterFlowSpeed = WaterFlowSpeed;
            TargetTerrain->WaterSystem->bEnableWaterSimulation = bEnableWaterSimulation;
            TargetTerrain->WaterSystem->WaterDamping = WaterDamping;
            TargetTerrain->WaterSystem->MaxWaterVelocity = MaxWaterVelocity;
            TargetTerrain->WaterSystem->MinWaterDepth = MinWaterDepth;
        }
        
        // *** APPLY EROSION SETTINGS AFTER WATER SYSTEM IS READY ***
        if (TargetTerrain && TargetTerrain->WaterSystem)
        {
            TargetTerrain->WaterSystem->bEnableErosion = bEnableErosion;
            TargetTerrain->WaterSystem->ErosionRate = ErosionRate;
            TargetTerrain->WaterSystem->MinErosionVelocity = MinErosionVelocity;
            TargetTerrain->WaterSystem->DepositionRate = DepositionRate;
            
            UE_LOG(LogTemp, Warning, TEXT("Applied erosion settings from TerrainController"));
        }
        
        // *** APPLY CLOUD SETTINGS TO ATMOSPHERIC SYSTEM ***
        if (TargetTerrain && TargetTerrain->AtmosphericSystem)
        {
            TargetTerrain->AtmosphericSystem->bEnableCloudRendering = bEnableCloudRendering;
            TargetTerrain->AtmosphericSystem->CloudStaticMesh = CloudStaticMesh;
            TargetTerrain->AtmosphericSystem->CloudMaterial = CloudMaterial;
            TargetTerrain->AtmosphericSystem->CloudAltitude = CloudAltitude;
            TargetTerrain->AtmosphericSystem->CloudOpacity = CloudOpacity;
            TargetTerrain->AtmosphericSystem->MaxCloudMeshes = MaxCloudMeshes;
            TargetTerrain->AtmosphericSystem->BaseTemperature = BaseTemperature;
            TargetTerrain->AtmosphericSystem->WindPattern = PrevailingWind;
            TargetTerrain->AtmosphericSystem->SeasonalAmplitude = SeasonalAmplitude;
            TargetTerrain->AtmosphericSystem->UpdateFrequency = AtmosphericUpdateRate;
            
            UE_LOG(LogTemp, Warning, TEXT("Applied cloud and atmospheric settings from TerrainController"));
        }
        
        // Create brush preview
        CreateBrushPreview();
        
        // *** SET VISUAL MODE AFTER WATER SYSTEM IS READY ***
        SetVisualMode(CurrentVisualMode);
        
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
            EnhancedInputComponent->BindAction(ToggleVisualizationAction, ETriggerEvent::Started, this, &ATerrainController::ToggleVisualMode);
        }
        if (ToggleRainAction)
        {
            EnhancedInputComponent->BindAction(ToggleRainAction, ETriggerEvent::Started, this, &ATerrainController::ToggleRainInput);
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
            EnhancedInputComponent->BindAction(IncreaseTimeSpeedAction, ETriggerEvent::Started, this, &ATerrainController::IncreaseTimeSpeedInput);
        }
        if (DecreaseTimeSpeedAction)
        {
            EnhancedInputComponent->BindAction(DecreaseTimeSpeedAction, ETriggerEvent::Started, this, &ATerrainController::DecreaseTimeSpeedInput);
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
    // UE 5.4 compatible smooth movement input
    SmoothedMovementInput.X = FMath::FInterpTo(SmoothedMovementInput.X, MovementInput.X, DeltaTime, MovementInputSmoothness);
    SmoothedMovementInput.Y = FMath::FInterpTo(SmoothedMovementInput.Y, MovementInput.Y, DeltaTime, MovementInputSmoothness);
    
    // Apply movement
    if (!SmoothedMovementInput.IsZero())
    {
        FVector ForwardDir = SpringArm->GetForwardVector();
        FVector RightDir = SpringArm->GetRightVector();
        
        // Project to ground plane
        ForwardDir.Z = 0;
        RightDir.Z = 0;
        ForwardDir.Normalize();
        RightDir.Normalize();
        
        FVector MovementDir = (ForwardDir * SmoothedMovementInput.Y) + (RightDir * SmoothedMovementInput.X);
        FVector NewLocation = GetActorLocation() + (MovementDir * CameraMoveSpeed * DeltaTime);
        SetActorLocation(NewLocation);
    }
    
    // Apply mouse look
    if (!LookInput.IsZero() && bMouseLookEnabled)
    {
        CameraRotation.Yaw += LookInput.X * MouseSensitivity;
        CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch + (LookInput.Y * MouseSensitivity), -80.0f, 80.0f);
        SpringArm->SetWorldRotation(CameraRotation);
    }
    
    // Apply zoom
    if (FMath::Abs(ZoomInput) > 0.1f)
    {
        float NewTargetArmLength = SpringArm->TargetArmLength - (ZoomInput * CameraZoomSpeed * DeltaTime);
        SpringArm->TargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomDistance, MaxZoomDistance);
    }
    
    // Apply vertical movement
    if (FMath::Abs(FlyUpInput) > 0.1f || FMath::Abs(FlyDownInput) > 0.1f)
    {
        float VerticalMovement = (FlyUpInput - FlyDownInput) * CameraFlySpeed * DeltaTime;
        FVector CurrentLocation = GetActorLocation();
        CurrentLocation.Z += VerticalMovement;
        SetActorLocation(CurrentLocation);
    }
    
    // Reset inputs
    LookInput = FVector2D::ZeroVector;
    ZoomInput = 0.0f;
    FlyUpInput = 0.0f;
    FlyDownInput = 0.0f;
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
    
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition != FVector::ZeroVector)
    {
        TargetTerrain->ModifyTerrain(CursorPosition, BrushRadius, BrushStrength * DeltaTime, bIsRaisingTerrain);
        LastCursorPosition = CursorPosition;
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
    if (!TargetTerrain)
    {
        return;
    }
    
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition != FVector::ZeroVector)
    {
        float WaterAmount = bIsAddingWater ?
            WaterBrushStrength * WaterAdditionRate * DeltaTime :
            WaterBrushStrength * DeltaTime;
            
        if (bIsAddingWater)
        {
            TargetTerrain->AddWater(CursorPosition, WaterAmount);
        }
        else if (bIsRemovingWater)
        {
            TargetTerrain->RemoveWater(CursorPosition, WaterAmount);
        }
        LastCursorPosition = CursorPosition;
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

void ATerrainController::ToggleVisualMode()
{
    int32 CurrentModeInt = static_cast<int32>(CurrentVisualMode);
    CurrentModeInt = (CurrentModeInt + 1) % 3;
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

void ATerrainController::ToggleRain()
{
    if (!TargetTerrain)
    {
        return;
    }
    
    if (TargetTerrain->WaterSystem && TargetTerrain->WaterSystem->bIsRaining)
    {
        TargetTerrain->StopRain();
        UE_LOG(LogTemp, Warning, TEXT("Rain stopped"));
    }
    else
    {
        TargetTerrain->StartRain(1.5f);
        UE_LOG(LogTemp, Warning, TEXT("Rain started"));
    }
}

void ATerrainController::ToggleRainInput(const FInputActionValue& Value)
{
    ToggleRain();
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
        FVector Start = MouseWorldLocation;
        FVector End = Start + (MouseWorldDirection * 50000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.bTraceComplex = false;
        CollisionParams.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, CollisionParams))
        {
            OutHitLocation = HitResult.Location;
            return true;
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
    
    FVector CursorPosition = GetCursorWorldPosition();
    if (CursorPosition != FVector::ZeroVector)
    {
        BrushPreview->SetWorldLocation(CursorPosition);
        
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
            default:
                VisualModeText = TEXT("Unknown Mode");
                break;
        }
        
        GEngine->AddOnScreenDebugMessage(32, 0.5f, FColor::White,
            FString::Printf(TEXT("Visual Mode: %s (V to toggle)"), *VisualModeText));
        
        StatUpdateTimer = 0.0f;
    }
}

void ATerrainController::UpdateTerrainInfo(float DeltaTime)
{
    // Additional terrain info can be displayed here
    // Currently handled in UpdatePerformanceStats
}

// ===== EROSION CONTROL FUNCTIONS =====

void ATerrainController::SetErosionEnabled(bool bEnable)
{
    bEnableErosion = bEnable;
    
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->bEnableErosion = bEnable;
        UE_LOG(LogTemp, Warning, TEXT("Erosion %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
    }
}

void ATerrainController::ResetErosionSettings()
{
    ErosionRate = 0.05f;
    MinErosionVelocity = 10.0f;
    DepositionRate = 0.03f;
    MaxSedimentCapacity = 2.0f;
    
    // Apply to water system
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->ErosionRate = ErosionRate;
        TargetTerrain->WaterSystem->MinErosionVelocity = MinErosionVelocity;
        TargetTerrain->WaterSystem->DepositionRate = DepositionRate;
        
        UE_LOG(LogTemp, Warning, TEXT("Erosion settings reset to scientifically accurate defaults"));
    }
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

void ATerrainController::IncreaseTimeSpeedInput(const FInputActionValue& Value)
{
    IncreaseTimeSpeed();
}

void ATerrainController::DecreaseTimeSpeedInput(const FInputActionValue& Value)
{
    DecreaseTimeSpeed();
}

// ===== WATER PHYSICS CONTROL FUNCTIONS =====

void ATerrainController::SetWaterSimulationEnabled(bool bEnable)
{
    bEnableWaterSimulation = bEnable;
    
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        TargetTerrain->WaterSystem->bEnableWaterSimulation = bEnable;
        UE_LOG(LogTemp, Warning, TEXT("Water simulation %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
    }
}

// ===== ATMOSPHERIC CONTROL FUNCTIONS =====

void ATerrainController::SetBaseTemperature(float NewTemperature)
{
    BaseTemperature = NewTemperature;
    
    if (TargetTerrain && TargetTerrain->AtmosphericSystem)
    {
        TargetTerrain->AtmosphericSystem->BaseTemperature = NewTemperature;
        UE_LOG(LogTemp, Warning, TEXT("Base temperature set to %.1f K (%.1f°C)"), NewTemperature, NewTemperature - 273.15f);
    }
}

void ATerrainController::SetPrevailingWind(FVector2D NewWindPattern)
{
    PrevailingWind = NewWindPattern;
    
    if (TargetTerrain && TargetTerrain->AtmosphericSystem)
    {
        TargetTerrain->AtmosphericSystem->WindPattern = NewWindPattern;
        UE_LOG(LogTemp, Warning, TEXT("Prevailing wind set to (%.1f, %.1f) m/s"), NewWindPattern.X, NewWindPattern.Y);
    }
}
