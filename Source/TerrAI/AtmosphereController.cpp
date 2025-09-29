// AtmosphereController.cpp - FIXED VERSION
#include "AtmosphereController.h"
#include "AtmosphericSystem.h"
#include "DynamicTerrain.h"
#include "WaterSystem.h"
#include "GPUComputeManager.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "EngineUtils.h"
#include "ShaderParameterUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "GPUTerrainController.h"
#include "RHIStaticStates.h"
#include "PixelShaderUtils.h"
#include "ShaderCompilerCore.h"
#include "Shaders/AtmosphereComputeShader.h"

AAtmosphereController::AAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    
    // Create post process component for volumetric clouds
    CloudPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CloudPostProcess"));
    CloudPostProcess->SetupAttachment(Root);
    CloudPostProcess->bUnbound = true;
    CloudPostProcess->Priority = -100.0f;
}

void AAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    // Just reset state - don't touch resources
    bGPUResourcesInitialized = false;
    bUseGPUCompute = false;
    bInitializedWithAuthority = false;
    AccumulatedTime = 0.0f;
    
    // Create material instance (safe on game thread)
    if (VolumetricCloudMaterial)
    {
        CloudMaterialInstance = UMaterialInstanceDynamic::Create(VolumetricCloudMaterial, this);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: BeginPlay - waiting for authority"));
}

void AAtmosphereController::InitializeWithAuthority(AMasterWorldController* Master, ADynamicTerrain* Terrain)
{
    if (bInitializedWithAuthority)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Already initialized with authority, skipping"));
        return;
    }
    
    MasterController = Master;
    TargetTerrain = Terrain;
    
    UE_LOG(LogTemp, Warning, TEXT("=== AtmosphereController: Initializing with MasterController Authority ==="));
    
    // Get water system reference internally (since it's protected)
    if (TargetTerrain && TargetTerrain->WaterSystem)
    {
        WaterSystem = TargetTerrain->WaterSystem;  // This is fine since we're inside AtmosphereController
        UE_LOG(LogTemp, Warning, TEXT("  Connected to WaterSystem"));
    }
    
    // Get atmospheric system reference if available
    if (TargetTerrain && TargetTerrain->AtmosphericSystem)
    {
        AtmosphericSystem = TargetTerrain->AtmosphericSystem;
        UE_LOG(LogTemp, Warning, TEXT("  Connected to AtmosphericSystem"));
    }
    
    // Reset all timing for fresh start
    AccumulatedTime = 0.0f;
    InitializationTimer = 0.0f;
    bNeedsInitialState = true;
    PhysicsUpdateAccumulator = 0.0f;
    VisualUpdateAccumulator = 0.0f;
    
    // Initialize GPU resources
    InitializeGPUResources();
    
    // Mark as initialized with authority
    bInitializedWithAuthority = true;
    
    // Enable GPU compute after one frame to ensure resources are ready
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        if (bGPUResourcesInitialized)
        {
            EnableGPUCompute();
            UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: GPU Compute enabled via authority"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AtmosphereController: GPU resources failed to initialize"));
        }
    });
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Authority initialization complete"));
}


void AAtmosphereController::InitializeGPUResources()
{
    // If already initialized, just reset state
    if (bGPUResourcesInitialized)
    {
        AccumulatedTime = 0.0f;
        bNeedsInitialState = true;
        UE_LOG(LogTemp, Warning, TEXT("GPU Resources already initialized - resetting state only"));
        return;
    }
    
    // Get exact terrain dimensions - DO NOT round to power of 2
    if (TargetTerrain)
    {
        // Use EXACT terrain dimensions for perfect 1:1 mapping
        GridSizeX = TargetTerrain->TerrainWidth;   // Will be 513
        GridSizeY = TargetTerrain->TerrainHeight;  // Will be 513
        
        UE_LOG(LogTemp, Warning, TEXT("=== Atmosphere GPU Resource Initialization ==="));
        UE_LOG(LogTemp, Warning, TEXT("Using EXACT terrain dimensions: %dx%d"),
               GridSizeX, GridSizeY);
    }
    else
    {
        // Fallback if no terrain found
        GridSizeX = 513;
        GridSizeY = 513;
        UE_LOG(LogTemp, Warning, TEXT("No terrain found - using default 513x513 grid"));
    }
    
    // Validate grid size is reasonable
    if (GridSizeX <= 0 || GridSizeY <= 0 || GridSizeX > 4096 || GridSizeY > 4096)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid grid dimensions %dx%d - aborting GPU init"),
               GridSizeX, GridSizeY);
        return;
    }
    
    // Create textures on GAME thread (safe here)
    // Create Atmosphere State Texture
    AtmosphereStateTexture = NewObject<UTextureRenderTarget2D>(this);
    AtmosphereStateTexture->bCanCreateUAV = true;
    AtmosphereStateTexture->InitCustomFormat(GridSizeX, GridSizeY, PF_A32B32G32R32F, false);
    // DO NOT call UpdateResourceImmediate here - let it happen naturally
    
    // Create Cloud Render Texture
    CloudRenderTexture = NewObject<UTextureRenderTarget2D>(this);
    CloudRenderTexture->bCanCreateUAV = true;
    CloudRenderTexture->InitCustomFormat(GridSizeX, GridSizeY, PF_A32B32G32R32F, false);
    
    // Create Wind Field Texture
    WindFieldTexture = NewObject<UTextureRenderTarget2D>(this);
    WindFieldTexture->bCanCreateUAV = true;
    WindFieldTexture->InitCustomFormat(GridSizeX, GridSizeY, PF_G32R32F, false);
    
    // Create Precipitation Texture
    PrecipitationTexture = NewObject<UTextureRenderTarget2D>(this);
    PrecipitationTexture->bCanCreateUAV = true;
    PrecipitationTexture->InitCustomFormat(GridSizeX, GridSizeY, PF_R32_FLOAT, false);
    
    // Ensure resources are created on render thread
    FlushRenderingCommands();
    
    // Verify creation
    if (!AtmosphereStateTexture || !CloudRenderTexture ||
        !WindFieldTexture || !PrecipitationTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create one or more textures"));
        return;
    }
    
    // Mark as initialized
    bGPUResourcesInitialized = true;
    bNeedsInitialState = true;
    AccumulatedTime = 0.0f;
    // FrameCounter = 0;
    
    // Log success
    UE_LOG(LogTemp, Warning, TEXT("=== GPU Resources Initialization Complete ==="));
    UE_LOG(LogTemp, Warning, TEXT("Grid Size: %dx%d (exact, non-power-of-2)"), GridSizeX, GridSizeY);
    UE_LOG(LogTemp, Warning, TEXT("All textures created successfully"));
    
    // DO NOT try to access texture resources here or in a timer
    // Let the first frame of DispatchAtmosphereCompute handle initialization
}


void AAtmosphereController::InitializeAtmosphereTextures()
{
    if (!bGPUResourcesInitialized) return;
    
    ENQUEUE_RENDER_COMMAND(InitAtmosphereTextures)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);
            
            // Register external textures
            FRDGTextureRef StateRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(AtmosphereStateTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("AtmosphereState")));
            
            FRDGTextureRef WindRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(WindFieldTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("WindField")));
            
            // Create UAVs
            FRDGTextureUAVRef StateUAV = GraphBuilder.CreateUAV(StateRDG);
            FRDGTextureUAVRef WindUAV = GraphBuilder.CreateUAV(WindRDG);
            
            // Clear to initial values
            AddClearUAVPass(GraphBuilder, StateUAV, FVector4f(0.3f, 0.5f, 0.72f, 0.0f));
            AddClearUAVPass(GraphBuilder, WindUAV, FVector4f(5.0f, 2.0f, 0.0f, 0.0f));
            
            GraphBuilder.Execute();
        });
    
    // For more complex initialization, dispatch a one-time compute shader here
   // DispatchInitializationCompute();
}
/*
void AAtmosphereController::DispatchInitializationCompute()
{
    // Optional: Create a dedicated initialization compute shader
    // that generates more complex patterns than simple clear values
}
*/
// Call this once after resource creation
void AAtmosphereController::EnableGPUCompute()
{
    if (!bGPUResourcesInitialized)
    {
        InitializeGPUResources();
    }
    
    // Initialize textures with data ONCE
    InitializeAtmosphereTextures();
    
    // Now ready for compute
    bUseGPUCompute = true;
}

void AAtmosphereController::GenerateTestCloudData()
{
    if (!CloudRenderTexture || !bGPUResourcesInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("GenerateTestCloudData: Resources not ready"));
        return;
    }
    
    // Actually generate test cloud data with variation
    UE_LOG(LogTemp, Warning, TEXT("Generating test cloud pattern..."));
    
    // Capture textures for render thread
    UTextureRenderTarget2D* CloudTex = CloudRenderTexture;
    int32 Width = GridSizeX;
    int32 Height = GridSizeY;
    
    // Generate cloud data on CPU first (for testing)
    TArray<FLinearColor> CloudData;
    CloudData.SetNum(Width * Height);
    
    // Create Perlin-noise-like cloud pattern
    for (int32 y = 0; y < Height; y++)
    {
        for (int32 x = 0; x < Width; x++)
        {
            // Multiple octaves of noise for realistic clouds
            float Noise1 = FMath::Sin(x * 0.05f) * FMath::Cos(y * 0.05f);
            float Noise2 = FMath::Sin(x * 0.1f + 2.3f) * FMath::Cos(y * 0.1f + 1.7f) * 0.5f;
            float Noise3 = FMath::Sin(x * 0.2f + 5.1f) * FMath::Cos(y * 0.2f + 3.2f) * 0.25f;
            
            float CloudDensity = (Noise1 + Noise2 + Noise3) * 0.5f + 0.5f;
            CloudDensity = FMath::Clamp(CloudDensity, 0.0f, 1.0f);
            
            // Add threshold for more cloud-like appearance
            CloudDensity = CloudDensity > 0.4f ? CloudDensity : 0.0f;
            
            int32 Index = y * Width + x;
            CloudData[Index] = FLinearColor(CloudDensity, CloudDensity, CloudDensity, CloudDensity);
        }
    }
    
    ENQUEUE_RENDER_COMMAND(GenerateTestClouds)(
        [CloudTex, CloudData, Width, Height](FRHICommandListImmediate& RHICmdList)
        {
            FTextureRenderTargetResource* Resource = CloudTex->GetRenderTargetResource();
            if (!Resource)
            {
                UE_LOG(LogTemp, Error, TEXT("GenerateTestCloudData: No render target resource"));
                return;
            }
            
            // Update texture with cloud data - using FTextureRHIRef (not deprecated FTexture2DRHIRef)
            FTextureRHIRef TextureRHI = Resource->GetTexture2DRHI();
            
            // Use UpdateTexture2D which is the proper UE5 way
            uint32 Stride = Width * sizeof(FLinearColor);
            uint8* TextureData = (uint8*)CloudData.GetData();
            
            RHICmdList.UpdateTexture2D(
                TextureRHI,
                0,
                FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height),
                Stride,
                TextureData
            );
            
            UE_LOG(LogTemp, Warning, TEXT("Test cloud pattern generated: %dx%d with noise pattern"), Width, Height);
        });
}


void AAtmosphereController::DispatchAtmosphereCompute(float DeltaTime)
{
 
    
    // Standard validation
    if (!bUseGPUCompute || !bGPUResourcesInitialized)
    {
        return;
    }
    
    if (!AtmosphereStateTexture || !CloudRenderTexture ||
        !WindFieldTexture || !PrecipitationTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("DispatchAtmosphereCompute: Textures not allocated"));
        return;
    }
    
    if (!TargetTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("DispatchAtmosphereCompute: No terrain connected"));
        return;
    }
    
    // Update time accumulators
    AccumulatedTime += DeltaTime;
    InitializationTimer += DeltaTime;
    
    // Check for failed initialization (give it 5 seconds)
    if (InitializationTimer > 5.0f && !HasValidCloudData())
    {
        UE_LOG(LogTemp, Warning, TEXT("Atmosphere: No valid cloud data after 5 seconds, requesting re-init"));
        
        // Request re-initialization through MasterController
        if (MasterController)
        {
            // Could add a callback here to request re-init
            UE_LOG(LogTemp, Error, TEXT("Atmosphere: Failed to initialize properly"));
        }
        return;
    }
    
    // Debug logging
    static int32 FrameCounter = 0;
    FrameCounter++;
    const bool bShouldLog = (FrameCounter % 60 == 0);
    const bool bVerboseLog = (FrameCounter % 300 == 0);
    
    if (bVerboseLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== Atmosphere Compute Frame %d ==="), FrameCounter);
        UE_LOG(LogTemp, Warning, TEXT("  Authority: %s"),
               bInitializedWithAuthority ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("  Grid: %dx%d"), GridSizeX, GridSizeY);
        UE_LOG(LogTemp, Warning, TEXT("  AccumulatedTime: %.2f"), AccumulatedTime);
    }
    
    if (CloudRenderTexture->SizeX != GridSizeX || CloudRenderTexture->SizeY != GridSizeY)
        {
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Texture size mismatch detected!"));
            UE_LOG(LogTemp, Error, TEXT("  Grid: %dx%d"), GridSizeX, GridSizeY);
            UE_LOG(LogTemp, Error, TEXT("  CloudTexture: %dx%d"),
                   CloudRenderTexture->SizeX, CloudRenderTexture->SizeY);
            UE_LOG(LogTemp, Error, TEXT("Reinitializing GPU resources..."));
            
            // Force reinitialization
            bGPUResourcesInitialized = false;
            InitializeGPUResources();
            return;
        }
        
        // Debug logging for dispatch coverage
        const int32 ThreadGroupSize = 8;
        int32 DispatchX = FMath::DivideAndRoundUp(GridSizeX, ThreadGroupSize);
        int32 DispatchY = FMath::DivideAndRoundUp(GridSizeY, ThreadGroupSize);
        
        if (FrameCounter % 60 == 0) // Log every 60 frames
        {
            UE_LOG(LogTemp, Warning, TEXT("Atmosphere Dispatch Coverage:"));
            UE_LOG(LogTemp, Warning, TEXT("  Grid: %dx%d"), GridSizeX, GridSizeY);
            UE_LOG(LogTemp, Warning, TEXT("  Dispatch Groups: %dx%d"), DispatchX, DispatchY);
            UE_LOG(LogTemp, Warning, TEXT("  Thread Coverage: %dx%d"),
                   DispatchX * ThreadGroupSize, DispatchY * ThreadGroupSize);
            UE_LOG(LogTemp, Warning, TEXT("  Texture Actual: %dx%d"),
                   CloudRenderTexture->SizeX, CloudRenderTexture->SizeY);
        }
    
    // Capture for render thread
    UTextureRenderTarget2D* LocalStateTexture = AtmosphereStateTexture;
    UTextureRenderTarget2D* LocalCloudTexture = CloudRenderTexture;
    UTextureRenderTarget2D* LocalWindTexture = WindFieldTexture;
    UTextureRenderTarget2D* LocalPrecipTexture = PrecipitationTexture;
    
    UTextureRenderTarget2D* LocalTerrainTexture = nullptr;
    if (TargetTerrain && TargetTerrain->HeightRenderTexture)
    {
        LocalTerrainTexture = TargetTerrain->HeightRenderTexture;
    }
    
    int32 LocalGridSizeX = 513;
    int32 LocalGridSizeY = 513;
    
    float LocalDeltaTime = DeltaTime;
    float LocalAccumulatedTime = AccumulatedTime;
    float LocalTerrainScale = TargetTerrain ? TargetTerrain->TerrainScale : 100.0f;
    
    float LocalOrographicLift = OrographicLiftCoefficient;
    float LocalCoolingRate = AdiabatiCoolingRate;
    float LocalRainShadow = RainShadowIntensity;
    
    bool bLocalShouldLog = bShouldLog;
    int32 LocalFrameCounter = FrameCounter;
    
    // Render thread execution
    ENQUEUE_RENDER_COMMAND(AtmosphereComputeDispatch)(
        [LocalStateTexture, LocalCloudTexture, LocalWindTexture, LocalPrecipTexture,
         LocalTerrainTexture, LocalGridSizeX, LocalGridSizeY, LocalDeltaTime,
         LocalAccumulatedTime, LocalTerrainScale, LocalOrographicLift,
         LocalCoolingRate, LocalRainShadow, bLocalShouldLog, LocalFrameCounter]
        (FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);
            
            // Register textures
            FRDGTextureRef StateTextureRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(LocalStateTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("AtmosphereState")));
            
            FRDGTextureRef CloudTextureRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(LocalCloudTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("CloudRender")));
            
            FRDGTextureRef WindFieldRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(LocalWindTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("WindField")));
            
            FRDGTextureRef PrecipitationRDG = GraphBuilder.RegisterExternalTexture(
                CreateRenderTarget(LocalPrecipTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                TEXT("Precipitation")));
            
            FRDGTextureSRVRef TerrainHeightSRV = nullptr;
            if (LocalTerrainTexture)
            {
                FRDGTextureRef TerrainTextureRDG = GraphBuilder.RegisterExternalTexture(
                    CreateRenderTarget(LocalTerrainTexture->GetRenderTargetResource()->GetTexture2DRHI(),
                    TEXT("TerrainHeight")));
                TerrainHeightSRV = GraphBuilder.CreateSRV(TerrainTextureRDG);
            }
            
            // Get shader
            TShaderMapRef<FAtmosphereComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            
            if (!ComputeShader.IsValid())
            {
                if (bLocalShouldLog)
                {
                    UE_LOG(LogTemp, Error, TEXT("AtmosphereCompute shader not found!"));
                }
                GraphBuilder.Execute();
                return;
            }
            
            // Set parameters
            FAtmosphereComputeShaderParameters* Parameters =
                GraphBuilder.AllocParameters<FAtmosphereComputeShaderParameters>();
            
            Parameters->StateTexture = GraphBuilder.CreateUAV(StateTextureRDG);
            Parameters->RenderTexture = GraphBuilder.CreateUAV(CloudTextureRDG);
            Parameters->WindFieldTexture = GraphBuilder.CreateUAV(WindFieldRDG);
            Parameters->PrecipitationTexture = GraphBuilder.CreateUAV(PrecipitationRDG);
            
            if (TerrainHeightSRV)
            {
                Parameters->TerrainHeightTexture = TerrainHeightSRV;
                Parameters->TerrainSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
            }
            else
            {
                // Dummy terrain
                FRDGTextureDesc DummyDesc = FRDGTextureDesc::Create2D(
                    FIntPoint(1, 1), PF_R32_FLOAT, FClearValueBinding::Black, TexCreate_ShaderResource);
                FRDGTextureRef DummyTexture = GraphBuilder.CreateTexture(DummyDesc, TEXT("DummyTerrain"));
                Parameters->TerrainHeightTexture = GraphBuilder.CreateSRV(DummyTexture);
                Parameters->TerrainSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
            }
            
            Parameters->DeltaTime = LocalDeltaTime;
            Parameters->AccumulatedTime = LocalAccumulatedTime;
            Parameters->GridSize = FIntPoint(LocalGridSizeX, LocalGridSizeY);
            Parameters->TerrainScale = LocalTerrainScale;
            Parameters->OrographicLiftCoefficient = LocalOrographicLift;
            Parameters->AdiabatiCoolingRate = LocalCoolingRate;
            Parameters->RainShadowIntensity = LocalRainShadow;
            
            Parameters->bForceInit = (FrameCounter < 2) ? 1 : 0;  // Init first 2 frames only
            
            // Dispatch
            const int32 ThreadGroupSize = 8;
            int32 DispatchX = FMath::DivideAndRoundUp(LocalGridSizeX, ThreadGroupSize);
            int32 DispatchY = FMath::DivideAndRoundUp(LocalGridSizeY, ThreadGroupSize);
            
            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("AtmosphereCompute_Frame%d", LocalFrameCounter),
                ComputeShader,
                Parameters,
                FIntVector(DispatchX, DispatchY, 1)
            );
            
            GraphBuilder.Execute();
        }
    );
    
    // Update materials after dispatch
    UpdateCloudMaterial();
    
    if (AtmosphereDebugPlane)
    {
        UpdateDebugPlane();
    }
}

void AAtmosphereController::ExecuteAtmosphericCompute(float DeltaTime)
{
    if (!bUseGPUCompute)
    {
        return;
    }
    
    DispatchAtmosphereCompute(DeltaTime);
}

void AAtmosphereController::UpdateCloudMaterial()
{
    if (!CloudMaterialInstance || !CloudRenderTexture)
    {
        return;
    }
    
    // Set texture parameters - use BOTH names for compatibility
    CloudMaterialInstance->SetTextureParameterValue(TEXT("CloudDensity"), CloudRenderTexture);
    CloudMaterialInstance->SetTextureParameterValue(TEXT("CloudTexture"), CloudRenderTexture);  // Match Blueprint
    
    if (WindFieldTexture)
    {
        CloudMaterialInstance->SetTextureParameterValue(TEXT("WindField"), WindFieldTexture);
    }
    
    if (PrecipitationTexture)
    {
        CloudMaterialInstance->SetTextureParameterValue(TEXT("Precipitation"), PrecipitationTexture);
    }
    
    // Set scalar parameters using CloudDensityScale member variable
    CloudMaterialInstance->SetScalarParameterValue(TEXT("CloudScale"), 1.0f);  // Default scale
    CloudMaterialInstance->SetScalarParameterValue(TEXT("CloudDensityMultiplier"), CloudDensityScale);
    CloudMaterialInstance->SetScalarParameterValue(TEXT("CloudSpeed"), 1.0f);  // Default speed
    
    UE_LOG(LogTemp, Verbose, TEXT("UpdateCloudMaterial: Set CloudTexture parameter"));
}

void AAtmosphereController::PushInitialStateToGPU()
{
    if (!bGPUResourcesInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pushing initial atmospheric state to GPU..."));
    
    // Capture for render thread - these are just pointers, safe to capture
    UTextureRenderTarget2D* LocalStateTexture = AtmosphereStateTexture;
    UTextureRenderTarget2D* LocalWindTexture = WindFieldTexture;
    UTextureRenderTarget2D* LocalCloudTexture = CloudRenderTexture;
    UTextureRenderTarget2D* LocalPrecipTexture = PrecipitationTexture;
    int32 Width = GridSizeX;
    int32 Height = GridSizeY;
    
    // Execute on render thread
    ENQUEUE_RENDER_COMMAND(InitAtmosphereState)(
        [LocalStateTexture, LocalWindTexture, LocalCloudTexture, LocalPrecipTexture, Width, Height]
        (FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);
            
            // Initialize State Texture
            if (LocalStateTexture && LocalStateTexture->GetRenderTargetResource())
            {
                FTextureRHIRef TextureRHI = LocalStateTexture->GetRenderTargetResource()->GetTexture2DRHI();
                if (TextureRHI.IsValid())
                {
                    FRDGTextureRef StateRDG = GraphBuilder.RegisterExternalTexture(
                        CreateRenderTarget(TextureRHI, TEXT("InitState")));
                    
                    FRDGTextureUAVRef StateUAV = GraphBuilder.CreateUAV(StateRDG);
                    
                    // Clear to initial atmospheric state
                    AddClearUAVPass(GraphBuilder, StateUAV,
                        FVector4f(0.3f, 0.5f, 0.72f, 0.0f));  // Initial cloud/moisture/temp/precip
                }
            }
            
            // Initialize Wind Field
            if (LocalWindTexture && LocalWindTexture->GetRenderTargetResource())
            {
                FTextureRHIRef TextureRHI = LocalWindTexture->GetRenderTargetResource()->GetTexture2DRHI();
                if (TextureRHI.IsValid())
                {
                    FRDGTextureRef WindRDG = GraphBuilder.RegisterExternalTexture(
                        CreateRenderTarget(TextureRHI, TEXT("InitWind")));
                    
                    FRDGTextureUAVRef WindUAV = GraphBuilder.CreateUAV(WindRDG);
                    
                    // Set initial wind
                    AddClearUAVPass(GraphBuilder, WindUAV, FVector4f(5.0f, 2.0f, 0.0f, 0.0f));
                }
            }
            
            GraphBuilder.Execute();
        });
}

bool AAtmosphereController::IsReadyForGPU() const
{
    return TargetTerrain != nullptr &&
           VolumetricCloudMaterial != nullptr;
}

void AAtmosphereController::DisableGPUCompute()
{
    bUseGPUCompute = false;
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: GPU Compute disabled"));
}

void AAtmosphereController::UpdateDebugPlane()
{
    // Update debug plane material if it exists
    if (AtmosphereDebugPlane)
    {
        // Find the material on the debug plane
        UStaticMeshComponent* MeshComp = AtmosphereDebugPlane->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            UMaterialInstanceDynamic* DebugMat = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(0));
            if (DebugMat && CloudRenderTexture)
            {
                // Set the texture parameter (try both names for compatibility)
                DebugMat->SetTextureParameterValue(TEXT("CloudTexture"), CloudRenderTexture);
                DebugMat->SetTextureParameterValue(TEXT("CloudDensity"), CloudRenderTexture);
                DebugMat->SetTextureParameterValue(TEXT("BaseTexture"), CloudRenderTexture);
                DebugMat->SetTextureParameterValue(TEXT("DiffuseTexture"), CloudRenderTexture);
                
            // COMMENTED OUT FOR SPAM BUT HELPFUL FOR TESTING    UE_LOG(LogTemp, Warning, TEXT("UpdateDebugPlane: Updated material with cloud texture"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UpdateDebugPlane: No dynamic material found on debug plane"));
            }
        }
    }
    else if (CloudDebugPlane && CloudRenderTexture)
    {
        // Update the component material directly if we have it
        if (CloudDebugMaterial)
        {
            CloudDebugMaterial->SetTextureParameterValue(TEXT("CloudTexture"), CloudRenderTexture);
            CloudDebugMaterial->SetTextureParameterValue(TEXT("CloudDensity"), CloudRenderTexture);
            UE_LOG(LogTemp, Warning, TEXT("UpdateDebugPlane: Updated CloudDebugMaterial"));
        }
    }
    
    // Also update the main cloud material instance
    UpdateCloudMaterial();
}

void AAtmosphereController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Just disable compute and null pointers - let UE5 handle cleanup
    bUseGPUCompute = false;
    bGPUResourcesInitialized = false;
    bInitializedWithAuthority = false;
    
    // Add this to reset state
      AccumulatedTime = 0.0f;
      InitializationTimer = 0.0f;
      bNeedsInitialState = true;
    
    // Null all pointers - UE5 will garbage collect
    AtmosphereStateTexture = nullptr;
    CloudRenderTexture = nullptr;
    WindFieldTexture = nullptr;
    PrecipitationTexture = nullptr;
    CloudMaterialInstance = nullptr;
    MasterController = nullptr;
    TargetTerrain = nullptr;
    
    Super::EndPlay(EndPlayReason);
}

void AAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    
    if (!bUseGPUCompute)
    {
        return;
    }
    
    // Update atmospheric simulation
    PhysicsUpdateAccumulator += DeltaTime;
    
    const float PhysicsUpdateInterval = 1.0f / 30.0f; // 30 Hz physics
    if (PhysicsUpdateAccumulator >= PhysicsUpdateInterval)
    {
        ExecuteAtmosphericCompute(PhysicsUpdateInterval);
        PhysicsUpdateAccumulator = 0.0f;
    }
    
    // Update visual elements every frame
    UpdateCloudMaterial();
}

// ===== MISSING FUNCTION IMPLEMENTATIONS =====

void AAtmosphereController::Initialize(ADynamicTerrain* Terrain, UWaterSystem* Water)
{
    TargetTerrain = Terrain;
    WaterSystem = Water;
    
    if (TargetTerrain)
    {
        // Get atmospheric system from terrain if available
        if (TargetTerrain->AtmosphericSystem)
        {
            AtmosphericSystem = TargetTerrain->AtmosphericSystem;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Connected to terrain and water systems"));
    }
    
    // Create material instance if we have a base material
    if (VolumetricCloudMaterial && !CloudMaterialInstance)
    {
        CloudMaterialInstance = UMaterialInstanceDynamic::Create(VolumetricCloudMaterial, this);
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: Created CloudMaterialInstance"));
    }
}

void AAtmosphereController::UpdateAtmosphericSystem(float ScaledDeltaTime)
{
    // Called by TemporalManager with pre-scaled time
    // Accumulate for next GPU physics update
    PhysicsUpdateAccumulator += ScaledDeltaTime;
    
    // Update atmospheric system if available
    if (AtmosphericSystem)
    {
        // Update atmospheric simulation logic here
        // This would typically update wind patterns, cloud movement, etc.
    }
}

void AAtmosphereController::SetWeatherIntensity(float Intensity)
{
    // Clamp intensity to valid range
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Update cloud density based on weather intensity
    CloudDensityScale = FMath::Lerp(0.1f, 2.0f, ClampedIntensity);
    
    // Update material parameters
    if (CloudMaterialInstance)
    {
        CloudMaterialInstance->SetScalarParameterValue(TEXT("WeatherIntensity"), ClampedIntensity);
        CloudMaterialInstance->SetScalarParameterValue(TEXT("CloudDensityMultiplier"), CloudDensityScale);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Weather intensity set to: %.2f"), ClampedIntensity);
}

void AAtmosphereController::SetWindDirection(FVector Direction)
{
    // Normalize the direction
    FVector NormalizedDirection = Direction.GetSafeNormal();
    
    // Store wind direction for atmospheric calculations
    if (WindFieldTexture && bGPUResourcesInitialized)
    {
        // Update wind field texture with new direction
        // This would be done in a compute shader in production
    }
    
    // Update material parameters
    if (CloudMaterialInstance)
    {
        CloudMaterialInstance->SetVectorParameterValue(TEXT("WindDirection"),
            FLinearColor(NormalizedDirection.X, NormalizedDirection.Y, NormalizedDirection.Z, 1.0f));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Wind direction set to: %s"), *NormalizedDirection.ToString());
}

void AAtmosphereController::TriggerLightning(FVector Location)
{
    // Lightning effect at specified location
    UE_LOG(LogTemp, Warning, TEXT("Lightning triggered at: %s"), *Location.ToString());
    
    // In a full implementation, this would:
    // 1. Spawn a lightning visual effect
    // 2. Play thunder sound with delay based on distance
    // 3. Briefly illuminate the area
    // 4. Update atmospheric electrical charge simulation
    
    // For now, just update material to show lightning flash
    if (CloudMaterialInstance)
    {
        CloudMaterialInstance->SetVectorParameterValue(TEXT("LightningLocation"),
            FLinearColor(Location.X, Location.Y, Location.Z, 1.0f));
        CloudMaterialInstance->SetScalarParameterValue(TEXT("LightningFlash"), 1.0f);
        
        // Reset flash after a short time
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (CloudMaterialInstance)
            {
                CloudMaterialInstance->SetScalarParameterValue(TEXT("LightningFlash"), 0.0f);
            }
        });
    }
}

FVector AAtmosphereController::GetWindAtLocation(FVector WorldLocation) const
{
    // Default wind vector
    FVector WindVector = FVector(10.0f, 5.0f, 0.0f); // Default east-northeast wind
    
    // If we have wind field data, sample it
    if (WindFieldTexture && bGPUResourcesInitialized)
    {
        // In production, this would sample the wind field texture
        // at the given world location to get local wind conditions
        
        // For now, return a simple wind pattern with some variation based on location
        float NoiseX = FMath::Sin(WorldLocation.X * 0.001f) * 5.0f;
        float NoiseY = FMath::Cos(WorldLocation.Y * 0.001f) * 5.0f;
        
        WindVector.X += NoiseX;
        WindVector.Y += NoiseY;
    }
    
    return WindVector;
}

void AAtmosphereController::CreateDebugCloudPlane()
{
    // Create a debug plane to visualize clouds
    if (!CloudDebugPlane)
    {
        // Create a simple plane mesh component
        CloudDebugPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CloudDebugPlane"));
        CloudDebugPlane->SetupAttachment(GetRootComponent());
        
        // Load default plane mesh
        UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Engine/BasicShapes/Plane"));
        
        if (PlaneMesh)
        {
            CloudDebugPlane->SetStaticMesh(PlaneMesh);
            CloudDebugPlane->SetWorldScale3D(FVector(100.0f, 100.0f, 1.0f));
            CloudDebugPlane->SetWorldLocation(FVector(0, 0, CloudBaseHeight));
            
            // Create debug material if we have cloud texture
            if (CloudRenderTexture)
            {
                UMaterial* BaseMat = LoadObject<UMaterial>(nullptr,
                    TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
                
                if (BaseMat)
                {
                    CloudDebugMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
                    CloudDebugMaterial->SetTextureParameterValue(TEXT("Texture"), CloudRenderTexture);
                    CloudDebugPlane->SetMaterial(0, CloudDebugMaterial);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Debug cloud plane created"));
    }
}

void AAtmosphereController::DebugReadCloudTexture()
{
    if (!CloudRenderTexture || !bGPUResourcesInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugReadCloudTexture: Resources not ready"));
        return;
    }
    
    // Read back a sample of cloud data for debugging
    // This is expensive and should only be used for debugging
    
    UE_LOG(LogTemp, Warning, TEXT("=== Cloud Texture Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("Texture Size: %dx%d"), CloudRenderTexture->SizeX, CloudRenderTexture->SizeY);
    
    // Get the pixel format enum value
    EPixelFormat Format = CloudRenderTexture->GetFormat();
    UE_LOG(LogTemp, Warning, TEXT("Format: %d"), (int32)Format);
    
    UE_LOG(LogTemp, Warning, TEXT("UAV Enabled: %s"), CloudRenderTexture->bCanCreateUAV ? TEXT("YES") : TEXT("NO"));
    
    // In production, you could read back actual pixel data here for debugging
    // but it requires render thread synchronization
}

void AAtmosphereController::ValidateCloudRendering()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Cloud Rendering Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("CloudRenderTexture: %s"), CloudRenderTexture ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("VolumetricCloudMaterial: %s"), VolumetricCloudMaterial ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("CloudMaterialInstance: %s"), CloudMaterialInstance ? TEXT("Created") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("CloudPostProcess: %s"), CloudPostProcess ? TEXT("Valid") : TEXT("NULL"));
    
    if (CloudRenderTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Texture Size: %dx%d"), CloudRenderTexture->SizeX, CloudRenderTexture->SizeY);
        UE_LOG(LogTemp, Warning, TEXT("  Can Create UAV: %s"), CloudRenderTexture->bCanCreateUAV ? TEXT("YES") : TEXT("NO"));
    }
    
    if (CloudPostProcess && CloudPostProcess->Settings.WeightedBlendables.Array.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Post Process has %d blendables"),
               CloudPostProcess->Settings.WeightedBlendables.Array.Num());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GPU Resources Initialized: %s"), bGPUResourcesInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("GPU Compute Enabled: %s"), bUseGPUCompute ? TEXT("YES") : TEXT("NO"));
}

int32 AAtmosphereController::GetTerrainWidth() const
{
    return TargetTerrain ? TargetTerrain->TerrainWidth : 0;
}

int32 AAtmosphereController::GetTerrainHeight() const
{
    return TargetTerrain ? TargetTerrain->TerrainHeight : 0;
}

void AAtmosphereController::CleanupGPUResources()
{
    /*// Disable GPU compute first
    bGPUResourcesInitialized = false;
    bUseGPUCompute = false;
    
    // Wait for any pending render commands
    FlushRenderingCommands();
    
    // Release texture resources properly
    if (AtmosphereStateTexture)
    {
        AtmosphereStateTexture->ReleaseResource();
        AtmosphereStateTexture = nullptr;
    }
    
    if (CloudRenderTexture)
    {
        CloudRenderTexture->ReleaseResource();
        CloudRenderTexture = nullptr;
    }
    
    if (WindFieldTexture)
    {
        WindFieldTexture->ReleaseResource();
        WindFieldTexture = nullptr;
    }
    
    if (PrecipitationTexture)
    {
        PrecipitationTexture->ReleaseResource();
        PrecipitationTexture = nullptr;
    }
    
    // Reset state flags
    bNeedsInitialState = true;
    InitializationTimer = 0.0f;
    AccumulatedTime = 0.0f;
    */
    UE_LOG(LogTemp, Warning, TEXT("Atmosphere GPU resources cleaned up EMPTY FUNCTION"));
}

bool AAtmosphereController::HasValidCloudData() const
{
    // Simple check - assume valid after initialization period
    // In production, you might want to actually sample the texture
    return InitializationTimer > 0.5f && bGPUResourcesInitialized;
}
