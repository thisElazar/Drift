import { QualityConfig } from './quality.js';

// Grid dimensions - now from quality config
export const getGridWidth = () => QualityConfig.gridWidth;
export const getGridHeight = () => QualityConfig.gridHeight;
export const getGridSize = () => QualityConfig.gridSize;
export const getWorldScale = () => QualityConfig.worldScale;
export const getMaxSimSteps = () => QualityConfig.maxSimSteps;

// Legacy exports for compatibility (use current quality settings)
export const GRID_WIDTH = 256;  // Default, but terrain.js will use getGridWidth()
export const GRID_HEIGHT = 256;
export const GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;
export const WORLD_SCALE = 8;

// Computed world size
export const getTerrainWidth = () => getGridWidth() * getWorldScale();
export const getTerrainHeight = () => getGridHeight() * getWorldScale();

// Height range
export const MAX_HEIGHT = 350;
export const MIN_HEIGHT = -100;
export const BASE_HEIGHT = 0;

// Terrain generation
export const NOISE_SCALE = 0.015;
export const NOISE_OCTAVES = 5;
export const NOISE_PERSISTENCE = 0.5;

// Brush settings
export const DEFAULT_BRUSH_RADIUS = 8;
export const DEFAULT_BRUSH_STRENGTH = 2;

// Water simulation
export const WATER_FLOW_RATE = 0.15;
export const WATER_MIN_DEPTH = 0.01;
export const EVAPORATION_RATE = 0.00005;

// Springs
export const SPRING_FLOW_RATE = 10.0;
export const NUM_AUTO_SPRINGS = 6;
