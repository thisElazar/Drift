#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "TemporalManager.generated.h"

// ===== TEMPORAL SCALE DEFINITIONS =====
UENUM(BlueprintType)
enum class ETemporalScale : uint8
{
    RealTime           UMETA(DisplayName = "Real Time"),
    Accelerated        UMETA(DisplayName = "Accelerated"),
    Extended           UMETA(DisplayName = "Extended"),
    Geological         UMETA(DisplayName = "Geological")
};

UENUM(BlueprintType)
enum class ESystemType : uint8
{
    PlayerInteraction  UMETA(DisplayName = "Player Interaction"),
    WaterPhysics      UMETA(DisplayName = "Water Physics"),
    Atmospheric       UMETA(DisplayName = "Atmospheric"),
    Erosion          UMETA(DisplayName = "Erosion"),
    Seasonal         UMETA(DisplayName = "Seasonal"),
    Ecosystem        UMETA(DisplayName = "Ecosystem")
};

USTRUCT()
struct FTemporalState
{
    GENERATED_BODY()
    
    float GameStartTime = 0.0f;
    float AccumulatedTime = 0.0f;
    float LastUpdateTime = 0.0f;
    
    TMap<ESystemType, float> SystemTimes;
    TMap<ESystemType, float> SystemDeltas;
    TMap<ESystemType, float> TimeScales;
    TMap<ESystemType, float> LastUpdateTimes;
    TMap<ESystemType, bool> SystemRegistered;
    TMap<ESystemType, float> UpdateFrequencies;
};

/**
 * UTemporalManager - Central time coordination for all TerrAI systems
 */
UCLASS(BlueprintType)
class TERRAI_API UTemporalManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, Category = "Temporal Management")
    void MarkSystemUpdated(ESystemType SystemType);
    
    // ===== CORE INTERFACE =====
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    float GetSystemTime(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    float GetSystemDeltaTime(ESystemType SystemType, float RealDeltaTime) const;
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    void SetSystemTimeScale(ESystemType SystemType, float TimeScale);
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    bool ShouldSystemUpdate(ESystemType SystemType, float UpdateFrequency = 1.0f) const;
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    void RegisterSystem(ESystemType SystemType, float InitialTimeScale = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void SetGlobalTimeAcceleration(float Acceleration);
    
    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void SetTemporalPause(bool bPaused, int32 SystemsMask = 0xFF);
    
    UFUNCTION(BlueprintPure, Category = "Time Control")
    float GetGlobalTimeAcceleration() const { return MasterTimeScale; }
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    void ForceSynchronization();
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    TArray<ESystemType> GetRegisteredSystems() const;
    
    UFUNCTION(BlueprintPure, Category = "Temporal")
    float GetSystemTimeScale(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintPure, Category = "Temporal")
    bool IsSystemRegistered(ESystemType SystemType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Time Control")
    void ResetAllTemporalSystems();
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetTemporalSystemStats() const;
    
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FString CreateTemporalSnapshot() const;
    
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool RestoreFromSnapshot(const FString& SnapshotData);
    
    UFUNCTION(BlueprintCallable, Category = "Temporal")
    void ResetFrameCounter() { SystemUpdateCounter = 0; }
    
    // ===== PAUSE STATE QUERY =====
    UFUNCTION(BlueprintPure, Category = "Time Control")
    bool IsPaused() const { return bPauseAllSystems; }

    UFUNCTION(BlueprintPure, Category = "Time Control")
    bool IsSystemPaused() const { return bPauseAllSystems; }

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    void InitializeDefaultTimeScales();
    void UpdateTemporalState(float RealDeltaTime);
    void HandleTemporalOverflow();

private:
    FTemporalState TemporalState;
    
    UPROPERTY(EditAnywhere, Category = "Temporal Settings")
    bool bEnableTemporalScaling = true;
    
    UPROPERTY(EditAnywhere, Category = "Temporal Settings")
    float MasterTimeScale = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Temporal Settings")
    bool bPauseAllSystems = false;
    
    // Missing member variables
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bLogTemporalUpdates = false;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxSystemsPerFrame = 4;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 SystemUpdateCounter = 0;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    float LastFrameTime = 0.0f;
    
    mutable FCriticalSection TemporalStateMutex;
    
    // Helper functions
    bool IsSystemTypeValid(ESystemType SystemType) const;
    float ApplyGlobalScaling(float LocalTime) const;
    void EnsureSystemInitialized(ESystemType SystemType);
    void UnregisterSystem(ESystemType SystemType);
    float GetSynchronizationRatio(ESystemType FromSystem, ESystemType ToSystem) const;
    void CalculateSystemDeltas(float RealDeltaTime);
    bool ShouldThrottleSystemUpdates() const;
    void UpdateFrameRateTracking(float DeltaTime);
    void LogTemporalState() const;
    
    static constexpr float SECONDS_PER_MINUTE = 60.0f;
    static constexpr float SECONDS_PER_HOUR = 3600.0f;
    static constexpr float SECONDS_PER_DAY = 86400.0f;
    static constexpr float SECONDS_PER_YEAR = 31536000.0f;
};


