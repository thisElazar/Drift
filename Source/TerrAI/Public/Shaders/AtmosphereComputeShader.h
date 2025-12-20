// AtmosphereComputeShader.h - Complete with Orographic Parameters
#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"

// Enhanced shader parameters with orographic inputs

BEGIN_SHADER_PARAMETER_STRUCT(FAtmosphereComputeShaderParameters, )
    // Read-write textures
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, StateTexture)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, RenderTexture)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>, WindFieldTexture)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, PrecipitationTexture)
    
    // Input terrain data
    SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, TerrainHeightTexture)
    SHADER_PARAMETER_SAMPLER(SamplerState, TerrainSampler)
    
    // Time parameters
    SHADER_PARAMETER(float, DeltaTime)
    SHADER_PARAMETER(float, AccumulatedTime)
    SHADER_PARAMETER(uint, bForceInit)
    
    // Grid parameters
    SHADER_PARAMETER(FIntPoint, GridSize)
    SHADER_PARAMETER(float, TerrainScale)
    
    // Orographic parameters
    SHADER_PARAMETER(float, OrographicLiftCoefficient)
    SHADER_PARAMETER(float, AdiabatiCoolingRate)
    SHADER_PARAMETER(float, RainShadowIntensity)
END_SHADER_PARAMETER_STRUCT()

// Compute shader class
class FAtmosphereComputeShader : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FAtmosphereComputeShader);
    SHADER_USE_PARAMETER_STRUCT(FAtmosphereComputeShader, FGlobalShader);

    using FParameters = FAtmosphereComputeShaderParameters;

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
                                            FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE"), 8);
    }
};
