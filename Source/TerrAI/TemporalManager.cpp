#include "TemporalManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "DrawDebugHelpers.h"

// ===== SUBSYSTEM INITIALIZATION =====

void UTemporalManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Initializing temporal coordination system"));
    
    // Initialize temporal state
    TemporalState.GameStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TemporalState.AccumulatedTime = 0.0f;
    TemporalState.LastUpdateTime = TemporalState.GameStartTime;
    
    // Set up default time scales for all systems
    InitializeDefaultTimeScales();
    
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Initialization complete"));
}

void UTemporalManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Shutting down temporal coordination"));
    
    // Clean up temporal state
    TemporalState.SystemTimes.Empty();
    TemporalState.SystemDeltas.Empty();
    TemporalState.TimeScales.Empty();
    TemporalState.LastUpdateTimes.Empty();
    TemporalState.SystemRegistered.Empty();
    TemporalState.UpdateFrequencies.Empty();
    
    Super::Deinitialize();
}

void UTemporalManager::InitializeDefaultTimeScales()
{
    // Immediate response systems
    SetSystemTimeScale(ESystemType::PlayerInteraction, 1.0f);     // Real-time
    
    // Short-term physics systems  
    SetSystemTimeScale(ESystemType::WaterPhysics, 1.0f);         // 10 seconds = 1 second WAS 10
    
    // Medium-term atmospheric systems
    SetSystemTimeScale(ESystemType::Atmospheric, 20.0f);          // 1 minute = 1 second WAS 60
    
    // Long-term geological systems
    SetSystemTimeScale(ESystemType::Erosion, 3600.0f);            // 1 hour = 1 second
    SetSystemTimeScale(ESystemType::Seasonal, 86400.0f);          // 1 day = 1 second
    
    // Biological systems
    SetSystemTimeScale(ESystemType::Ecosystem, 3600.0f);          // 1 hour = 1 second
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Default time scales initialized"));
}

// ===== CORE TEMPORAL INTERFACE =====

float UTemporalManager::GetSystemTime(ESystemType SystemType) const
{
    if (!IsSystemTypeValid(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Invalid system type in GetSystemTime"));
        return 0.0f;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const float* SystemTime = TemporalState.SystemTimes.Find(SystemType);
    if (!SystemTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: System %d not registered in GetSystemTime"), (int32)SystemType);
        return 0.0f;
    }
    
    return ApplyGlobalScaling(*SystemTime);
}

float UTemporalManager::GetSystemDeltaTime(ESystemType SystemType, float RealDeltaTime) const
{
    if (!IsSystemTypeValid(SystemType) || !bEnableTemporalScaling)
    {
        return RealDeltaTime * MasterTimeScale;
    }
    
    if (bPauseAllSystems)
    {
        return 0.0f;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const float* TimeScale = TemporalState.TimeScales.Find(SystemType);
    if (!TimeScale)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("TemporalManager: System %d not registered, using real-time"), (int32)SystemType);
        return RealDeltaTime * MasterTimeScale;
    }
    
    // Apply system-specific time scaling and global acceleration
    float ScaledDelta = RealDeltaTime * (*TimeScale) * MasterTimeScale;
    
    // Clamp to prevent temporal instability
    const float MaxDeltaTime = 1.0f; // Max 1 second per frame in scaled time
    ScaledDelta = FMath::Clamp(ScaledDelta, 0.0f, MaxDeltaTime);
    
    if (bLogTemporalUpdates)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("TemporalManager: System %d DeltaTime: Real=%.4f, Scale=%.2f, Scaled=%.4f"), 
               (int32)SystemType, RealDeltaTime, *TimeScale, ScaledDelta);
    }
    
    return ScaledDelta;
}

void UTemporalManager::SetSystemTimeScale(ESystemType SystemType, float TimeScale)
{
    if (!IsSystemTypeValid(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Invalid system type in SetSystemTimeScale"));
        return;
    }
    
    TimeScale = FMath::Max(0.1f, TimeScale); // Minimum scale to prevent division issues
    
    FScopeLock Lock(&TemporalStateMutex);
    
    TemporalState.TimeScales.Add(SystemType, TimeScale);
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Set time scale for system %d to %.2f"), 
           (int32)SystemType, TimeScale);
}

// ===== TEMPORAL COORDINATION =====

bool UTemporalManager::ShouldSystemUpdate(ESystemType SystemType, float UpdateFrequency) const
{
    if (!IsSystemTypeValid(SystemType) || bPauseAllSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Fail 145"));
        return false;
    }
    
    // Performance throttling - limit systems per frame
    if (ShouldThrottleSystemUpdates())
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Fail 152"));
        return false;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const float* LastUpdateTime = TemporalState.LastUpdateTimes.Find(SystemType);
    if (!LastUpdateTime)
    {
        // First update - allow it
        return true;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float TimeSinceLastUpdate = CurrentTime - *LastUpdateTime;
    
    // Check if enough time has passed based on update frequency
    float UpdateInterval = 1.0f / FMath::Max(0.1f, UpdateFrequency);
    
    bool ShouldUpdate = TimeSinceLastUpdate >= UpdateInterval;
    
    if (ShouldUpdate && bLogTemporalUpdates)
    {
        UE_LOG(LogTemp, Log, TEXT("TemporalManager: System %d updating (%.4fs since last)"),
               (int32)SystemType, TimeSinceLastUpdate);
    }
    
    return ShouldUpdate;
}

void UTemporalManager::RegisterSystem(ESystemType SystemType, float InitialTimeScale)
{
    if (!IsSystemTypeValid(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Attempted to register invalid system type"));
        return;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    EnsureSystemInitialized(SystemType);
    
    TemporalState.SystemRegistered.Add(SystemType, true);
    TemporalState.TimeScales.Add(SystemType, InitialTimeScale);
    TemporalState.SystemTimes.Add(SystemType, 0.0f);
    TemporalState.SystemDeltas.Add(SystemType, 0.0f);
    TemporalState.LastUpdateTimes.Add(SystemType, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
    TemporalState.UpdateFrequencies.Add(SystemType, 1.0f); // Default 1Hz
    
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Registered system %d with time scale %.2f"), 
           (int32)SystemType, InitialTimeScale);
}

void UTemporalManager::UnregisterSystem(ESystemType SystemType)
{
    if (!IsSystemTypeValid(SystemType))
    {
        return;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    TemporalState.SystemRegistered.Remove(SystemType);
    TemporalState.TimeScales.Remove(SystemType);
    TemporalState.SystemTimes.Remove(SystemType);
    TemporalState.SystemDeltas.Remove(SystemType);
    TemporalState.LastUpdateTimes.Remove(SystemType);
    TemporalState.UpdateFrequencies.Remove(SystemType);
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Unregistered system %d"), (int32)SystemType);
}

// ===== TIME CONTROL INTERFACE =====

void UTemporalManager::SetGlobalTimeAcceleration(float Acceleration)
{
    MasterTimeScale = FMath::Max(0.0f, Acceleration);
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Global time acceleration set to %.2f"), MasterTimeScale);
}

void UTemporalManager::SetTemporalPause(bool bPaused, int32 SystemsMask)
{
    bPauseAllSystems = bPaused;
    
    // TODO: Implement per-system pause masks if needed
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Temporal systems %s"), 
           bPaused ? TEXT("PAUSED") : TEXT("RESUMED"));
}

// ===== TEMPORAL SYNCHRONIZATION =====

void UTemporalManager::ForceSynchronization()
{
    FScopeLock Lock(&TemporalStateMutex);
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Update all system times to synchronized state
    for (auto& SystemTimePair : TemporalState.SystemTimes)
    {
        TemporalState.LastUpdateTimes.Add(SystemTimePair.Key, CurrentTime);
    }
    
    UE_LOG(LogTemp, Log, TEXT("TemporalManager: Forced synchronization of all systems"));
}

float UTemporalManager::GetSynchronizationRatio(ESystemType FromSystem, ESystemType ToSystem) const
{
    if (!IsSystemTypeValid(FromSystem) || !IsSystemTypeValid(ToSystem))
    {
        return 1.0f;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const float* FromScale = TemporalState.TimeScales.Find(FromSystem);
    const float* ToScale = TemporalState.TimeScales.Find(ToSystem);
    
    if (!FromScale || !ToScale || *ToScale == 0.0f)
    {
        return 1.0f;
    }
    
    return *FromScale / *ToScale;
}

// ===== SYSTEM INFORMATION =====

TArray<ESystemType> UTemporalManager::GetRegisteredSystems() const
{
    FScopeLock Lock(&TemporalStateMutex);
    
    TArray<ESystemType> RegisteredSystems;
    for (const auto& SystemPair : TemporalState.SystemRegistered)
    {
        if (SystemPair.Value)
        {
            RegisteredSystems.Add(SystemPair.Key);
        }
    }
    
    return RegisteredSystems;
}

float UTemporalManager::GetSystemTimeScale(ESystemType SystemType) const
{
    if (!IsSystemTypeValid(SystemType))
    {
        return 1.0f;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const float* TimeScale = TemporalState.TimeScales.Find(SystemType);
    return TimeScale ? *TimeScale : 1.0f;
}

bool UTemporalManager::IsSystemRegistered(ESystemType SystemType) const
{
    if (!IsSystemTypeValid(SystemType))
    {
        return false;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    const bool* IsRegistered = TemporalState.SystemRegistered.Find(SystemType);
    return IsRegistered ? *IsRegistered : false;
}

// ===== INTERNAL TEMPORAL MECHANICS =====

void UTemporalManager::UpdateTemporalState(float RealDeltaTime)
{
    FScopeLock Lock(&TemporalStateMutex);
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TemporalState.AccumulatedTime += RealDeltaTime;
    
    CalculateSystemDeltas(RealDeltaTime);
    
    // Update system times
    for (auto& SystemTimePair : TemporalState.SystemTimes)
    {
        ESystemType SystemType = SystemTimePair.Key;
        const float* TimeScale = TemporalState.TimeScales.Find(SystemType);
        
        if (TimeScale)
        {
            UE_LOG(LogTemp, Log, TEXT("TemporalManager: UpdateTemporalStateCalled"));
            float ScaledDelta = RealDeltaTime * (*TimeScale) * MasterTimeScale;
            SystemTimePair.Value += ScaledDelta;
            TemporalState.SystemDeltas.Add(SystemType, ScaledDelta);
        }
    }
    
    TemporalState.LastUpdateTime = CurrentTime;
    
    // Handle overflow prevention
    HandleTemporalOverflow();
    
    // Update frame rate tracking
    UpdateFrameRateTracking(RealDeltaTime);
}

void UTemporalManager::CalculateSystemDeltas(float RealDeltaTime)
{
    for (const auto& SystemPair : TemporalState.SystemRegistered)
    {
        if (!SystemPair.Value) continue;
        
        ESystemType SystemType = SystemPair.Key;
        float SystemDelta = GetSystemDeltaTime(SystemType, RealDeltaTime);
        TemporalState.SystemDeltas.Add(SystemType, SystemDelta);
    }
}

void UTemporalManager::HandleTemporalOverflow()
{
    // Prevent time values from becoming too large (reset after 24 hours game time)
    const float MaxAccumulatedTime = 86400.0f; // 24 hours
    
    if (TemporalState.AccumulatedTime > MaxAccumulatedTime)
    {
        float Overflow = TemporalState.AccumulatedTime - MaxAccumulatedTime;
        TemporalState.AccumulatedTime = Overflow;
        
        // Adjust all system times proportionally
        for (auto& SystemTimePair : TemporalState.SystemTimes)
        {
            float SystemOverflow = Overflow * GetSystemTimeScale(SystemTimePair.Key);
            SystemTimePair.Value = SystemOverflow;
        }
        
        UE_LOG(LogTemp, Log, TEXT("TemporalManager: Handled temporal overflow, reset to %.2f seconds"), Overflow);
    }
}

// ===== INTERNAL HELPERS =====

bool UTemporalManager::IsSystemTypeValid(ESystemType SystemType) const
{
    return SystemType >= ESystemType::PlayerInteraction && SystemType <= ESystemType::Ecosystem;
}

float UTemporalManager::ApplyGlobalScaling(float LocalTime) const
{
    return LocalTime * MasterTimeScale;
}

void UTemporalManager::EnsureSystemInitialized(ESystemType SystemType)
{
    if (!TemporalState.SystemTimes.Contains(SystemType))
    {
        TemporalState.SystemTimes.Add(SystemType, 0.0f);
    }
    
    if (!TemporalState.SystemDeltas.Contains(SystemType))
    {
        TemporalState.SystemDeltas.Add(SystemType, 0.0f);
    }
    
    if (!TemporalState.LastUpdateTimes.Contains(SystemType))
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        TemporalState.LastUpdateTimes.Add(SystemType, CurrentTime);
    }
}

// ===== FRAME RATE MANAGEMENT =====

bool UTemporalManager::ShouldThrottleSystemUpdates() const
{
    return SystemUpdateCounter >= MaxSystemsPerFrame;
}

void UTemporalManager::UpdateFrameRateTracking(float DeltaTime)
{
    // Simple frame rate tracking for performance monitoring
    LastFrameTime = DeltaTime;
    
    // Reset system update counter each frame
    SystemUpdateCounter = 0;
}

// ===== DEBUGGING & MONITORING =====

void UTemporalManager::LogTemporalState() const
{
    if (!bLogTemporalUpdates) return;
    
    FScopeLock Lock(&TemporalStateMutex);
    
    UE_LOG(LogTemp, Log, TEXT("=== TEMPORAL STATE ==="));
    UE_LOG(LogTemp, Log, TEXT("Accumulated Time: %.2f"), TemporalState.AccumulatedTime);
    UE_LOG(LogTemp, Log, TEXT("Master Time Scale: %.2f"), MasterTimeScale);
    UE_LOG(LogTemp, Log, TEXT("Systems Paused: %s"), bPauseAllSystems ? TEXT("YES") : TEXT("NO"));
    
    for (const auto& SystemPair : TemporalState.SystemTimes)
    {
        ESystemType SystemType = SystemPair.Key;
        float SystemTime = SystemPair.Value;
        float TimeScale = GetSystemTimeScale(SystemType);
        
        UE_LOG(LogTemp, Log, TEXT("System %d: Time=%.2f, Scale=%.2f"), 
               (int32)SystemType, SystemTime, TimeScale);
    }
}

FString UTemporalManager::GetTemporalSystemStats() const
{
    FScopeLock Lock(&TemporalStateMutex);
    
    FString Stats = FString::Printf(TEXT("Temporal Manager Stats:\n"));
    Stats += FString::Printf(TEXT("Accumulated Time: %.2f seconds\n"), TemporalState.AccumulatedTime);
    Stats += FString::Printf(TEXT("Master Scale: %.2fx\n"), MasterTimeScale);
    Stats += FString::Printf(TEXT("Systems Registered: %d\n"), TemporalState.SystemRegistered.Num());
    Stats += FString::Printf(TEXT("Last Frame Time: %.4f seconds\n"), LastFrameTime);
    
    return Stats;
}

// ===== ADVANCED FEATURES =====

FString UTemporalManager::CreateTemporalSnapshot() const
{
    FScopeLock Lock(&TemporalStateMutex);
    
    // Create a JSON-like string with temporal state
    FString Snapshot = TEXT("{");
    Snapshot += FString::Printf(TEXT("\"AccumulatedTime\":%.6f,"), TemporalState.AccumulatedTime);
    Snapshot += FString::Printf(TEXT("\"MasterTimeScale\":%.6f,"), MasterTimeScale);
    Snapshot += TEXT("\"SystemStates\":[");
    
    bool FirstSystem = true;
    for (const auto& SystemPair : TemporalState.SystemTimes)
    {
        if (!FirstSystem) Snapshot += TEXT(",");
        FirstSystem = false;
        
        Snapshot += FString::Printf(TEXT("{\"System\":%d,\"Time\":%.6f,\"Scale\":%.6f}"),
                                   (int32)SystemPair.Key,
                                   SystemPair.Value,
                                   GetSystemTimeScale(SystemPair.Key));
    }
    
    Snapshot += TEXT("]}");
    return Snapshot;
}

bool UTemporalManager::RestoreFromSnapshot(const FString& SnapshotData)
{
    // Basic snapshot restoration - in production you'd want proper JSON parsing
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Restoring from snapshot (basic implementation)"));
    
    // Reset all systems first
    ResetAllTemporalSystems();
    
    return true; // Simplified - implement full JSON parsing for production
}

void UTemporalManager::ResetAllTemporalSystems()
{
    FScopeLock Lock(&TemporalStateMutex);
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    TemporalState.GameStartTime = CurrentTime;
    TemporalState.AccumulatedTime = 0.0f;
    TemporalState.LastUpdateTime = CurrentTime;
    
    // Reset all system times but preserve registration and scales
    for (auto& SystemTimePair : TemporalState.SystemTimes)
    {
        SystemTimePair.Value = 0.0f;
    }
    
    for (auto& LastUpdatePair : TemporalState.LastUpdateTimes)
    {
        LastUpdatePair.Value = CurrentTime;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TemporalManager: Reset all temporal systems"));
}

void UTemporalManager::MarkSystemUpdated(ESystemType SystemType)
{
    if (!IsSystemTypeValid(SystemType))
    {
        return;
    }
    
    FScopeLock Lock(&TemporalStateMutex);
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TemporalState.LastUpdateTimes.Add(SystemType, CurrentTime);
    
    // Increment update counter for this frame
    SystemUpdateCounter++;
}
