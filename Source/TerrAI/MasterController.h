/**
 * ============================================
 * TERRAI MASTER WORLD CONTROLLER
 * ============================================
 * Purpose: Central orchestrator for all world systems
 * Benefits: Single initialization point, centralized save/load, cross-system coordination
 * Architecture: Manages controllers, not direct systems
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "MasterController.generated.h"

// Forward declarations
class AWaterController;
class AAtmosphereController;
class AEcosystemController;
class AGeologyController;
class ADynamicTerrain;
class ATerrainController;

UENUM(BlueprintType)
enum class EWorldState : uint8
{
    Uninitialized    UMETA(DisplayName = "Uninitialized"),
    Initializing     UMETA(DisplayName = "Initializing"),
    Running          UMETA(DisplayName = "Running"),
    Paused           UMETA(DisplayName = "Paused"),
    Saving           UMETA(DisplayName = "Saving"),
    Loading          UMETA(DisplayName = "Loading"),
    Error            UMETA(DisplayName = "Error")
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AMasterWorldController : public AActor
{
    GENERATED_BODY()

public:
    AMasterWorldController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== SYSTEM CONTROLLERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AWaterController* WaterController;
    
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
     AAtmosphereController* AtmosphereController;  
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AEcosystemController* EcosystemController;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controllers")
    AGeologyController* GeologyController;
    
    // Core system references (not owned, just coordinated)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems")
    ADynamicTerrain* MainTerrain;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems")
    ATerrainController* PlayerController;

    // ===== WORLD STATE =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World State")
    EWorldState CurrentWorldState = EWorldState::Uninitialized;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    bool bAutoInitializeOnBeginPlay = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float WorldTimeScale = 1.0f;

    // ===== MASTER CONTROLS =====
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void InitializeWorld();
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void ShutdownWorld();
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    bool SaveWorldState(const FString& SaveName);
    
    UFUNCTION(BlueprintCallable, Category = "World Management")
    bool LoadWorldState(const FString& SaveName);

    // ===== SYSTEM COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void SynchronizeAllSystems();
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void PauseAllSystems();
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void ResumeAllSystems();
    
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void SetWorldTimeScale(float NewTimeScale);

    // ===== PERFORMANCE MANAGEMENT =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQuality = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFrameRate = 0.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentPerformance();
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityPreset(int32 QualityLevel);

    // ===== CROSS-SYSTEM EVENTS =====
    
    UFUNCTION(BlueprintCallable, Category = "System Events")
    void OnSeasonChanged(float SeasonValue);
    
    UFUNCTION(BlueprintCallable, Category = "System Events")
    void OnWeatherChanged(float Temperature, float Humidity, float WindSpeed);
    
    UFUNCTION(BlueprintCallable, Category = "System Events")
    void OnTerrainModified(FVector Location, float Radius);

    // ===== DEBUG & MONITORING =====
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintSystemStatus() const;
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableDebugVisualization(bool bEnable);
    
    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetDetailedSystemReport() const;

private:
    // ===== INTERNAL FUNCTIONS =====
    
    void AutoDiscoverSystems();
    void ValidateSystemReferences();
    bool InitializeController(AActor* Controller, const FString& ControllerName);
    void UpdatePerformanceMetrics(float DeltaTime);
    void HandleSystemErrors();
    
    // ===== INTERNAL STATE =====
    
    UPROPERTY()
    float FrameTimeAccumulator = 0.0f;
    
    UPROPERTY()
    int32 FrameCount = 0;
    
    UPROPERTY()
    TArray<FString> SystemErrors;
    
    UPROPERTY()
    bool bSystemsInitialized = false;
};
