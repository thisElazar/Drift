/**
 * ============================================
 * TERRAI WATER CONTROLLER - UNIFIED INTERFACE
 * ============================================
 * Purpose: Centralized Blueprint interface for all water system controls
 * Benefits: Clean separation, easier material assignment, better organization
 * Version: Complete with Niagara FX integration
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "WaterController.generated.h"

class UWaterSystem;
class ADynamicTerrain;

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
    
    // ===== NIAGARA FX SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    bool bEnableNiagaraFX = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    UNiagaraSystem* RiverFlowEmitterTemplate = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    UNiagaraSystem* FoamEmitterTemplate = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    UNiagaraSystem* LakeMistEmitterTemplate = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    UNiagaraSystem* RainImpactEmitterTemplate = nullptr;
    
    // Niagara Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float NiagaraUpdateRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "500", ClampMax = "10000"))
    float MaxNiagaraDistance = 3000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "1", ClampMax = "128"))
    int32 MaxActiveNiagaraComponents = 32;
    
    // Niagara Effect Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float FoamIntensityScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MistDensityScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float MinFlowSpeedForFX = 5.0f;
    
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
    
    // ===== EROSION SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bEnableErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float MinErosionVelocity = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float DepositionRate = 0.05f;
    
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
    void AddWater(FVector WorldPosition, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void RemoveWater(FVector WorldPosition, float Amount);
    
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

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    UWaterSystem* WaterSystem = nullptr;
    
    // Apply settings to water system
    void ApplySettingsToWaterSystem();
    void UpdateWaterSystemFromController();
    UMaterialInterface* GetCurrentWaterMaterial() const;
};
