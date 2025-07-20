// TerrainController.h - Enhanced with Water System Controls
#pragma once

#include "CoreMinimal.h"
#include "TerrAI.h"  // Include for validation macros and constants
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
#include "MasterController.h"  // For IBrushReceiver interface and FUniversalBrushSettings
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
    Water          UMETA(DisplayName = "Water Editing"),
    Atmosphere     UMETA(DisplayName = "Atmosphere Editing")
};

// Brush types for atmospheric editing
UENUM(BlueprintType) 
enum class EAtmosphericBrushType : uint8
{
    Wind        UMETA(DisplayName = "Wind Brush"),
    Pressure    UMETA(DisplayName = "Pressure Brush"),
    Temperature UMETA(DisplayName = "Temperature Brush"),
    Humidity    UMETA(DisplayName = "Humidity Brush")
};

// Forward declarations
class UInputMappingContext;
class UInputAction;
class ADynamicTerrain;
class UMaterial;
class AWaterController;
class AMasterWorldController;
struct FUniversalBrushSettings;
class IBrushReceiver;

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API ATerrainController : public APawn, public IBrushReceiver
{
    GENERATED_BODY()

public:
    ATerrainController();

    // Visual mode system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Modes")
    ETerrainVisualMode CurrentVisualMode = ETerrainVisualMode::Wireframe;
    
    
    // ===== AUTHORITY INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Authority")
    void InitializeControllerWithAuthority();

    
    // Editing mode system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editing")
    EEditingMode CurrentEditingMode = EEditingMode::Terrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    EAtmosphericBrushType CurrentAtmosphericBrush = EAtmosphericBrushType::Wind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    float AtmosphericBrushRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    float AtmosphericBrushIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    FVector WindForce = FVector(10.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    float PressureDelta = 1000.0f; // Pa

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    float TemperatureDelta = 5.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Editing")
    float HumidityDelta = 0.1f; // 0-1 range

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

    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleBrushCycle();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Editing")
    void CycleAtmosphericBrush();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Editing")
    void ApplyAtmosphericBrush(FVector Position);

    void UpdateAtmosphericEditing(float DeltaTime);
    FString GetCurrentBrushDisplayName() const;

    // Atmospheric fog functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Fog")
    void UpdateAtmosphericFog();
    
    float CalculateFogDensity(FVector Position) const;
    void InitializeFogSystem();

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

    // ===== ATMOSPHERIC FOG SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    bool bEnableAtmosphericFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float BaseFogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float FogStartDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float FogMaxOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float HumidityFogMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float TemperatureFogMultiplier = 1.5f;
    

  

    // ===== CONTROLLER REFERENCES =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AWaterController* WaterController = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    class AMasterWorldController* MasterController = nullptr;

    // Enhanced Input - Shift key detection for delegation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LeftShiftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* RightShiftAction;

    // Atmospheric brush cycling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* CycleBrushModeAction;

    // Reference to atmospheric system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    class UAtmosphericSystem* AtmosphericSystem = nullptr;

    // Camera switching input actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* SwitchCameraAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ReturnToOverheadAction;

    // Camera system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    ECameraMode CurrentCameraMode = ECameraMode::Overhead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    float FirstPersonHeight = TerrAIConstants::FIRST_PERSON_HEIGHT;  // 5 feet in cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
    float TransitionSpeed = TerrAIConstants::CAMERA_TRANSITION_SPEED;
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

    // ===== IBRUSHRECEIVER INTERFACE IMPLEMENTATION =====
    
    virtual void ApplyBrush(FVector WorldPosition, const FUniversalBrushSettings& Settings, float DeltaTime) override;
    virtual void UpdateBrushSettings(const FUniversalBrushSettings& Settings) override;
    virtual bool CanReceiveBrush() const override;
    
    // ===== BRUSH AUTHORITY DELEGATION =====
    // Note: These delegate to MasterController - no local implementations
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    float GetBrushRadius() const;
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    float GetBrushStrength() const;

    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SetBrushRadius(float NewRadius);
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SetBrushStrength(float NewStrength);
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    const FUniversalBrushSettings& GetCurrentBrushSettings() const;
    
    // ===== UNIVERSAL BRUSH SYSTEM DEBUG =====
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush Debug", CallInEditor)
    void TestUniversalBrushConnection();
    
    // ===== BRUSH SIZE/STRENGTH ADJUSTMENT FUNCTIONS =====
    
    void IncreaseBrushSize(const FInputActionValue& Value);
    void DecreaseBrushSize(const FInputActionValue& Value);
    void IncreaseBrushStrength(const FInputActionValue& Value);
    void DecreaseBrushStrength(const FInputActionValue& Value);

    
    void UpdateTerrainModification(float DeltaTime);
   

    // ===== DUPLICATE BRUSH PROPERTIES REMOVED =====
    // Brush properties now managed by Universal Brush System in MasterController
    // Use GetBrushRadius() and GetBrushStrength() instead

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

    // Brush Materials - SET THESE IN BLUEPRINT!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Materials")
    UMaterialInterface* TerrainEditMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Materials")
    UMaterialInterface* WaterAddMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Materials")
    UMaterialInterface* WaterRemoveMaterial = nullptr;

    // Legacy material support (deprecated - use above instead)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview", meta = (DeprecatedProperty, DeprecationMessage = "Use TerrainEditMaterial instead"))
    UMaterial* RaiseBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview", meta = (DeprecatedProperty, DeprecationMessage = "Use TerrainEditMaterial instead"))
    UMaterial* LowerBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview", meta = (DeprecatedProperty, DeprecationMessage = "Use WaterAddMaterial instead"))
    UMaterial* AddWaterBrushMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Preview", meta = (DeprecatedProperty, DeprecationMessage = "Use WaterRemoveMaterial instead"))
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
    void CycleCameraMode();  // Renamed from ReturnToOverhead
    
    // Simple camera system helper function
    UFUNCTION(BlueprintCallable, Category = "Camera")
    ECameraMode GetNextCameraMode() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    FVector GetCursorWorldPosition() const;

    // Master controller coordinate integration
    UFUNCTION(BlueprintCallable, Category = "Cursor System")
    FVector GetMasterCursorWorldPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Cursor System")
    float GetMasterBrushScale() const;

    UFUNCTION(BlueprintCallable, Category = "Cursor System")
    bool ValidateCursorPosition(FVector CursorPos) const;



    // Camera system helper functions
    void UpdateCameraTransition(float DeltaTime);
    void UpdateOverheadCamera(float DeltaTime);
    void UpdateFirstPersonCamera(float DeltaTime);
    FVector GetTerrainHeightAtCursor() const;
    
    // Terrain reset detection
    UFUNCTION(BlueprintCallable, Category = "Terrain Safety")
    void SetTerrainResetting(bool bResetting);
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Safety")
    bool IsTerrainResetting() const { return bTerrainResetting; }

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
  //  void IncreaseBrushSize(const FInputActionValue& Value);
  //  void DecreaseBrushSize(const FInputActionValue& Value);
  //  void IncreaseBrushStrength(const FInputActionValue& Value);
  //  void DecreaseBrushStrength(const FInputActionValue& Value);
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

    // Camera transition state - simplified
    bool bTransitioning = false;
    FVector TargetLocation;
    FRotator TargetRotation;
    ECameraMode TargetCameraMode = ECameraMode::Overhead; // Explicit target mode tracking
    float MinCameraHeight = 200.0f;
    float MaxCameraHeight = 10000.0f;

    // Chunk update priority system
    struct FChunkUpdateRequest
    {
        int32 ChunkIndex;
        float Priority;
        float RequestTime;
        
        bool operator<(const FChunkUpdateRequest& Other) const
        {
            return Priority > Other.Priority; // Higher priority first
        }
    };
    
    // Unified cursor system
    FVector UnifiedCursorPosition = FVector::ZeroVector;
    FVector PreviousCursorPosition = FVector::ZeroVector;
    float CursorUpdateTimer = 0.0f;
    float CursorUpdateRate = 0.033f; // 30fps cursor updates
    bool bUnifiedCursorValid = false;
    
    // Authority caching
    bool bMasterControllerValid = false;
    float AuthorityCacheTimer = 0.0f;
    float AuthorityCacheRate = 0.1f; // Check authority 10x per second
    
    // AUTHORITY: Initialization state
    bool bWaitingForMasterController = false;
    bool bInitializationComplete = false;
    
    // Missing member variables for compilation
    UPROPERTY()
    FUniversalBrushSettings CurrentBrushSettings;
    
    // ===== UNIVERSAL BRUSH SYSTEM INTEGRATION =====
    
    // Terrain editing constraints
    float MaxTerrainSlopeAngle = 60.0f; // Prevent vertical walls
    float TerrainSmoothingRadius = 200.0f; // Smooth sharp edges

    // Performance tracking
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    float StatUpdateTimer = 0.0f;

    // Brush preview - REMOVED: Now handled by Universal Brush System
    // void CreateBrushPreview();
    // void UpdateBrushPreview();

    // Helper functions
    void UpdateCameraPosition(float DeltaTime);
   // void UpdateTerrainModification(float DeltaTime);
    void UpdateWaterModification(float DeltaTime);
    void UpdateUnifiedCursor(float DeltaTime);
    void UpdateAuthorityCache(float DeltaTime);
    void ValidateFirstPersonHeight(float DeltaTime);
    void ResetInputs();
    void UpdateBrushPreview(float DeltaTime);  // Missing declaration
    
    // New optimization helper functions
    bool IsTerrainValid() const;
    float GetSafeTerrainHeight(const FVector& Position) const;
    
    // Fix race condition in height validation
    float FirstPersonValidationTimer = 0.0f;
    bool bTerrainResetting = false;

    void ApplyTerrainSmoothing(FVector Position, float Radius, float Strength);
    void RequestChunkUpdate(int32 ChunkIndex, float Priority);
    float CalculateChunkPriority(int32 ChunkIndex) const;
    void SetupInputMapping();
    bool PerformCursorTrace(FVector& OutHitLocation) const;
    
    // ===== UNIVERSAL BRUSH PREVIEW SYSTEM =====
    // These are now properly implemented in the Universal Brush System
    // No duplicate declarations
};
