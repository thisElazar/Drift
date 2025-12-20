// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ===== TERRAI VALIDATION MACROS =====
// Centralized validation patterns to eliminate code duplication

// Water system validation
#define TERRAI_VALIDATE_WATER_SYSTEM(WaterSystemPtr) \
    if (!WaterSystemPtr || !WaterSystemPtr->IsSystemReady()) \
    { \
        UE_LOG(LogTemp, VeryVerbose, TEXT("%s: Water system not ready"), *FString(__FUNCTION__)); \
        return; \
    }

#define TERRAI_VALIDATE_WATER_SYSTEM_RET(WaterSystemPtr, RetVal) \
    if (!WaterSystemPtr || !WaterSystemPtr->IsSystemReady()) \
    { \
        UE_LOG(LogTemp, VeryVerbose, TEXT("%s: Water system not ready"), *FString(__FUNCTION__)); \
        return RetVal; \
    }

// Terrain system validation
#define TERRAI_VALIDATE_TERRAIN_SYSTEM(TerrainPtr) \
    if (!TerrainPtr) \
    { \
        UE_LOG(LogTemp, Log, TEXT("%s: Terrain system is null"), *FString(__FUNCTION__)); \
        return; \
    }

#define TERRAI_VALIDATE_TERRAIN_SYSTEM_RET(TerrainPtr, RetVal) \
    if (!TerrainPtr) \
    { \
        UE_LOG(LogTemp, Log, TEXT("%s: Terrain system is null"), *FString(__FUNCTION__)); \
        return RetVal; \
    }

// Chunk index validation
#define TERRAI_VALIDATE_CHUNK_INDEX(ChunkIndex, MaxChunks) \
    if (ChunkIndex < 0 || ChunkIndex >= MaxChunks) \
    { \
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid chunk index %d (max: %d)"), *FString(__FUNCTION__), ChunkIndex, MaxChunks); \
        return; \
    }

#define TERRAI_VALIDATE_CHUNK_INDEX_RET(ChunkIndex, MaxChunks, RetVal) \
    if (ChunkIndex < 0 || ChunkIndex >= MaxChunks) \
    { \
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid chunk index %d (max: %d)"), *FString(__FUNCTION__), ChunkIndex, MaxChunks); \
        return RetVal; \
    }

// Simulation data validation
#define TERRAI_VALIDATE_SIMULATION_DATA(SimDataPtr) \
    if (!SimDataPtr || !SimDataPtr->IsValid()) \
    { \
        UE_LOG(LogTemp, VeryVerbose, TEXT("%s: Simulation data not valid"), *FString(__FUNCTION__)); \
        return; \
    }

// Master controller validation
#define TERRAI_VALIDATE_MASTER_CONTROLLER(MasterPtr) \
    if (!MasterPtr) \
    { \
        UE_LOG(LogTemp, VeryVerbose, TEXT("%s: Master controller not available"), *FString(__FUNCTION__)); \
        return; \
    }

// ===== TERRAI NAMED CONSTANTS =====
// Replace magic numbers with meaningful constants

namespace TerrAIConstants
{
    // Performance and timing
    static constexpr float STATS_UPDATE_INTERVAL = 0.25f;           // 4 times per second
    static constexpr float MODIFICATION_COOLDOWN = 0.05f;           // 20 modifications per second max
    static constexpr float CURSOR_UPDATE_RATE = 0.033f;             // 30fps cursor updates
    static constexpr float FRUSTUM_CULLING_UPDATE_RATE = 0.1f;       // 10Hz visibility updates
    static constexpr float WATER_SHADER_UPDATE_INTERVAL = 0.1f;      // Water shader updates
    
    // LOD and distance thresholds
    static constexpr float LOD_DISTANCE_THRESHOLD_1 = 1000.0f;       // First LOD transition
    static constexpr float LOD_DISTANCE_THRESHOLD_2 = 2500.0f;       // Second LOD transition
    static constexpr float LOD_DISTANCE_THRESHOLD_3 = 4000.0f;       // Third LOD transition
    static constexpr float MAX_NIAGARA_DISTANCE = 3000.0f;          // Niagara effect culling
    static constexpr float VOLUME_UPDATE_DISTANCE = 2000.0f;        // Volumetric water updates
    
    // Chunk system
    static constexpr int32 MAX_CHUNK_UPDATES_PER_FRAME = 5;         // Standard chunk updates
    static constexpr int32 MAX_WATER_UPDATES_PER_FRAME = 25;        // Water-only updates
    static constexpr int32 EMERGENCY_CHUNK_UPDATES = 50;            // High pressure mode
    static constexpr int32 CHUNK_OVERLAP = 1;                       // Vertex overlap for seamless chunks
    
    // Physics and simulation
    static constexpr float MIN_WATER_DEPTH = 0.01f;                 // Minimum water depth
    static constexpr float MIN_EROSION_VELOCITY = 15.0f;            // Minimum velocity for erosion
    static constexpr float MIN_FLOW_SPEED_FOR_FX = 5.0f;           // Minimum flow for Niagara FX
    static constexpr float WATER_DEPTH_SCALE = 1.0f;              // Water depth scaling factor CHANGED FROM 25.5
    
    // Camera and input
    static constexpr float FIRST_PERSON_HEIGHT = 152.4f;           // 5 feet in cm
    static constexpr float MIN_CAMERA_HEIGHT = -100000.0f;             // Minimum camera height
    static constexpr float MAX_CAMERA_HEIGHT = 100000.0f;           // Maximum camera height
    static constexpr float CAMERA_TRANSITION_SPEED = 3.0f;         // Camera transition speed
    
    // Atmospheric system
    static constexpr int32 ATMOSPHERIC_GRID_SIZE = 64;             // 64x64x8 atmospheric grid
    static constexpr int32 ATMOSPHERIC_LAYERS = 8;                 // Vertical atmospheric layers
    static constexpr float BASE_FOG_DENSITY = 0.02f;              // Base atmospheric fog
    static constexpr float CLOUD_ALTITUDE = 3500.0f;              // Default cloud altitude
}

