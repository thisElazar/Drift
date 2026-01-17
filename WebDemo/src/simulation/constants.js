// Grid dimensions (1/4 scale of full game's 513x513)
export const GRID_WIDTH = 128;
export const GRID_HEIGHT = 128;
export const GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;

// World scale (how big the terrain appears)
export const WORLD_SCALE = 10; // World units per grid cell
export const TERRAIN_WIDTH = GRID_WIDTH * WORLD_SCALE;
export const TERRAIN_HEIGHT = GRID_HEIGHT * WORLD_SCALE;

// Height range
export const MAX_HEIGHT = 200;
export const MIN_HEIGHT = -50;
export const BASE_HEIGHT = 0;

// Terrain generation
export const NOISE_SCALE = 0.02;
export const NOISE_OCTAVES = 4;
export const NOISE_PERSISTENCE = 0.5;

// Brush settings
export const DEFAULT_BRUSH_RADIUS = 5;
export const DEFAULT_BRUSH_STRENGTH = 2;

// Water simulation
export const WATER_FLOW_RATE = 0.15;
export const WATER_MIN_DEPTH = 0.01;
export const EVAPORATION_RATE = 0.0001; // Very slow evaporation

// Springs
export const SPRING_FLOW_RATE = 8.0; // Strong flow to create visible rivers
export const NUM_AUTO_SPRINGS = 4;
