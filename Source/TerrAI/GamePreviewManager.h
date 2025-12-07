// GamePreviewManager.h - Menu World Preview with Authority Pattern
// CLEANED: November 2025 - Removed legacy patterns, pure authority delegation
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerrAIGameInstance.h"
#include "GamePreviewManager.generated.h"

// Forward declarations
class ADynamicTerrain;
class UAtmosphericSystem;
class UWaterSystem;
class AMasterWorldController;
class AGeologyController;

// ============================================================================
// CONFIGURATION STRUCTURES
// ============================================================================

/**
 * Configuration structure for menu preview behavior
 * Centralizes all timing and performance parameters
 */
USTRUCT(BlueprintType)
struct TERRAI_API FMenuPreviewConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Positioning")
    FVector PreviewOffset = FVector(-6000.0f, -6000.0f, -500.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MaterialLoadDelay = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float WaterFeatureDelay = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float RegenerationDebounce = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MenuMaxUpdatesPerFrame = 8;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensitivity")
    float HeightVariationThreshold = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensitivity")
    float NoiseScaleThreshold = 0.0005f;
    
    // Spring system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Springs")
    float InitialGroundwaterVolume = 100000000000000.0f; // m³ - enough for sustained preview
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Springs")
    int32 NumberOfSprings = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Springs")
    float MinSpringFlow = 15000.0f; // m³/s - weakest spring
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Springs")
    float MaxSpringFlow = 3333333.0f; // m³/s - strongest spring

    FMenuPreviewConfig() = default;
};

/**
 * Time mode for preview world (Pulse/Trace/Drift)
 */
UENUM(BlueprintType)
enum class EDriftTimeMode : uint8
{
    Pulse   UMETA(DisplayName = "Pulse Mode"),
    Trace   UMETA(DisplayName = "Trace Mode"),
    Drift   UMETA(DisplayName = "Drift Mode"),
};

/**
 * Game settings bundle for transition to main game
 */
USTRUCT(BlueprintType)
struct TERRAI_API FGameSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EWorldSize WorldSize;
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EDefaultTexture DefaultTexture;
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EDriftTimeMode StartingTimeMode;
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    bool bEnableWeather;
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    float TimeAcceleration;
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    bool bAdvancedAtmosphere;

    FGameSettings();
};

// ============================================================================
// GAME PREVIEW MANAGER - AUTHORITY-BASED MENU SYSTEM
// ============================================================================
/**
 * PURPOSE:
 * Manages interactive terrain preview in main menu with proper authority delegation.
 *
 * ARCHITECTURE PATTERN: Authority Delegation
 * - BeginPlay: Waits for MasterController authority
 * - InitializeWithAuthority: Called after MasterController completes 8-phase init
 * - Does NOT spawn its own systems - uses level-placed MasterController/Terrain
 *
 * MODERNIZATION (November 2025 - CLEANED):
 * - Replaced manual spawning with authority pattern
 * - Eliminated timer-based initialization soup
 * - Removed stub functions and deprecated ownership tracking
 * - Aligned with AtmosphereController/TerrainController patterns
 * - Proper dependency guarantees through authority chain
 *
 * RESPONSIBILITIES:
 * - Generate procedural preview terrain for menu
 * - Apply visual settings (texture, weather effects)
 * - Handle UI-triggered setting changes
 * - Provide settings bundle for game transition
 * - Manage preview-specific optimizations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAI_API UGamePreviewManager : public UActorComponent
{
    GENERATED_BODY()

public:
    // ========================================================================
    // SECTION 1: CONSTRUCTION & LIFECYCLE
    // ========================================================================
    
    UGamePreviewManager();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // SECTION 2: AUTHORITY-BASED INITIALIZATION ⭐
    // ========================================================================
    
    /**
     * Initialize with MasterController authority
     * Called after MasterController completes its 8-phase initialization
     *
     * @param Master - Authority reference to MasterController
     * @param Terrain - Terrain reference from authority (NOT owned by GamePreviewManager)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void InitializeWithAuthority(AMasterWorldController* Master, ADynamicTerrain* Terrain);

    // ========================================================================
    // SECTION 3: UI-TRIGGERED ACTIONS
    // ========================================================================
    
    /**
     * Update world size setting (triggered from menu UI)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void UpdateWorldSize(EWorldSize NewSize);
    
    /**
     * Update visual texture mode (triggered from menu UI)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void UpdateVisualMode(EDefaultTexture NewTexture);
    
    /**
     * Update time mode setting (triggered from menu UI)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void UpdateTimeMode(EDriftTimeMode NewMode);
    
    /**
     * Update weather settings (triggered from menu UI)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void UpdateWeatherSettings(bool bEnableRain);
    
    /**
     * Generate completely new random preview world
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void GenerateNewPreviewWorld();
    
    /**
     * Reset preview world to default parameters
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void ResetPreviewWorld();
    
    /**
     * Regenerate preview with current settings (UI refresh button)
     */
    UFUNCTION(BlueprintCallable, Category = "Menu Preview")
    void RegenerateWithCurrentSettings();
    
    /**
     * Get current settings bundle for game transition
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Menu Preview")
    FGameSettings GetCurrentSettings() const;

    // ========================================================================
    // SECTION 4: PUBLIC PROPERTIES
    // ========================================================================
    
    // Current preview settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview State")
    EWorldSize PreviewWorldSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview State")
    EDefaultTexture PreviewTexture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview State")
    EDriftTimeMode PreviewTimeMode;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview State")
    bool bRainEnabled = true;
    
    // Time scaling for preview
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Animation")
    float TimeScale = 1.0f;
    
    // Camera rotation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Animation", meta = (CallInEditor = true))
    bool bEnableRotation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Animation", meta = (CallInEditor = true))
    float RotationSpeed = 15.0f;
    
    // Procedural generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural", meta = (CallInEditor = true))
    int32 PreviewChunkSize = 8;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural", meta = (CallInEditor = true))
    float HeightVariation = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural", meta = (CallInEditor = true))
    float NoiseScale = 0.01f;
    
    // Configuration for preview behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Configuration")
    FMenuPreviewConfig PreviewConfig;

protected:
    // ========================================================================
    // SECTION 5: INTERNAL SYSTEMS
    // ========================================================================
    
    /**
     * Generate procedural terrain through authority
     */
    void GenerateProceduralTerrain();
    
    /**
     * Apply material/texture settings to terrain
     */
    void ApplySettingsToTerrain();
    
    /**
     * Update visual feedback (rotation, effects)
     */
    void UpdateVisualFeedback();
    
    /**
     * Add water features to preview terrain
     */
    void AddWaterFeatures();
    
    /**
     * Seed atmospheric patterns
     */
    void SeedAtmosphericPatterns();

private:
    // ========================================================================
    // SECTION 6: AUTHORITY REFERENCES
    // ========================================================================
    
    /**
     * Authority reference to MasterController
     * Set via InitializeWithAuthority()
     */
    UPROPERTY()
    AMasterWorldController* CachedMasterController = nullptr;
    
    /**
     * Terrain reference from authority
     * NOT owned by GamePreviewManager - authority manages lifecycle
     */
    UPROPERTY()
    ADynamicTerrain* PreviewTerrain = nullptr;
    
    /**
     * Atmospheric system reference (from terrain)
     */
    UPROPERTY()
    UAtmosphericSystem* PreviewAtmosphere = nullptr;
    
    // ========================================================================
    // SECTION 7: STATE FLAGS
    // ========================================================================
    
    /**
     * Initialization state flag
     * Prevents operations before authority setup
     */
    bool bInitializedWithAuthority = false;
    
    // Visual feedback timers
    float TimeModePulseTimer = 0.0f;
    float WeatherEffectTimer = 0.0f;
};
