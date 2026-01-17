// Grid dimensions (doubled from 128 for more detail)
export const GRID_WIDTH = 256;
export const GRID_HEIGHT = 256;
export const GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;

// World scale (how big the terrain appears)
export const WORLD_SCALE = 8; // Slightly smaller per cell to keep similar total size
export const TERRAIN_WIDTH = GRID_WIDTH * WORLD_SCALE;
export const TERRAIN_HEIGHT = GRID_HEIGHT * WORLD_SCALE;

// Height range
export const MAX_HEIGHT = 350;
export const MIN_HEIGHT = -100;
export const BASE_HEIGHT = 0;

// Terrain generation
export const NOISE_SCALE = 0.015;  // Slightly smaller for larger terrain
export const NOISE_OCTAVES = 5;    // Extra octave for more detail
export const NOISE_PERSISTENCE = 0.5;

// Brush settings
export const DEFAULT_BRUSH_RADIUS = 8;   // Larger brush for bigger terrain
export const DEFAULT_BRUSH_STRENGTH = 2;

// Water simulation
export const WATER_FLOW_RATE = 0.15;
export const WATER_MIN_DEPTH = 0.01;
export const EVAPORATION_RATE = 0.00005; // Even slower for larger area

// Springs
export const SPRING_FLOW_RATE = 10.0; // Slightly stronger for larger area
export const NUM_AUTO_SPRINGS = 6;    // More springs for bigger terrain
