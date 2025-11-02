// VolumetricCloudsComputeShader.h
#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"

/**
 * Volumetric cloud rendering compute shader
 * Performs full raymarching with orographic displacement
 */

BEGIN_SHADER_PARAMETER_STRUCT(FVolumetricCloudsComputeShaderParameters, )
    // Output render target
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
    
    // Input textures from atmosphere simulation
    SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, CloudDataTexture)
    SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float2>, WindFieldTexture)
    SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, SceneDepthTexture)
    SHADER_PARAMETER_SAMPLER(SamplerState, CloudDataSampler)
    SHADER_PARAMETER_SAMPLER(SamplerState, WindFieldSampler)
    
    // Cloud layer parameters
    SHADER_PARAMETER(float, CloudBaseHeight)
    SHADER_PARAMETER(float, CloudTopHeight)
    SHADER_PARAMETER(float, CloudDensityScale)
    SHADER_PARAMETER(FVector3f, CloudBoundsMin)
    SHADER_PARAMETER(FVector3f, CloudBoundsMax)
    SHADER_PARAMETER(FVector2f, TerrainOffset)
    
    // Rendering parameters
    SHADER_PARAMETER(FVector3f, CameraPosition)
    SHADER_PARAMETER(FVector3f, LightDirection)
    SHADER_PARAMETER(FMatrix44f, InvViewProjectionMatrix)
    SHADER_PARAMETER(FIntPoint, RenderTargetSize)
    SHADER_PARAMETER(float, Time)
    
    // Orographic parameters
    SHADER_PARAMETER(float, OrographicStrength)
    SHADER_PARAMETER(float, MaxOrographicLift)
END_SHADER_PARAMETER_STRUCT()

class FVolumetricCloudsComputeShader : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FVolumetricCloudsComputeShader);
    SHADER_USE_PARAMETER_STRUCT(FVolumetricCloudsComputeShader, FGlobalShader);

    using FParameters = FVolumetricCloudsComputeShaderParameters;

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
    }
};
