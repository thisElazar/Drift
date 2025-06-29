// GamePreviewManager.h - Procedural Menu World Generation
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GamePreviewManager.generated.h"

// Forward declarations
class ADynamicTerrain;
class UAtmosphericSystem;
class UWaterSystem;

// Forward declare enums to avoid circular dependency
enum class EWorldSize : uint8;
enum class EDefaultTexture : uint8;

// Configuration structure for preview settings
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

    FMenuPreviewConfig() = default;
};

UENUM(BlueprintType)
enum class EDriftTimeMode : uint8
{
    Pulse   UMETA(DisplayName = "Pulse Mode"),
    Trace   UMETA(DisplayName = "Trace Mode"), 
    Drift   UMETA(DisplayName = "Drift Mode"),
    Epoch   UMETA(DisplayName = "Epoch Mode"),
    Myth    UMETA(DisplayName = "Myth Mode")
};

USTRUCT(BlueprintType)
struct TERRAI_API FGameSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWorldSize WorldSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDefaultTexture DefaultTexture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDriftTimeMode StartingTimeMode;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableWeather;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeAcceleration;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAdvancedAtmosphere;

    FGameSettings();
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAI_API UGamePreviewManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UGamePreviewManager();

    // Core Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    EWorldSize PreviewWorldSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview") 
    EDefaultTexture PreviewTexture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    EDriftTimeMode PreviewTimeMode = EDriftTimeMode::Pulse;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    bool bRainEnabled = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    float TimeScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    bool bUseCustomWorldSize = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    int32 CustomTerrainWidth = 257;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    int32 CustomTerrainHeight = 257;

    // System References
    UPROPERTY()
    ADynamicTerrain* PreviewTerrain = nullptr;
    
    UPROPERTY()
    UAtmosphericSystem* PreviewAtmosphere = nullptr;
    
    UPROPERTY()
    class AMasterWorldController* CachedMasterController = nullptr;

    // Live Updates
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void UpdateWorldSize(EWorldSize NewSize);
    
    UFUNCTION(BlueprintCallable, Category = "Preview") 
    void UpdateVisualMode(EDefaultTexture NewTexture);
    
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void UpdateTimeMode(EDriftTimeMode NewMode);
    
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void UpdateWeatherSettings(bool bEnableRain);
    
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void GenerateNewPreviewWorld();
    
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void ResetPreviewWorld();
    
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void RegenerateWithCurrentSettings();

    // Settings Transfer
    UFUNCTION(BlueprintCallable, Category = "Preview")
    FGameSettings GetCurrentSettings() const;
    
    // Real-time Blueprint properties with automatic C++ sync
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (CallInEditor = true))
    bool bEnableRotation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (CallInEditor = true))
    float RotationSpeed = 15.0f;
    
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
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

private:
    void InitializePreviewSystems();
    void GenerateProceduralTerrain();
    void ApplySettingsToTerrain();
    void UpdateVisualFeedback();
    
    // Visual feedback timers
    float TimeModePulseTimer = 0.0f;
    float WeatherEffectTimer = 0.0f;
    
    // Terrain generation functions
    void AddWaterFeatures();
    void SeedAtmosphericPatterns();
    
    // Visual feedback systems
    void ShowWorldSizeOverlay(EWorldSize Size);
    void TriggerTimeModeTransition(EDriftTimeMode Mode);
    void EnableFlowVisualization(bool bEnable);
    void EnableErosionPreview(bool bEnable);
    
    // Terrain ownership tracking
    bool bOwnsTerrain = false;
    
protected:
    virtual void BeginDestroy() override;
    
public:
};
