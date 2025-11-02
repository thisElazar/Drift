// VolumetricCloudsComputeShader.cpp
#include "Shaders/VolumetricCloudsComputeShader.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphUtils.h"
#include "ShaderCore.h"

IMPLEMENT_GLOBAL_SHADER(FVolumetricCloudsComputeShader, "/Project/VolumetricClouds.usf", "VolumetricCloudsCS", SF_Compute);
