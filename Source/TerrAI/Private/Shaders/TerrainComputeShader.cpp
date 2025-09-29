#include "Shaders/TerrainComputeShader.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphUtils.h"
#include "ShaderCore.h"
#include "RenderCore.h"

IMPLEMENT_GLOBAL_SHADER(FTerrainComputeCS, "/Project/TerrainCompute.usf", "TerrainComputeCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FOrographicComputeCS, "/Project/TerrainCompute.usf", "OrographicComputeCS", SF_Compute);
