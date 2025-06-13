// TerrainController.h - Enhanced with Water System Controls
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TerrainController.generated.h"

UENUM(BlueprintType)
enum class ETerrainVisualMode : uint8
{
    Wireframe      UMETA(DisplayName = "Digital Wireframe"),
    Naturalistic   UMETA(DisplayName = "Natural Landscape"),
    Hybrid         UMETA(DisplayName = "Hybrid Mode"),
    Chrome         UMETA(DisplayName = "Chrome"),
    Glass          UMETA(DisplayName = "Glass")
};

UENUM(BlueprintType)
enum class ECameraMode : uint8
{
    Overhead    UMETA(DisplayName = "Overhead View"),
    FirstPerson UMETA(DisplayName = "First Person")
};

UENUM(BlueprintType)
enum class EEditingMode : uint8
{
    Terrain        UMETA(DisplayName = "Terrain Editing"),
    Water          UMETA(DisplayName = "Water Editing")
};

// Forward declarations
class UInputMappingContext;
class UInputAction;
class ADynamicTerrain;
class UMaterial;
class AWaterController;

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API ATerrainController : public APawn
{
    GENERATED_BODY()

public:
    ATerrainController();

    // Visual mode system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Modes")
    ETerrainVisualMode CurrentVisualMode = ETerrainVisualMode::Wireframe;
    


    // Editing mode system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editing")
    EEditingMode CurrentEditingMode = EEditingMode::Terrain;

    // Material references - SET THESE IN BLUEPRINT!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WireframeMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* NaturalisticMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HybridMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ChromeMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GlassMaterial = nullptr;

    // Input action for V key
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ToggleVisualizationAction;

    // Water system input actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AddWaterAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* RemoveWaterAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ToggleRainAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ToggleEditingModeAction;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Visual Modes")
    void ToggleVisualMode();

    UFUNCTION(BlueprintCallable, Category = "Visual Modes")
    void SetVisualMode(ETerrainVisualMode NewMode);
    


    UFUNCTION(BlueprintCallable, Category = "Editing")
    void ToggleEditingMode();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ToggleRain();

    // ===== TIME CONTROL SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float TimeSpeed = 1.0f;  // 1.0 = normal speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float MinTimeSpeed = 0.1f;  // Slowest (1/10 speed)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float MaxTimeSpeed = 10.0f;  // Fastest (10x speed)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float TimeSpeedChangeRate = 0.5f;  // How much to change per keypress

    // ===== CLOUD RENDERING SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System")
    bool bEnableCloudRendering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (EditCondition = "bEnableCloudRendering"))
    UStaticMesh* CloudStaticMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (EditCondition = "bEnableCloudRendering"))
    UMaterialInterface* CloudMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (EditCondition = "bEnableCloudRendering"))
    float CloudAltitude = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (EditCondition = "bEnableCloudRendering"))
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud System", meta = (EditCondition = "bEnableCloudRendering"))
    int32 MaxCloudMeshes = 64;

    // ===== ATMOSPHERIC CONTROL SETTINGS - MOVED TO ATMOSPHERECONTROLLER =====
    // All atmospheric functions moved to AtmosphereController for clean separation

    // ===== CONTROLLER REFERENCES =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AWaterController* WaterController = nullptr;

    // Enhanced Input - Shift key detection for delegation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LeftShiftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* RightShiftAction;

    // Camera switching input actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* SwitchCameraAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ReturnToOverheadAction;

    // Camera system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    ECameraMode CurrentCameraMode = ECameraMode::Overhead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    float FirstPersonHeight = 152.4f;  // 5 feet in cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    float TransitionSpeed = 3.0f;
    UFUNCTION()
    void HandleWaterToggle();

    UFUNCTION()
    void HandleRainToggle();

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

    // Root Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* SceneRoot;

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArm;

    // Brush Preview Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brush")
    class UStaticMeshComponent* BrushPreview;

    // References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    class ADynamicTerrain* TargetTerrain;

    // Enhanced Input - Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ZoomAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* FlyUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* FlyDownAction;

    // Enhanced Input - Terrain Editing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* RaiseTerrainAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LowerTerrainAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* IncreaseBrushSizeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* DecreaseBrushSizeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* IncreaseBrushStrengthAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* DecreaseBrushStrengthAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ResetTerrainAction;

    // Time control input actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* IncreaseTimeSpeedAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* DecreaseTimeSpeedAction;

    // Camera Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraMoveSpeed = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraFlySpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float MouseSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraZoomSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float MinZoomDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float MaxZoomDistance = 20000.0f;

    // Terrain Editing Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float BrushRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float BrushStrength = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float MinBrushRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float MaxBrushRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float MinBrushStrength = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float MaxBrushStrength = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float BrushSizeChangeRate = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Editing")
    float BrushStrengthChangeRate = 20.0f;

    // Water Editing Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float WaterBrushStrength = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float WaterAdditionRate = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float LakeCreationMultiplier = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float MinWaterBrushStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float MaxWaterBrushStrength = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Editing")
    float WaterStrengthChangeRate = 5.0f;

    // Brush Preview Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    bool bShowBrushPreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    UMaterial* RaiseBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    UMaterial* LowerBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    UMaterial* AddWaterBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    UMaterial* RemoveWaterBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview")
    float BrushPreviewOpacity = 0.3f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowPerformanceStats = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowTerrainInfo = true;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Terrain editing functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void StartTerrainEditing(bool bRaise);

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void StopTerrainEditing();

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void SetBrushRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void SetBrushStrength(float NewStrength);

    // Water editing functions
    UFUNCTION(BlueprintCallable, Category = "Water Editing")
    void StartWaterEditing(bool bAdd);

    UFUNCTION(BlueprintCallable, Category = "Water Editing")
    void StopWaterEditing();

    UFUNCTION(BlueprintCallable, Category = "Water Editing")
    void SetWaterBrushStrength(float NewStrength);

    // Time control functions
    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void IncreaseTimeSpeed();

    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void DecreaseTimeSpeed();

    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void SetTimeSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void ResetTimeSpeed();

    // Atmospheric control functions - MOVED TO ATMOSPHERECONTROLLER
    // All atmospheric functions moved to AtmosphereController for clean separation

    // Camera system functions
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SwitchCameraMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void WarpToFirstPerson();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ReturnToOverhead();

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    FVector GetCursorWorldPosition() const;

    // Camera system helper functions
    void UpdateCameraTransition(float DeltaTime);
    void UpdateOverheadCamera(float DeltaTime);
    void UpdateFirstPersonCamera(float DeltaTime);
    FVector GetTerrainHeightAtCursor() const;

protected:
    // Enhanced Input functions - Movement
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Zoom(const FInputActionValue& Value);
    void FlyUp(const FInputActionValue& Value);
    void FlyDown(const FInputActionValue& Value);

    // Enhanced Input functions - Terrain Editing
    void StartRaiseTerrain(const FInputActionValue& Value);
    void StopRaiseTerrain(const FInputActionValue& Value);
    void StartLowerTerrain(const FInputActionValue& Value);
    void StopLowerTerrain(const FInputActionValue& Value);
    void IncreaseBrushSize(const FInputActionValue& Value);
    void DecreaseBrushSize(const FInputActionValue& Value);
    void IncreaseBrushStrength(const FInputActionValue& Value);
    void DecreaseBrushStrength(const FInputActionValue& Value);
    void ResetTerrain(const FInputActionValue& Value);

    // Enhanced Input functions - Water Editing
    void StartAddWater(const FInputActionValue& Value);
    void StopAddWater(const FInputActionValue& Value);
    void StartRemoveWater(const FInputActionValue& Value);
    void StopRemoveWater(const FInputActionValue& Value);
    void ToggleRainInput(const FInputActionValue& Value);
    void ToggleEditingModeInput(const FInputActionValue& Value);

    // Enhanced Input functions - Shift key tracking
    void OnLeftShiftPressed(const FInputActionValue& Value);
    void OnLeftShiftReleased(const FInputActionValue& Value);
    void OnRightShiftPressed(const FInputActionValue& Value);
    void OnRightShiftReleased(const FInputActionValue& Value);

    // Performance and UI
    void UpdatePerformanceStats(float DeltaTime);
    void UpdateTerrainInfo(float DeltaTime);

private:
    // Camera movement and look
    FVector2D MovementInput = FVector2D::ZeroVector;
    FVector2D LookInput = FVector2D::ZeroVector;
    float ZoomInput = 0.0f;
    float FlyUpInput = 0.0f;
    float FlyDownInput = 0.0f;
    
    // Camera state
    FRotator CameraRotation = FRotator(-45.0f, 0.0f, 0.0f);
    bool bMouseLookEnabled = true;
    
    // Store current material for reference
    UPROPERTY()
    UMaterialInterface* CurrentActiveMaterial = nullptr;

    void ApplyMaterialToAllChunks(UMaterialInterface* Material);
    void SetShadowCastingForAllChunks(bool bCastShadows);
    
    // Input smoothing
    FVector2D SmoothedMovementInput = FVector2D::ZeroVector;
    float MovementInputSmoothness = 10.0f;

    // Editing state
    bool bIsEditingTerrain = false;
    bool bIsRaisingTerrain = false;
    bool bIsLoweringTerrain = false;
    bool bIsEditingWater = false;
    bool bIsAddingWater = false;
    bool bIsRemovingWater = false;
    FVector LastCursorPosition = FVector::ZeroVector;
    
    // Enhanced Input - Shift key state
    bool bLeftShiftHeld = false;
    bool bRightShiftHeld = false;

    // Camera transition state
    bool bTransitioning = false;
    FVector TargetLocation;
    FRotator TargetRotation;
    FVector StoredOverheadLocation;
    FRotator StoredOverheadRotation;
    float StoredCameraHeight;
    float MinCameraHeight = 200.0f;
    float MaxCameraHeight = 10000.0f;

    // Cursor stabilization
    FVector SmoothedCursorPosition = FVector::ZeroVector;
    FVector LastValidCursorPosition = FVector::ZeroVector;
    float CursorSmoothingSpeed = 15.0f;
    bool bCursorInitialized = false;

    // Performance tracking
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    float StatUpdateTimer = 0.0f;

    // Brush preview
    void CreateBrushPreview();
    void UpdateBrushPreview();
    void UpdateBrushPreviewMaterial();

    // Helper functions
    void UpdateCameraPosition(float DeltaTime);
    void UpdateTerrainModification(float DeltaTime);
    void UpdateWaterModification(float DeltaTime);
    void SetupInputMapping();
    bool PerformCursorTrace(FVector& OutHitLocation) const;
};
