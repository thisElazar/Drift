// WaveComputeShader.cpp - Updated for UE5
#include "Shaders/WaveComputeShader.h"
#include "ShaderCore.h"
#include "RenderGraphUtils.h"

IMPLEMENT_GLOBAL_SHADER(FWaveComputeCS, "/Project/WaveCompute.usf", "WaveComputeCS", SF_Compute);
