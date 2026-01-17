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

    // PHASE 1.5: Wave energy system for pressure-driven waves
    this.previousDepth = new Float32Array(GRID_SIZE);      // For displacement detection
    this.previousTerrain = new Float32Array(GRID_SIZE);    // For terrain change detection
    this.waveEnergy = new Float32Array(GRID_SIZE);         // Wave energy at each cell
    this.waveDirectionX = new Float32Array(GRID_SIZE);     // Wave propagation direction
    this.waveDirectionY = new Float32Array(GRID_SIZE);
    this.waveEnergyNext = new Float32Array(GRID_SIZE);     // Double buffer for wave energy

    // Springs: array of {x, y, flowRate}
    this.springs = [];

    // Simulation parameters
    this.flowRate = WATER_FLOW_RATE;
    this.evaporationRate = EVAPORATION_RATE;
    this.minDepth = WATER_MIN_DEPTH;

    // Wave parameters
    this.waveDecay = 0.96;          // How fast waves dissipate
    this.wavePropagation = 0.35;    // How much energy spreads to neighbors
    this.displacementSensitivity = 15.0;  // How much terrain changes create waves
    this.impactWaveStrength = 2.0;  // Strength of wall impact waves
    this.pressureWaveStrength = 0.8; // How much pressure gradients create waves

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
  // Amount is total water to add, distributed across the brush area
  addWater(centerX, centerY, amount = 10, radius = 3) {
    // Scale amount by area so larger brushes spread water thinner
    // (like a wider spray pattern adding the same total volume)
    const area = Math.PI * radius * radius;
    const perCellAmount = amount / Math.max(1, area * 0.5);  // Distribute across ~half the area (falloff)

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (!this.inBounds(x, y)) continue;

        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist > radius) continue;

        const falloff = 1 - (dist / radius);
        const idx = this.index(x, y);
        this.depth[idx] += perCellAmount * falloff;
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
    // Reset wave energy system
    this.previousDepth.fill(0);
    this.previousTerrain.set(this.terrain.heightMap);
    this.waveEnergy.fill(0);
    this.waveEnergyNext.fill(0);
    this.waveDirectionX.fill(0);
    this.waveDirectionY.fill(0);
    this.dirty = true;
  }

  // Main simulation step - pressure-based flow with wave energy
  simulate(dt = 1/60) {
    const terrain = this.terrain.heightMap;

    // Copy current depth to next buffer
    this.depthNext.set(this.depth);
    this.waveEnergyNext.set(this.waveEnergy);

    // Process springs first - add water and create ripples
    for (const spring of this.springs) {
      if (this.inBounds(spring.x, spring.y)) {
        const idx = this.index(spring.x, spring.y);
        this.depthNext[idx] += spring.flowRate * dt;

        // Springs create continuous ripples emanating outward
        const springEnergy = spring.flowRate * 0.5 * dt;
        this.waveEnergyNext[idx] += springEnergy;

        // Also add energy to neighbors for visible ripple spread
        for (const [dx, dy] of [[-1,0], [1,0], [0,-1], [0,1]]) {
          const nx = spring.x + dx;
          const ny = spring.y + dy;
          if (this.inBounds(nx, ny)) {
            const nIdx = this.index(nx, ny);
            this.waveEnergyNext[nIdx] += springEnergy * 0.3;
            // Set outward wave direction
            this.waveDirectionX[nIdx] += dx * springEnergy;
            this.waveDirectionY[nIdx] += dy * springEnergy;
          }
        }
      }
    }

    // PHASE 1.5: Detect terrain displacement and generate waves
    for (let y = 1; y < this.height - 1; y++) {
      for (let x = 1; x < this.width - 1; x++) {
        const idx = this.index(x, y);
        const waterHere = this.depth[idx];

        if (waterHere > this.minDepth) {
          // Detect terrain change (raising/lowering under water)
          const terrainChange = terrain[idx] - this.previousTerrain[idx];
          if (Math.abs(terrainChange) > 0.1) {
            // Generate radial waves from displacement
            const displacementEnergy = Math.abs(terrainChange) * this.displacementSensitivity;
            this.waveEnergyNext[idx] += displacementEnergy;

            // Set wave direction to radiate outward from center of change
            // (normalized based on terrain gradient)
            const gradX = (terrain[this.index(x+1, y)] - terrain[this.index(x-1, y)]) * 0.5;
            const gradY = (terrain[this.index(x, y+1)] - terrain[this.index(x, y-1)]) * 0.5;
            const gradMag = Math.sqrt(gradX * gradX + gradY * gradY) + 0.001;

            // Wave direction is perpendicular/outward from terrain change
            this.waveDirectionX[idx] += (terrainChange > 0 ? -gradX : gradX) / gradMag * displacementEnergy;
            this.waveDirectionY[idx] += (terrainChange > 0 ? -gradY : gradY) / gradMag * displacementEnergy;
          }
        }
      }
    }

    // Pressure-based flow simulation with wave generation
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
        let blockedFlow = 0;  // Track flow blocked by walls

        // Calculate surface gradient for pressure-driven waves
        let surfaceGradX = 0;
        let surfaceGradY = 0;

        // Check all 8 neighbors
        for (const [dx, dy, distMult] of NEIGHBORS) {
          const nx = x + dx;
          const ny = y + dy;
          const nIdx = this.index(nx, ny);

          const terrainNeighbor = terrain[nIdx];
          const waterNeighbor = this.depth[nIdx];
          const surfaceNeighbor = terrainNeighbor + waterNeighbor;

          // Accumulate surface gradient
          if (dx !== 0) surfaceGradX += (surfaceNeighbor - surfaceHere) * dx / distMult;
          if (dy !== 0) surfaceGradY += (surfaceNeighbor - surfaceHere) * dy / distMult;

          // Calculate pressure difference (water flows downhill)
          const pressureDiff = surfaceHere - surfaceNeighbor;

          if (pressureDiff > 0) {
            // Check if flow is blocked by terrain wall
            const heightDiff = terrainNeighbor - terrainHere;
            if (heightDiff > waterHere * 0.5) {
              // Wall impact - generate wave energy bouncing back
              blockedFlow += pressureDiff * this.flowRate / distMult;
              continue;
            }

            // Flow rate based on pressure difference
            const maxFlow = waterHere * 0.25;
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

        // Wall impact creates sloshing waves
        if (blockedFlow > 0.01) {
          const impactEnergy = blockedFlow * this.impactWaveStrength;
          this.waveEnergyNext[idx] += impactEnergy;
          // Reverse wave direction on impact
          this.waveDirectionX[idx] -= this.velocityX[idx] * impactEnergy * 0.5;
          this.waveDirectionY[idx] -= this.velocityY[idx] * impactEnergy * 0.5;
        }

        // Pressure gradient creates waves
        const gradMag = Math.sqrt(surfaceGradX * surfaceGradX + surfaceGradY * surfaceGradY);
        if (gradMag > 0.1) {
          const pressureEnergy = gradMag * this.pressureWaveStrength * waterHere * 0.1;
          this.waveEnergyNext[idx] += pressureEnergy;
          // Waves propagate down-gradient (toward lower surface)
          this.waveDirectionX[idx] += surfaceGradX * pressureEnergy;
          this.waveDirectionY[idx] += surfaceGradY * pressureEnergy;
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

    // Propagate wave energy to neighbors
    for (let y = 1; y < this.height - 1; y++) {
      for (let x = 1; x < this.width - 1; x++) {
        const idx = this.index(x, y);
        const energy = this.waveEnergy[idx];

        if (energy > 0.01 && this.depth[idx] > this.minDepth) {
          // Get wave direction
          const dirX = this.waveDirectionX[idx];
          const dirY = this.waveDirectionY[idx];
          const dirMag = Math.sqrt(dirX * dirX + dirY * dirY) + 0.001;
          const normDirX = dirX / dirMag;
          const normDirY = dirY / dirMag;

          // Spread energy to neighbors, biased in wave direction
          for (const [dx, dy, distMult] of NEIGHBORS) {
            const nx = x + dx;
            const ny = y + dy;
            const nIdx = this.index(nx, ny);

            if (this.depth[nIdx] > this.minDepth) {
              // Directional bias: more energy goes in wave direction
              const dot = (dx * normDirX + dy * normDirY) / distMult;
              const dirBias = Math.max(0.1, 0.5 + dot * 0.5);  // 0.1 to 1.0

              const spreadEnergy = energy * this.wavePropagation * dirBias / 8;
              this.waveEnergyNext[nIdx] += spreadEnergy;
              this.waveEnergyNext[idx] -= spreadEnergy;

              // Propagate direction too
              this.waveDirectionX[nIdx] += normDirX * spreadEnergy * 0.5;
              this.waveDirectionY[nIdx] += normDirY * spreadEnergy * 0.5;
            }
          }
        }
      }
    }

    // Apply decay to wave energy and direction
    for (let i = 0; i < GRID_SIZE; i++) {
      this.waveEnergyNext[i] *= this.waveDecay;
      this.waveDirectionX[i] *= 0.92;
      this.waveDirectionY[i] *= 0.92;
    }

    // Apply evaporation (very subtle)
    for (let i = 0; i < GRID_SIZE; i++) {
      if (this.depthNext[i] > 0) {
        this.depthNext[i] = Math.max(0, this.depthNext[i] - this.evaporationRate * dt);
      }
    }

    // Store previous state for next frame's displacement detection
    this.previousDepth.set(this.depth);
    this.previousTerrain.set(terrain);

    // Swap buffers
    let temp = this.depth;
    this.depth = this.depthNext;
    this.depthNext = temp;

    temp = this.waveEnergy;
    this.waveEnergy = this.waveEnergyNext;
    this.waveEnergyNext = temp;

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
