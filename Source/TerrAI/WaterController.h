/**
 * ============================================
 * TERRAI WATER CONTROLLER - UNIFIED INTERFACE
 * ============================================
 * Purpose: Centralized Blueprint interface for all water system controls
 * Benefits: Clean separation, easier material assignment, better organization
 * Version: GPU Water Added, Niagara Stripped to only Includes
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "MasterController.h"
#include "WaterController.generated.h"

class UWaterSystem;
class ADynamicTerrain;
class UTextureRenderTarget2D;

UENUM(BlueprintType)
enum class EWaterVisualMode : uint8
{
    Water          UMETA(DisplayName = "Water"),
    Milk           UMETA(DisplayName = "Milk"),
    Debug          UMETA(DisplayName = "Debug")
};

UENUM(BlueprintType)
enum class EWaterQuality : uint8
{
    Low            UMETA(DisplayName = "Low (16x16)"),
    Medium         UMETA(DisplayName = "Medium (32x32)"),
    High           UMETA(DisplayName = "High (64x64)"),
    Ultra          UMETA(DisplayName = "Ultra (128x128)")
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AWaterController : public AActor
{
    GENERATED_BODY()
    
public:
    AWaterController();
    
    // ===== CORE SYSTEM =====
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void Initialize(ADynamicTerrain* Terrain);
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ResetWaterSystem();
    
    // ===== WATER-SPECIFIC BRUSH FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Brush")
    void ApplyWaterBrush(FVector WorldPosition, float Amount, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Water Brush")
    void ApplyWaterRemovalBrush(FVector WorldPosition, float Amount, float Radius);
    
    // ✅ TERRAIN SYNCHRONIZATION
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void NotifyTerrainChanged();
    
    // ===== MATERIALS - EASY ACCESS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Materials")
    UMaterialInterface* WaterMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Materials")
    UMaterialInterface* MilkMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Materials")
    UMaterialInterface* DebugMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Materials")
    UMaterialInterface* VolumeMaterial = nullptr;  // Surface water material
    
    // ===== VISUAL CONTROLS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Visuals")
    EWaterVisualMode CurrentVisualMode = EWaterVisualMode::Water;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Visuals")
    EWaterQuality WaterQuality = EWaterQuality::Medium;
    
    UFUNCTION(BlueprintCallable, Category = "Water Visuals")
    void ToggleWaterVisualMode();
    
    UFUNCTION(BlueprintCallable, Category = "Water Visuals")
    void SetWaterVisualMode(EWaterVisualMode NewMode);
    
    
    // ===== PHYSICS PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    bool bEnableWaterSimulation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterFlowSpeed = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterEvaporationRate = 0.01f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterAbsorptionRate = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterDamping = 0.95f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float MaxWaterVelocity = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float MinWaterDepth = 0.01f;
    
    // ===== VOLUMETRIC WATER =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    bool bEnableVolumetricWater = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    float MinVolumeDepth = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    float VolumeUpdateDistance = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    int32 MaxVolumeChunks = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    int32 BaseSurfaceResolution = 64;
    
    
    // ===== WEATHER SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeather = false;
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRain(float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopRain();
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ToggleRain();
    
    // ===== INTERACTION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void AddWater(FVector WorldPosition, float Amount, float Radius = 100.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void RemoveWater(FVector WorldPosition, float Amount, float Radius = 100.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    float GetWaterDepthAtPosition(FVector WorldPosition) const;
    
    // ===== SHADER SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shaders")
    bool bUseShaderWater = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shaders")
    float WaterShaderUpdateInterval = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shaders")
    float WaterDepthScale = 25.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shaders")
    UMaterialParameterCollection* WaterParameterCollection = nullptr;
    
    // ===== DEBUG =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowWaterStats = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowWaterDebugTexture = false;
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo() const;
    
    // ===== UTILITIES =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Info")
    float GetTotalWaterInSystem() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Info")
    int32 GetWaterCellCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Info")
    float GetMaxFlowSpeed() const;
    
    // ===== SYSTEM ACCESS =====
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    UWaterSystem* GetWaterSystem() const { return WaterSystem; }
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    ADynamicTerrain* GetTargetTerrain() const { return TargetTerrain; }
    
    // Waving Tuning
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Classification")
    float RapidsFlowThreshold = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Classification")
    float RapidsGradientThreshold = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Classification")
    float RiverFlowThreshold = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Classification")
    float PuddleDepthThreshold = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Classification")
    float PondDepthThreshold = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Collision")
    float CollisionFlowThreshold = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Collision")
    float CollisionSampleDistance = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Collision")
    float CollisionThreshold = -0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Collision")
    float CollisionWaveScale = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Tuning|Collision")
    float CollisionTimeScale = 12.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Advanced",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GPUWaveDamping = 0.9f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Advanced",
              meta = (ClampMin = "0.05", ClampMax = "0.5"))
    float GPUMaxWaveHeightRatio = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Advanced",
              meta = (ClampMin = "0.05", ClampMax = "0.2"))
    float GPUSafeWaveHeightRatio = 0.125f;
    
    // New Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "GPU Water|Debug", CallInEditor)
    void ResetGPUWaveSystem();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Water|Debug")
    bool ValidateGPUParameters();
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
private:
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
 
    
    UPROPERTY()
    class AMasterWorldController* MasterController = nullptr;
    
    // Water system helpers
    void RegisterWithMasterController();
    bool CanReceiveBrush() const;
    
    // Apply settings to water system
    void ApplySettingsToWaterSystem();
    void UpdateWaterSystemFromController();
    UMaterialInterface* GetCurrentWaterMaterial() const;
    
    bool bPendingGPUInit = false;
     int32 GPUInitRetryCount = 0;
    
    void SetupDynamicMaterialParameters(UMaterialInstanceDynamic* DynMat);
    
    
public:
    // ===== GPU VERTEX DISPLACEMENT CONTROLS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water", meta = (DisplayPriority = 1))
    bool bUseGPUVertexDisplacement = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water", meta = (DisplayPriority = 2))
    UMaterialInterface* GPUWaterMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water", meta = (DisplayPriority = 3))
    float GPUWaveScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water", meta = (DisplayPriority = 4))
    float GPUWaveSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Wind", meta = (DisplayPriority = 5))
    FVector2D WindDirection = FVector2D(1.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Wind", meta = (DisplayPriority = 6))
    float WindStrength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water",
              meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float GPUWaveAnimationSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU Water|Debug")
    bool bShowGPUStats = false;
    
    // Blueprint callable functions for runtime control
    UFUNCTION(BlueprintCallable, Category = "GPU Water", CallInEditor)
    void EnableGPUWater();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Water", CallInEditor)
    void DisableGPUWater();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Water", CallInEditor)
    void ToggleGPUWater();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Water")
    void UpdateGPUWaveParameters();
    
    UFUNCTION(BlueprintCallable, Category = "GPU Water", CallInEditor)
    void InitializeGPUWaterSystem();
    
    // Console command overrides
    UFUNCTION(Exec)
    void SetGPUWaves(bool bEnable);
    
    UFUNCTION(Exec)
    void SetWaveScale(float Scale);
    
    UFUNCTION(Exec)
    void SetWaveSpeed(float Speed);
    
    UFUNCTION(Exec)
    void SetWind(float X, float Y, float Strength);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugGPUWater();
    
public:
    // Precipitation input from atmosphere
    UFUNCTION(BlueprintCallable, Category = "Water GPU")
    void SetPrecipitationInput(UTextureRenderTarget2D* PrecipitationTexture);
    
    // Flow intensity for erosion feedback
    UFUNCTION(BlueprintPure, Category = "Water Flow")
    float GetAverageFlowIntensity() const;

// Add to protected section:
protected:
    // Cached precipitation texture
    UPROPERTY()
    UTextureRenderTarget2D* CurrentPrecipitationTexture = nullptr;
    
protected:
    // Override PostEditChangeProperty to handle changes in editor
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    
};
