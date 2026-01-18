import { fbm } from './noise.js';
import {
  getGridWidth,
  getGridHeight,
  getGridSize,
  MAX_HEIGHT,
  MIN_HEIGHT,
  NOISE_SCALE,
  NOISE_OCTAVES,
  NOISE_PERSISTENCE,
  DEFAULT_BRUSH_RADIUS,
  DEFAULT_BRUSH_STRENGTH,
} from './constants.js';

// Terrain generation presets
export const TerrainPreset = {
  DEFAULT: 'default',
  HILLS: 'hills',
  MOUNTAINS: 'mountains',
  PLAINS: 'plains',
};

export class Terrain {
  constructor() {
    this.width = getGridWidth();
    this.height = getGridHeight();
    this.heightMap = new Float32Array(getGridSize());
    this.dirty = false; // Flag for mesh updates
    this.currentPreset = TerrainPreset.DEFAULT;

    this.brushRadius = DEFAULT_BRUSH_RADIUS;
    this.brushStrength = DEFAULT_BRUSH_STRENGTH;

    this.generate();
  }

  // Get array index from grid coordinates
  index(x, y) {
    return y * this.width + x;
  }

  // Check if coordinates are within bounds
  inBounds(x, y) {
    return x >= 0 && x < this.width && y >= 0 && y < this.height;
  }

  // Get height at grid position
  getHeight(x, y) {
    if (!this.inBounds(x, y)) return 0;
    return this.heightMap[this.index(x, y)];
  }

  // Set height at grid position
  setHeight(x, y, value) {
    if (!this.inBounds(x, y)) return;
    this.heightMap[this.index(x, y)] = Math.max(MIN_HEIGHT, Math.min(MAX_HEIGHT, value));
    this.dirty = true;
  }

  // Generate terrain using fractal Brownian motion
  generate(preset = null, seed = Math.random() * 1000) {
    if (preset) this.currentPreset = preset;

    // Preset parameters
    let noiseScale, octaves, persistence, heightMult, falloffPower, baseHeight;

    switch (this.currentPreset) {
      case TerrainPreset.HILLS:
        noiseScale = 0.012;
        octaves = 4;
        persistence = 0.4;
        heightMult = 0.4;
        falloffPower = 1.5;
        baseHeight = 20;
        break;
      case TerrainPreset.MOUNTAINS:
        noiseScale = 0.02;
        octaves = 6;
        persistence = 0.55;
        heightMult = 1.0;
        falloffPower = 3;
        baseHeight = -50;
        break;
      case TerrainPreset.PLAINS:
        noiseScale = 0.008;
        octaves = 3;
        persistence = 0.3;
        heightMult = 0.15;
        falloffPower = 1.2;
        baseHeight = 30;
        break;
      default: // DEFAULT
        noiseScale = NOISE_SCALE;
        octaves = NOISE_OCTAVES;
        persistence = NOISE_PERSISTENCE;
        heightMult = 0.7;
        falloffPower = 2;
        baseHeight = 0;
    }

    for (let y = 0; y < this.height; y++) {
      for (let x = 0; x < this.width; x++) {
        // Get noise value (-1 to 1 range)
        const nx = (x + seed) * noiseScale;
        const ny = (y + seed) * noiseScale;
        let noiseVal = fbm(nx, ny, octaves, persistence);

        // Add some variation with a second layer
        noiseVal += 0.3 * fbm(nx * 2 + 100, ny * 2 + 100, 2, 0.5);

        // Edge falloff to create an island effect
        const dx = (x / this.width) * 2 - 1;
        const dy = (y / this.height) * 2 - 1;
        const distFromCenter = Math.sqrt(dx * dx + dy * dy);
        const falloff = 1 - Math.pow(distFromCenter, falloffPower);

        // Scale to height range
        const height = baseHeight + noiseVal * Math.max(0, falloff) * MAX_HEIGHT * heightMult;

        this.heightMap[this.index(x, y)] = Math.max(MIN_HEIGHT, Math.min(MAX_HEIGHT, height));
      }
    }
    this.dirty = true;
  }

  // Apply Gaussian brush at grid position
  applyBrush(centerX, centerY, strength = null) {
    const radius = this.brushRadius;
    const str = strength ?? this.brushStrength;

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (!this.inBounds(x, y)) continue;

        // Gaussian falloff
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist > radius) continue;

        const falloff = Math.exp(-(dist * dist) / (radius * radius / 2));
        const delta = str * falloff;

        const idx = this.index(x, y);
        this.heightMap[idx] = Math.max(
          MIN_HEIGHT,
          Math.min(MAX_HEIGHT, this.heightMap[idx] + delta)
        );
      }
    }
    this.dirty = true;
  }

  // Raise terrain at position
  raise(x, y) {
    this.applyBrush(x, y, Math.abs(this.brushStrength));
  }

  // Lower terrain at position
  lower(x, y) {
    this.applyBrush(x, y, -Math.abs(this.brushStrength));
  }

  // Reset terrain with new random seed
  reset() {
    this.generate();
  }

  // Get interpolated height at world position (for smoother sampling)
  getHeightInterpolated(x, y) {
    const x0 = Math.floor(x);
    const y0 = Math.floor(y);
    const x1 = x0 + 1;
    const y1 = y0 + 1;

    const fx = x - x0;
    const fy = y - y0;

    const h00 = this.getHeight(x0, y0);
    const h10 = this.getHeight(x1, y0);
    const h01 = this.getHeight(x0, y1);
    const h11 = this.getHeight(x1, y1);

    // Bilinear interpolation
    const h0 = h00 * (1 - fx) + h10 * fx;
    const h1 = h01 * (1 - fx) + h11 * fx;

    return h0 * (1 - fy) + h1 * fy;
  }
}
