// GPUTerrainController.h - COMPREHENSIVE FIX
// Adds proper validation to prevent "disappearing controller" issue

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
 *
 * CRITICAL FIX: Now includes proper pointer validation to prevent
 * the "disappearing controller" bug that stops atmosphere compute
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
    
    // ===== VALIDATION (NEW) =====
    
    /**
     * Validates that all system references are valid UE objects
     * Uses IsValid() to catch garbage-collected or destroyed actors
     * @return true if all systems are valid and connected
     */
    UFUNCTION(BlueprintPure, Category = "GPU Terrain")
    bool ValidateSystemReferences() const;
    
    /**
     * Returns detailed validation status for debugging
     */
    UFUNCTION(BlueprintCallable, Category = "GPU Terrain")
    FString GetValidationStatus() const;
    
    // ===== GPU PIPELINE CONTROL =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bEnableGPUPipeline = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bAutoSyncGPUCPU = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    float GPUSyncInterval = 0.1f; // Sync GPU->CPU every 100ms
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bEnableAtmosphereOnStart = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bShowGPUStats = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Pipeline")
    bool bEnableDebugLogging = false;
    
    // ===== ATMOSPHERE CONTROL =====
    
    UFUNCTION(BlueprintCallable, Category = "GPU Atmosphere")
    void EnableAtmosphereGPU();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Atmosphere")
    void DisableAtmosphereGPU();
    
    UFUNCTION(BlueprintPure, Category = "GPU Atmosphere")
    bool IsAtmosphereGPUEnabled() const;
    
    void EnableAtmosphereGPUDeferred();
    
    // ===== OROGRAPHIC FEEDBACK =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic")
    float HydraulicErosionStrength = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic")
    float OrographicLiftCoefficient = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic")
    float AdiabaticCoolingRate = 0.65f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orographic")
    float RainShadowIntensity = 0.8f;
    
    void UpdateOrographicFeedback(float DeltaTime);
    void UpdateOrographicParameters();
    void UpdateErosionParameters();
    
    // ===== SYSTEM SYNCHRONIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "GPU Terrain")
    void SynchronizeGridDimensions();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Terrain")
    void SynchronizeGPUSystems();
    
    // ===== DEBUG & VISUALIZATION =====
    
    void DisplayGPUStats();
    void LogDebugInfo(const FString& Category, const FString& Message, bool bError = false);
    
    // ===== CONSOLE COMMANDS =====
    
    static void ConsoleEnableAtmosphereGPU();
    static void ConsoleDisableAtmosphereGPU();
    static void ConsoleCheckAtmosphereStatus();
    static void ConsolePipelineStatus();
    
    // Additional stubs for compatibility (implement if needed)
    static void ConsoleInitAtmosphereResources() {}
    static void ConsoleQuickTest() {}
    static void ConsoleTestAtmosphereGenerate() {}
    
    // ===== STATIC INSTANCE (for console commands) =====
    
    /**
     * Static pointer to active controller instance for console commands
     * IMPORTANT: Always validated with IsValid() before use
     */
    static AGPUTerrainController* ActiveInstance;
    
private:
    // ===== SYSTEM REFERENCES =====
    // CRITICAL: These are validated every frame with IsValid()
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    AWaterController* WaterController = nullptr;
    
    UPROPERTY()
    AAtmosphereController* AtmosphereController = nullptr;
    
    // ===== STATE FLAGS =====
    
    bool bGPUSystemsConnected = false;
    bool bPendingAtmosphereGPUEnable = false;
    
    // ===== TIMING & PERFORMANCE =====
    
    float GPUSyncAccumulator = 0.0f;
    double LastGPUComputeTime = 0.0;
    int32 GPUDispatchCount = 0;
    
    FTimerHandle AtmosphereEnableTimer;
    
    // ===== INTERNAL METHODS =====
    
    void ExecuteGPUWatershedPipeline(float DeltaTime);
};
