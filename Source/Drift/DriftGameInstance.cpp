// DriftGameInstance.cpp - Game Settings & Map Management Implementation
// SIMPLIFIED: Fixed 513x513 grid, world scaling handles physical size
#include "DriftGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MasterController.h"

// ============================================================================
// TERRAIN MAP DEFINITION UTILITY FUNCTIONS
// ============================================================================

FString FTerrainMapDefinition::GetGenerationModeName() const
{
    switch (GenerationMode)
    {
        case ETerrainGenerationMode::Procedural:
            return TEXT("Random Procedural");
        case ETerrainGenerationMode::ProceduralSeed:
            return TEXT("Seeded Procedural");
        case ETerrainGenerationMode::DEM:
            return TEXT("Real-World DEM");
        case ETerrainGenerationMode::Preset:
            return TEXT("Preset Heightmap");
        default:
            return TEXT("Unknown");
    }
}

// ============================================================================
// GAME INSTANCE INITIALIZATION
// ============================================================================

UDriftGameInstance::UDriftGameInstance()
{
    // Initialize default settings
    DefaultTexture = EDefaultTexture::Natural;
    bEnableWaterPhysics = true;
    bHasPreviewSettings = false;

    // Initialize map system
    SelectedMapIndex = -1; // Default to random
    bHasCurrentMapDefinition = false;

    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: SIMPLIFIED MODE - Fixed 513x513 grid, scaling via TerrainScale"));

    // Populate default maps on first run
    InitializeDefaultMaps();

    // SURGICAL FIX: Auto-select map 0 if starting from BaseMap directly
      // This ensures we always have a valid map definition for direct BaseMap loads
      if (AvailableMaps.Num() > 0)
      {
          SelectMap(0);  // Select first available map (Random Terrain)
          UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Auto-selected map 0 for direct BaseMap loads"));
      }
}

void UDriftGameInstance::InitializeDefaultMaps()
{
    // Only initialize if empty (allows manual configuration in editor)
    if (AvailableMaps.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Using %d pre-configured maps"),
               AvailableMaps.Num());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Initializing default map configurations (513x513 grid)"));

    // Default Map 1: Random Procedural (always available)
    FTerrainMapDefinition RandomMap;
    RandomMap.DisplayName = FText::FromString(TEXT("Random Terrain"));
    RandomMap.Description = FText::FromString(TEXT("Generates a new procedural landscape each time"));
    RandomMap.GenerationMode = ETerrainGenerationMode::Procedural;
    RandomMap.ProceduralSeed = -1; // Random
    RandomMap.HeightVariation = 500.0f;
    RandomMap.NoiseScale = 0.01f;
    RandomMap.TerrainScale = 100.0f; // 51.3km world
    RandomMap.DefaultLatitude = 45.0f;
    RandomMap.NumProceduralSprings = 5;
    AvailableMaps.Add(RandomMap);

    // Default Map 2: Seeded Valley (reproducible, smaller scale)
    FTerrainMapDefinition ValleyMap;
    ValleyMap.DisplayName = FText::FromString(TEXT("Tutorial Valley"));
    ValleyMap.Description = FText::FromString(TEXT("A gentle valley perfect for learning water flow (25km)"));
    ValleyMap.GenerationMode = ETerrainGenerationMode::ProceduralSeed;
    ValleyMap.ProceduralSeed = -1; // Fixed seed
    ValleyMap.HeightVariation = 300.0f;
    ValleyMap.NoiseScale = 0.008f;
    ValleyMap.TerrainScale = 100.0f; // 25.65km world (smaller scale)
    ValleyMap.DefaultLatitude = 40.0f;
    ValleyMap.NumProceduralSprings = 3;
    AvailableMaps.Add(ValleyMap);

    // Default Map 3: Mountain Range (dramatic terrain, larger scale)
    FTerrainMapDefinition MountainMap;
    MountainMap.DisplayName = FText::FromString(TEXT("Mountain Range"));
    MountainMap.Description = FText::FromString(TEXT("Steep mountains with dramatic watersheds (100km)"));
    MountainMap.GenerationMode = ETerrainGenerationMode::ProceduralSeed;
    MountainMap.ProceduralSeed = -1;
    MountainMap.HeightVariation = 1200.0f;
    MountainMap.NoiseScale = 0.015f;
    MountainMap.NoiseOctaves = 6;
    MountainMap.TerrainScale = 100.0f; // 102.6km world (larger scale)
    MountainMap.DefaultLatitude = 50.0f;
    MountainMap.NumProceduralSprings = 8;
    AvailableMaps.Add(MountainMap);

    // Default Map 4: Coastal Plains (flat terrain for testing)
    FTerrainMapDefinition PlainsMap;
    PlainsMap.DisplayName = FText::FromString(TEXT("Coastal Plains"));
    PlainsMap.Description = FText::FromString(TEXT("Gentle rolling plains near sea level (51km)"));
    PlainsMap.GenerationMode = ETerrainGenerationMode::ProceduralSeed;
    PlainsMap.ProceduralSeed = -1;
    PlainsMap.HeightVariation = 150.0f;
    PlainsMap.NoiseScale = 0.005f;
    PlainsMap.NoiseOctaves = 3;
    PlainsMap.TerrainScale = 100.0f; // Standard 51.3km
    PlainsMap.DefaultLatitude = 30.0f;
    PlainsMap.NumProceduralSprings = 4;
    AvailableMaps.Add(PlainsMap);

    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Initialized %d default maps (all 513x513 grid)"),
           AvailableMaps.Num());
}

// ============================================================================
// MAP SELECTION & LOADING
// ============================================================================

void UDriftGameInstance::SelectMap(int32 MapIndex)
{
    SelectedMapIndex = MapIndex;

    if (MapIndex < 0)
    {
        // Random map
        UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Selected RANDOM map generation"));
        CurrentMapDefinition = GenerateRandomMapDefinition();
        bHasCurrentMapDefinition = true;
    }
    else if (MapIndex < AvailableMaps.Num())
    {
        // Specific map
        CurrentMapDefinition = AvailableMaps[MapIndex];
        bHasCurrentMapDefinition = true;
        UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Selected map %d: %s (%.1fkm, scale=%.1f)"),
               MapIndex,
               *CurrentMapDefinition.DisplayName.ToString(),
               CurrentMapDefinition.GetWorldSizeKm(),
               CurrentMapDefinition.TerrainScale);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DriftGameInstance: Invalid map index %d (max %d)"),
               MapIndex, AvailableMaps.Num() - 1);
        // Fallback to random
        SelectedMapIndex = -1;
        CurrentMapDefinition = GenerateRandomMapDefinition();
        bHasCurrentMapDefinition = true;
    }
}

// Legacy Function
void UDriftGameInstance::SetWorldSize(EWorldSize NewSize)
{
    WorldSize = NewSize;
    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: World size set to %d"), (int32)NewSize);

    // If we're in a game world, update the MasterController immediately
    //ApplyWorldSizeToMasterController();
}


void UDriftGameInstance::StartNewWorldWithMap(int32 MapIndex)
{
    UE_LOG(LogTemp, Verbose, TEXT("DriftGameInstance: Starting new world with map index %d"), MapIndex);

    // Select and prepare the map
    SelectMap(MapIndex);

    // Load BaseMap - it will read CurrentMapDefinition when it initializes
    UGameplayStatics::OpenLevel(this, FName("BaseMap"));
}

void UDriftGameInstance::StartNewWorldWithSelectedMap()
{
    StartNewWorldWithMap(SelectedMapIndex);
}

FTerrainMapDefinition UDriftGameInstance::GenerateRandomMapDefinition()
{
    FTerrainMapDefinition RandomDef;

    // Basic metadata
    int32 RandomSeed = FMath::Rand();
    RandomDef.DisplayName = FText::FromString(TEXT("Random Terrain"));
    RandomDef.Description = FText::FromString(
        FString::Printf(TEXT("Randomly generated (Seed: %d)"), RandomSeed));

    // Random generation mode
    RandomDef.GenerationMode = ETerrainGenerationMode::Procedural;
    RandomDef.ProceduralSeed = -1; // Truly random

    // Random parameters within sensible ranges
    RandomDef.HeightVariation = FMath::RandRange(200.0f, 1000.0f);
    RandomDef.NoiseScale = FMath::RandRange(0.005f, 0.02f);
    RandomDef.NoiseOctaves = FMath::RandRange(3, 6);

    // Random terrain scale (affects physical world size)
    // 50.0 = 25km, 100.0 = 51km, 150.0 = 77km, 200.0 = 103km
    float ScaleChoices[] = { 50.0f, 75.0f, 100.0f, 150.0f, 200.0f };
    RandomDef.TerrainScale = ScaleChoices[FMath::RandRange(0, 4)];

    // Random latitude (affects weather patterns)
    RandomDef.DefaultLatitude = FMath::RandRange(0.0f, 60.0f);

    // Random spring count
    RandomDef.NumProceduralSprings = FMath::RandRange(3, 10);
    RandomDef.SpringFlowRate = SpringFlowRate;
    RandomDef.bEnableWeather = bEnableWaterPhysics;

    UE_LOG(LogTemp, Log, TEXT("Generated random map: Height=%.1f, Scale=%.4f, TerrainScale=%.1f (%.1fkm), Springs=%d"),
           RandomDef.HeightVariation,
           RandomDef.NoiseScale,
           RandomDef.TerrainScale,
           RandomDef.GetWorldSizeKm(),
           RandomDef.NumProceduralSprings);

    return RandomDef;
}

// ============================================================================
// MAP QUERY FUNCTIONS
// ============================================================================

FTerrainMapDefinition UDriftGameInstance::GetMapDefinitionByIndex(int32 Index) const
{
    if (Index >= 0 && Index < AvailableMaps.Num())
    {
        return AvailableMaps[Index];
    }

    // Return random map definition as fallback
    UE_LOG(LogTemp, Warning, TEXT("GetMapDefinitionByIndex: Invalid index %d, returning random"), Index);
    FTerrainMapDefinition RandomDef;
    RandomDef.DisplayName = FText::FromString(TEXT("Random Terrain"));
    RandomDef.GenerationMode = ETerrainGenerationMode::Procedural;
    RandomDef.ProceduralSeed = -1;
    RandomDef.TerrainScale = 100.0f;
    return RandomDef;
}

// ============================================================================
// LEGACY FUNCTIONS (SIMPLIFIED)
// ============================================================================

void UDriftGameInstance::StartNewWorld()
{
    UE_LOG(LogTemp, Verbose, TEXT("DriftGameInstance: Starting new world (legacy method - using default procedural)"));

    // Legacy method - create default procedural definition
    CurrentMapDefinition = CreateDefaultProceduralDefinition();
    bHasCurrentMapDefinition = true;

    UGameplayStatics::OpenLevel(this, FName("BaseMap"));
}

FTerrainMapDefinition UDriftGameInstance::CreateDefaultProceduralDefinition()
{
    FTerrainMapDefinition DefaultDef;
    DefaultDef.DisplayName = FText::FromString(TEXT("Default Procedural"));
    DefaultDef.GenerationMode = ETerrainGenerationMode::Procedural;
    DefaultDef.ProceduralSeed = -1;
    DefaultDef.HeightVariation = 500.0f;
    DefaultDef.NoiseScale = 0.01f;
    DefaultDef.TerrainScale = 100.0f; // Standard 51.3km
    DefaultDef.NumProceduralSprings = NumProceduralSprings;
    DefaultDef.SpringFlowRate = SpringFlowRate;
    DefaultDef.bEnableWeather = bEnableWaterPhysics;
    return DefaultDef;
}

void UDriftGameInstance::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

// ============================================================================
// MENU INTEGRATION (SIMPLIFIED)
// ============================================================================

void UDriftGameInstance::SetPreviewSettings(EDefaultTexture InTexture, bool bInWeather)
{
    DefaultTexture = InTexture;
    bEnableWaterPhysics = bInWeather;
    bHasPreviewSettings = true;

    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Preview settings stored"));
}

void UDriftGameInstance::StartGameWithPreviewSettings()
{
    UE_LOG(LogTemp, Verbose, TEXT("DriftGameInstance: Starting game with preview settings"));

    // Apply settings before level transition
    if (bHasPreviewSettings)
    {
        ApplyPreviewSettingsToGame();
    }

    // If no map selected, use current settings to create definition
    if (!bHasCurrentMapDefinition)
    {
        CurrentMapDefinition = CreateDefaultProceduralDefinition();
        bHasCurrentMapDefinition = true;
    }

    // Load the main game level
    UGameplayStatics::OpenLevel(this, FName("BaseMap"));
}

void UDriftGameInstance::ReturnToMainMenu()
{
    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Returning to main menu"));
    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UDriftGameInstance::ApplyPreviewSettingsToGame()
{
    // Apply texture and weather settings
    UE_LOG(LogTemp, Log, TEXT("DriftGameInstance: Applying preview settings to game"));

    // These are already stored in member variables, ready for MasterController to read
    bHasPreviewSettings = false; // Clear flag after applying
}
