import {
  GRID_WIDTH,
  GRID_HEIGHT,
  GRID_SIZE,
  WATER_FLOW_RATE,
  WATER_MIN_DEPTH,
  EVAPORATION_RATE,
  SPRING_FLOW_RATE,
  NUM_AUTO_SPRINGS,
} from './constants.js';

// 8-directional neighbors (dx, dy, distance multiplier)
const NEIGHBORS = [
  [-1, 0, 1.0],   // West
  [1, 0, 1.0],    // East
  [0, -1, 1.0],   // North
  [0, 1, 1.0],    // South
  [-1, -1, 1.414], // NW (diagonal)
  [1, -1, 1.414],  // NE
  [-1, 1, 1.414],  // SW
  [1, 1, 1.414],   // SE
];

export class Water {
  constructor(terrain) {
    this.terrain = terrain;
    this.width = GRID_WIDTH;
    this.height = GRID_HEIGHT;

    // Water state arrays
    this.depth = new Float32Array(GRID_SIZE);
    this.depthNext = new Float32Array(GRID_SIZE); // Double buffer
    this.velocityX = new Float32Array(GRID_SIZE);
    this.velocityY = new Float32Array(GRID_SIZE);

    // Springs: array of {x, y, flowRate}
    this.springs = [];

    // Simulation parameters
    this.flowRate = WATER_FLOW_RATE;
    this.evaporationRate = EVAPORATION_RATE;
    this.minDepth = WATER_MIN_DEPTH;

    this.dirty = false;
  }

  index(x, y) {
    return y * this.width + x;
  }

  inBounds(x, y) {
    return x >= 0 && x < this.width && y >= 0 && y < this.height;
  }

  getDepth(x, y) {
    if (!this.inBounds(x, y)) return 0;
    return this.depth[this.index(x, y)];
  }

  // Add water at a position
  addWater(centerX, centerY, amount = 10, radius = 3) {
    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (!this.inBounds(x, y)) continue;

        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist > radius) continue;

        const falloff = 1 - (dist / radius);
        const idx = this.index(x, y);
        this.depth[idx] += amount * falloff;
      }
    }
    this.dirty = true;
  }

  // Add a spring
  addSpring(x, y, flowRate = SPRING_FLOW_RATE) {
    this.springs.push({
      x: Math.floor(x),
      y: Math.floor(y),
      flowRate,
    });
  }

  // Remove springs near a position
  removeSpringNear(x, y, radius = 3) {
    this.springs = this.springs.filter(spring => {
      const dx = spring.x - x;
      const dy = spring.y - y;
      return Math.sqrt(dx * dx + dy * dy) > radius;
    });
  }

  // Clear all springs
  clearSprings() {
    this.springs = [];
  }

  // Reset water (keep springs)
  reset() {
    this.depth.fill(0);
    this.depthNext.fill(0);
    this.velocityX.fill(0);
    this.velocityY.fill(0);
    this.dirty = true;
  }

  // Main simulation step - pressure-based flow
  simulate(dt = 1/60) {
    const terrain = this.terrain.heightMap;

    // Copy current depth to next buffer
    this.depthNext.set(this.depth);

    // Process springs first
    for (const spring of this.springs) {
      if (this.inBounds(spring.x, spring.y)) {
        const idx = this.index(spring.x, spring.y);
        this.depthNext[idx] += spring.flowRate * dt;
      }
    }

    // Pressure-based flow simulation
    for (let y = 1; y < this.height - 1; y++) {
      for (let x = 1; x < this.width - 1; x++) {
        const idx = this.index(x, y);
        const waterHere = this.depth[idx];

        // Skip if no water
        if (waterHere < this.minDepth) continue;

        const terrainHere = terrain[idx];
        const surfaceHere = terrainHere + waterHere;

        let totalFlow = 0;
        let flowVelX = 0;
        let flowVelY = 0;

        // Check all 8 neighbors
        for (const [dx, dy, distMult] of NEIGHBORS) {
          const nx = x + dx;
          const ny = y + dy;
          const nIdx = this.index(nx, ny);

          const terrainNeighbor = terrain[nIdx];
          const waterNeighbor = this.depth[nIdx];
          const surfaceNeighbor = terrainNeighbor + waterNeighbor;

          // Calculate pressure difference (water flows downhill)
          const pressureDiff = surfaceHere - surfaceNeighbor;

          if (pressureDiff > 0) {
            // Flow rate based on pressure difference
            // Limit flow to available water and prevent oscillation
            const maxFlow = waterHere * 0.25; // Don't move more than 25% per step
            const flow = Math.min(
              pressureDiff * this.flowRate / distMult,
              maxFlow
            );

            if (flow > 0.0001) {
              this.depthNext[idx] -= flow;
              this.depthNext[nIdx] += flow;
              totalFlow += flow;

              // Track velocity direction
              flowVelX += dx * flow;
              flowVelY += dy * flow;
            }
          }
        }

        // Update velocity (smoothed)
        if (totalFlow > 0) {
          this.velocityX[idx] = this.velocityX[idx] * 0.8 + flowVelX * 0.2;
          this.velocityY[idx] = this.velocityY[idx] * 0.8 + flowVelY * 0.2;
        } else {
          this.velocityX[idx] *= 0.95;
          this.velocityY[idx] *= 0.95;
        }
      }
    }

    // Apply evaporation (very subtle)
    for (let i = 0; i < GRID_SIZE; i++) {
      if (this.depthNext[i] > 0) {
        this.depthNext[i] = Math.max(0, this.depthNext[i] - this.evaporationRate * dt);
      }
    }

    // Swap buffers
    const temp = this.depth;
    this.depth = this.depthNext;
    this.depthNext = temp;

    this.dirty = true;
  }

  // Get total water in system (for debugging)
  getTotalWater() {
    let total = 0;
    for (let i = 0; i < GRID_SIZE; i++) {
      total += this.depth[i];
    }
    return total;
  }

  // Find local peaks in terrain for spring placement
  findPeaks(count = NUM_AUTO_SPRINGS) {
    const terrain = this.terrain.heightMap;
    const peaks = [];
    const margin = 10; // Stay away from edges

    // Find all local maxima
    for (let y = margin; y < this.height - margin; y++) {
      for (let x = margin; x < this.width - margin; x++) {
        const idx = this.index(x, y);
        const height = terrain[idx];

        // Check if higher than all neighbors
        let isPeak = true;
        for (const [dx, dy] of NEIGHBORS) {
          const nIdx = this.index(x + dx, y + dy);
          if (terrain[nIdx] >= height) {
            isPeak = false;
            break;
          }
        }

        if (isPeak && height > 20) { // Only consider elevated peaks
          peaks.push({ x, y, height });
        }
      }
    }

    // Sort by height (highest first) and take top N
    peaks.sort((a, b) => b.height - a.height);

    // Filter to avoid placing springs too close together
    const selectedPeaks = [];
    const minDist = 15;

    for (const peak of peaks) {
      let tooClose = false;
      for (const selected of selectedPeaks) {
        const dx = peak.x - selected.x;
        const dy = peak.y - selected.y;
        if (Math.sqrt(dx * dx + dy * dy) < minDist) {
          tooClose = true;
          break;
        }
      }
      if (!tooClose) {
        selectedPeaks.push(peak);
        if (selectedPeaks.length >= count) break;
      }
    }

    return selectedPeaks;
  }

  // Auto-generate springs at terrain peaks
  autoGenerateSprings(count = NUM_AUTO_SPRINGS) {
    this.clearSprings();
    const peaks = this.findPeaks(count);

    for (const peak of peaks) {
      this.addSpring(peak.x, peak.y, SPRING_FLOW_RATE);
    }

    console.log(`Generated ${this.springs.length} springs at peaks`);
  }
}
