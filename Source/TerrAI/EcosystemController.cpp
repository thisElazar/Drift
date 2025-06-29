/**
 * ============================================
 * TERRAI ECOSYSTEM CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "EcosystemController.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

AEcosystemController::AEcosystemController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EcosystemRoot"));
    
    // Initialize default biomes
    AvailableBiomes.Empty();
    
    // Create default grassland biome
    FBiomeData Grassland;
    Grassland.BiomeType = EBiomeType::Grassland;
    Grassland.TemperatureRange = FVector2D(15.0f, 25.0f);
    Grassland.HumidityRange = FVector2D(0.4f, 0.7f);
    Grassland.ElevationRange = FVector2D(0.0f, 300.0f);
    Grassland.BiomeColor = FLinearColor::Green;
    AvailableBiomes.Add(Grassland);
    
    // Initialize default vegetation types
    VegetationTypes.Empty();
    
    FVegetationData Grass;
    Grass.VegetationType = EVegetationType::Grass;
    Grass.MinWaterRequirement = 0.1f;
    Grass.OptimalWaterLevel = 0.3f;
    Grass.GrowthRate = 2.0f;
    Grass.MaxDensity = 50.0f;
    Grass.ElevationRange = FVector2D(0.0f, 1000.0f);
    Grass.PreferredBiomes.Add(EBiomeType::Grassland);
    VegetationTypes.Add(Grass);
}

void AEcosystemController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVegetationMeshes();
}

void AEcosystemController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemInitialized && bEnableVegetationGrowth)
    {
        VegetationUpdateTimer += DeltaTime;
        if (VegetationUpdateTimer >= VegetationUpdateInterval)
        {
            UpdateVegetation(DeltaTime);
            VegetationUpdateTimer = 0.0f;
        }
    }
}

// ===== INITIALIZATION =====

void AEcosystemController::Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water)
{
    if (!Terrain || !Water)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemController: Invalid initialization parameters"));
        return;
    }
    
    TargetTerrain = Terrain;
    WaterSystem = Water;
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemController: Successfully initialized"));
    
    // Initialize biome distribution
    UpdateBiomes();
}

// ===== CORE FUNCTIONS =====

void AEcosystemController::UpdateVegetation(float DeltaTime)
{
    if (!TargetTerrain || !WaterSystem) return;
    
    UpdateVegetationGrowth(DeltaTime);
    
    // Optionally spawn new vegetation
    if (VegetationLocations.Num() < MaxVegetationInstances)
    {
        // Simple random placement - could be made more sophisticated
        FVector RandomLocation = TargetTerrain->GetActorLocation();
        RandomLocation.X += FMath::RandRange(-5000.0f, 5000.0f);
        RandomLocation.Y += FMath::RandRange(-5000.0f, 5000.0f);
        RandomLocation.Z = TargetTerrain->GetHeightAtPosition(RandomLocation);
        
        if (VegetationTypes.Num() > 0 && CanVegetationGrowAt(RandomLocation, VegetationTypes[0]))
        {
            PlantSeed(RandomLocation, EVegetationType::Grass);
        }
    }
}

void AEcosystemController::UpdateBiomes()
{
    if (!TargetTerrain) return;
    
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Updating biome distribution"));
    CalculateBiomeDistribution();
}

void AEcosystemController::PlantSeed(FVector Location, EVegetationType PlantType)
{
    if (VegetationLocations.Num() >= MaxVegetationInstances) return;
    
    VegetationLocations.Add(Location);
    VegetationTypes_Runtime.Add(PlantType);
    
    SpawnVegetationInstance(Location, PlantType);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("EcosystemController: Planted %d at %s"), 
           (int32)PlantType, *Location.ToString());
}

void AEcosystemController::RemoveVegetationInRadius(FVector Location, float Radius)
{
    int32 RemovedCount = 0;
    
    for (int32 i = VegetationLocations.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(VegetationLocations[i], Location);
        if (Distance <= Radius)
        {
            EVegetationType VegType = VegetationTypes_Runtime[i];
            RemoveVegetationInstance(i, VegType);
            
            VegetationLocations.RemoveAt(i);
            VegetationTypes_Runtime.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("EcosystemController: Removed %d vegetation instances"), RemovedCount);
    }
}

// ===== BIOME FUNCTIONS =====

EBiomeType AEcosystemController::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (!TargetTerrain) return EBiomeType::Grassland;
    
    float Elevation = WorldLocation.Z;
    float WaterDepth = WaterSystem ? WaterSystem->GetWaterDepthAtPosition(WorldLocation) : 0.0f;
    
    // Simple biome classification based on elevation and water
    if (WaterDepth > 10.0f)
    {
        return EBiomeType::Wetland;
    }
    else if (Elevation > 800.0f)
    {
        return EBiomeType::Alpine;
    }
    else if (Elevation < 50.0f)
    {
        return EBiomeType::Coastal;
    }
    else
    {
        return EBiomeType::Grassland; // Default
    }
}

float AEcosystemController::GetVegetationDensityAtLocation(FVector WorldLocation) const
{
    EBiomeType LocalBiome = GetBiomeAtLocation(WorldLocation);
    float WaterLevel = WaterSystem ? WaterSystem->GetWaterDepthAtPosition(WorldLocation) : 0.0f;
    
    // Base density on biome and water availability
    float BaseDensity = 0.5f;
    
    switch (LocalBiome)
    {
    case EBiomeType::Desert:
        BaseDensity = 0.1f;
        break;
    case EBiomeType::Forest:
        BaseDensity = 0.9f;
        break;
    case EBiomeType::Wetland:
        BaseDensity = 0.8f;
        break;
    case EBiomeType::Grassland:
        BaseDensity = 0.6f;
        break;
    default:
        BaseDensity = 0.5f;
        break;
    }
    
    // Modify by water availability
    if (WaterLevel > 0.1f)
    {
        BaseDensity *= 1.2f; // More vegetation near water
    }
    
    return FMath::Clamp(BaseDensity, 0.0f, 1.0f);
}

float AEcosystemController::GetTemperatureAtLocation(FVector WorldLocation) const
{
    float Elevation = WorldLocation.Z;
    
    // Temperature decreases with elevation (lapse rate)
    float Temperature = BaseTemperature - (Elevation * 0.006f); // 6°C per 1000m
    
    // Add seasonal variation
    Temperature += FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * SeasonalVariation * 10.0f;
    
    return Temperature;
}

float AEcosystemController::GetHumidityAtLocation(FVector WorldLocation) const
{
    float WaterDepth = WaterSystem ? WaterSystem->GetWaterDepthAtPosition(WorldLocation) : 0.0f;
    
    float Humidity = BaseHumidity;
    
    // Increase humidity near water
    if (WaterDepth > 0.1f)
    {
        Humidity += 0.2f;
    }
    
    // Add seasonal variation
    Humidity += FMath::Sin(GetWorld()->GetTimeSeconds() * 0.15f) * SeasonalVariation;
    
    return FMath::Clamp(Humidity, 0.0f, 1.0f);
}

// ===== SYSTEM COORDINATION =====

void AEcosystemController::OnWeatherChanged(float Temperature, float Humidity)
{
    BaseTemperature = Temperature;
    BaseHumidity = Humidity;
    
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Weather updated - Temp:%.1f Humidity:%.2f"), 
           Temperature, Humidity);
}

void AEcosystemController::OnSeasonChanged(float SeasonValue)
{
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Season changed to %.2f"), SeasonValue);
    
    // Adjust growth rates based on season
    VegetationGrowthRate = 1.0f + (FMath::Sin(SeasonValue * 2.0f * PI) * 0.5f);
}

void AEcosystemController::OnTerrainModified(FVector Location, float Radius)
{
    // Remove vegetation in modified areas
    RemoveVegetationInRadius(Location, Radius);
    
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Terrain modification at %s"), *Location.ToString());
}

// ===== DEBUG & VISUALIZATION =====

void AEcosystemController::DrawBiomeMap(bool bEnable)
{
    if (!bEnable || !TargetTerrain) return;
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemController: Drawing biome map"));
    
    // Simple debug visualization
    for (int32 x = -10; x <= 10; x++)
    {
        for (int32 y = -10; y <= 10; y++)
        {
            FVector TestLocation = TargetTerrain->GetActorLocation();
            TestLocation.X += x * 500.0f;
            TestLocation.Y += y * 500.0f;
            TestLocation.Z = TargetTerrain->GetHeightAtPosition(TestLocation);
            
            EBiomeType LocalBiome = GetBiomeAtLocation(TestLocation);
            
            FColor BiomeColor = FColor::Green; // Default
            switch (LocalBiome)
            {
            case EBiomeType::Desert: BiomeColor = FColor::Yellow; break;
            case EBiomeType::Forest: BiomeColor = FColor::Green; break;
            case EBiomeType::Wetland: BiomeColor = FColor::Blue; break;
            case EBiomeType::Alpine: BiomeColor = FColor::White; break;
            case EBiomeType::Coastal: BiomeColor = FColor::Cyan; break;
            }
            
            DrawDebugSphere(GetWorld(), TestLocation, 50.0f, 8, BiomeColor, false, 5.0f);
        }
    }
}

void AEcosystemController::ShowVegetationStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ECOSYSTEM STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Vegetation Instances: %d / %d"), VegetationLocations.Num(), MaxVegetationInstances);
    UE_LOG(LogTemp, Warning, TEXT("Growth Rate: %.2f"), VegetationGrowthRate);
    UE_LOG(LogTemp, Warning, TEXT("Base Temperature: %.1f°C"), BaseTemperature);
    UE_LOG(LogTemp, Warning, TEXT("Base Humidity: %.2f"), BaseHumidity);
    UE_LOG(LogTemp, Warning, TEXT("Available Biomes: %d"), AvailableBiomes.Num());
}

// ===== PRIVATE FUNCTIONS =====

void AEcosystemController::InitializeVegetationMeshes()
{
    // Create instanced mesh components for each vegetation type
    VegetationMeshes.Empty();
    
    for (const FVegetationData& VegData : VegetationTypes)
    {
        if (VegData.VegetationMesh)
        {
            UInstancedStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
                *FString::Printf(TEXT("VegetationMesh_%d"), (int32)VegData.VegetationType));
            
            MeshComponent->SetupAttachment(RootComponent);
            MeshComponent->SetStaticMesh(VegData.VegetationMesh);
            MeshComponent->SetCastShadow(false); // Performance optimization
            
            VegetationMeshes.Add(VegData.VegetationType, MeshComponent);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Initialized %d vegetation mesh components"), VegetationMeshes.Num());
}

void AEcosystemController::UpdateVegetationGrowth(float DeltaTime)
{
    // Simple growth simulation - could be expanded
    for (int32 i = 0; i < VegetationLocations.Num(); i++)
    {
        FVector& Location = VegetationLocations[i];
        
        // Check if vegetation can still survive at this location
        if (VegetationTypes.Num() > 0 && !CanVegetationGrowAt(Location, VegetationTypes[0]))
        {
            // Remove vegetation that can no longer survive
            EVegetationType VegType = VegetationTypes_Runtime[i];
            RemoveVegetationInstance(i, VegType);
            
            VegetationLocations.RemoveAt(i);
            VegetationTypes_Runtime.RemoveAt(i);
            i--; // Adjust index after removal
        }
    }
}

void AEcosystemController::CalculateBiomeDistribution()
{
    // Update biome distribution based on current environmental conditions
    // This is a simplified implementation - could use noise functions for more realistic distribution
    
    UE_LOG(LogTemp, Log, TEXT("EcosystemController: Calculating biome distribution"));
}

bool AEcosystemController::CanVegetationGrowAt(FVector Location, const FVegetationData& VegData) const
{
    if (!WaterSystem) return false;
    
    float WaterDepth = WaterSystem->GetWaterDepthAtPosition(Location);
    float Elevation = Location.Z;
    
    // Check water requirements
    if (WaterDepth < VegData.MinWaterRequirement)
    {
        return false;
    }
    
    // Check elevation range
    if (Elevation < VegData.ElevationRange.X || Elevation > VegData.ElevationRange.Y)
    {
        return false;
    }
    
    // Check biome compatibility
    EBiomeType LocalBiome = GetBiomeAtLocation(Location);
    if (VegData.PreferredBiomes.Num() > 0 && !VegData.PreferredBiomes.Contains(LocalBiome))
    {
        return false;
    }
    
    return true;
}

void AEcosystemController::SpawnVegetationInstance(FVector Location, EVegetationType Type)
{
    UInstancedStaticMeshComponent** MeshPtr = VegetationMeshes.Find(Type);
    if (MeshPtr && *MeshPtr)
    {
        FTransform InstanceTransform;
        InstanceTransform.SetLocation(Location);
        InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
        InstanceTransform.SetScale3D(FVector(FMath::RandRange(0.8f, 1.2f))); // Random scale variation
        
        (*MeshPtr)->AddInstance(InstanceTransform);
    }
}

void AEcosystemController::RemoveVegetationInstance(int32 Index, EVegetationType Type)
{
    UInstancedStaticMeshComponent** MeshPtr = VegetationMeshes.Find(Type);
    if (MeshPtr && *MeshPtr)
    {
        // Note: This is simplified - in a real implementation you'd need to track
        // which mesh instance corresponds to which vegetation location
        if ((*MeshPtr)->GetInstanceCount() > 0)
        {
            (*MeshPtr)->RemoveInstance((*MeshPtr)->GetInstanceCount() - 1);
        }
    }
}

// ===== ISCALABLESYSTEM INTERFACE IMPLEMENTATION =====

void AEcosystemController::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[ECOSYSTEM SCALING] Configuring from master controller"));
    
    CurrentWorldConfig = Config;
    
    // Configure biome grid based on terrain size  
    int32 NewBiomeGridWidth = FMath::Max(32, Config.TerrainWidth / 8); // 1/8 terrain resolution
    int32 NewBiomeGridHeight = FMath::Max(32, Config.TerrainHeight / 8);
    
    // Calculate cell size to cover terrain area
    float TerrainWorldSizeX = Config.TerrainWidth * Config.TerrainScale;
    float TerrainWorldSizeY = Config.TerrainHeight * Config.TerrainScale;
    
    float NewBiomeCellSize = TerrainWorldSizeX / NewBiomeGridWidth;
    
    // Scale vegetation instances based on world size
    int32 NewMaxVegetationInstances = Config.EcosystemConfig.MaxVegetationInstances;
    
    // Scale based on terrain size - larger worlds get more vegetation
    float TerrainSizeRatio = (Config.TerrainWidth * Config.TerrainHeight) / (513.0f * 513.0f);
    NewMaxVegetationInstances = FMath::RoundToInt(NewMaxVegetationInstances * TerrainSizeRatio);
    NewMaxVegetationInstances = FMath::Clamp(NewMaxVegetationInstances, 1000, 100000);
    
    MaxVegetationInstances = NewMaxVegetationInstances;
    
    // Apply vegetation density scaling
    VegetationGrowthRate *= Config.EcosystemConfig.VegetationDensityScale;
    
    UE_LOG(LogTemp, Log, TEXT("[ECOSYSTEM SCALING] Biome grid: %dx%d, Cell size: %.1fm, Max vegetation: %d"),
           NewBiomeGridWidth, NewBiomeGridHeight, NewBiomeCellSize, MaxVegetationInstances);
    
    // Remove excess vegetation if scaling down
    if (VegetationLocations.Num() > MaxVegetationInstances)
    {
        int32 ExcessCount = VegetationLocations.Num() - MaxVegetationInstances;
        for (int32 i = 0; i < ExcessCount; i++)
        {
            int32 LastIndex = VegetationLocations.Num() - 1;
            if (LastIndex >= 0)
            {
                EVegetationType VegType = VegetationTypes_Runtime[LastIndex];
                RemoveVegetationInstance(LastIndex, VegType);
                VegetationLocations.RemoveAt(LastIndex);
                VegetationTypes_Runtime.RemoveAt(LastIndex);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("[ECOSYSTEM SCALING] Removed %d excess vegetation instances"), ExcessCount);
    }
    
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[ECOSYSTEM SCALING] Configuration complete"));
}

void AEcosystemController::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[ECOSYSTEM SCALING] Synchronizing coordinates"));
    
    CurrentCoordinateSystem = Coords;
    
    // Update existing vegetation positions relative to new coordinate system
    for (int32 i = 0; i < VegetationLocations.Num(); i++)
    {
        FVector& Location = VegetationLocations[i];
        
        // Convert to relative coordinates, then back to world coordinates
        FVector RelativePos = Location - CurrentCoordinateSystem.WorldOrigin;
        Location = CurrentCoordinateSystem.WorldOrigin + RelativePos;
        
        // Update the actual mesh instance position
        if (i < VegetationTypes_Runtime.Num())
        {
            EVegetationType VegType = VegetationTypes_Runtime[i];
            UInstancedStaticMeshComponent** MeshPtr = VegetationMeshes.Find(VegType);
            if (MeshPtr && *MeshPtr && i < (*MeshPtr)->GetInstanceCount())
            {
                FTransform NewTransform;
                NewTransform.SetLocation(Location);
                NewTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
                NewTransform.SetScale3D(FVector(FMath::RandRange(0.8f, 1.2f)));
                
                (*MeshPtr)->UpdateInstanceTransform(i, NewTransform, true);
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("[ECOSYSTEM SCALING] Coordinate synchronization complete"));
}

bool AEcosystemController::IsSystemScaled() const
{
    return bIsScaledByMaster && bIsRegisteredWithMaster;
}

// ===== MASTER CONTROLLER INTEGRATION =====

void AEcosystemController::RegisterWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("[ECOSYSTEM SCALING] Cannot register with null master controller"));
        return;
    }
    
    MasterController = Master;
    bIsRegisteredWithMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[ECOSYSTEM SCALING] EcosystemController registered with master controller"));
}

bool AEcosystemController::IsRegisteredWithMaster() const
{
    return bIsRegisteredWithMaster && MasterController != nullptr;
}

FString AEcosystemController::GetScalingDebugInfo() const
{
    FString DebugInfo = TEXT("Ecosystem System Scaling:\n");
    DebugInfo += FString::Printf(TEXT("  - Registered: %s\n"), IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("  - Scaled: %s\n"), IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
    
    if (bIsScaledByMaster)
    {
        DebugInfo += FString::Printf(TEXT("  - Biome Grid: %dx%d\n"), 
                                   CurrentWorldConfig.EcosystemConfig.BiomeGridWidth,
                                   CurrentWorldConfig.EcosystemConfig.BiomeGridHeight);
        DebugInfo += FString::Printf(TEXT("  - Cell Size: %.1fm\n"), CurrentWorldConfig.EcosystemConfig.BiomeCellSize);
        DebugInfo += FString::Printf(TEXT("  - Max Vegetation: %d\n"), MaxVegetationInstances);
        DebugInfo += FString::Printf(TEXT("  - Current Vegetation: %d\n"), VegetationLocations.Num());
        DebugInfo += FString::Printf(TEXT("  - Density Scale: %.2fx\n"), CurrentWorldConfig.EcosystemConfig.VegetationDensityScale);
    }
    else
    {
        DebugInfo += TEXT("  - Not yet configured by master\n");
    }
    
    return DebugInfo;
}