// DynamicTerrain.h - Clean Terrain System (Water System Separate)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "WaterSystem.h"
#include "AtmosphericSystem.h"
#include "DynamicTerrain.generated.h"

// Terrain chunk structure for organized mesh management
USTRUCT(BlueprintType)
struct FTerrainChunk
{
    GENERATED_BODY()

    // Mesh component for this chunk
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProceduralMeshComponent* MeshComponent = nullptr;

    // Chunk grid position
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ChunkX = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ChunkY = 0;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bNeedsUpdate = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsActive = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LastUpdateTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsVisible = true;

    FTerrainChunk()
    {
        MeshComponent = nullptr;
        ChunkX = 0;
        ChunkY = 0;
        bNeedsUpdate = false;
        bIsActive = true;
        LastUpdateTime = 0.0f;
        bIsVisible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API ADynamicTerrain : public AActor
{
    GENERATED_BODY()
    
public:
    ADynamicTerrain();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== CORE TERRAIN FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateSimpleTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateProceduralTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void ModifyTerrain(FVector WorldPosition, float Radius, float Strength, bool bRaise = true);

    UFUNCTION(BlueprintCallable, Category = "Terrain Editing")
    void ModifyTerrainAtIndex(int32 X, int32 Y, float Radius, float Strength, bool bRaise = true);

    // ===== UTILITY FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    FVector2D WorldToTerrainCoordinates(FVector WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    FVector TerrainToWorldPosition(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtPosition(FVector WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtIndex(int32 X, int32 Y) const;

    // Safe height map access methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightSafe(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    void SetHeightSafe(int32 X, int32 Y, float Height);

    // ===== CHUNK MANAGEMENT =====
    
    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void UpdateChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    int32 GetChunkIndexFromCoordinates(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Chunk System")
    void MarkChunkForUpdate(int32 ChunkIndex);
    
    // ===== VOLUMETRIC WATER SUPPORT FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Volumetric Water")
    FVector GetChunkWorldPosition(int32 ChunkIndex) const;

    // ===== WATER SYSTEM INTEGRATION =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water System")
    UWaterSystem* WaterSystem;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric System")
    UAtmosphericSystem* AtmosphericSystem;
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void InitializeWaterSystem();
    
    // ===== WATER SYSTEM ACCESS ONLY =====
    
    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsWaterSystemReady() const;
    
    // ===== ATMOSPHERIC SYSTEM INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void InitializeAtmosphericSystem();
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    float GetTemperatureAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    float GetPrecipitationAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    FVector GetWindAt(FVector WorldPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateWeatherSystem(int32 WeatherType, FVector2D Center, float Strength);

    // ===== PUBLIC TERRAIN SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 TerrainWidth = 513;  // Large terrain for open world
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 TerrainHeight = 513;  // Large terrain for open world
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float TerrainScale = 100.0f;  // 51.3km x 51.3km terrain

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float MaxTerrainHeight = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float MinTerrainHeight = -2000.0f;

    // ===== CHUNK SYSTEM CONFIGURATION =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
    int32 ChunkSize = 33;  // 33x33 vertices per chunk

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
    int32 ChunkOverlap = 1;  // 1 vertex overlap for seamless chunks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
    int32 MaxUpdatesPerFrame = 4;  // Max chunks to update per frame

    // ===== BRUSH SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
    float BrushRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
    float BrushStrength = 200.0f;

    // ===== PERFORMANCE SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ModificationCooldown = 0.05f; // 20 modifications per second max

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowPerformanceStats = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowChunkBounds = false;

    // ===== PUBLIC CHUNK ACCESS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk System")
    TArray<FTerrainChunk> TerrainChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk System")
    int32 ChunksX = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk System")
    int32 ChunksY = 0;
    
    // ===== FRUSTUM CULLING SYSTEM =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrustumCulling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingUpdateRate = 0.1f;
    
    // ===== MATERIAL MANAGEMENT =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
    UMaterialInterface* CurrentActiveMaterial = nullptr;
    
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetActiveMaterial(UMaterialInterface* Material);
    
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetWaterVolumeMaterial(UMaterialInterface* Material);

private:
    // ===== INTERNAL COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere)
    USceneComponent* TerrainRoot;
    
    // ===== TERRAIN DATA =====
    
    TArray<float> HeightMap;
    
    // ===== CHUNK MANAGEMENT =====
    
    TSet<int32> PendingChunkUpdates;
    
    // ===== PERFORMANCE OPTIMIZATION =====
    
    float LastModificationTime = 0.0f;
    int32 TotalChunkUpdatesThisFrame = 0;
    float StatUpdateTimer = 0.0f;
    
    // Frustum culling variables
    float CullingUpdateTimer = 0.0f;
    int32 CurrentVisibleChunks = 0;
    
    // ===== INTERNAL FUNCTIONS =====
    
    // Terrain generation and modification
    void InitializeChunks();
    void GenerateChunkMesh(int32 ChunkX, int32 ChunkY);
    void ProcessPendingChunkUpdates();
    void UpdatePerformanceStats(float DeltaTime);
    
    // Helper functions
    FVector2D GetChunkWorldPosition(int32 ChunkX, int32 ChunkY) const;
    FVector CalculateVertexNormal(int32 X, int32 Y) const;
    FLinearColor GetHeightBasedColor(float NormalizedHeight) const;
    
    // Frustum culling functions
    void UpdateFrustumCulling(float DeltaTime);
    bool IsChunkVisible(const FTerrainChunk& Chunk, const FSceneView* View) const;
    FBoxSphereBounds GetChunkWorldBounds(const FTerrainChunk& Chunk) const;
    
    // Thread-safe height map modifications
    FCriticalSection HeightMapMutex;
};
