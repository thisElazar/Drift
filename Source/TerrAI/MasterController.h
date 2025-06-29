#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TemporalManager.h"
#include "Curves/CurveFloat.h"
#include "TerrAIGameInstance.h"
#include "MasterController.generated.h"

// Forward declarations
class AWaterController;
class AAtmosphereController;
class AEcosystemController;
class AGeologyController;
class ADynamicTerrain;
class UCurveFloat;

// Use existing EWorldSize from TerrAIGameInstance.h

// ===== UNIVERSAL BRUSH SYSTEM =====

UENUM(BlueprintType)
enum class EBrushMode : uint8
{
    Add         UMETA(DisplayName = "Add/Raise"),
    Remove      UMETA(DisplayName = "Remove/Lower"), 
    Smooth      UMETA(DisplayName = "Smooth")
};

UENUM(BlueprintType)
enum class EBrushFalloffType : uint8
{
    Linear      UMETA(DisplayName = "Linear"),
    Smooth      UMETA(DisplayName = "Smooth (Smoothstep)"),
    Gaussian    UMETA(DisplayName = "Gaussian"),
    Exponential UMETA(DisplayName = "Exponential"),
    Custom      UMETA(DisplayName = "Custom Curve")
};

USTRUCT(BlueprintType)
struct TERRAI_API FUniversalBrushSettings
{
    GENERATED_BODY()
    
    // ===== CORE BRUSH PARAMETERS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush")
    EBrushMode BrushMode = EBrushMode::Add;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush",
              meta = (ClampMin = "10.0", ClampMax = "5000.0", ToolTip = "Brush radius in world units"))
    float BrushRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush",
              meta = (ClampMin = "1.0", ClampMax = "10000.0", ToolTip = "Brush modification strength"))
    float BrushStrength = 200.0f;
    
    // ===== FALLOFF SYSTEM =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falloff")
    EBrushFalloffType FalloffType = EBrushFalloffType::Smooth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falloff",
              meta = (ClampMin = "0.0", ClampMax = "0.9", ToolTip = "Inner radius (full strength) as ratio of brush radius"))
    float InnerRadius = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falloff",
              meta = (ClampMin = "0.1", ClampMax = "1.0", ToolTip = "Outer radius (zero strength) as ratio of brush radius"))
    float OuterRadius = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falloff",
              meta = (ClampMin = "0.1", ClampMax = "5.0", ToolTip = "Falloff curve exponent (higher = sharper)"))
    float FalloffExponent = 2.0f;
    
    // ===== VISUAL FEEDBACK =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor BrushColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowPreview = true;
    
    FUniversalBrushSettings()
    {
        BrushMode = EBrushMode::Add;
        BrushRadius = 500.0f;
        BrushStrength = 200.0f;
        FalloffType = EBrushFalloffType::Smooth;
        InnerRadius = 0.2f;
        OuterRadius = 1.0f;
        FalloffExponent = 2.0f;
        BrushColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
        bShowPreview = true;
    }
    
    // Assignment operator
    FUniversalBrushSettings& operator=(const FUniversalBrushSettings& Other)
    {
        if (this != &Other)
        {
            BrushMode = Other.BrushMode;
            BrushRadius = Other.BrushRadius;
            BrushStrength = Other.BrushStrength;
            FalloffType = Other.FalloffType;
            InnerRadius = Other.InnerRadius;
            OuterRadius = Other.OuterRadius;
            FalloffExponent = Other.FalloffExponent;
            BrushColor = Other.BrushColor;
            bShowPreview = Other.bShowPreview;
        }
        return *this;
    }
};

// ===== BRUSH RECEIVER INTERFACE =====

UINTERFACE(BlueprintType)
class TERRAI_API UBrushReceiver : public UInterface
{
    GENERATED_BODY()
};

class TERRAI_API IBrushReceiver
{
    GENERATED_BODY()
    
public:
    virtual void ApplyBrush(FVector WorldPosition, const FUniversalBrushSettings& Settings, float DeltaTime) = 0;
    virtual void UpdateBrushSettings(const FUniversalBrushSettings& Settings) = 0;
    virtual bool CanReceiveBrush() const = 0;
};

/**
 * IScalableSystem interface for master controller coordination
 */
UINTERFACE(BlueprintType)
class TERRAI_API UScalableSystem : public UInterface
{
    GENERATED_BODY()
};

class TERRAI_API IScalableSystem
{
    GENERATED_BODY()
    
public:
    virtual void ConfigureFromMaster(const struct FWorldScalingConfig& Config) = 0;
    virtual void SynchronizeCoordinates(const struct FWorldCoordinateSystem& Coords) = 0;
    virtual bool IsSystemScaled() const = 0;
};

/**
 * Water system scaling configuration
 */
USTRUCT(BlueprintType)
struct TERRAI_API FWaterSystemScaling
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SimulationArrayWidth = 513;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SimulationArrayHeight = 513;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterCellScale = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OptimalChunkSize = 32;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinateScale = 1.0f;
    
    FWaterSystemScaling()
    {
        SimulationArrayWidth = 513;
        SimulationArrayHeight = 513;
        WaterCellScale = 100.0f;
        OptimalChunkSize = 32;
        CoordinateScale = 1.0f;
    }
};


/**
 * Atmospheric system scaling configuration
 */
USTRUCT(BlueprintType)
struct TERRAI_API FAtmosphericSystemScaling
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridWidth = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridHeight = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridLayers = 12;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinateScale = 1.0f;
    
    FAtmosphericSystemScaling()
    {
        GridWidth = 64;
        GridHeight = 64;
        GridLayers = 12;
        CellSize = 1000.0f;
        CoordinateScale = 1.0f;
    }
};

/**
 * Geology system scaling configuration
 */
USTRUCT(BlueprintType)
struct TERRAI_API FGeologySystemScaling
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ErosionGridWidth = 128;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ErosionGridHeight = 128;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ErosionCellSize = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinateScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GeologicalTimeScale = 3600.0f;
    
    FGeologySystemScaling()
    {
        ErosionGridWidth = 128;
        ErosionGridHeight = 128;
        ErosionCellSize = 400.0f;
        CoordinateScale = 1.0f;
        GeologicalTimeScale = 3600.0f;
    }
};

/**
 * Ecosystem system scaling configuration
 */
USTRUCT(BlueprintType)
struct TERRAI_API FEcosystemSystemScaling
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BiomeGridWidth = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BiomeGridHeight = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BiomeCellSize = 800.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxVegetationInstances = 10000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensityScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinateScale = 1.0f;
    
    FEcosystemSystemScaling()
    {
        BiomeGridWidth = 64;
        BiomeGridHeight = 64;
        BiomeCellSize = 800.0f;
        MaxVegetationInstances = 10000;
        VegetationDensityScale = 1.0f;
        CoordinateScale = 1.0f;
    }
};

/**
 * World coordinate system from master controller
 */
USTRUCT(BlueprintType)
struct TERRAI_API FWorldCoordinateSystem
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldOrigin = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorldScale = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D TerrainBounds = FVector2D(51300, 51300);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalChunks = 256;
    
    FWorldCoordinateSystem()
    {
        WorldOrigin = FVector::ZeroVector;
        WorldScale = 100.0f;
        TerrainBounds = FVector2D(51300, 51300);
        TotalChunks = 256;
    }
};

/**
 * World scaling configuration
 */
USTRUCT(BlueprintType)
struct TERRAI_API FWorldScalingConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TerrainWidth = 513;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TerrainHeight = 513;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainScale = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OptimalChunkSize = 32;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FWaterSystemScaling WaterConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAtmosphericSystemScaling AtmosphericConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGeologySystemScaling GeologyConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEcosystemSystemScaling EcosystemConfig;
    
    // ===== BRUSH AUTHORITY =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushStrength = 200.0f;
    
    FWorldScalingConfig()
    {
        TerrainWidth = 513;
        TerrainHeight = 513;
        TerrainScale = 100.0f;
        OptimalChunkSize = 32;
        BrushRadius = 500.0f;
        BrushStrength = 200.0f;
        WaterConfig = FWaterSystemScaling();
        AtmosphericConfig = FAtmosphericSystemScaling();
        GeologyConfig = FGeologySystemScaling();
        EcosystemConfig = FEcosystemSystemScaling();
    }
};

/**
 * AMasterWorldController - Central orchestrator for all world systems
 */

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AMasterWorldController : public AActor

{
    GENERATED_BODY()

public:
    AMasterWorldController();
protected:
    // ===== ACTOR LIFECYCLE =====
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // ===== INITIALIZATION PHASES =====
    void InitializeTemporalManager();
    void InitializeSystemControllers();
    void RegisterSystemsWithTemporalManager();
    void EstablishSystemConnections();
    void FinalizeInitialization();
    
    // ===== SYSTEM COORDINATION =====
    void UpdateSystemsWithTiming(float DeltaTime);
    void ProcessSystemDataExchange();
    void MonitorAndOptimizePerformance(float DeltaTime);
    
    // ===== ERROR HANDLING =====
    void HandleSystemInitializationError(const FString& SystemName, const FString& ErrorMessage);
    bool AttemptSystemRecovery(const FString& SystemName);

private:
    // ===== WORLD SCALING STATE =====
    
    // Current world scaling configuration
    FWorldScalingConfig WorldScalingConfig;
    FWorldCoordinateSystem WorldCoordinateSystem;
    
    // Registered scalable systems
    TArray<class IScalableSystem*> RegisteredScalableSystems;
    
    // Scaling coordination helpers
    void CalculateOptimalWorldScaling();
    void ConfigureSystemBounds();
    FWorldScalingConfig GenerateScalingConfig() const;
    FWorldCoordinateSystem GenerateCoordinateSystem() const;
    
    // ===== INITIALIZATION STATE =====
    enum class EInitializationPhase : uint8
    {
        NotStarted,
        MasterController,
        TemporalManager,
        SystemControllers,
        TemporalRegistration,
        SystemConnections,
        Validation,
        Complete,
        Failed
    };
    
    EInitializationPhase CurrentInitPhase = EInitializationPhase::NotStarted;
    
    UPROPERTY()
    TArray<FString> InitializationErrors;
    
    // ===== PERFORMANCE TRACKING =====
    struct FPerformanceMetrics
    {
        float AverageFrameTime = 0.0f;
        float PeakFrameTime = 0.0f;
        int32 FrameSampleCount = 0;
        float LastPerformanceCheck = 0.0f;
        
        TMap<ESystemType, float> SystemUpdateTimes;
        TMap<ESystemType, int32> SystemUpdateCounts;
    };
    
    FPerformanceMetrics PerformanceMetrics;
    
    // ===== TEMPORAL COORDINATION STATE =====
    struct FSystemUpdateState
    {
        bool bNeedsUpdate = false;
        float LastUpdateTime = 0.0f;
        float UpdateFrequency = 1.0f;
        int32 UpdatePriority = 0;
    };
    
    TMap<ESystemType, FSystemUpdateState> SystemUpdateStates;
    
    // ===== INTERNAL HELPERS =====
    void FindOrCreateSystemControllers();
    void ConfigureSystemTemporalSettings();
    void UpdatePerformanceMetrics(float DeltaTime);
    bool ShouldOptimizePerformance() const;
    void ApplyPerformanceOptimizations();
    bool ValidateSystemReferences() const;
    void LogSystemStatus() const;
    
    // ===== CONSTANTS =====
    static constexpr float PERFORMANCE_CHECK_INTERVAL = 1.0f;
    static constexpr float TARGET_FRAME_TIME = 1.0f / 60.0f;
    static constexpr int32 PERFORMANCE_SAMPLE_SIZE = 60;

public:
    // ===== EVENTS & DELEGATES =====
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemInitialized, FString, SystemName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemError, FString, SystemName, FString, ErrorMessage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChange, bool, bOptimalPerformance);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTemporalStateChanged, float, GlobalTimeAcceleration);
    
    UPROPERTY(BlueprintAssignable, Category = "Master Controller Events")
    FOnSystemInitialized OnSystemInitialized;
    
    UPROPERTY(BlueprintAssignable, Category = "Master Controller Events")
    FOnSystemError OnSystemError;
    
    UPROPERTY(BlueprintAssignable, Category = "Master Controller Events")
    FOnPerformanceChange OnPerformanceChange;
    
    UPROPERTY(BlueprintAssignable, Category = "Master Controller Events")
    FOnTemporalStateChanged OnTemporalStateChanged;
    
    // ===== ADVANCED SYSTEM CONTROLS =====
    UFUNCTION(BlueprintCallable, Category = "Advanced Control")
    void SetSystemEnabled(ESystemType SystemType, bool bEnabled);
    
    UFUNCTION(BlueprintPure, Category = "Advanced Control")
    bool IsSystemEnabled(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Advanced Control")
    void SetSystemUpdatePriority(ESystemType SystemType, int32 Priority);
    
    UFUNCTION(BlueprintCallable, Category = "Advanced Control")
    void ForceSynchronization();
    
    UFUNCTION(BlueprintCallable, Category = "Advanced Control")
    FString CreateWorldSnapshot() const;
    
    UFUNCTION(BlueprintCallable, Category = "Advanced Control")
    bool RestoreWorldFromSnapshot(const FString& SnapshotData);




    // ===== SYSTEM CONTROLLERS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AWaterController* WaterController = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers") 
    AAtmosphereController* AtmosphereController = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AEcosystemController* EcosystemController = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AGeologyController* GeologyController = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems")
    ADynamicTerrain* MainTerrain = nullptr;
    
    // ===== WORLD SCALING INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void ConfigureWorldScaling();
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void RegisterScalableSystems();
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    bool AreAllSystemsScaled() const;
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void SynchronizeSystemCoordinates();
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    FString GetWorldScalingDebugInfo() const;
    
    // ===== WORLD SCALING TEST FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void TestWorldScalingIntegration();
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void LogAllSystemScalingStatus();
    
    // ===== COORDINATE TRANSFORM FUNCTIONS - AUTHORITATIVE =====
    
    UFUNCTION(BlueprintPure, Category = "Coordinate Transforms")
    FVector2D WorldToTerrainCoordinates(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintPure, Category = "Coordinate Transforms")
    FVector TerrainToWorldPosition(FVector2D TerrainPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Coordinate Transforms")
    float GetBrushScaleForCurrentWorld() const;
    
    UFUNCTION(BlueprintPure, Category = "Coordinate Transforms")
    float GetBrushScaleMultiplier() const;
    
    UFUNCTION(BlueprintCallable, Category = "Coordinate Transforms")
    bool ValidateWorldPosition(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintPure, Category = "Coordinate Transforms")
    FVector2D GetWorldBounds() const;
    
    // ===== SINGLE SOURCE OF TRUTH FUNCTIONS =====
    
    UFUNCTION(BlueprintPure, Category = "World Authority")
    FVector2D GetWorldDimensions() const;
    
    UFUNCTION(BlueprintPure, Category = "World Authority") 
    int32 GetOptimalChunkSize() const;
    
    UFUNCTION(BlueprintPure, Category = "World Authority")
    FVector2D GetChunkDimensions() const;
    
    UFUNCTION(BlueprintPure, Category = "World Authority")
    int32 GetWaterGridSize() const;
    
    UFUNCTION(BlueprintPure, Category = "World Authority")
    float GetTerrainScale() const;
    
    UFUNCTION(BlueprintCallable, Category = "World Authority")
    void SetWorldDimensions(int32 Width, int32 Height);
    
    UFUNCTION(BlueprintCallable, Category = "World Authority")
    void PropagateWorldChanges();
    
    // ===== WORLD SIZE MANAGEMENT =====
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Management")
    EWorldSize CurrentWorldSize = EWorldSize::Medium;
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void SetWorldSizeFromUI(EWorldSize NewSize);
    
    UFUNCTION(BlueprintPure, Category = "World Management")
    EWorldSize GetCurrentWorldSize() const { return CurrentWorldSize; }
    
    // Direct UI functions for Blueprint buttons
    UFUNCTION(BlueprintCallable, Category = "World Management", CallInEditor)
    void SetSmallWorld() { SetWorldSizeFromUI(EWorldSize::Small); }
    
    UFUNCTION(BlueprintCallable, Category = "World Management", CallInEditor)
    void SetMediumWorld() { SetWorldSizeFromUI(EWorldSize::Medium); }
    
    UFUNCTION(BlueprintCallable, Category = "World Management", CallInEditor)
    void SetLargeWorld() { SetWorldSizeFromUI(EWorldSize::Large); }
    
    UFUNCTION(BlueprintPure, Category = "World Management")
    FString GetWorldSizeDisplayName() const;
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void ApplyGameInstanceSettings();
    
    // ===== UNIVERSAL BRUSH AUTHORITY =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Brush", 
              meta = (ShowOnlyInnerProperties))
    FUniversalBrushSettings UniversalBrushSettings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Brush")
    TObjectPtr<UCurveFloat> CustomBrushCurve = nullptr;
    
    // ===== BRUSH AUTHORITY FUNCTIONS =====
    
    UFUNCTION(BlueprintPure, Category = "Universal Brush")
    const FUniversalBrushSettings& GetUniversalBrushSettings() const { return UniversalBrushSettings; }
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void UpdateUniversalBrushSettings(const FUniversalBrushSettings& NewSettings);
    
    UFUNCTION(BlueprintPure, Category = "Universal Brush")
    float GetBrushRadius() const { return UniversalBrushSettings.BrushRadius; }
    
    UFUNCTION(BlueprintPure, Category = "Universal Brush")
    float GetBrushStrength() const { return UniversalBrushSettings.BrushStrength; }
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SetBrushRadius(float NewRadius);
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SetBrushStrength(float NewStrength);
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SetBrushFalloffType(EBrushFalloffType NewType);
    
    // ===== BRUSH CALCULATION FUNCTIONS =====
    
    UFUNCTION(BlueprintPure, Category = "Universal Brush")
    float CalculateBrushFalloff(float Distance, const FUniversalBrushSettings& Settings) const;
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void ApplyBrushToReceivers(FVector WorldPosition, float DeltaTime);
    
    // ===== BRUSH RECEIVERS MANAGEMENT =====
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void RegisterBrushReceiver(TScriptInterface<IBrushReceiver> Receiver);
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void UnregisterBrushReceiver(TScriptInterface<IBrushReceiver> Receiver);
    
    // ===== MATERIAL PARAMETER SYNC =====
    
    UFUNCTION(BlueprintCallable, Category = "Universal Brush")
    void SyncBrushToMaterials();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Brush")
    UMaterialParameterCollection* BrushParameterCollection = nullptr;
    
    // ===== BRUSH SCALING FUNCTIONS =====
    
    UFUNCTION(BlueprintPure, Category = "Brush Scaling")
    float GetScaledBrushRadius(float BaseBrushRadius) const;
    
    UFUNCTION(BlueprintPure, Category = "Brush Scaling")
    FUniversalBrushSettings GetScaledBrushSettings() const;
    
    // ===== SINGLE SOURCE OF TRUTH VALIDATION =====
    
    UFUNCTION(BlueprintCallable, Category = "World Authority")
    bool ValidateWorldAuthority() const;
    
    UFUNCTION(BlueprintCallable, Category = "World Authority")
    FString GetAuthorityDebugInfo() const;
    
    // ===== TEMPORAL MANAGEMENT =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Temporal Management")
    UTemporalManager* TemporalManager = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal Management")
    bool bEnableUnifiedTiming = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal Management")
    float GlobalTimeAcceleration = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal Management")
    bool bPauseSimulation = false;
    
    // ===== DEBUG & PERFORMANCE SETTINGS =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bAdaptiveQuality = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowSystemDebugInfo = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bLogSystemUpdates = false;
    
    UFUNCTION(BlueprintCallable, Category = "Water System Debug")
    bool ValidateWaterSystemIntegration() const;
    
    // ===== FUNCTIONS =====
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void InitializeWorld();
    
    UFUNCTION(BlueprintCallable, Category = "Temporal Control")
    void SetGlobalTimeAcceleration(float Acceleration);
    
    UFUNCTION(BlueprintCallable, Category = "Temporal Control")
    void PauseAllSystems(bool bPause);
    
    UFUNCTION(BlueprintPure, Category = "Temporal Control")
    float GetSystemTime(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintPure, Category = "System Status")
    bool AreAllSystemsInitialized() const;
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawSystemDebugInfo();
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogSystemPerformance();
    
    // ===== ADDITIONAL HELPER FUNCTIONS =====
    UFUNCTION(BlueprintCallable, Category = "Temporal Control")
    void ResetTemporalState();
    
    UFUNCTION(BlueprintPure, Category = "System Status")
    bool IsSystemRegistered(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetTemporalStats() const;
    
    // ===== INITIALIZATION STATUS =====
    UFUNCTION(BlueprintPure, Category = "System Status")
    bool IsInitializationComplete() const { return CurrentInitPhase == EInitializationPhase::Complete; }
    
private:
    // ===== MISSING HELPER FUNCTIONS =====
    bool IsPerformanceOptimal() const;
    void SynchronizeAllSystems();
    void UpdateSystemConnections();
    
    // ===== INITIALIZATION FIX =====
    void DelayedInitializeWorld();
    
    // ===== UNIVERSAL BRUSH SYSTEM PRIVATE =====
    
    // Registered brush receivers
    UPROPERTY()
    TArray<TScriptInterface<IBrushReceiver>> RegisteredBrushReceivers;
    
    // Brush performance tracking
    mutable float LastBrushApplicationTime = 0.0f;
    mutable int32 LastBrushCellsProcessed = 0;
    
    // ===== PHASE 1: AUTHORITY ESTABLISHMENT =====
    
    // Find or create terrain with immediate initialization
    ADynamicTerrain* FindOrCreateTerrain();
    
    // Initialize TerrainController with authority
    void InitializeTerrainController();
    
    // Sequential system initialization (no retry logic)
    void InitializeSystemControllersSequentially();

};

