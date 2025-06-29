/**
 * ============================================
 * TERRAI WATER SYSTEM - PHYSICS & SHADERS
 * ============================================
 * Purpose: Real-time water simulation with GPU shader integration
 * Performance: Optimized for 256+ terrain chunks, throttled debug logging
 * Dependencies: DynamicTerrain, UE5.4 material system, PF_R8 textures
 * 
 * Key Features:
 * - Pressure-based flow simulation with edge drainage
 * - Real-time texture updates for material shaders  
 * - Weather system with procedural rain
 * - Erosion simulation for terrain modification
 * - Scientific accuracy with 60+ FPS performance
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h" 
#include "Engine/Texture2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "TemporalManager.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "Engine/World.h"
#include "Stats/Stats.h"
#include "MasterController.h"
#include "WaterSystem.generated.h"

// Forward declarations
class AMasterWorldController;
class ADynamicTerrain;
class UProceduralMeshComponent;

/**
 * Localized water mesh region for precise water rendering
 */
USTRUCT()
struct FWaterMeshRegion
{
    GENERATED_BODY()
    
    FVector2D CenterPosition;
    float MeshRadius;
    bool bNeedsUpdate;
    UProceduralMeshComponent* MeshComponent;
    
    FWaterMeshRegion()
        : CenterPosition(FVector2D::ZeroVector)
        , MeshRadius(100.0f)
        , bNeedsUpdate(true)
        , MeshComponent(nullptr)
    {}
};

/**
 * Consolidated water material properties for volumetric rendering
 */
USTRUCT(BlueprintType)
struct FWaterMaterialParams
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float Clarity = 0.8f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float Absorption = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CausticStrength = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DeepColor = FLinearColor(0.1f, 0.3f, 0.6f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ShallowColor = FLinearColor(0.6f, 0.8f, 0.9f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Turbidity = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float WaveStrength = 0.02f;
    
    FWaterMaterialParams()
    {
        Clarity = 0.8f;
        Absorption = 0.3f;
        CausticStrength = 0.2f;
        DeepColor = FLinearColor(0.1f, 0.3f, 0.6f, 1.0f);
        ShallowColor = FLinearColor(0.6f, 0.8f, 0.9f, 1.0f);
        Turbidity = 0.1f;
        WaveStrength = 0.02f;
    }
};

/**
 * Water simulation data container for physics calculations
 * Layout: 2D grid flattened to 1D arrays using index = Y * Width + X
 * Memory: ~20MB for 513x513 terrain (5 arrays * 4 bytes * 263k cells)
 */
USTRUCT(BlueprintType)
struct FWaterSimulationData
{
    GENERATED_BODY()

    // Core water physics arrays - all same size as terrain
    TArray<float> WaterDepthMap;      // Water depth in world units (0-1000+)
    TArray<float> WaterVelocityX;     // Flow velocity East/West (-100 to +100)
    TArray<float> WaterVelocityY;     // Flow velocity North/South (-100 to +100)
    TArray<float> SedimentMap;        // Suspended sediment for erosion (0-10)
    TArray<float> FoamMap;            // Foam intensity for rendering (0-1)

    // System state
    bool bIsInitialized = false;
    int32 TerrainWidth = 0;
    int32 TerrainHeight = 0;

    FWaterSimulationData()
    {
        bIsInitialized = false;
        TerrainWidth = 0;
        TerrainHeight = 0;
    }

    void Initialize(int32 Width, int32 Height)
    {
        TerrainWidth = Width;
        TerrainHeight = Height;
        int32 TotalSize = Width * Height;

        WaterDepthMap.SetNum(TotalSize);
        WaterVelocityX.SetNum(TotalSize);
        WaterVelocityY.SetNum(TotalSize);
        SedimentMap.SetNum(TotalSize);
        FoamMap.SetNum(TotalSize);

        // Initialize all to zero
        for (int32 i = 0; i < TotalSize; i++)
        {
            WaterDepthMap[i] = 0.0f;
            WaterVelocityX[i] = 0.0f;
            WaterVelocityY[i] = 0.0f;
            SedimentMap[i] = 0.0f;
            FoamMap[i] = 0.0f;
        }

        bIsInitialized = true;
    }

    bool IsValid() const
    {
        return bIsInitialized && WaterDepthMap.Num() > 0;
    }
};

/**
 * Surface-based water chunk for realistic water visualization
 * Creates smooth water surfaces with proper optical depth representation
 * Replaces box-based volumes for natural pooling and flowing water
 * Integrates Beer-Lambert law for scientifically accurate light absorption
 * 
 * PHASE 1-2 ENHANCEMENT: All data synchronized from simulation authority
 */
USTRUCT()
struct FWaterSurfaceChunk
{
    GENERATED_BODY()

    // Chunk identification and state
    int32 ChunkIndex = -1;                           // Index into terrain chunks array
    UProceduralMeshComponent* SurfaceMesh = nullptr; // Water surface mesh component
    UProceduralMeshComponent* UndersideMesh = nullptr; // For underwater viewing
    float LastUpdateTime = 0.0f;                     // For temporal optimization
    bool bNeedsUpdate = true;                        // Marks chunk for regeneration
    
    // PHASE 1-2: Surface properties DERIVED from simulation data
    float MaxDepth = 0.0f;                          // Calculated from WaterDepthMap
    float AverageDepth = 0.0f;                      // Calculated from WaterDepthMap
    int32 SurfaceResolution = 32;                   // Vertices per edge at LOD0
    int32 CurrentLOD = 0;                           // 0=highest detail, 3=lowest
    
    // PHASE 1-2: Flow data SYNCHRONIZED from simulation velocity
    float WavePhase = 0.0f;                         // Animation phase offset
    FVector2D FlowDirection = FVector2D::ZeroVector; // Derived from VelocityX/Y arrays
    float FlowSpeed = 0.0f;                         // Calculated from velocity magnitude
    
    // PHASE 1-2: Visual effects driven by simulation state
    bool bHasCaustics = false;                      // Based on depth and flow patterns
    bool bHasFoam = false;                          // Based on FoamMap values
    
    FWaterSurfaceChunk()
    {
        ChunkIndex = -1;
        SurfaceMesh = nullptr;
        UndersideMesh = nullptr;
        LastUpdateTime = 0.0f;
        bNeedsUpdate = true;
        MaxDepth = 0.0f;
        AverageDepth = 0.0f;
        SurfaceResolution = 32;
        CurrentLOD = 0;
        WavePhase = 0.0f;
        FlowDirection = FVector2D::ZeroVector;
        FlowSpeed = 0.0f;
        bHasCaustics = false;
        bHasFoam = false;
    }
};

UCLASS(BlueprintType)
class TERRAI_API UWaterSystem : public UObject, public IScalableSystem
{
    GENERATED_BODY()

public:
    UWaterSystem();
    

    
    // ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====
    
    virtual void ConfigureFromMaster(const FWorldScalingConfig& Config) override;
    virtual void SynchronizeCoordinates(const FWorldCoordinateSystem& Coords) override;
   
    
    // ===== MASTER CONTROLLER COORDINATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Master Controller Integration")
    void RegisterWithMasterController(AMasterWorldController* Master);
    
    UFUNCTION(BlueprintPure, Category = "Master Controller Integration")
    bool IsRegisteredWithMaster() const;
    
    UFUNCTION(BlueprintPure, Category = "Master Controller Integration")
    FString GetScalingDebugInfo() const;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void ApplyWaterTextureToMaterial(class UMaterialInstanceDynamic* Material);

    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void Initialize(ADynamicTerrain* InTerrain, AMasterWorldController* InMasterController);
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsSystemReady() const;

    // ===== WATER PHYSICS SIMULATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Physics")
    void UpdateWaterSimulation(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Water Physics")
    void ResetWaterSystem();

    // ===== PLAYER INTERACTION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void AddWater(FVector WorldPosition, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void AddWaterAtIndex(int32 X, int32 Y, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void AddWaterInRadius(int32 CenterX, int32 CenterY, float Radius, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    void RemoveWater(FVector WorldPosition, float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    float GetWaterDepthAtPosition(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Interaction")
    float GetWaterDepthAtIndex(int32 X, int32 Y) const;

    // ===== WEATHER SYSTEM =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRain(float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopRain();
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetAutoWeather(bool bEnable);

    // ===== UTILITIES =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Utilities")
    float GetTotalWaterInSystem() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Utilities")
    int32 GetWaterCellCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Utilities")
    float GetMaxFlowSpeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Debug")
    void DrawDebugInfo() const;

    /**
     * ============================================
     * WATER PHYSICS PARAMETERS
     * ============================================
     * Fine-tuned for realistic behavior at 60+ FPS
     */
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    bool bEnableWaterSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterFlowSpeed = 50.0f;        // Base flow rate (50 = realistic streams)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterEvaporationRate = 0.01f;  // Per second (realistic desert rate)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterAbsorptionRate = 0.02f;   // Terrain absorption per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterDamping = 0.95f;          // Prevents oscillation (0.95 = stable)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float MaxWaterVelocity = 100.0f;     // Clamp for stability (prevents explosion)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float MinWaterDepth = 0.01f;         // Ignore tiny amounts (performance)
    
    // ===== EROSION SETTINGS - MOVED TO GEOLOGYCONTROLLER =====
    // All erosion functionality moved to GeologyController for clean separation
    
    // ===== EDGE DRAINAGE SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edge Drainage")
    bool bEnableEdgeDrainage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edge Drainage")
    float EdgeDrainageStrength = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edge Drainage")
    bool bEnhancedWaterfallEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edge Drainage")
    float WaterfallDrainageMultiplier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edge Drainage")
    float MinDepthThreshold = 0.1f;                // Minimum depth for triangle generation
    
    

    // ===== WEATHER SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeather = false;

    // ===== DEBUG SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Debug")
    bool bShowWaterStats = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Debug")
    bool bShowWaterDebugTexture = false;

    // ===== SHADER SYSTEM SETTINGS (DISABLED FOR NOW) =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    bool bUseShaderWater = true;  // Disabled until advanced materials working
    
    // ===== VOLUMETRIC WATER MATERIAL PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Visuals")
    FWaterMaterialParams MaterialParams;
    
    // Track if water changed this frame for efficient updates
    bool bWaterChangedThisFrame = false;
    
    // ===== SURFACE WATER SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water")
    bool bEnableWaterVolumes = true;                // OPTIMIZED PERFORMANCE ENABLED
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water")
    float MinVolumeDepth = 0.5f;                    // Minimum depth for surface generation (legacy name)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water")
    float VolumeUpdateDistance = 2000.0f;           // Max distance for surface updates (legacy name)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water")
    int32 MaxVolumeChunks = 64;                     // Performance limit on active surfaces (legacy name)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water", meta = (ClampMin = "8", ClampMax = "256"))
    int32 BaseSurfaceResolution = 64;               // Base resolution for water surfaces (higher = smoother)
    
    // ===== WATER AUTHORITY & QUALITY SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Authority", 
              meta = (ToolTip = "Minimum water depth required for mesh creation"))
    float MinMeshDepth = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Authority",
              meta = (ToolTip = "Minimum contiguous water cells required"))
    int32 MinContiguousWaterCells = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Authority",
              meta = (ToolTip = "Always create meshes where simulation has water"))
    bool bAlwaysShowWaterMeshes = true;

    // ===== CONFIGURABLE WATER QUALITY =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Quality",
              meta = (ClampMin = "8", ClampMax = "256", ToolTip = "Water mesh resolution - higher = smoother water"))
    int32 WaterMeshQuality = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Quality",
              meta = (ToolTip = "Quality presets for easy adjustment"))
    TMap<FString, int32> WaterQualityPresets;
    
    // ===== PERFORMANCE OPTIMIZATION SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Performance")
    int32 MaxMeshUpdatesPerFrame = 4;          // Throttle mesh updates per frame
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Performance")
    bool bEnableComponentPooling = true;       // Reuse mesh components
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxPoolSize = 32;
    
    // Mesh component pool with validation
    TArray<UProceduralMeshComponent*> ValidatedMeshPool;
    TMap<UProceduralMeshComponent*, float> PoolComponentLastUsed;

    // LOD transition tracking
    TMap<int32, float> ChunkLODTransitionTime;
    TMap<int32, int32> ChunkTargetLOD;
    
    // Performance tracking
    int32 MeshUpdatesThisFrame = 0;
    float LastCameraPosition = 0.0f;
    
    // Runtime surface chunk storage
    TArray<FWaterSurfaceChunk> WaterSurfaceChunks;
    
    // Localized water mesh regions
    TArray<FWaterMeshRegion> WaterMeshRegions;
    
    // ===== PERFORMANCE OPTIMIZATION FUNCTIONS =====
    // LOD calculation moved to MasterController for single authority
    // Use MasterController->CalculateSystemLOD() instead
    
    // ===== SCALABLE LOD FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Scalable LOD")
    float GetDynamicLODDistance(int32 LODLevel) const;
    
    UFUNCTION(BlueprintCallable, Category = "Scalable LOD", CallInEditor)
    void SetLODScaling(float ScaleMultiplier, float LOD0Factor, float LOD1Factor, float LOD2Factor);
    
    UFUNCTION(BlueprintPure, Category = "Scalable LOD")
    FString GetLODDebugInfo() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Performance")
    UProceduralMeshComponent* GetPooledMeshComponent();
    
    UFUNCTION(BlueprintCallable, Category = "Water Performance")
    void ReturnMeshComponentToPool(UProceduralMeshComponent* Component);
    
    UFUNCTION(BlueprintCallable, Category = "Water Performance")
    bool ShouldUpdateWaterMesh(int32 ChunkIndex, FVector CameraLocation) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Performance")
    void OptimizeWaterMeshUpdates(FVector CameraLocation, float DeltaTime);
    
    // ===== WATER QUALITY MANAGEMENT FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Quality")
    void SetWaterMeshQuality(int32 NewQuality);
    
    UFUNCTION(BlueprintPure, Category = "Water Quality")
    int32 GetWaterMeshQuality() const { return WaterMeshQuality; }
    
    UFUNCTION(BlueprintCallable, Category = "Water Quality")
    void SetWaterQualityPreset(const FString& PresetName);
    
    UFUNCTION(BlueprintCallable, Category = "Water Quality")
    void InitializeWaterQualityPresets();
    
    UFUNCTION(BlueprintPure, Category = "Water Quality")
    TArray<FString> GetWaterQualityPresetNames() const;
    
    UFUNCTION(BlueprintPure, Category = "Water Quality")
    FString GetWaterQualityPerformanceInfo() const;
    
    // ===== AUTHORITY-BASED WATER FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Authority")
    bool ShouldGenerateSurfaceForChunk_AuthorityOnly(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Authority")
    void UpdateWaterSurfaceChunks_AlwaysVisible();
    
    UFUNCTION(BlueprintCallable, Category = "Water Authority")
    bool ValidateShaderDataForChunk(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Authority")
    int32 CountContiguousWaterCells(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Authority")
    int32 FloodFillContiguousWater(int32 StartX, int32 StartY, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, TSet<int32>& VisitedCells) const;
    
    // ===== LOCALIZED MESH FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Localized Water")
    void GenerateLocalizedWaterMeshes();
    
    UFUNCTION(BlueprintCallable, Category = "Localized Water")
    bool ShouldGenerateLocalizedMesh(FVector2D WorldPosition, float SampleRadius) const;
    
    UFUNCTION(BlueprintCallable, Category = "Localized Water")
    float GetSimulationDepthAuthority(FVector2D WorldPos) const;
    
    void CreateRegionMesh(FWaterMeshRegion& Region);
    
    // Edge seam fix functions
    UFUNCTION(BlueprintCallable, Category = "Water Utilities")
    float GetExactWaterDepthAtWorld(FVector2D WorldPos) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Utilities") 
    FVector2D GetFlowVectorAtWorld(FVector2D WorldPos) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Utilities")
    FVector CalculateWaterNormal(FVector2D WorldPos, FVector2D FlowVector, float WaterDepth) const;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Water")
    UMaterialInterface* VolumeMaterial = nullptr;          // Dedicated surface water material (legacy name)
    
    // ===== PHASE 4: NIAGARA FX INTEGRATION =====
    
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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    float NiagaraUpdateRate = 0.1f; // Update FX every 100ms
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    float MaxNiagaraDistance = 3000.0f; // Max distance for FX spawning
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara FX")
    int32 MaxActiveNiagaraComponents = 32; // Performance limit
    
    // Runtime Niagara component pooling
    TMap<int32, UNiagaraComponent*> ActiveNiagaraComponents;
    TArray<UNiagaraComponent*> NiagaraComponentPool;
    
    // ===== ADVANCED SHADER SYSTEM PROPERTIES =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    UTexture2D* WaterDepthTexture = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    UTexture2D* WaterDataTexture = nullptr;  // RGBA: Depth, Speed, FlowX, FlowY

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    UMaterialParameterCollection* WaterParameterCollection = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    float WaterShaderUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shader")
    float WaterDepthScale = 25.5f;
    
    // ===== PHASE 1 & 2: FLOW DISPLACEMENT SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Displacement")
    UTexture2D* FlowDisplacementTexture = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Displacement")
    float DisplacementScale = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Displacement")
    float WaveAnimationSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Displacement")
    float DisplacementStrength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Displacement")
    float FlowDisplacementUpdateInterval = 0.05f;
    
    // ===== VOLUMETRIC WATER OPTICAL PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Water")
    FWaterMaterialParams VolumetricParams;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    void ApplyVolumetricWaterToMaterial(UMaterialInstanceDynamic* Material);
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    bool IsPlayerUnderwater(APlayerController* Player) const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    float GetWaterDepthAtPlayer(APlayerController* Player) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateShaderSystem(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    void ApplyWaterMaterialParams(UMaterialInstanceDynamic* Material, const FWaterMaterialParams& Params);
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    float GetDynamicTurbidity() const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    FLinearColor GetWeatherAdjustedWaterColor() const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    void UpdateAllWaterVisuals(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    bool HasWaterChanged() const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    int32 GetActiveWaterCellCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    float GetTotalWaterVolume() const;
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    int32 GetActiveVolumeChunkCount() const;

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CreateWaterDepthTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CreateAdvancedWaterTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateWaterDepthTexture();
    
    // UE5.4 Render Thread Texture Update
    void UpdateWaterDepthTextureRenderThread();
    
    // UE5.4 Enhanced Input System Integration
    void HandleEnhancedInput(const struct FInputActionValue& ActionValue, FVector CursorWorldPosition);
    
    // UE5.4 Lumen Integration
    void ConfigureLumenReflections();
    
    // UE5.4 Nanite Compatibility
    void ConfigureNaniteCompatibility();
    
    // UE5.4 World Partition Support
    void ConfigureWorldPartitionStreaming();
    
    // UE5.4 Niagara 5.0 Integration
    void UpdateNiagaraFX_UE54(float DeltaTime);
    
    // UE5.4 Performance Profiling
    void ProfileWaterSystemPerformance();
    
    // UE5.4 Initialization Sequence
    void InitializeForUE54();
    
    // UE5.4 Async Texture Loading
    void LoadWaterTexturesAsync();
    
    // UE5.4 Memory Management
    void OptimizeMemoryForUE54();
    
    // UE5.4 Validation Functions
    void ValidateShaderTextureAlignment();
    void FixChunkUVMapping();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateAdvancedWaterTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CalculateFoamData();
    
    // ===== FLOW DISPLACEMENT FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Displacement")
    void CreateFlowDisplacementTexture();
    
    UFUNCTION(BlueprintCallable, Category = "Water Displacement")
    void UpdateFlowDisplacementTexture();
    
    UFUNCTION(BlueprintCallable, Category = "Water Displacement")
    void UpdateWaterShaderForDisplacement(UMaterialInstanceDynamic* Material);
    
    UFUNCTION(BlueprintCallable, Category = "Water Displacement")
    FVector2D GetDominantFlowDirection() const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Displacement")
    float GenerateWavePhase(int32 Index) const;
    
    // ===== SURFACE-BASED WATER FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    void UpdateWaterSurfaceChunks();
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    bool ShouldGenerateSurfaceForChunk(int32 ChunkIndex) const;
    
    void CreateWaterSurfaceMesh(FWaterSurfaceChunk& SurfaceChunk);
    
    void CreateWaterSurfaceMesh_AlwaysVisible(FWaterSurfaceChunk& SurfaceChunk);
    
    void GenerateSmoothWaterSurface(FWaterSurfaceChunk& SurfaceChunk,
                                   TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                   TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                   TArray<FColor>& VertexColors);
                                   
    void GenerateSmoothWaterSurface_HighQuality(FWaterSurfaceChunk& SurfaceChunk,
                                                TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                                TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                                TArray<FColor>& VertexColors);
                                   
    float GetInterpolatedWaterDepth(FVector2D WorldPosition) const;
    
    void UpdateSurfaceUVMapping(FWaterSurfaceChunk& SurfaceChunk);
    
    // ===== EDGE SEAM FIX FUNCTIONS =====
    
    /** Helper function for exact boundary vertex matching */
 //   float GetExactWaterDepthAtWorld(FVector2D WorldPos) const;
    
    /** Calculate distance to water edge for shore blending */
    float CalculateDistanceToWaterEdge(FVector2D WorldPos, const FWaterSurfaceChunk& SurfaceChunk) const;
    
    /** Calculate shore blend factor for seamless transitions */
    float CalculateShoreBlendFactor(float WaterDepth, float DistanceToEdge) const;
    
    /** Apply shore blending to pull water surface below terrain */
    float ApplyShoreBlending(float WaterDepth, float BlendFactor, float DistanceToEdge) const;
    
    /** Enhanced normal calculation considering flow direction */
 //   FVector CalculateWaterNormal(FVector2D WorldPos, FVector2D FlowVector, float WaterDepth) const;
    
    /** Get flow vector at world position */
 //   FVector2D GetFlowVectorAtWorld(FVector2D WorldPos) const;
    
    /** Generate triangle indices for water surface */
    void GenerateWaterSurfaceTriangles(int32 Resolution, TArray<int32>& Triangles);
    
    // PHASE 1-2: SIMULATION AUTHORITY FUNCTIONS
    
    void SynchronizeChunkWithSimulation(FWaterSurfaceChunk& SurfaceChunk);
    
    UFUNCTION(BlueprintCallable, Category = "Water Simulation Authority")
    float GetChunkMaxDepthFromSimulation(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Simulation Authority")
    float GetChunkAverageDepthFromSimulation(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Simulation Authority")
    FVector2D GetChunkFlowDirectionFromSimulation(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Simulation Authority")
    float GetChunkFlowSpeedFromSimulation(int32 ChunkIndex) const;
    
    UFUNCTION(BlueprintCallable, Category = "Water Simulation Authority")
    bool GetChunkFoamStateFromSimulation(int32 ChunkIndex) const;
    
    // ===== PHASE 4: NIAGARA FX FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Niagara FX")
    void UpdateNiagaraFX(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Niagara FX")
    void SpawnNiagaraFXForChunk(int32 ChunkIndex);
    
    void UpdateNiagaraParameters(UNiagaraComponent* NiagaraComp, const FWaterSurfaceChunk& SurfaceChunk);
    
    UFUNCTION(BlueprintCallable, Category = "Niagara FX")
    UNiagaraComponent* GetPooledNiagaraComponent(UNiagaraSystem* SystemTemplate);
    
    UFUNCTION(BlueprintCallable, Category = "Niagara FX")
    void ReturnNiagaraComponentToPool(UNiagaraComponent* Component);
    
    UFUNCTION(BlueprintCallable, Category = "Niagara FX")
    void CleanupDistantNiagaraFX(FVector CameraLocation);

    // ==============================================
    // ENHANCED LOD & POOLING SYSTEM FUNCTIONS
    // ==============================================
    
    void LogWaterMeshStats() const;

    // ===== TERRAIN SYNCHRONIZATION =====
    void NotifyTerrainChanged() { bTerrainChanged = true; bForceTerrainResync = true; }
    void SynchronizeWithTerrainChanges();
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void NotifyTerrainEdited() { bTerrainChanged = true; bForceTerrainResync = true; }
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ForceTerrainSync();
    
    bool bTerrainChanged = false;
    
    // Terrain height caching for performance
    TArray<float> CachedTerrainHeights;
    float LastTerrainSyncTime = 0.0f;
    bool bForceTerrainResync = false;
    
    void RefreshTerrainHeightCache();
    void ForceWaterReflow();
    
    // Enhanced pooling functions
    UProceduralMeshComponent* GetValidatedPooledComponent();
    void ReturnComponentToValidatedPool(UProceduralMeshComponent* Component);
    bool ValidateComponentForPooling(UProceduralMeshComponent* Component) const;
    void ValidateAndCleanPool();
    UProceduralMeshComponent* CreateNewMeshComponent();
    
    // Fixed update system functions
    void UpdateWaterSurfaceChunksFixed();
    void GetVisibleWaterChunks(FVector CameraLocation, TArray<int32>& OutVisibleChunks);
    void UpdateExistingChunk(FWaterSurfaceChunk& Chunk, FVector CameraLocation);
    int32 CalculateWaterMeshLODStable(float Distance) const;
    bool HasWaterDepthChangedSignificantly(int32 ChunkIndex) const;
    void CreateNewWaterChunk(int32 ChunkIndex);
    void CreateWaterSurfaceMeshStable(FWaterSurfaceChunk& SurfaceChunk);
    void GenerateStableWaterMesh(FWaterSurfaceChunk& SurfaceChunk,
        TArray<FVector>& Vertices, TArray<int32>& Triangles,
        TArray<FVector>& Normals, TArray<FVector2D>& UVs,
        TArray<FColor>& VertexColors);
    void ApplyMaterialToChunk(FWaterSurfaceChunk& Chunk);
    void CleanupDistantChunks(FVector CameraLocation);
    void ReplaceExistingUpdateFunction();
    
    // Fixed coordinate authority function
    float GetInterpolatedWaterDepthFixed(FVector2D WorldPosition) const;
    
    // Get camera location helper
    FVector GetCameraLocation() const;

private:
    // ===== AUTHORITY CONSOLIDATION =====
    
    UPROPERTY()
    class AMasterWorldController* CachedMasterController = nullptr;
    
    // ===== ISCALABLESYSTEM STATE =====
    
    FWorldScalingConfig CurrentWorldConfig;
    FWorldCoordinateSystem CurrentCoordinateSystem;
    bool bIsScaledByMaster = false;
    
    // Initialize with authority chain
    void Initialize(ADynamicTerrain* InTerrain);
    
    // Helper function for simulation validation
    int32 CountActiveWaterCellsInChunk(int32 ChunkIndex) const;
    
    // Enhanced validation functions for phantom water elimination
    bool CheckForContiguousWater(int32 ChunkIndex) const;
    int32 FloodFillWaterArea(int32 StartX, int32 StartY, int32 MinX, int32 MinY, 
                            int32 MaxX, int32 MaxY, TSet<int32>& VisitedCells) const;
    void ValidateAndCleanupPhantomChunks();
    
    // Missing function declarations
    int32 GetSimulationIndex(FVector2D WorldPos) const;
    float GetSimulationDepth(FVector2D WorldPos) const;
    FVector2D GetSimulationVelocity(FVector2D WorldPos) const;
   
    
  
  /*
    // Direct simulation data access
    int32 GetSimulationIndex(FVector2D WorldPos) const;
    float GetSimulationDepth(FVector2D WorldPos) const;
    FVector2D GetSimulationVelocity(FVector2D WorldPos) const;
    
    // Shore blending functions
    float CalculateDistanceToWaterEdge(FVector2D WorldPos, const FWaterSurfaceChunk& SurfaceChunk) const;
    float CalculateShoreBlendFactor(float WaterDepth, float DistanceToEdge) const;
    float ApplyShoreBlending(float WaterDepth, float BlendFactor, float DistanceToEdge) const;
  */
    // ===== DEBUG OPTIMIZATION VARIABLES =====
    // Debug optimization variables
    static float LastDebugLogTime;
    static int32 DebugLogCounter;
    static const float DEBUG_LOG_INTERVAL;
    
public:
    // Debug control flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Control")
    bool bEnableVerboseLogging = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Control")
    bool bThrottleDebugLogs = true;
    
    // Public interface function
    UFUNCTION(BlueprintPure, Category = "System Status")
    bool IsSystemScaled() const { return CachedMasterController != nullptr; }

private:
    
    // ===== INTERNAL STATE =====
    
    UPROPERTY()
    ADynamicTerrain* OwnerTerrain = nullptr;
    
    // Water simulation data
    FWaterSimulationData SimulationData;
    
    // Weather system
    float WeatherTimer = 0.0f;
    
    // Performance tracking
    TSet<int32> ChunksWithWater;
    float TotalWaterAmount = 0.0f;
    
    // ===== INTERNAL FUNCTIONS =====
    
    // Core water simulation
    void CalculateWaterFlow(float DeltaTime);
    void ApplyWaterFlow(float DeltaTime);
    void ProcessWaterEvaporation(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void ApplyRain(float DeltaTime);
    
    // Erosion simulation
    void UpdateErosion(float DeltaTime);
    
    // Helper functions
    float GetWaterDepthSafe(int32 X, int32 Y) const;
    void SetWaterDepthSafe(int32 X, int32 Y, float Depth);
    int32 GetTerrainIndex(int32 X, int32 Y) const;
    bool IsValidCoordinate(int32 X, int32 Y) const;
    
    // Terrain interface functions
    float GetTerrainHeightSafe(int32 X, int32 Y) const;
    FVector2D WorldToTerrainCoordinates(FVector WorldPosition) const;
    void MarkChunkForUpdate(int32 X, int32 Y);
    
    // Shader system functions
    void UpdateWaterShaderParameters();
    
    // ===== CONSTRUCTOR INITIALIZATION =====
    void InitializeWaterQualityDefaults();
    
};
