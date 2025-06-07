// WaterSystem.h - Simplified Water Physics System (No Advanced Materials Yet)
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "WaterSystem.generated.h"


// Forward declarations
class ADynamicTerrain;

USTRUCT(BlueprintType)
struct FWaterSimulationData
{
    GENERATED_BODY()

    // Core water physics arrays
    TArray<float> WaterDepthMap;      // Water depth at each point
    TArray<float> WaterVelocityX;     // Water flow velocity East/West
    TArray<float> WaterVelocityY;     // Water flow velocity North/South
    TArray<float> SedimentMap;        // Suspended sediment for erosion
    TArray<float> FoamMap;            // Foam intensity for advanced rendering

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

UCLASS(BlueprintType)
class TERRAI_API UWaterSystem : public UObject
{
    GENERATED_BODY()

public:
    UWaterSystem();

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void ApplyWaterTextureToMaterial(class UMaterialInstanceDynamic* Material);

    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void Initialize(ADynamicTerrain* InTerrain);
    
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

    // ===== WATER SYSTEM SETTINGS =====
    
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
    
    // ===== EROSION SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bEnableErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float MinErosionVelocity = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float DepositionRate = 0.05f;
    
    

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
    
    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateShaderSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CreateWaterDepthTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CreateAdvancedWaterTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateWaterDepthTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void UpdateAdvancedWaterTexture();

    UFUNCTION(BlueprintCallable, Category = "Water Shader")
    void CalculateFoamData();

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
    
};
