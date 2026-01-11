#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RHI.h"
#include "RHIResources.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GPUComputeManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGPUConfigChanged, const struct FGPUComputeConfig&);

UENUM(BlueprintType)
enum class EGPUQualityTier : uint8
{
    Mobile      UMETA(DisplayName = "Mobile/Low Power"),
    Balanced    UMETA(DisplayName = "Steam Deck/Console"),
    Desktop     UMETA(DisplayName = "Desktop/High End"),
    Enthusiast  UMETA(DisplayName = "Enthusiast/4K")
};

USTRUCT(BlueprintType)
struct FGPUComputeConfig
{
    GENERATED_BODY()
    
    // Core capabilities
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    EGPUQualityTier QualityTier = EGPUQualityTier::Desktop;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    bool bSupportsCompute = true;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    bool bSupportsAsyncCompute = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    bool bSupports16BitOps = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    bool bSupportsWaveOps = false;
    
    // Resource limits
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 MaxTextureSize = 2048;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 MaxComputeThreadsPerGroup = 1024;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 MaxUAVs = 8;
    
    // Recommended grid sizes for different systems
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 SimulationGridSize = 512;  // For physics sims
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 RenderGridSize = 512;      // For visual effects
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 DetailGridSize = 128;      // For detail layers
    
    // Update rates
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    float PhysicsUpdateRate = 30.0f;  // Hz
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    float VisualUpdateRate = 60.0f;   // Hz
    
    // Memory budgets (MB)
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 TextureMemoryBudget = 256;
    
    UPROPERTY(BlueprintReadOnly, Category = "GPU")
    int32 ComputeMemoryBudget = 128;
};

/**
 * Singleton subsystem that manages GPU compute resources globally
 * Shared by atmosphere, water, erosion, and other GPU systems
 */
UCLASS(BlueprintType)
class DRIFT_API UGPUComputeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // Global singleton access
    UFUNCTION(BlueprintPure, Category = "GPU", meta = (DisplayName = "Get GPU Manager"))
    static UGPUComputeManager* Get(const UObject* WorldContext);
    
    // Configuration access
    UFUNCTION(BlueprintPure, Category = "GPU")
    const FGPUComputeConfig& GetGPUConfig() const { return CurrentConfig; }
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    bool IsGPUReady() const { return bGPUInitialized; }
    
    // Platform detection and configuration
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void DetectGPUCapabilities();
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void SetQualityTier(EGPUQualityTier NewTier);
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void AutoConfigureQuality();
    
    // System-specific grid sizes
    UFUNCTION(BlueprintPure, Category = "GPU")
    int32 GetAtmosphereGridSize() const;
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    int32 GetWaterGridSize() const;
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    int32 GetErosionGridSize() const;
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    int32 GetVegetationGridSize() const;
    
    // Shared texture pool management
    UFUNCTION(BlueprintCallable, Category = "GPU")
    UTextureRenderTarget2D* AllocateComputeTexture(
        const FString& Name,
        int32 Width,
        int32 Height,
        ETextureRenderTargetFormat Format,
        bool bUAVCompatible = true
    );
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void ReleaseComputeTexture(const FString& Name);
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void ReleaseAllTextures();
    
    // Performance monitoring
    UFUNCTION(BlueprintPure, Category = "GPU")
    float GetGPUFrameTime() const { return LastGPUFrameTime; }
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    float GetGPUMemoryUsageMB() const;
    
    UFUNCTION(BlueprintPure, Category = "GPU")
    bool IsGPUBottlenecked() const;
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void UpdatePerformanceMetrics();
    
    // Event for config changes
    FOnGPUConfigChanged OnGPUConfigChanged;
    
    // Console command registration
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    
protected:
    UPROPERTY()
    FGPUComputeConfig CurrentConfig;
    
    // Shared resource pool
    UPROPERTY()
    TMap<FString, UTextureRenderTarget2D*> TexturePool;
    
    // Performance tracking
    float LastGPUFrameTime = 0.0f;
    float GPUBudgetMS = 16.67f;  // Target 60fps
    float MemoryUsageMB = 0.0f;
    bool bGPUInitialized = false;
    
    // Console command handles
    TArray<IConsoleObject*> ConsoleCommands;
    
private:
    void DetectPlatformCapabilities();
    void ConfigureForTier(EGPUQualityTier Tier);
    void LogConfiguration() const;
    ETextureRenderTargetFormat GetOptimalFormat(bool bNeedsPrecision) const;
};
