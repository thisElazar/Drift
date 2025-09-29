// TerrainComputeShader.h - GPU Terrain Height Processing
#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"
#include "RenderGraphBuilder.h"

/**
 * Terrain compute shader for GPU-based height processing
 * Handles erosion, orographic effects, and height modifications
 */
class TERRAI_API FTerrainComputeCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FTerrainComputeCS);
    SHADER_USE_PARAMETER_STRUCT(FTerrainComputeCS, FGlobalShader);
    
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        // Input/Output height texture (read-write)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, HeightTexture)
        
        // Water depth for erosion calculations
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, WaterDepthTexture)
        
        // Flow velocity for erosion
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float2>, FlowVelocityTexture)
        
        // Atmospheric moisture for orographic effects
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, MoistureTexture)
        
        // Wind field for orographic calculations
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float2>, WindFieldTexture)
        
        // Rock hardness map
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HardnessTexture)
        
        // Output erosion amount for feedback
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, ErosionOutputTexture)
        
        // Simulation parameters
        SHADER_PARAMETER(FVector4f, TerrainParams) // xy = dimensions, z = scale, w = time
        SHADER_PARAMETER(FVector4f, ErosionParams) // x = rate, y = deposition, z = capacity, w = hardness mult
        SHADER_PARAMETER(FVector4f, OrographicParams) // x = lift rate, y = moisture threshold, z = precip rate, w = unused
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(uint32, SimulationMode) // 0=height only, 1=erosion, 2=orographic, 3=all
        
        // Brush modification parameters (for real-time editing)
        SHADER_PARAMETER(FVector4f, BrushParams) // xy = position, z = radius, w = strength
        SHADER_PARAMETER(uint32, BrushActive)
        
        SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
    END_SHADER_PARAMETER_STRUCT()
    
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
    
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
                                            FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), 8);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), 8);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), 1);
    }
};

/**
 * Orographic lift compute shader - atmospheric interaction
 */
class TERRAI_API FOrographicComputeCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FOrographicComputeCS);
    SHADER_USE_PARAMETER_STRUCT(FOrographicComputeCS, FGlobalShader);
    
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightTexture)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float2>, WindFieldTexture)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, VerticalVelocityTexture)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, PrecipitationTexture)
        SHADER_PARAMETER(FVector4f, TerrainParams)
        SHADER_PARAMETER(FVector4f, AtmosphericParams)
        SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
    END_SHADER_PARAMETER_STRUCT()
    
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};
