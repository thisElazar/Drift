// WaveComputeShader.cpp - Wave Compute Shader Implementation
#include "Shaders/WaveComputeShader.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphUtils.h"
#include "ShaderCore.h"


// Register the shader with the engine
// This links the C++ class to the HLSL shader file
IMPLEMENT_GLOBAL_SHADER(FWaveComputeCS, "/Project/WaveCompute.usf", "WaveComputeCS", SF_Compute);
