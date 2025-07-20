/**
 * ============================================
 * TERRAI SAND DUNE CONTROLLER - IMPLEMENTATION
 * ============================================
 */
#include "SandDuneController.h"
#include "DynamicTerrain.h"
#include "GeologyController.h"
#include "AtmosphericSystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

ASandDuneController::ASandDuneController()
{
    // DISABLE individual ticking
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SandDuneRoot"));
}

void ASandDuneController::BeginPlay()
{
    Super::BeginPlay();
    
    // ENSURE ticking is disabled
    SetActorTickEnabled(false);
}

void ASandDuneController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // REMOVE all sand dune simulation from Tick
    // System will be called by GeologyController through MasterController
}

// ===== INITIALIZATION =====

void ASandDuneController::Initialize(ADynamicTerrain* Terrain, AGeologyController* Geology)
{
    if (!Terrain)
    {
        UE_LOG(LogTemp, Error, TEXT("SandDuneController: Invalid terrain reference"));
        return;
    }
    
    TargetTerrain = Terrain;
    GeologyController = Geology;
    
    // Cache atmosphere system if available
    if (TargetTerrain->AtmosphericSystem)
    {
        CachedAtmosphereSystem = TargetTerrain->AtmosphericSystem;
    }
    
    GenerateTransportGrid();
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("SandDuneController: Successfully initialized with %d existing dunes"), 
           SandDunes.Num());
}

// ===== CORE FUNCTIONS =====

void ASandDuneController::UpdateSandDunes(float DeltaTime)
{
    TArray<int32> DunesToRemove;
    TArray<TPair<int32, int32>> DunesToMerge;
    
    // Update each dune
    for (int32 i = 0; i < SandDunes.Num(); i++)
    {
        FSandDuneData& Dune = SandDunes[i];
        
        // Age the dune
        Dune.Age += DeltaTime;
        
        // Evolve dune shape based on wind patterns
        EvolveDuneShape(Dune, DeltaTime);
        
        // Process sand transport for this dune
        float WindSpeed = CurrentWindVector.Size();
        if (WindSpeed > SandTransportThreshold)
        {
            // Calculate migration
            float MobilityFactor = 1.0f - (Dune.Stability * VegetationStabilizationFactor);
            FVector Migration = Dune.MigrationDirection * Dune.MigrationRate * MobilityFactor * DeltaTime;
            Dune.Location += Migration;
            
            // Process avalanching on steep slopes
            ProcessAvalanche(Dune, DeltaTime);
        }
        
        // Vegetation growth and stabilization
        StabilizeDune(Dune, DeltaTime);
        
        // Check for dune merging
        for (int32 j = i + 1; j < SandDunes.Num(); j++)
        {
            float Distance = FVector::Dist(Dune.Location, SandDunes[j].Location);
            if (Distance < DuneMergeDistance)
            {
                DunesToMerge.Add(TPair<int32, int32>(i, j));
            }
        }
        
        // Remove small dunes
        if (Dune.SandVolume < DuneFormationThreshold * 0.1f)
        {
            DunesToRemove.Add(i);
        }
    }
    
    // Merge close dunes
    for (const auto& MergePair : DunesToMerge)
    {
        if (!DunesToRemove.Contains(MergePair.Key) && !DunesToRemove.Contains(MergePair.Value))
        {
            MergeDunes(MergePair.Key, MergePair.Value);
            DunesToRemove.Add(MergePair.Value);
        }
    }
    
    // Remove marked dunes
    for (int32 i = DunesToRemove.Num() - 1; i >= 0; i--)
    {
        SandDunes.RemoveAt(DunesToRemove[i]);
    }
    
    // Apply dune heights to terrain if needed
    if (TargetTerrain)
    {
        ApplyDunesToTerrain();
    }
}

void ASandDuneController::CalculateSandTransport(float DeltaTime)
{
    // Update transport grid based on current wind
    for (FSandTransportData& Transport : TransportGrid)
    {
        FVector EffectiveWind = GetEffectiveWindAtLocation(Transport.Position);
        float WindSpeed = EffectiveWind.Size();
        
        if (WindSpeed > SandTransportThreshold)
        {
            // Bagnold's equation for sand flux
            float ExcessVelocity = WindSpeed - SandTransportThreshold;
            Transport.SandFlux = SaltationEfficiency * FMath::Pow(ExcessVelocity, 3.0f);
            Transport.SandFlux = FMath::Min(Transport.SandFlux, MaxTransportRate);
            
            Transport.TransportDirection = EffectiveWind.GetSafeNormal();
            Transport.SaltationHeight = 0.1f * ExcessVelocity; // Simplified
        }
        else
        {
            Transport.SandFlux = 0.0f;
            Transport.TransportDirection = FVector::ZeroVector;
            Transport.SaltationHeight = 0.0f;
        }
    }
    
    // Process different transport modes
    ProcessSaltation(DeltaTime);
    ProcessCreep(DeltaTime);
}

float ASandDuneController::GetSandFluxAtLocation(FVector Location) const
{
    // Find nearest transport grid point
    float MinDistance = FLT_MAX;
    float NearestFlux = 0.0f;
    
    for (const FSandTransportData& Transport : TransportGrid)
    {
        float Distance = FVector::Dist2D(Location, Transport.Position);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestFlux = Transport.SandFlux;
        }
    }
    
    return NearestFlux;
}

// ===== DUNE FORMATION =====

void ASandDuneController::FormNewDune(FVector Location, float InitialVolume, EDuneType Type)
{
    FSandDuneData NewDune;
    NewDune.Location = Location;
    NewDune.SandVolume = InitialVolume;
    NewDune.DuneType = Type;
    NewDune.MigrationDirection = PrevailingWindDirection;
    
    // Set initial dimensions based on type
    switch (Type)
    {
    case EDuneType::Barchan:
        NewDune.Height = FMath::Pow(InitialVolume, 0.33f) * 2.0f;
        NewDune.Width = NewDune.Height * 3.0f;
        NewDune.Length = NewDune.Height * 4.0f;
        break;
        
    case EDuneType::Transverse:
        NewDune.Height = FMath::Pow(InitialVolume, 0.33f) * 1.5f;
        NewDune.Width = NewDune.Height * 10.0f;
        NewDune.Length = NewDune.Height * 2.0f;
        break;
        
    case EDuneType::Linear:
        NewDune.Height = FMath::Pow(InitialVolume, 0.33f) * 1.8f;
        NewDune.Width = NewDune.Height * 2.0f;
        NewDune.Length = NewDune.Height * 20.0f;
        break;
        
    case EDuneType::Star:
        NewDune.Height = FMath::Pow(InitialVolume, 0.33f) * 2.5f;
        NewDune.Width = NewDune.Height * 4.0f;
        NewDune.Length = NewDune.Height * 4.0f;
        break;
        
    case EDuneType::Parabolic:
        NewDune.Height = FMath::Pow(InitialVolume, 0.33f) * 1.2f;
        NewDune.Width = NewDune.Height * 5.0f;
        NewDune.Length = NewDune.Height * 8.0f;
        NewDune.VegetationCover = 0.3f; // Parabolic dunes need some vegetation
        break;
    }
    
    SandDunes.Add(NewDune);
    
    UE_LOG(LogTemp, Log, TEXT("SandDuneController: Formed new %s dune at %s (Volume: %.1f)"), 
           *UEnum::GetDisplayValueAsText(Type).ToString(), *Location.ToString(), InitialVolume);
}

void ASandDuneController::MergeDunes(int32 DuneIndex1, int32 DuneIndex2)
{
    if (!SandDunes.IsValidIndex(DuneIndex1) || !SandDunes.IsValidIndex(DuneIndex2))
        return;
    
    FSandDuneData& Dune1 = SandDunes[DuneIndex1];
    FSandDuneData& Dune2 = SandDunes[DuneIndex2];
    
    // Combine volumes
    float TotalVolume = Dune1.SandVolume + Dune2.SandVolume;
    
    // Weight-average properties
    float Weight1 = Dune1.SandVolume / TotalVolume;
    float Weight2 = Dune2.SandVolume / TotalVolume;
    
    Dune1.Location = Dune1.Location * Weight1 + Dune2.Location * Weight2;
    Dune1.SandVolume = TotalVolume;
    Dune1.Height = FMath::Max(Dune1.Height, Dune2.Height) * 1.1f; // Slight height increase
    Dune1.Width = Dune1.Width + Dune2.Width * 0.5f;
    Dune1.Stability = Dune1.Stability * Weight1 + Dune2.Stability * Weight2;
    Dune1.VegetationCover = Dune1.VegetationCover * Weight1 + Dune2.VegetationCover * Weight2;
    
    // Keep the older dune's type unless merging creates a star dune
    if (Dune1.DuneType != Dune2.DuneType)
    {
        Dune1.DuneType = EDuneType::Star; // Complex multi-directional dune
    }
    
    UE_LOG(LogTemp, Log, TEXT("SandDuneController: Merged dunes at %s (New volume: %.1f)"), 
           *Dune1.Location.ToString(), TotalVolume);
}

void ASandDuneController::EvolveDuneShape(FSandDuneData& Dune, float DeltaTime)
{
    // Evolve dune shape based on type and wind conditions
    float WindSpeed = CurrentWindVector.Size();
    float WindDirectionChange = FVector::DotProduct(
        CurrentWindVector.GetSafeNormal(), 
        Dune.MigrationDirection
    );
    
    // Update migration direction with inertia
    Dune.MigrationDirection = FMath::Lerp(
        Dune.MigrationDirection, 
        CurrentWindVector.GetSafeNormal(), 
        0.1f * DeltaTime
    ).GetSafeNormal();
    
    // Shape evolution based on dune type
    switch (Dune.DuneType)
    {
    case EDuneType::Barchan:
        if (WindDirectionChange < 0.7f) // Wind direction changed significantly
        {
            // May evolve into linear or star dune
            if (FMath::RandRange(0.0f, 1.0f) < 0.01f * DeltaTime)
            {
                Dune.DuneType = EDuneType::Linear;
                Dune.Length *= 2.0f;
                Dune.Width *= 0.5f;
            }
        }
        break;
        
    case EDuneType::Linear:
        if (WindSpeed < SandTransportThreshold * 0.8f)
        {
            // May break up into barchans
            if (FMath::RandRange(0.0f, 1.0f) < 0.005f * DeltaTime && Dune.SandVolume > DuneFormationThreshold * 2)
            {
                // Create a new barchan from part of this dune
                FVector NewLocation = Dune.Location + FVector(Dune.Length * 0.25f, 0, 0);
                FormNewDune(NewLocation, Dune.SandVolume * 0.3f, EDuneType::Barchan);
                Dune.SandVolume *= 0.7f;
                Dune.Length *= 0.7f;
            }
        }
        break;
        
    case EDuneType::Star:
        // Star dunes are stable but grow vertically
        Dune.Height += 0.1f * DeltaTime * (WindSpeed / BaseWindStrength);
        break;
    }
}

// ===== WIND INTEGRATION =====

void ASandDuneController::UpdateWindData()
{
    if (CachedAtmosphereSystem)
    {
        FVector TerrainCenter = TargetTerrain ? TargetTerrain->GetActorLocation() : FVector::ZeroVector;
        CurrentWindVector = CachedAtmosphereSystem->GetWindAt(TerrainCenter);
    }
    else
    {
        // Use configured wind if no atmosphere system
        CurrentWindVector = PrevailingWindDirection * BaseWindStrength;
        
        // Add some variability
        float Variation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * WindVariability;
        CurrentWindVector *= (1.0f + Variation);
    }
}

FVector ASandDuneController::GetEffectiveWindAtLocation(FVector Location) const
{
    FVector BaseWind = CurrentWindVector;
    
    // Modify wind based on nearby dunes (wind shadow effects)
    for (const FSandDuneData& Dune : SandDunes)
    {
        FVector ToDune = Location - Dune.Location;
        float Distance = ToDune.Size();
        
        if (Distance < Dune.Width * 2.0f)
        {
            // Check if location is downwind of dune
            float Alignment = FVector::DotProduct(ToDune.GetSafeNormal(), BaseWind.GetSafeNormal());
            if (Alignment > 0.5f)
            {
                // Wind shadow effect
                float ShadowStrength = 1.0f - (Distance / (Dune.Width * 2.0f));
                ShadowStrength *= (Dune.Height / 50.0f); // Normalize by typical dune height
                BaseWind *= (1.0f - ShadowStrength * 0.7f);
            }
        }
    }
    
    return BaseWind;
}

void ASandDuneController::OnWindChanged(FVector NewWindDirection, float NewWindSpeed)
{
    PrevailingWindDirection = NewWindDirection.GetSafeNormal();
    BaseWindStrength = NewWindSpeed;
    UpdateWindData();
    
    UE_LOG(LogTemp, Log, TEXT("SandDuneController: Wind updated - Direction:%s Speed:%.1f"), 
           *NewWindDirection.ToString(), NewWindSpeed);
}

// ===== TERRAIN INTERACTION =====

void ASandDuneController::ApplyDunesToTerrain()
{
    if (!TargetTerrain) return;
    
    // This would modify the terrain heightmap based on dune positions
    // For now, just log the intent
    UE_LOG(LogTemp, VeryVerbose, TEXT("SandDuneController: Would apply %d dunes to terrain"), 
           SandDunes.Num());
}

float ASandDuneController::GetDuneHeightAtLocation(FVector Location) const
{
    float TotalHeight = 0.0f;
    
    for (const FSandDuneData& Dune : SandDunes)
    {
        FVector ToDune = Location - Dune.Location;
        float Distance2D = FVector(ToDune.X, ToDune.Y, 0).Size();
        
        // Simple gaussian falloff for dune height
        if (Distance2D < Dune.Width)
        {
            float NormalizedDist = Distance2D / Dune.Width;
            float HeightContribution = Dune.Height * FMath::Exp(-NormalizedDist * NormalizedDist * 3.0f);
            TotalHeight += HeightContribution;
        }
    }
    
    return TotalHeight;
}

// ===== VISUALIZATION =====

void ASandDuneController::ShowSandDunes(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    for (const FSandDuneData& Dune : SandDunes)
    {
        FColor DuneColor;
        switch (Dune.DuneType)
        {
        case EDuneType::Barchan: DuneColor = FColor::Yellow; break;
        case EDuneType::Transverse: DuneColor = FColor::Orange; break;
        case EDuneType::Linear: DuneColor = FColor::Red; break;
        case EDuneType::Star: DuneColor = FColor::Purple; break;
        case EDuneType::Parabolic: DuneColor = FColor::Green; break;
        default: DuneColor = FColor::White;
        }
        
        // Draw dune shape
        DrawDebugSphere(GetWorld(), Dune.Location, Dune.Height, 12, DuneColor, false, 5.0f);
        DrawDebugBox(GetWorld(), Dune.Location, FVector(Dune.Length/2, Dune.Width/2, Dune.Height/2), 
                     DuneColor, false, 5.0f);
        
        // Show migration direction
        FVector MigrationEnd = Dune.Location + (Dune.MigrationDirection * 100.0f);
        DrawDebugDirectionalArrow(GetWorld(), Dune.Location, MigrationEnd, 50.0f, 
                                  FColor::Cyan, false, 5.0f, 0, 3.0f);
        
        // Show vegetation coverage
        if (Dune.VegetationCover > 0.1f)
        {
            DrawDebugString(GetWorld(), Dune.Location + FVector(0, 0, Dune.Height + 20), 
                            FString::Printf(TEXT("Veg: %.0f%%"), Dune.VegetationCover * 100), 
                            nullptr, FColor::Green, 5.0f);
        }
    }
}

void ASandDuneController::ShowSandTransport(bool bEnable)
{
    if (!bEnable || !GetWorld()) return;
    
    for (const FSandTransportData& Transport : TransportGrid)
    {
        if (Transport.SandFlux > 0.01f)
        {
            FColor FluxColor = FColor::MakeRedToGreenColorFromScalar(
                Transport.SandFlux / MaxTransportRate
            );
            
            FVector ArrowEnd = Transport.Position + 
                              (Transport.TransportDirection * Transport.SandFlux * 10.0f);
            
            DrawDebugDirectionalArrow(GetWorld(), Transport.Position, ArrowEnd, 
                                      20.0f, FluxColor, false, 5.0f, 0, 2.0f);
            
            // Show saltation height
            if (Transport.SaltationHeight > 0.0f)
            {
                FVector SaltationTop = Transport.Position + 
                                      FVector(0, 0, Transport.SaltationHeight * 100.0f);
                DrawDebugLine(GetWorld(), Transport.Position, SaltationTop, 
                              FColor::Blue, false, 5.0f, 0, 1.0f);
            }
        }
    }
}

void ASandDuneController::PrintSandDuneStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SAND DUNE STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Dunes: %d"), SandDunes.Num());
    
    int32 TypeCounts[5] = {0};
    float TotalVolume = 0.0f;
    float TotalVegetated = 0.0f;
    
    for (const FSandDuneData& Dune : SandDunes)
    {
        TypeCounts[(int32)Dune.DuneType]++;
        TotalVolume += Dune.SandVolume;
        if (Dune.VegetationCover > 0.1f)
            TotalVegetated++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Barchan: %d, Transverse: %d, Linear: %d, Star: %d, Parabolic: %d"),
           TypeCounts[0], TypeCounts[1], TypeCounts[2], TypeCounts[3], TypeCounts[4]);
    UE_LOG(LogTemp, Warning, TEXT("Total Sand Volume: %.0f m³"), TotalVolume);
    UE_LOG(LogTemp, Warning, TEXT("Vegetated Dunes: %.0f%%"), (TotalVegetated / SandDunes.Num()) * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Current Wind: %s at %.1f m/s"), 
           *CurrentWindVector.GetSafeNormal().ToString(), CurrentWindVector.Size());
}

// ===== ISCALABLESYSTEM INTERFACE =====

void ASandDuneController::ConfigureFromMaster(const FWorldScalingConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("[SAND DUNE SCALING] Configuring from master controller"));
    
    CurrentWorldConfig = Config;
    
    // Scale dune parameters based on world size
    float WorldScale = Config.TerrainScale;
    
    // Adjust dune dimensions
    DuneMergeDistance *= WorldScale;
    
    // Adjust transport parameters  
    MaxTransportRate *= WorldScale;
    
    // Generate appropriately scaled transport grid
    GenerateTransportGrid();
    
    bIsScaledByMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[SAND DUNE SCALING] Configuration complete"));
}

void ASandDuneController::SynchronizeCoordinates(const FWorldCoordinateSystem& Coords)
{
    CurrentCoordinateSystem = Coords;
    
    // Update dune positions if needed
    for (FSandDuneData& Dune : SandDunes)
    {
        // Convert to world coordinates if needed
        FVector RelativePos = Dune.Location - CurrentCoordinateSystem.WorldOrigin;
        Dune.Location = CurrentCoordinateSystem.WorldOrigin + RelativePos;
    }
}

bool ASandDuneController::IsSystemScaled() const
{
    return bIsScaledByMaster && bIsRegisteredWithMaster;
}

// ===== MASTER CONTROLLER INTEGRATION =====

void ASandDuneController::RegisterWithMasterController(AMasterWorldController* Master)
{
    if (!Master)
    {
        UE_LOG(LogTemp, Error, TEXT("[SAND DUNE SCALING] Cannot register with null master controller"));
        return;
    }
    
    MasterController = Master;
    bIsRegisteredWithMaster = true;
    
    UE_LOG(LogTemp, Warning, TEXT("[SAND DUNE SCALING] SandDuneController registered with master controller"));
}

bool ASandDuneController::IsRegisteredWithMaster() const
{
    return bIsRegisteredWithMaster && MasterController != nullptr;
}

FString ASandDuneController::GetScalingDebugInfo() const
{
    FString DebugInfo = TEXT("Sand Dune System Scaling:\n");
    DebugInfo += FString::Printf(TEXT("  - Registered: %s\n"), IsRegisteredWithMaster() ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("  - Scaled: %s\n"), IsSystemScaled() ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("  - Transport Grid Size: %d\n"), TransportGrid.Num());
    DebugInfo += FString::Printf(TEXT("  - Active Dunes: %d\n"), SandDunes.Num());
    return DebugInfo;
}

// ===== INTERNAL FUNCTIONS =====

void ASandDuneController::ProcessSaltation(float DeltaTime)
{
    // Saltation: sand grains hopping along surface
    // This is the primary transport mechanism
    
    for (const FSandTransportData& Transport : TransportGrid)
    {
        if (Transport.SandFlux > 0.0f)
        {
            // Find source dunes
            for (FSandDuneData& Dune : SandDunes)
            {
                FVector ToDune = Transport.Position - Dune.Location;
                float Distance = ToDune.Size2D();
                
                // Check if transport point is on windward side of dune
                float WindAlignment = FVector::DotProduct(
                    ToDune.GetSafeNormal(), 
                    Transport.TransportDirection
                );
                
                if (Distance < Dune.Width && WindAlignment < -0.5f)
                {
                    // Erode sand from windward side
                    float ErosionRate = Transport.SandFlux * 0.1f * DeltaTime;
                    ErosionRate *= (1.0f - Dune.Stability); // Stability reduces erosion
                    
                    Dune.SandVolume -= ErosionRate;
                    
                    // Find deposition location (leeward side)
                    FVector DepositionLocation = Dune.Location + 
                        (Transport.TransportDirection * Dune.Length);
                    
                    // Check if we should create a new dune or add to existing
                    bool bDeposited = false;
                    for (FSandDuneData& TargetDune : SandDunes)
                    {
                        if (&TargetDune != &Dune && 
                            FVector::Dist(TargetDune.Location, DepositionLocation) < DuneMergeDistance)
                        {
                            TargetDune.SandVolume += ErosionRate;
                            bDeposited = true;
                            break;
                        }
                    }
                    
                    if (!bDeposited && ErosionRate > DuneFormationThreshold * 0.01f)
                    {
                        // Accumulate sand for potential new dune formation
                        // (In full implementation, track accumulation grid)
                    }
                }
            }
        }
    }
}

void ASandDuneController::ProcessCreep(float DeltaTime)
{
    // Surface creep: slow movement of sand grains along surface
    // Happens at lower wind speeds than saltation
    
    float CreepThreshold = SandTransportThreshold * 0.6f;
    
    for (FSandDuneData& Dune : SandDunes)
    {
        FVector EffectiveWind = GetEffectiveWindAtLocation(Dune.Location);
        float WindSpeed = EffectiveWind.Size();
        
        if (WindSpeed > CreepThreshold && WindSpeed < SandTransportThreshold)
        {
            // Slow migration in wind direction
            float CreepRate = 0.1f * (WindSpeed - CreepThreshold) / CreepThreshold;
            CreepRate *= (1.0f - Dune.Stability);
            
            Dune.Location += EffectiveWind.GetSafeNormal() * CreepRate * DeltaTime;
        }
    }
}

void ASandDuneController::ProcessAvalanche(FSandDuneData& Dune, float DeltaTime)
{
    // Check if dune exceeds angle of repose
    float AngleOfRepose = CalculateAngleOfRepose(Dune);
    float CurrentSlope = FMath::Atan(Dune.Height / (Dune.Length * 0.5f)) * (180.0f / PI);
    
    if (CurrentSlope > AngleOfRepose)
    {
        // Avalanche! Redistribute sand
        float ExcessAngle = CurrentSlope - AngleOfRepose;
        float AvalancheVolume = Dune.SandVolume * (ExcessAngle / 90.0f) * 0.1f * DeltaTime;
        
        // Reduce height
        float VolumeRatio = (Dune.SandVolume - AvalancheVolume) / Dune.SandVolume;
        Dune.Height *= FMath::Sqrt(VolumeRatio);
        Dune.SandVolume -= AvalancheVolume;
        
        // Create debris at base (simplified - in reality would redistribute)
        Dune.Length *= 1.05f; // Dune spreads out
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("SandDuneController: Avalanche at %s (Volume: %.1f)"), 
               *Dune.Location.ToString(), AvalancheVolume);
    }
}

float ASandDuneController::CalculateAngleOfRepose(const FSandDuneData& Dune) const
{
    // Base angle of repose for dry sand
    float BaseAngle = 34.0f;
    
    // Moisture content would reduce angle (not implemented yet)
    // Vegetation increases angle
    float VegetationBonus = Dune.VegetationCover * 15.0f;
    
    // Compaction increases angle slightly
    float CompactionBonus = FMath::Min(Dune.Age / 100.0f, 1.0f) * 5.0f;
    
    return BaseAngle + VegetationBonus + CompactionBonus;
}

void ASandDuneController::StabilizeDune(FSandDuneData& Dune, float DeltaTime)
{
    // Vegetation growth
    if (Dune.VegetationCover < 1.0f)
    {
        // Vegetation grows faster on stable dunes
        float GrowthRate = VegetationGrowthRate * (1.0f + Dune.Stability);
        
        // Reduced growth on very mobile dunes
        if (Dune.MigrationRate > 10.0f)
        {
            GrowthRate *= 0.1f;
        }
        
        Dune.VegetationCover += GrowthRate * DeltaTime;
        Dune.VegetationCover = FMath::Clamp(Dune.VegetationCover, 0.0f, 1.0f);
    }
    
    // Update stability based on vegetation and age
    float TargetStability = Dune.VegetationCover * 0.7f + 
                           FMath::Min(Dune.Age / 1000.0f, 0.3f);
    
    Dune.Stability = FMath::Lerp(Dune.Stability, TargetStability, 0.1f * DeltaTime);
    
    // Reduce migration rate as stability increases
    Dune.MigrationRate = 5.0f * (1.0f - Dune.Stability);
}

void ASandDuneController::GenerateTransportGrid()
{
    TransportGrid.Empty();
    
    if (!TargetTerrain) return;
    
    // Create a grid of transport monitoring points
    int32 GridSize = 20; // Simplified for performance
    float GridSpacing = (CurrentWorldConfig.TerrainWidth * CurrentWorldConfig.TerrainScale) / GridSize;
    
    FVector TerrainOrigin = TargetTerrain->GetActorLocation();
    
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FSandTransportData TransportPoint;
            TransportPoint.Position = TerrainOrigin + FVector(
                (X - GridSize/2) * GridSpacing,
                (Y - GridSize/2) * GridSpacing,
                0.0f
            );
            
            TransportGrid.Add(TransportPoint);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SandDuneController: Generated transport grid with %d points"), 
           TransportGrid.Num());
}
