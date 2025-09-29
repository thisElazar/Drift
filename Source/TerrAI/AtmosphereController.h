#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "AtmosphereController.generated.h"

// Forward declarations
class UAtmosphericSystem;
class ADynamicTerrain;
class UWaterSystem;
class AGPUTerrainController;
class UTextureRenderTarget2D;
struct IPooledRenderTarget;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;


UCLASS(BlueprintType, Blueprintable)
class TERRAI_API AAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphereController();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeGPUResources();
    
    // ===== TEMPORAL INTEGRATION =====
    
    void UpdateAtmosphericSystem(float ScaledDeltaTime);
    
    // ===== GPU RESOURCES =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* AtmosphereStateTexture;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* CloudRenderTexture;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* WindFieldTexture;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPU Resources")
    UTextureRenderTarget2D* PrecipitationTexture;
    
    // ===== RENDERING =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UPostProcessComponent* CloudPostProcess;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    UMaterialInterface* VolumetricCloudMaterial;
    
    UPROPERTY()
    UMaterialInstanceDynamic* CloudMaterialInstance;
    
    // ===== OROGRAPHIC PARAMETERS =====
      
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Orographic",
                meta = (ClampMin = "0.0", ClampMax = "10.0"))
      float OrographicLiftCoefficient = 2.0f;
      
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Orographic",
                meta = (ClampMin = "0.0", ClampMax = "50.0",
                ToolTip = "Temperature drop rate in Kelvin per 100m of elevation"))
      float AdiabatiCoolingRate = 9.8f;  // Standard atmospheric lapse rate
      
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Orographic",
                meta = (ClampMin = "0.0", ClampMax = "1.0",
                ToolTip = "Intensity of rain shadow effect on leeward side of mountains"))
      float RainShadowIntensity = 0.7f;
    
    // ===== CLOUD PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings", meta=(ClampMin="500", ClampMax="5000"))
    float CloudBaseHeight = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings", meta=(ClampMin="500", ClampMax="5000"))
    float CloudLayerThickness = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings", meta=(ClampMin="0.1", ClampMax="5.0"))
    float CloudDensityScale = 1.0f;
    
    // ===== SYSTEM INTEGRATION =====
    
    UTextureRenderTarget2D* GetWindTexture() const { return WindFieldTexture; }
    UTextureRenderTarget2D* GetPrecipitationTexture() const { return PrecipitationTexture; }
    
    // ===== BLUEPRINT API =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherIntensity(float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWindDirection(FVector Direction);
    
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void TriggerLightning(FVector Location);
    
    FVector GetWindAtLocation(FVector WorldLocation) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Debug")
    void ValidateCloudRendering();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings")
    FVector CloudBoundsMin = FVector(-10000, -10000, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings")
    FVector CloudBoundsMax = FVector(10000, 10000, 5000);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Debug")
    void GenerateTestCloudData();
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Debug")
    void DebugReadCloudTexture();
    
    UFUNCTION(BlueprintCallable, Category = "Cloud Debug")
    void CreateDebugCloudPlane();
    
    UPROPERTY(VisibleAnywhere, Category = "Cloud Debug")
    class UStaticMeshComponent* CloudDebugPlane;
    
    UPROPERTY()
    class UMaterialInstanceDynamic* CloudDebugMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Plane")
    class AActor* AtmosphereDebugPlane;
    
    UFUNCTION(BlueprintCallable, Category = "Debug Plane")
    void UpdateDebugPlane();
    
    UPROPERTY()
    UAtmosphericSystem* AtmosphericSystem;
    
    void CleanupGPUResources();
    
    bool HasValidCloudData() const;
    
    void ResetAccumulatedTime() { AccumulatedTime = 0.0f; }
    
    // Authority reference
    UPROPERTY()
    class AMasterWorldController* MasterController;
    
    // Authority-based initialization
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeWithAuthority(AMasterWorldController* Master, ADynamicTerrain* Terrain);
    
    // Check if properly initialized
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    bool IsInitializedWithAuthority() const { return bInitializedWithAuthority; }
    
    
    // GPU compute state
    bool bGPUResourcesInitialized = false;
    int32 GridSizeX = 513;
    int32 GridSizeY = 513;
    
    // Time tracking (moved from static to member)
    float AccumulatedTime = 0.0f;
    float InitializationTimer = 0.0f;
    bool bNeedsInitialState = true;
   
    
protected:
    // Authority tracking
    bool bInitializedWithAuthority = false;
    
    void InitializeAtmosphereTextures();

    // System references
    UPROPERTY()
    ADynamicTerrain* TargetTerrain;
    
    UPROPERTY()
    UWaterSystem* WaterSystem;
    

    
    // Update accumulators
    float PhysicsUpdateAccumulator = 0.0f;
    float VisualUpdateAccumulator = 0.0f;
    float MinPhysicsStep = 0.033f;
    
    // Internal methods
    void DispatchAtmosphereCompute(float DeltaTime);
  
    void UpdateSystemIntegrations();
    void PushInitialStateToGPU();
    
private:
    // Debug visualization helpers
    void CreateDebugVisualizationPlane();
    void CreateTranslucentCloudMaterial();
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU")
    bool bUseGPUCompute = false;
    
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void ExecuteAtmosphericCompute(float DeltaTime);
    
    void UpdateCloudMaterial();
    
    
    UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
    bool IsReadyForGPU() const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere GPU")
       void EnableGPUCompute();
       
       UFUNCTION(BlueprintCallable, Category = "Atmosphere GPU")
       void DisableGPUCompute();
       
       UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
       bool IsGPUComputeEnabled() const { return bUseGPUCompute; }
       
       UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
       bool IsGPUResourcesInitialized() const { return bGPUResourcesInitialized; }
    
       UFUNCTION(BlueprintPure, Category = "Atmosphere")
       int32 GetTerrainWidth() const;
       
       UFUNCTION(BlueprintPure, Category = "Atmosphere")
       int32 GetTerrainHeight() const;
    

        UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
        int32 GetGridSizeX() const { return GridSizeX; }
        
        UFUNCTION(BlueprintPure, Category = "Atmosphere GPU")
        int32 GetGridSizeY() const { return GridSizeY; }
    
protected:
    // Cached reference to GPU terrain controller
    UPROPERTY()
    AGPUTerrainController* CachedGPUController = nullptr;
    
    
    

};
