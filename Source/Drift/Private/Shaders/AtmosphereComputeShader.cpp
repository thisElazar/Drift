// AtmosphereComputeShader.cpp - COMPLETE FIXED FILE
#include "Shaders/AtmosphereComputeShader.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphUtils.h"
#include "ShaderCore.h"

// CRITICAL FIX: Use IMPLEMENT_GLOBAL_SHADER with correct syntax for UE5.5
IMPLEMENT_GLOBAL_SHADER(FAtmosphereComputeShader, "/Project/AtmosphereCompute.usf", "MainCS", SF_Compute);
