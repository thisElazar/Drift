// DriftGameInstance.h - Game Settings & Map Management
// SIMPLIFIED: Fixed 513x513 grid, world scaling handles physical size
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DriftGameInstance.generated.h"

// Forward declarations
class AMasterWorldController;
class UDEMImporter;

// ============================================================================
// WORLD CONFIGURATION
// ============================================================================

/**
 * FIXED GRID SIZE - All terrains use 513x513 heightmap
 * Physical world size varies through TerrainScale parameter
 * Future: Streaming will tile multiple 513x513 grids
 */
UENUM(BlueprintType)
enum class EWorldSize : uint8
{
    Small   UMETA(DisplayName = "Small (256x256)"),
    Medium  UMETA(DisplayName = "Medium (513x513)"),
    Large   UMETA(DisplayName = "Large (1024x1024)")
};

static constexpr int32 TERRAIN_GRID_SIZE = 513;

UENUM(BlueprintType)
enum class EDefaultTexture : uint8
{
    Wireframe    UMETA(DisplayName = "Digital Wireframe"),
    Natural      UMETA(DisplayName = "Natural Landscape"),
    Hybrid       UMETA(DisplayName = "Hybrid Mode")
};

// ============================================================================
// TERRAIN GENERATION SYSTEM
// ============================================================================

/**
 * Terrain generation modes for map creation
 */
UENUM(BlueprintType)
enum class ETerrainGenerationMode : uint8
{
    Procedural      UMETA(DisplayName = "Random Procedural"),
    ProceduralSeed  UMETA(DisplayName = "Seeded Procedural"),
    DEM             UMETA(DisplayName = "Real-World DEM"),
    Preset          UMETA(DisplayName = "Preset Heightmap")
};

/**
 * Complete map definition including generation method and parameters
 * All maps use 513x513 grid - physical size controlled by TerrainScale
 */
USTRUCT(BlueprintType)
struct FTerrainMapDefinition
{
    GENERATED_BODY()

    // ===== METADATA =====

    /** Display name shown in menu */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FText DisplayName = FText::FromString(TEXT("Unnamed Map"));

    /** Description for menu tooltip */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FText Description = FText::FromString(TEXT(""));

    /** Preview thumbnail for map selection UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    UTexture2D* PreviewThumbnail = nullptr;

    /** Geographic location name (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FString LocationName = TEXT("");

    // ===== GENERATION SETTINGS =====

    /** Primary generation mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    ETerrainGenerationMode GenerationMode = ETerrainGenerationMode::Procedural;

    /**
     * Physical world scale in UE units per grid cell
     * Default 100.0 = 51.3km world (513 * 100 = 51,300 units = 51.3km)
     * Scale 50.0 = 25.65km world (smaller physical area)
     * Scale 200.0 = 102.6km world (larger physical area)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainScale = 100.0f;

    // ===== PROCEDURAL PARAMETERS =====

    /** Procedural seed (-1 = random, >=0 = deterministic) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 ProceduralSeed = -1;

    /** Height variation multiplier (vertical scale) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    float HeightVariation = 500.0f;

    /** Noise scale for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    float NoiseScale = 0.01f;

    /** Number of octaves for multi-frequency terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 NoiseOctaves = 4;

    // ===== DEM PARAMETERS =====

    /** Path to DEM file (relative to project or absolute) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEM")
    FString DEMFilePath = TEXT("");

    /** DEM file format (Auto = detect from extension) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEM")
    uint8 DEMFormat = 0; // Using uint8 to avoid enum dependency

    /** Whether to normalize DEM elevation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEM")
    bool bNormalizeDEMElevation = true;

    /** Target max height after normalization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEM")
    float DEMNormalizedHeight = 2000.0f;

    // ===== PRESET HEIGHTMAP =====

    /** Preset heightmap texture (for curated maps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    UTexture2D* PresetHeightmap = nullptr;

    // ===== ENVIRONMENTAL SETTINGS =====

    /** Default latitude for atmospheric simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float DefaultLatitude = 45.0f;

    /** Number of procedural springs to generate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 NumProceduralSprings = 5;

    /** Initial spring flow rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SpringFlowRate = 20000.0f;

    /** Enable weather simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableWeather = true;

    // ===== UTILITY FUNCTIONS =====


    /** Check if this is a random map */
    bool IsRandomMap() const
    {
        return GenerationMode == ETerrainGenerationMode::Procedural && ProceduralSeed < 0;
    }

    /** Check if DEM file path is valid */
    bool HasValidDEMPath() const
    {
        return !DEMFilePath.IsEmpty();
    }

    /** Get human-readable generation mode name */
    FString GetGenerationModeName() const;

    /** Get physical world size in kilometers */
    float GetWorldSizeKm() const
    {
        return (TERRAIN_GRID_SIZE * TerrainScale) / 100000.0f; // Convert cm to km
    }

    /** Get grid size (always 513x513) */
    int32 GetGridSize() const
    {
        return TERRAIN_GRID_SIZE;
    }

    /** Constructor with defaults */
    FTerrainMapDefinition()
    {
        DisplayName = FText::FromString(TEXT("Random Procedural"));
        GenerationMode = ETerrainGenerationMode::Procedural;
        ProceduralSeed = -1;
        TerrainScale = 100.0f; // Default 51.3km world
    }
};

// ============================================================================
// GAME INSTANCE CLASS
// ============================================================================

UCLASS(BlueprintType)
class DRIFT_API UDriftGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UDriftGameInstance();

    // ===== GAME SETTINGS =====

    /** Visual rendering mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    EDefaultTexture DefaultTexture = EDefaultTexture::Natural;

    /** Enable water physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    bool bEnableWaterPhysics = true;

    /** Water simulation time scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float WaterPhysicsTimeScale = 5.0f;

    /** Atmospheric simulation time scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float AtmosphericTimeScale = 5.0f;

    /** Default spring flow rate (m^3/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float SpringFlowRate = 200000.0f;

    /** Default number of springs to generate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    int32 NumProceduralSprings = 5;

    // ===== MAP SYSTEM =====

    /**
     * Registry of available maps
     * Edit this in Project Settings -> Maps & Modes -> Game Instance
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
    TArray<FTerrainMapDefinition> AvailableMaps;

    /**
     * Currently selected map index (-1 = random, 0+ = specific map)
     */
    UPROPERTY(BlueprintReadWrite, Category = "Maps")
    int32 SelectedMapIndex = -1;

    /**
     * Current map definition being used
     */
    UPROPERTY(BlueprintReadOnly, Category = "Maps")
    FTerrainMapDefinition CurrentMapDefinition;

    /**
     * Whether CurrentMapDefinition is valid and ready to use
     */
    UPROPERTY(BlueprintReadOnly, Category = "Maps")
    bool bHasCurrentMapDefinition = false;

    // ===== MAP SELECTION FUNCTIONS =====

    /**
     * Select a specific map by index
     * @param MapIndex - Index into AvailableMaps (-1 for random)
     */
    UFUNCTION(BlueprintCallable, Category = "Maps")
    void SelectMap(int32 MapIndex);

    /**
     * Start new game with specific map
     * @param MapIndex - Index into AvailableMaps (-1 for random)
     */
    UFUNCTION(BlueprintCallable, Category = "Maps")
    void StartNewWorldWithMap(int32 MapIndex);

    /**
     * Start new game with current selected map
     */
    UFUNCTION(BlueprintCallable, Category = "Maps")
    void StartNewWorldWithSelectedMap();

    /**
     * Generate a random map definition
     */
    UFUNCTION(BlueprintCallable, Category = "Maps")
    FTerrainMapDefinition GenerateRandomMapDefinition();

    // ===== MAP QUERY FUNCTIONS =====

    /**
     * Get the current map definition
     */
    UFUNCTION(BlueprintPure, Category = "Maps")
    FTerrainMapDefinition GetCurrentMapDefinition() const
    {
        return CurrentMapDefinition;
    }

    /**
     * Check if we have a valid map definition ready
     */
    UFUNCTION(BlueprintPure, Category = "Maps")
    bool HasMapDefinition() const
    {
        return bHasCurrentMapDefinition;
    }

    /**
     * Get all available map definitions
     */
    UFUNCTION(BlueprintPure, Category = "Maps")
    TArray<FTerrainMapDefinition> GetAvailableMapDefinitions() const
    {
        return AvailableMaps;
    }

    /**
     * Get number of available predefined maps
     */
    UFUNCTION(BlueprintPure, Category = "Maps")
    int32 GetAvailableMapCount() const
    {
        return AvailableMaps.Num();
    }

    /**
     * Get specific map definition by index
     */
    UFUNCTION(BlueprintPure, Category = "Maps")
    FTerrainMapDefinition GetMapDefinitionByIndex(int32 Index) const;

    // ===== UTILITY FUNCTIONS =====

    /**
     * Get fixed terrain grid size (always 513)
     */
    UFUNCTION(BlueprintPure, Category = "Game Settings")
    int32 GetTerrainSize() const
    {
        return TERRAIN_GRID_SIZE;
    }

    /**
     * Start new world with default settings (legacy)
     */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    void StartNewWorld();

    // Legacy Property
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    EWorldSize WorldSize = EWorldSize::Medium;

    // Legacy Function
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    void SetWorldSize(EWorldSize NewSize);

    /**
     * Quit the game
     */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    void QuitGame();

    // ===== MENU INTEGRATION =====

    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    bool bHasPreviewSettings = false;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SetPreviewSettings(EDefaultTexture InTexture, bool bInWeather);

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartGameWithPreviewSettings();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ReturnToMainMenu();

private:
    // ===== INTERNAL HELPERS =====

    void ApplyPreviewSettingsToGame();

    /** Generate default procedural definition with current settings */
    FTerrainMapDefinition CreateDefaultProceduralDefinition();

    /** Initialize default maps if AvailableMaps is empty */
    void InitializeDefaultMaps();
};
