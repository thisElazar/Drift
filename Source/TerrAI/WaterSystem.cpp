// WaterSystem.cpp - Simplified Water Physics Implementation
#include "WaterSystem.h"
#include "DynamicTerrain.h"
#include "Engine/Engine.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UWaterSystem::UWaterSystem()
{
    // Initialize default settings
    bEnableWaterSimulation = true;
    WaterFlowSpeed = 50.0f;
    WaterEvaporationRate = 0.01f;
    WaterAbsorptionRate = 0.02f;
    WaterDamping = 0.95f;
    MaxWaterVelocity = 100.0f;
    MinWaterDepth = 0.01f;
    
    // Weather settings
    bIsRaining = false;
    RainIntensity = 1.0f;
    WeatherChangeInterval = 60.0f;
    bAutoWeather = false;
    WeatherTimer = 0.0f;
    
    // Debug settings
    bShowWaterStats = true;
    bShowWaterDebugTexture = false;
    
    // Shader system enabled for debugging
    bUseShaderWater = true;
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Created"));
}

// ===== INITIALIZATION =====

void UWaterSystem::Initialize(ADynamicTerrain* InTerrain)
{
    if (!InTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterSystem: Cannot initialize with null terrain"));
        return;
    }
    
    OwnerTerrain = InTerrain;
    
    // Initialize simulation data
    SimulationData.Initialize(OwnerTerrain->TerrainWidth, OwnerTerrain->TerrainHeight);
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Initialized with terrain %dx%d (%d water cells)"),
           SimulationData.TerrainWidth, SimulationData.TerrainHeight, SimulationData.WaterDepthMap.Num());
    
    // Initialize shader system if enabled
    if (bUseShaderWater)
    {
        UE_LOG(LogTemp, Error, TEXT("INITIALIZING SHADER WATER SYSTEM"));
        CreateWaterDepthTexture();
        CreateAdvancedWaterTexture();
        UE_LOG(LogTemp, Error, TEXT("WaterSystem: Shader system initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Shader water system DISABLED - bUseShaderWater=false"));
    }
     
     UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Initialized with terrain %dx%d (%d water cells)"),
            SimulationData.TerrainWidth, SimulationData.TerrainHeight, SimulationData.WaterDepthMap.Num());

}

bool UWaterSystem::IsSystemReady() const
{
    return OwnerTerrain != nullptr && SimulationData.IsValid();
}

// ===== WATER PHYSICS SIMULATION =====

void UWaterSystem::UpdateWaterSimulation(float DeltaTime)
{
    if (!IsSystemReady() || !bEnableWaterSimulation)
    {
        return;
    }
    
    // Step 1: Update weather system
    if (bAutoWeather)
    {
        UpdateWeatherSystem(DeltaTime);
    }
    
    // Step 2: Apply rain if active
    if (bIsRaining)
    {
        ApplyRain(DeltaTime);
    }
    
    // Step 3: Calculate water flow forces
    CalculateWaterFlow(DeltaTime);
    
    // Step 4: Move water based on flow
    ApplyWaterFlow(DeltaTime);
    
    // Step 5: Handle evaporation and absorption
    ProcessWaterEvaporation(DeltaTime);
    
    // Step 6: Process erosion if enabled
    if (bEnableErosion)
    {
        UpdateErosion(DeltaTime);
    }
    
    // Step 7: Update shader system
    if (bUseShaderWater)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updating shader system - bUseShaderWater=true"));
        
        // Calculate foam data for advanced rendering
        CalculateFoamData();
        
        UpdateShaderSystem(DeltaTime);
        
        // Also update parameters every frame
        UpdateWaterShaderParameters();
    }
    else
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Shader system disabled - bUseShaderWater=false"));
    }
}

void UWaterSystem::ResetWaterSystem()
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    // Reset all water data
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        SimulationData.WaterDepthMap[i] = 0.0f;
        SimulationData.WaterVelocityX[i] = 0.0f;
        SimulationData.WaterVelocityY[i] = 0.0f;
        SimulationData.SedimentMap[i] = 0.0f;
    }
    
    // Reset weather
    bIsRaining = false;
    WeatherTimer = 0.0f;
    
    // Clear tracking data
    ChunksWithWater.Empty();
    TotalWaterAmount = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Reset complete"));
}

// ===== CORE SIMULATION FUNCTIONS =====

void UWaterSystem::CalculateWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    // Create temporary arrays for new velocities
    TArray<float> NewVelocityX = SimulationData.WaterVelocityX;
    TArray<float> NewVelocityY = SimulationData.WaterVelocityY;
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Process all cells including edges for water outflow
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            // Only process cells with significant water
            if (SimulationData.WaterDepthMap[Index] <= MinWaterDepth)
            {
                continue;
            }
            
            // Calculate water surface height (terrain + water depth)
            float TerrainHeight = GetTerrainHeightSafe(X, Y);
            float WaterSurfaceHeight = TerrainHeight + SimulationData.WaterDepthMap[Index];
            
            // Get neighbor indices (with edge handling)
            int32 LeftIdx = (X > 0) ? Y * Width + (X - 1) : -1;
            int32 RightIdx = (X < Width - 1) ? Y * Width + (X + 1) : -1;
            int32 UpIdx = (Y > 0) ? (Y - 1) * Width + X : -1;
            int32 DownIdx = (Y < Height - 1) ? (Y + 1) * Width + X : -1;
            
            // Calculate pressure gradients with proper neighbor terrain heights
            float TerrainScale = OwnerTerrain ? OwnerTerrain->TerrainScale : 100.0f;
            float ForceX = 0.0f;
            float ForceY = 0.0f;
            
            // X-direction force calculation
            if (X == 0) // Left edge - drain left with terrain consideration
            {
                if (RightIdx != -1)
                {
                    float RightTerrainHeight = GetTerrainHeightSafe(X + 1, Y);
                    float RightWaterHeight = RightTerrainHeight + SimulationData.WaterDepthMap[RightIdx];
                    ForceX = (WaterSurfaceHeight - RightWaterHeight) / TerrainScale + WaterSurfaceHeight * 0.2f; // Enhanced edge drain
                }
                else
                {
                    ForceX = WaterSurfaceHeight * 0.3f; // Enhanced fallback edge drain
                }
            }
            else if (X == Width - 1) // Right edge - drain right with terrain consideration
            {
                if (LeftIdx != -1)
                {
                    float LeftTerrainHeight = GetTerrainHeightSafe(X - 1, Y);
                    float LeftWaterHeight = LeftTerrainHeight + SimulationData.WaterDepthMap[LeftIdx];
                    ForceX = (LeftWaterHeight - WaterSurfaceHeight) / TerrainScale + WaterSurfaceHeight * 0.2f; // Enhanced edge drain
                }
                else
                {
                    ForceX = WaterSurfaceHeight * 0.1f; // Fallback edge drain
                }
            }
            else // Interior - normal gradient with proper neighbor terrain heights
            {
                float LeftTerrainHeight = GetTerrainHeightSafe(X - 1, Y);
                float RightTerrainHeight = GetTerrainHeightSafe(X + 1, Y);
                float LeftWaterHeight = LeftTerrainHeight + SimulationData.WaterDepthMap[LeftIdx];
                float RightWaterHeight = RightTerrainHeight + SimulationData.WaterDepthMap[RightIdx];
                ForceX = (LeftWaterHeight - RightWaterHeight) / (2.0f * TerrainScale);
            }
            
            // Y-direction force calculation
            if (Y == 0) // Top edge - drain up with terrain consideration
            {
                if (DownIdx != -1)
                {
                    float DownTerrainHeight = GetTerrainHeightSafe(X, Y + 1);
                    float DownWaterHeight = DownTerrainHeight + SimulationData.WaterDepthMap[DownIdx];
                    ForceY = (WaterSurfaceHeight - DownWaterHeight) / TerrainScale + WaterSurfaceHeight * 0.2f; // Enhanced edge drain
                }
                else
                {
                    ForceY = -WaterSurfaceHeight * 0.3f; // Enhanced fallback edge drain
                }
            }
            else if (Y == Height - 1) // Bottom edge - drain down with terrain consideration
            {
                if (UpIdx != -1)
                {
                    float UpTerrainHeight = GetTerrainHeightSafe(X, Y - 1);
                    float UpWaterHeight = UpTerrainHeight + SimulationData.WaterDepthMap[UpIdx];
                    ForceY = (UpWaterHeight - WaterSurfaceHeight) / TerrainScale + WaterSurfaceHeight * 0.2f; // Enhanced edge drain
                }
                else
                {
                    ForceY = WaterSurfaceHeight * 0.3f; // Enhanced fallback edge drain
                }
            }
            else // Interior - normal gradient with proper neighbor terrain heights
            {
                float UpTerrainHeight = GetTerrainHeightSafe(X, Y - 1);
                float DownTerrainHeight = GetTerrainHeightSafe(X, Y + 1);
                float UpWaterHeight = UpTerrainHeight + SimulationData.WaterDepthMap[UpIdx];
                float DownWaterHeight = DownTerrainHeight + SimulationData.WaterDepthMap[DownIdx];
                ForceY = (UpWaterHeight - DownWaterHeight) / (2.0f * TerrainScale);
            }
            
            // Apply forces to velocity with damping to prevent oscillation
            NewVelocityX[Index] = (SimulationData.WaterVelocityX[Index] + ForceX * WaterFlowSpeed * DeltaTime) * WaterDamping;
            NewVelocityY[Index] = (SimulationData.WaterVelocityY[Index] + ForceY * WaterFlowSpeed * DeltaTime) * WaterDamping;
            
            // Limit velocity to prevent instability
            NewVelocityX[Index] = FMath::Clamp(NewVelocityX[Index], -MaxWaterVelocity, MaxWaterVelocity);
            NewVelocityY[Index] = FMath::Clamp(NewVelocityY[Index], -MaxWaterVelocity, MaxWaterVelocity);
        }
    }
    
    // Update velocity arrays
    SimulationData.WaterVelocityX = NewVelocityX;
    SimulationData.WaterVelocityY = NewVelocityY;
}

void UWaterSystem::ApplyWaterFlow(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    TArray<float> NewWaterDepth = SimulationData.WaterDepthMap;
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Process ALL cells including edges for waterfall effect
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (SimulationData.WaterDepthMap[Index] <= MinWaterDepth)
            {
                continue;
            }
            
            // Get current velocities
            float VelX = SimulationData.WaterVelocityX[Index];
            float VelY = SimulationData.WaterVelocityY[Index];
            
            // Calculate flow amounts based on velocity and available water
            float FlowRate = SimulationData.WaterDepthMap[Index] * DeltaTime * 0.1f;  // Scale factor for stability
            float FlowRight = FMath::Max(0.0f, VelX * FlowRate);
            float FlowLeft = FMath::Max(0.0f, -VelX * FlowRate);
            float FlowDown = FMath::Max(0.0f, VelY * FlowRate);
            float FlowUp = FMath::Max(0.0f, -VelY * FlowRate);
            
            // Ensure total outflow doesn't exceed available water
            float TotalOutflow = FlowRight + FlowLeft + FlowDown + FlowUp;
            if (TotalOutflow > SimulationData.WaterDepthMap[Index])
            {
                float Scale = SimulationData.WaterDepthMap[Index] / TotalOutflow;
                FlowRight *= Scale;
                FlowLeft *= Scale;
                FlowDown *= Scale;
                FlowUp *= Scale;
            }
            
            // Remove water from current cell
            NewWaterDepth[Index] -= TotalOutflow;
            
            // Add water to neighbor cells OR let flow off-world (waterfall effect)
            if (X < Width - 1)
                NewWaterDepth[Y * Width + (X + 1)] += FlowRight;
            // else: FlowRight disappears off right edge as waterfall
            
            if (X > 0)
                NewWaterDepth[Y * Width + (X - 1)] += FlowLeft;
            // else: FlowLeft disappears off left edge as waterfall
            
            if (Y < Height - 1)
                NewWaterDepth[(Y + 1) * Width + X] += FlowDown;
            // else: FlowDown disappears off bottom edge as waterfall
            
            if (Y > 0)
                NewWaterDepth[(Y - 1) * Width + X] += FlowUp;
            // else: FlowUp disappears off top edge as waterfall
        }
    }
    
    // Update water depths, ensuring no negative values
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        SimulationData.WaterDepthMap[i] = FMath::Max(0.0f, NewWaterDepth[i]);
    }
}



void UWaterSystem::ProcessWaterEvaporation(float DeltaTime)
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    for (int32 i = 0; i < SimulationData.WaterDepthMap.Num(); i++)
    {
        if (SimulationData.WaterDepthMap[i] > MinWaterDepth)
        {
            // Evaporation (faster in shallow areas)
            float EvaporationAmount = WaterEvaporationRate * DeltaTime;
            SimulationData.WaterDepthMap[i] -= EvaporationAmount;
            
            // Absorption into terrain
            float AbsorptionAmount = WaterAbsorptionRate * DeltaTime;
            SimulationData.WaterDepthMap[i] -= AbsorptionAmount;
            
            // Ensure water depth doesn't go negative
            SimulationData.WaterDepthMap[i] = FMath::Max(0.0f, SimulationData.WaterDepthMap[i]);
        }
    }
}

// ===== ADVANCED WATER TEXTURE SYSTEM =====

void UWaterSystem::CreateAdvancedWaterTexture()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating advanced multi-channel water texture"));
    
    if (!IsSystemReady())
    {
        UE_LOG(LogTemp, Error, TEXT("System not ready for advanced texture creation"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create RGBA texture for multi-channel data
    // R: Water Depth, G: Flow Speed, B: Flow Direction X, A: Flow Direction Y
    WaterDataTexture = UTexture2D::CreateTransient(Width, Height, PF_FloatRGBA);
    
    if (WaterDataTexture)
    {
        WaterDataTexture->Filter = TextureFilter::TF_Bilinear;
        WaterDataTexture->AddressX = TextureAddress::TA_Clamp;
        WaterDataTexture->AddressY = TextureAddress::TA_Clamp;
        WaterDataTexture->SRGB = false;
        WaterDataTexture->UpdateResource();
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced water texture created: %dx%d RGBA"), Width, Height);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create advanced water texture"));
    }
}

void UWaterSystem::UpdateAdvancedWaterTexture()
{
    if (!WaterDataTexture || !SimulationData.IsValid())
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create RGBA texture data
    TArray<FLinearColor> TextureData;
    TextureData.SetNum(Width * Height);
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                float Depth = SimulationData.WaterDepthMap[Index];
                float VelX = SimulationData.WaterVelocityX[Index];
                float VelY = SimulationData.WaterVelocityY[Index];
                
                // Calculate flow speed and normalize direction
                float FlowSpeed = FMath::Sqrt(VelX * VelX + VelY * VelY);
                float NormVelX = FlowSpeed > 0.01f ? VelX / FlowSpeed : 0.0f;
                float NormVelY = FlowSpeed > 0.01f ? VelY / FlowSpeed : 0.0f;
                
                // Pack data into RGBA channels
                TextureData[Index] = FLinearColor(
                    Depth * WaterDepthScale,                           // R: Depth
                    FMath::Clamp(FlowSpeed / 50.0f, 0.0f, 1.0f),      // G: Speed normalized
                    (NormVelX + 1.0f) * 0.5f,                         // B: Direction X [-1,1] → [0,1]
                    (NormVelY + 1.0f) * 0.5f                          // A: Direction Y [-1,1] → [0,1]
                );
            }
        }
    }
    
    // Upload to GPU
    if (WaterDataTexture->GetPlatformData() && WaterDataTexture->GetPlatformData()->Mips.Num() > 0)
    {
        FTexture2DMipMap& Mip = WaterDataTexture->GetPlatformData()->Mips[0];
        void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
        if (Data)
        {
            FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num() * sizeof(FLinearColor));
            Mip.BulkData.Unlock();
            WaterDataTexture->UpdateResource();
        }
    }
}

void UWaterSystem::CalculateFoamData()
{
    if (!SimulationData.IsValid())
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Calculate foam based on multiple conditions
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            
            float WaterDepth = SimulationData.WaterDepthMap[Index];
            if (WaterDepth < MinWaterDepth)
            {
                SimulationData.FoamMap[Index] = 0.0f;
                continue;
            }
            
            // Edge foam (shallow water)
            float EdgeFoam = 1.0f - FMath::Clamp(WaterDepth / 0.5f, 0.0f, 1.0f);
            
            // Velocity foam (turbulence)
            float FlowSpeed = FMath::Sqrt(
                SimulationData.WaterVelocityX[Index] * SimulationData.WaterVelocityX[Index] +
                SimulationData.WaterVelocityY[Index] * SimulationData.WaterVelocityY[Index]
            );
            float VelocityFoam = FMath::Clamp(FlowSpeed / 20.0f, 0.0f, 1.0f);
            
            // Flow convergence foam (where flows meet)
            float LeftVelX = SimulationData.WaterVelocityX[Y * Width + (X - 1)];
            float RightVelX = SimulationData.WaterVelocityX[Y * Width + (X + 1)];
            float UpVelY = SimulationData.WaterVelocityY[(Y - 1) * Width + X];
            float DownVelY = SimulationData.WaterVelocityY[(Y + 1) * Width + X];
            
            float Divergence = (RightVelX - LeftVelX) + (DownVelY - UpVelY);
            float ConvergenceFoam = FMath::Clamp(-Divergence * 5.0f, 0.0f, 1.0f);
            
            // Terrain slope foam (waterfalls)
            float TerrainHeight = GetTerrainHeightSafe(X, Y);
            float LeftHeight = GetTerrainHeightSafe(X - 1, Y);
            float RightHeight = GetTerrainHeightSafe(X + 1, Y);
            float UpHeight = GetTerrainHeightSafe(X, Y - 1);
            float DownHeight = GetTerrainHeightSafe(X, Y + 1);
            
            float MaxGradient = FMath::Max(
                FMath::Abs(TerrainHeight - LeftHeight),
                FMath::Max(
                    FMath::Abs(TerrainHeight - RightHeight),
                    FMath::Max(
                        FMath::Abs(TerrainHeight - UpHeight),
                        FMath::Abs(TerrainHeight - DownHeight)
                    )
                )
            );
            float SlopeFoam = FMath::Clamp(MaxGradient / 100.0f, 0.0f, 1.0f);
            
            // Combine foam factors
            float TotalFoam = FMath::Clamp(
                EdgeFoam + VelocityFoam * 0.5f + ConvergenceFoam + SlopeFoam,
                0.0f, 1.0f
            );
            
            SimulationData.FoamMap[Index] = TotalFoam;
        }
    }
}

void UWaterSystem::UpdateWeatherSystem(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    
    if (WeatherTimer >= WeatherChangeInterval)
    {
        // Simple weather state changes
        bIsRaining = FMath::RandBool();
        RainIntensity = FMath::RandRange(0.1f, 2.0f);
        WeatherTimer = 0.0f;
        
        FString RainText = bIsRaining ? TEXT("Yes") : TEXT("No");
        UE_LOG(LogTemp, Log, TEXT("WaterSystem: Weather changed - Rain=%s, Intensity=%.2f"),
               *RainText, RainIntensity);
    }
}


void UWaterSystem::ApplyRain(float DeltaTime)
    {
        if (!SimulationData.IsValid())
        {
            return;
        }
        
        float WaterPerCell = RainIntensity * DeltaTime * 0.1f;
        
        // Add water to random coordinates (not indices)
        int32 RainCells = FMath::RandRange(100, 1000);
        for (int32 i = 0; i < RainCells; i++)
        {
            int32 RandomX = FMath::RandRange(0, SimulationData.TerrainWidth - 1);
            int32 RandomY = FMath::RandRange(0, SimulationData.TerrainHeight - 1);
            int32 Index = RandomY * SimulationData.TerrainWidth + RandomX;
            
            SimulationData.WaterDepthMap[Index] += WaterPerCell;
        }
    }


// ===== PLAYER INTERACTION =====

void UWaterSystem::AddWater(FVector WorldPosition, float Amount)
{
    if (!IsSystemReady())
    {
        return;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    AddWaterAtIndex(X, Y, Amount);
}

void UWaterSystem::AddWaterAtIndex(int32 X, int32 Y, float Amount)
{
    if (!IsValidCoordinate(X, Y))
    {
        return;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        SimulationData.WaterDepthMap[Index] += Amount;
        
        // Mark chunk for visual update
        MarkChunkForUpdate(X, Y);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("WaterSystem: Added %.2f water at (%d,%d), depth now %.2f"),
               Amount, X, Y, SimulationData.WaterDepthMap[Index]);
    }
}

void UWaterSystem::RemoveWater(FVector WorldPosition, float Amount)
{
    if (!IsSystemReady())
    {
        return;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    if (IsValidCoordinate(X, Y))
    {
        int32 Index = GetTerrainIndex(X, Y);
        if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
        {
            SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, SimulationData.WaterDepthMap[Index] - Amount);
            
            // Mark chunk for visual update
            MarkChunkForUpdate(X, Y);
        }
    }
}

float UWaterSystem::GetWaterDepthAtPosition(FVector WorldPosition) const
{
    if (!IsSystemReady())
    {
        return 0.0f;
    }
    
    FVector2D TerrainCoords = WorldToTerrainCoordinates(WorldPosition);
    int32 X = FMath::RoundToInt(TerrainCoords.X);
    int32 Y = FMath::RoundToInt(TerrainCoords.Y);
    
    return GetWaterDepthAtIndex(X, Y);
}

float UWaterSystem::GetWaterDepthAtIndex(int32 X, int32 Y) const
{
    return GetWaterDepthSafe(X, Y);
}

// ===== WEATHER SYSTEM =====

void UWaterSystem::StartRain(float Intensity)
{
    bIsRaining = true;
    RainIntensity = Intensity;
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Rain started with intensity %.2f"), Intensity);
}

void UWaterSystem::StopRain()
{
    bIsRaining = false;
    UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Rain stopped"));
}

void UWaterSystem::SetAutoWeather(bool bEnable)
{
    bAutoWeather = bEnable;
    if (bEnable)
    {
        WeatherTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Auto weather enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystem: Auto weather disabled"));
    }
}

// ===== UTILITIES =====

float UWaterSystem::GetTotalWaterInSystem() const
{
    if (!SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    float TotalWater = 0.0f;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        TotalWater += WaterDepth;
    }
    return TotalWater;
}

int32 UWaterSystem::GetWaterCellCount() const
{
    if (!SimulationData.IsValid())
    {
        return 0;
    }
    
    int32 WaterCells = 0;
    for (float WaterDepth : SimulationData.WaterDepthMap)
    {
        if (WaterDepth > MinWaterDepth)
        {
            WaterCells++;
        }
    }
    return WaterCells;
}

float UWaterSystem::GetMaxFlowSpeed() const
{
    if (!SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    float MaxFlow = 0.0f;
    for (int32 i = 0; i < SimulationData.WaterVelocityX.Num(); i++)
    {
        float Flow = FMath::Sqrt(
            SimulationData.WaterVelocityX[i] * SimulationData.WaterVelocityX[i] +
            SimulationData.WaterVelocityY[i] * SimulationData.WaterVelocityY[i]
        );
        MaxFlow = FMath::Max(MaxFlow, Flow);
    }
    return MaxFlow;
}

void UWaterSystem::DrawDebugInfo() const
{
    if (!GEngine || !bShowWaterStats)
    {
        return;
    }
    
    int32 WaterCells = GetWaterCellCount();
    float TotalWater = GetTotalWaterInSystem();
    float MaxFlow = GetMaxFlowSpeed();
    
    GEngine->AddOnScreenDebugMessage(20, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Water Cells: %d"), WaterCells));
    GEngine->AddOnScreenDebugMessage(21, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Total Water: %.1f"), TotalWater));
    GEngine->AddOnScreenDebugMessage(22, 0.5f, FColor::Blue,
        FString::Printf(TEXT("Max Flow Speed: %.1f"), MaxFlow));
    
    FString RainStatus = bIsRaining ? TEXT("ON") : TEXT("OFF");
    GEngine->AddOnScreenDebugMessage(23, 0.5f, FColor::Cyan,
        FString::Printf(TEXT("Rain: %s (%.1f)"), *RainStatus, RainIntensity));
    
    FString SystemStatus = bEnableWaterSimulation ? TEXT("ACTIVE") : TEXT("DISABLED");
    GEngine->AddOnScreenDebugMessage(24, 0.5f, FColor::Cyan,
        FString::Printf(TEXT("Water System: %s"), *SystemStatus));
}

// ===== HELPER FUNCTIONS =====

float UWaterSystem::GetWaterDepthSafe(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y) || !SimulationData.IsValid())
    {
        return 0.0f;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        return SimulationData.WaterDepthMap[Index];
    }
    return 0.0f;
}

void UWaterSystem::SetWaterDepthSafe(int32 X, int32 Y, float Depth)
{
    if (!IsValidCoordinate(X, Y) || !SimulationData.IsValid())
    {
        return;
    }
    
    int32 Index = GetTerrainIndex(X, Y);
    if (Index >= 0 && Index < SimulationData.WaterDepthMap.Num())
    {
        SimulationData.WaterDepthMap[Index] = FMath::Max(0.0f, Depth);
    }
}

int32 UWaterSystem::GetTerrainIndex(int32 X, int32 Y) const
{
    if (!IsValidCoordinate(X, Y))
    {
        return -1;
    }
    
    return Y * SimulationData.TerrainWidth + X;
}

bool UWaterSystem::IsValidCoordinate(int32 X, int32 Y) const
{
    return X >= 0 && X < SimulationData.TerrainWidth &&
           Y >= 0 && Y < SimulationData.TerrainHeight;
}

float UWaterSystem::GetTerrainHeightSafe(int32 X, int32 Y) const
{
    if (!OwnerTerrain)
    {
        return 0.0f;
    }
    
    return OwnerTerrain->GetHeightSafe(X, Y);
}

FVector2D UWaterSystem::WorldToTerrainCoordinates(FVector WorldPosition) const
{
    if (!OwnerTerrain)
    {
        return FVector2D::ZeroVector;
    }
    
    return OwnerTerrain->WorldToTerrainCoordinates(WorldPosition);
}

void UWaterSystem::MarkChunkForUpdate(int32 X, int32 Y)
{
    if (!OwnerTerrain)
    {
        return;
    }
    
    int32 ChunkIndex = OwnerTerrain->GetChunkIndexFromCoordinates(X, Y);
    if (ChunkIndex >= 0)
    {
        OwnerTerrain->MarkChunkForUpdate(ChunkIndex);
    }
}

void UWaterSystem::UpdateShaderSystem(float DeltaTime)
{
    static float ShaderUpdateTimer = 0.0f;
    ShaderUpdateTimer += DeltaTime;
    
    if (ShaderUpdateTimer >= WaterShaderUpdateInterval)
    {
        UpdateWaterDepthTexture();
        ShaderUpdateTimer = 0.0f;
    }
}

void UWaterSystem::ApplyWaterTextureToMaterial(UMaterialInstanceDynamic* Material)
{
    UE_LOG(LogTemp, Error, TEXT("APPLYING WATER TEXTURE TO MATERIAL"));
    
    if (!Material)
    {
        UE_LOG(LogTemp, Error, TEXT("Material is NULL"));
        return;
    }
    
    if (!WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterDepthTexture is NULL - forcing creation"));
        CreateWaterDepthTexture();
        
        if (!WaterDepthTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("FAILED to create WaterDepthTexture"));
            return;
        }
    }
    Material->SetTextureParameterValue(FName("WaterDepthTexture"), WaterDepthTexture);
     Material->SetScalarParameterValue(FName("WaterDepthScale"), 1.0f / WaterDepthScale);
     
     UE_LOG(LogTemp, Error, TEXT("Texture applied: SUCCESS, Scale applied: SUCCESS"));
     
     // Force material to update
     Material->RecacheUniformExpressions(true);
     
     UE_LOG(LogTemp, Error, TEXT("Material updated with water texture"));
 }

    
    // Apply water texture and scaling parameters


void UWaterSystem::UpdateWaterShaderParameters()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("UPDATING SHADER PARAMETERS"));
    
    if (!WaterParameterCollection)
    {
        UE_LOG(LogTemp, Warning, TEXT("No WaterParameterCollection assigned"));
        return;
    }
    
    if (!OwnerTerrain || !OwnerTerrain->GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No world context for parameter collection"));
        return;
    }
    
    UWorld* World = OwnerTerrain->GetWorld();
    UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(WaterParameterCollection);
    
    if (Instance)
    {
        // Update time for wave animation
        float GameTime = World->GetTimeSeconds();
        bool bTimeSet = Instance->SetScalarParameterValue(FName("WaterTime"), GameTime);
        
        // Update flow strength
        float MaxFlow = GetMaxFlowSpeed();
        bool bFlowSet = Instance->SetScalarParameterValue(FName("WaterFlowStrength"), MaxFlow);
        
        // Update rain intensity
        float RainValue = bIsRaining ? RainIntensity : 0.0f;
        bool bRainSet = Instance->SetScalarParameterValue(FName("RainIntensity"), RainValue);
        
        FString TimeResult = bTimeSet ? TEXT("SUCCESS") : TEXT("FAILED");
        FString FlowResult = bFlowSet ? TEXT("SUCCESS") : TEXT("FAILED");
        FString RainResult = bRainSet ? TEXT("SUCCESS") : TEXT("FAILED");
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Parameters updated - Time:%s Flow:%s Rain:%s"),
               *TimeResult, *FlowResult, *RainResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get parameter collection instance"));
    }
}

void UWaterSystem::CreateWaterDepthTexture()
{
    UE_LOG(LogTemp, Error, TEXT("CREATING WATER DEPTH TEXTURE"));
    
    if (!IsSystemReady())
    {
        UE_LOG(LogTemp, Error, TEXT("System not ready - TerrainWidth=%d, TerrainHeight=%d"), 
               SimulationData.TerrainWidth, SimulationData.TerrainHeight);
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    UE_LOG(LogTemp, Error, TEXT("Attempting to create texture: %dx%d"), Width, Height);
    
    // Check if we're on the game thread
    if (!IsInGameThread())
    {
        UE_LOG(LogTemp, Error, TEXT("NOT ON GAME THREAD - deferring texture creation"));
        // Defer to game thread
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            CreateWaterDepthTexture();
        });
        return;
    }
    
    // Destroy existing texture if it exists
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Destroying existing texture"));
        WaterDepthTexture = nullptr;
    }
    
    // Create texture with explicit settings
    WaterDepthTexture = UTexture2D::CreateTransient(Width, Height, PF_R8);
    
    if (WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Texture created successfully"));
        
        // Configure texture settings
        WaterDepthTexture->Filter = TextureFilter::TF_Nearest;
        WaterDepthTexture->AddressX = TextureAddress::TA_Clamp;
        WaterDepthTexture->AddressY = TextureAddress::TA_Clamp;
        WaterDepthTexture->SRGB = false;
        WaterDepthTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
        
        // Force update
        WaterDepthTexture->UpdateResource();
        
        UE_LOG(LogTemp, Error, TEXT("Texture configured and updated"));
        
        // Immediately populate with test data
        UpdateWaterDepthTexture();
        
        UE_LOG(LogTemp, Error, TEXT("Initial texture data populated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TEXTURE CREATION FAILED - CreateTransient returned NULL"));
        
        // Try alternative creation method
        WaterDepthTexture = NewObject<UTexture2D>();
        if (WaterDepthTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("Trying NewObject approach..."));
            // This would need additional setup - but let's see if CreateTransient is the issue
        }
    }
}

void UWaterSystem::UpdateWaterDepthTexture()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("UPDATING WATER DEPTH TEXTURE"));
    
    if (!WaterDepthTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update - WaterDepthTexture is NULL"));
        return;
    }
    
    if (!SimulationData.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update - SimulationData invalid"));
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Create texture data array
    TArray<uint8> TextureData;
    TextureData.SetNum(Width * Height);
    
    // Track water statistics for debugging
    int32 NonZeroPixels = 0;
    float MaxWaterDepth = 0.0f;
    float TotalWater = 0.0f;
    
    // Convert water depth to texture values
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            if (Index < SimulationData.WaterDepthMap.Num())
            {
                float WaterDepth = SimulationData.WaterDepthMap[Index];
                
                if (WaterDepth > 0.01f)
                {
                    NonZeroPixels++;
                    TotalWater += WaterDepth;
                    MaxWaterDepth = FMath::Max(MaxWaterDepth, WaterDepth);
                }
                
                // Convert depth to 0-255 range
                uint8 TextureValue = 0;  // Default to no water
                if (WaterDepth > MinWaterDepth)  // Only set value if significant water
                {
                    TextureValue = FMath::Clamp(
                                                FMath::RoundToInt(WaterDepth * WaterDepthScale),
                                                1, 255  // Minimum 1 if water exists
                                                );
                    
                    TextureData[Index] = TextureValue;
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Texture sample values: [0]=%d [middle]=%d [end]=%d"),
               TextureData[0], TextureData[TextureData.Num()/2], TextureData[TextureData.Num()-1]);
        
        UE_LOG(LogTemp, Log, TEXT("Texture update: %d pixels with water, Max depth: %.2f, Total: %.2f"), 
               NonZeroPixels, MaxWaterDepth, TotalWater);
        
        // Upload to GPU with error checking
        if (WaterDepthTexture->GetPlatformData() && WaterDepthTexture->GetPlatformData()->Mips.Num() > 0)
        {
            FTexture2DMipMap& Mip = WaterDepthTexture->GetPlatformData()->Mips[0];
            
            void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
            if (Data)
            {
                FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num());
                Mip.BulkData.Unlock();
                
                // Force GPU update
                WaterDepthTexture->UpdateResource();
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Texture data uploaded to GPU"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to lock texture mip data"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Texture platform data or mips invalid"));
        }
    }
    
}

// ===== EROSION SIMULATION =====

void UWaterSystem::UpdateErosion(float DeltaTime)
{
    if (!SimulationData.IsValid() || !OwnerTerrain)
    {
        return;
    }
    
    const int32 Width = SimulationData.TerrainWidth;
    const int32 Height = SimulationData.TerrainHeight;
    
    // Process erosion for cells with significant water
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            
            if (SimulationData.WaterDepthMap[Index] > MinWaterDepth)
            {
                // Calculate flow speed
                float FlowSpeed = FMath::Sqrt(
                    SimulationData.WaterVelocityX[Index] * SimulationData.WaterVelocityX[Index] + 
                    SimulationData.WaterVelocityY[Index] * SimulationData.WaterVelocityY[Index]
                );
                
                if (FlowSpeed > MinErosionVelocity) // Fast water erodes
                {
                    float ErosionAmount = ErosionRate * FlowSpeed * DeltaTime;
                    
                    // Lower terrain height
                    float CurrentHeight = OwnerTerrain->GetHeightSafe(X, Y);
                    OwnerTerrain->SetHeightSafe(X, Y, CurrentHeight - ErosionAmount);
                    
                    // Store sediment in water
                    SimulationData.SedimentMap[Index] += ErosionAmount;
                    
                    // Mark chunk for visual update
                    MarkChunkForUpdate(X, Y);
                    
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Erosion at (%d,%d): %.3f, Flow: %.1f"), 
                           X, Y, ErosionAmount, FlowSpeed);
                }
                else if (SimulationData.SedimentMap[Index] > 0.0f) // Slow water deposits sediment
                {
                    float DepositionAmount = SimulationData.SedimentMap[Index] * DepositionRate * DeltaTime;
                    
                    // Raise terrain height
                    float CurrentHeight = OwnerTerrain->GetHeightSafe(X, Y);
                    OwnerTerrain->SetHeightSafe(X, Y, CurrentHeight + DepositionAmount);
                    
                    // Remove sediment from water
                    SimulationData.SedimentMap[Index] -= DepositionAmount;
                    
                    // Mark chunk for visual update
                    MarkChunkForUpdate(X, Y);
                    
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Deposition at (%d,%d): %.3f"), 
                           X, Y, DepositionAmount);
                }
            }
        }
    }
}
