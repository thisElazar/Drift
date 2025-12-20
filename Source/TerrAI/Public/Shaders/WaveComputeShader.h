// WaveComputeShader.h - GPU Wave Compute Shader Declaration
#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

/**
 * Wave compute shader for GPU-based water wave calculation
 * Processes water depth and flow data to generate wave heights
 */
class TERRAI_API FWaveComputeCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FWaveComputeCS);
    SHADER_USE_PARAMETER_STRUCT(FWaveComputeCS, FGlobalShader);
    
    // Shader parameters structure
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        // Input textures
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, WaterDepthTexture)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FlowDataTexture)
    
        
        // Output texture
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, WaveOutputTexture)
        
        // Simulation parameters
        SHADER_PARAMETER(float, Time)
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(FVector4f, WindParams)  // xy = direction, z = strength, w = unused
        SHADER_PARAMETER(FVector4f, WaveParams)  // x = scale, y = speed, z = damping, w = terrain scale
        SHADER_PARAMETER(FVector4f, TerrainParams)  // xy = dimensions, zw = unused
        
        // Sampler for texture reads
        SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
    END_SHADER_PARAMETER_STRUCT()
    
    // Shader compilation conditions
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        // Compile for SM5 and above (DX11/DX12/Vulkan)
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
    
    // Modify compilation environment
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
                                            FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        
        // Set thread group size
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), 8);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), 8);
        OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), 1);
    }
};
