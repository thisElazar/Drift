// GPUTerrainController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "GPUTerrainController.generated.h"

// Forward declarations
class ADynamicTerrain;
class AWaterController;
class AAtmosphereController;

/**
 * GPU Terrain Controller - Orchestrates GPU terrain, water, and atmosphere
 * Manages the GPU compute pipeline for watershed simulation
 * Thread-safe implementation for UE 5.5
 * Includes integrated debugging and console commands
 */
UCLASS()
class TERRAI_API AGPUTerrainController : public AActor
{
    GENERATED_BODY()
    
public:
    AGPUTerrainController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "GPU Terrain")
    void InitializeGPUPipeline();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Terrain")
    void ConnectSystems(ADynamicTerrain* Terrain, AWaterController* Water, AAtmosphereController* Atmosphere);
    
    // ===== GPU PIPELINE CONTROL =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bEnableGPUPipeline = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bAutoSyncGPUCPU = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    float GPUSyncInterval = 0.1f; // Sync GPU->CPU every 100ms
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bEnableAtmosphereOnStart = false; // Whether to auto-enable atmosphere GPU
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bShowGPUStats = true;
    
    // ===== ATMOSPHERE GPU CONTROL =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere GPU", meta = (CallInEditor = "true"))
    void EnableAtmosphereGPU();
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere GPU", meta = (CallInEditor = "true"))
    void DisableAtmosphereGPU();
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
    bool IsAtmosphereGPUEnabled() const;
    
    // ===== EROSION PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float HydraulicErosionStrength = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ThermalErosionStrength = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion",
              meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ErosionTimeScale = 0.1f;
    
    // ===== OROGRAPHIC PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic",
              meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float OrographicLiftCoefficient = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AdiabatiCoolingRate = 0.6f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainShadowIntensity = 0.7f;
    
    // ===== GRID SYNCHRONIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "GPU Pipeline")
    void SynchronizeGridDimensions();
    
    // ===== CONSOLE COMMANDS =====
    // These are called via console commands like "gpu.EnableAtmosphereGPU"
    
    static void ConsoleEnableAtmosphereGPU();
    static void ConsoleDisableAtmosphereGPU();
    static void ConsoleCheckAtmosphereStatus();
    static void ConsoleInitAtmosphereResources();
    static void ConsolePipelineStatus();
    static void ConsoleQuickTest();
    static void ConsoleTestAtmosphereGenerate();
    
    // ===== DEBUG LOGGING =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugLogging = true;
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogDebugInfo(const FString& Category, const FString& Message, bool bError = false);
    
    // Static instance for console commands
    static AGPUTerrainController* ActiveInstance;
    
    
    // System references MOVED FROM PROTECTED FOR DEBUGGING
    UPROPERTY()
    ADynamicTerrain* TargetTerrain;
    
    UPROPERTY()
    AWaterController* WaterController;
    
    UPROPERTY()
    AAtmosphereController* AtmosphereController;
    
protected:

    
    // Pipeline state
    bool bGPUSystemsConnected = false;
    bool bPendingAtmosphereGPUEnable = false;
    
    // Timing
    float GPUSyncAccumulator = 0.0f;
    double LastGPUComputeTime = 0.0;
    int32 GPUDispatchCount = 0;
    
    // Timer handles
    FTimerHandle AtmosphereEnableTimer;
    

    // Internal methods
    void ExecuteGPUWatershedPipeline(float DeltaTime);
    void SynchronizeGPUSystems();
    void UpdateErosionParameters();
    void UpdateOrographicParameters();
    void UpdateOrographicFeedback(float DeltaTime);
    void DisplayGPUStats();
    
    // Deferred atmosphere enable
    void EnableAtmosphereGPUDeferred();
};
