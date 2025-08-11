#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"
#include "RenderGraphBuilder.h"
#include "DataDrivenShaderPlatformInfo.h"

class FWaveComputeCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FWaveComputeCS);
    SHADER_USE_PARAMETER_STRUCT(FWaveComputeCS, FGlobalShader);

public:
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, WaterDepthTexture)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, FlowDataTexture)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, WaveOutputTexture)
        SHADER_PARAMETER(float, Time)
        SHADER_PARAMETER(FVector4f, WindParams)
        SHADER_PARAMETER(FVector4f, WaveParams)
        SHADER_PARAMETER(FVector4f, TerrainParams)
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
    }
};
