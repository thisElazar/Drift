import * as THREE from 'three';
import {
  GRID_WIDTH,
  GRID_HEIGHT,
  WORLD_SCALE,
} from '../simulation/constants.js';

const MIN_RENDER_DEPTH = 0.3;

// Ambient wave parameters (wind-driven background waves)
const WAVE_SPEED = 0.8;            // Slower, more natural
const WAVE_SCALE = 0.06;           // Slightly larger wavelength
const AMBIENT_WAVE_HEIGHT = 1.5;   // Subtle background

// Pressure/displacement wave parameters
const PRESSURE_WAVE_HEIGHT = 4.0;  // Visible but not overwhelming
const WAVE_ENERGY_SCALE = 0.5;     // Gentler response to energy
const PRESSURE_WAVE_SPEED = 1.5;   // Slower pressure wave animation

export class WaterMesh {
  constructor(water, terrain) {
    this.water = water;
    this.terrain = terrain;
    this.time = 0;

    // Create geometry
    this.geometry = new THREE.PlaneGeometry(
      GRID_WIDTH * WORLD_SCALE,
      GRID_HEIGHT * WORLD_SCALE,
      GRID_WIDTH - 1,
      GRID_HEIGHT - 1
    );

    this.geometry.rotateX(-Math.PI / 2);

    // Store base positions for wave animation
    const vertexCount = GRID_WIDTH * GRID_HEIGHT;
    this.baseHeights = new Float32Array(vertexCount);

    // Smoothed wave heights to prevent flickering
    this.smoothedWaveHeights = new Float32Array(vertexCount);
    this.waveSmoothing = 0.15;  // Blend factor (lower = smoother)

    // Add vertex colors (RGBA)
    const colors = new Float32Array(vertexCount * 4);
    this.geometry.setAttribute('color', new THREE.BufferAttribute(colors, 4));

    // Water material - shiny to catch wave highlights
    this.material = new THREE.MeshStandardMaterial({
      color: 0x3399ff,
      transparent: true,
      opacity: 1.0,
      roughness: 0.15,  // Slightly rough for softer highlights
      metalness: 0.6,   // More reflective
      side: THREE.FrontSide,
      vertexColors: true,
      depthWrite: false,
      envMapIntensity: 1.5,
    });

    this.material.onBeforeCompile = (shader) => {
      shader.fragmentShader = shader.fragmentShader.replace(
        '#include <color_fragment>',
        `#include <color_fragment>
         diffuseColor.a *= vColor.a;`
      );
    };

    this.mesh = new THREE.Mesh(this.geometry, this.material);
    this.mesh.renderOrder = 1;

    this.updateGeometry();
  }

  // Check if a cell is at the shoreline (has dry neighbors)
  isShorelineCell(x, y, waterDepths) {
    const depth = waterDepths[y * GRID_WIDTH + x];
    if (depth <= MIN_RENDER_DEPTH) return false;

    // Check 4-connected neighbors
    const neighbors = [[-1, 0], [1, 0], [0, -1], [0, 1]];
    for (const [dx, dy] of neighbors) {
      const nx = x + dx;
      const ny = y + dy;
      if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) continue;
      const nDepth = waterDepths[ny * GRID_WIDTH + nx];
      if (nDepth <= MIN_RENDER_DEPTH) return true;
    }
    return false;
  }

  // Get flow speed at a cell
  getFlowSpeed(x, y) {
    const idx = y * GRID_WIDTH + x;
    const vx = this.water.velocityX[idx];
    const vy = this.water.velocityY[idx];
    return Math.sqrt(vx * vx + vy * vy);
  }

  // Get wave energy at a cell
  getWaveEnergy(x, y) {
    const idx = y * GRID_WIDTH + x;
    return this.water.waveEnergy[idx];
  }

  updateGeometry() {
    const positions = this.geometry.attributes.position.array;
    const colors = this.geometry.attributes.color.array;
    const terrainHeights = this.terrain.heightMap;
    const waterDepths = this.water.depth;

    let hasWater = false;

    // Water color palette (depth-based)
    // Shallow: turquoise/cyan
    const shallowColor = { r: 0.3, g: 0.75, b: 0.8 };
    // Mid: ocean blue
    const midColor = { r: 0.15, g: 0.45, b: 0.75 };
    // Deep: dark blue
    const deepColor = { r: 0.05, g: 0.15, b: 0.4 };
    // Foam: white with slight blue tint
    const foamColor = { r: 0.9, g: 0.95, b: 1.0 };

    for (let y = 0; y < GRID_HEIGHT; y++) {
      for (let x = 0; x < GRID_WIDTH; x++) {
        const gridIdx = y * GRID_WIDTH + x;
        const vertIdx = gridIdx * 3;
        const colorIdx = gridIdx * 4;

        const depth = waterDepths[gridIdx];
        const terrainHeight = terrainHeights[gridIdx];

        if (depth > MIN_RENDER_DEPTH) {
          // Store base height for wave animation
          this.baseHeights[gridIdx] = terrainHeight + depth;

          // === DEPTH-BASED COLOR ===
          // Smooth transitions between shallow -> mid -> deep
          const shallowDepth = 2.0;   // Full shallow color
          const midDepth = 6.0;       // Transition to mid
          const deepDepth = 15.0;     // Full deep color

          let baseR, baseG, baseB;
          if (depth < shallowDepth) {
            // Very shallow - pure turquoise
            baseR = shallowColor.r;
            baseG = shallowColor.g;
            baseB = shallowColor.b;
          } else if (depth < midDepth) {
            // Shallow to mid transition
            const t = (depth - shallowDepth) / (midDepth - shallowDepth);
            baseR = shallowColor.r + (midColor.r - shallowColor.r) * t;
            baseG = shallowColor.g + (midColor.g - shallowColor.g) * t;
            baseB = shallowColor.b + (midColor.b - shallowColor.b) * t;
          } else if (depth < deepDepth) {
            // Mid to deep transition
            const t = (depth - midDepth) / (deepDepth - midDepth);
            baseR = midColor.r + (deepColor.r - midColor.r) * t;
            baseG = midColor.g + (deepColor.g - midColor.g) * t;
            baseB = midColor.b + (deepColor.b - midColor.b) * t;
          } else {
            // Very deep
            baseR = deepColor.r;
            baseG = deepColor.g;
            baseB = deepColor.b;
          }

          // === FOAM CALCULATION ===
          const isShore = this.isShorelineCell(x, y, waterDepths);
          const flowSpeed = this.getFlowSpeed(x, y);
          const waveEnergy = this.getWaveEnergy(x, y);

          // Foam from multiple sources
          let foamAmount = 0;

          // Shoreline foam (strongest)
          if (isShore) {
            foamAmount += 0.5;
          }

          // Flow foam (rapids/rivers)
          foamAmount += Math.min(0.4, flowSpeed * 0.3);

          // Wave energy foam (splashing/turbulence)
          foamAmount += Math.min(0.3, waveEnergy * 0.15);

          // Shallow water gets a bit more foam visibility
          if (depth < shallowDepth) {
            foamAmount += 0.1 * (1 - depth / shallowDepth);
          }

          // Clamp total foam
          foamAmount = Math.min(0.8, foamAmount);

          // Blend base color with foam
          const r = baseR + (foamColor.r - baseR) * foamAmount;
          const g = baseG + (foamColor.g - baseG) * foamAmount;
          const b = baseB + (foamColor.b - baseB) * foamAmount;

          // Alpha: more opaque in deep water, slightly transparent in shallow
          const depthAlpha = Math.min(1.0, 0.5 + depth / 10.0);
          const a = Math.min(0.9, depthAlpha * 0.85 + foamAmount * 0.1);

          colors[colorIdx] = r;
          colors[colorIdx + 1] = g;
          colors[colorIdx + 2] = b;
          colors[colorIdx + 3] = a;

          hasWater = true;
        } else {
          this.baseHeights[gridIdx] = terrainHeight - 10;
          colors[colorIdx] = 0;
          colors[colorIdx + 1] = 0;
          colors[colorIdx + 2] = 0;
          colors[colorIdx + 3] = 0;
        }
      }
    }

    this.geometry.attributes.color.needsUpdate = true;
    this.mesh.visible = hasWater;
    this.water.dirty = false;

    // Apply initial wave positions
    this.updateWaveAnimation(0);
  }

  updateWaveAnimation(dt) {
    this.time += dt;
    const positions = this.geometry.attributes.position.array;
    const waterDepths = this.water.depth;
    const waveEnergy = this.water.waveEnergy;
    const waveDirectionX = this.water.waveDirectionX;
    const waveDirectionY = this.water.waveDirectionY;

    for (let y = 0; y < GRID_HEIGHT; y++) {
      for (let x = 0; x < GRID_WIDTH; x++) {
        const gridIdx = y * GRID_WIDTH + x;
        const vertIdx = gridIdx * 3;
        const depth = waterDepths[gridIdx];

        if (depth > MIN_RENDER_DEPTH) {
          const worldX = x * WORLD_SCALE;
          const worldZ = y * WORLD_SCALE;

          // === AMBIENT WAVES (wind-driven background) ===
          // Primary wave - large slow swells
          const wave1 = Math.sin(worldX * WAVE_SCALE + this.time * WAVE_SPEED) *
                       Math.cos(worldZ * WAVE_SCALE * 0.7 + this.time * WAVE_SPEED * 0.6);

          // Secondary wave - gentle crossing pattern
          const wave2 = Math.sin((worldX - worldZ) * WAVE_SCALE * 1.2 + this.time * WAVE_SPEED * 0.9) * 0.4;

          const depthFactor = Math.min(1.0, depth / 3.0);
          const ambientWave = (wave1 + wave2) * AMBIENT_WAVE_HEIGHT * depthFactor;

          // === PRESSURE-DRIVEN WAVES (from simulation) ===
          const energy = waveEnergy[gridIdx];
          let pressureWave = 0;

          if (energy > 0.05) {  // Higher threshold to reduce noise
            const dirX = waveDirectionX[gridIdx];
            const dirY = waveDirectionY[gridIdx];
            const dirMag = Math.sqrt(dirX * dirX + dirY * dirY) + 0.001;
            const normDirX = dirX / dirMag;
            const normDirY = dirY / dirMag;

            // Smooth directional wave - travels in wave direction
            const travelPhase = (worldX * normDirX + worldZ * normDirY) * 0.08;
            const directionalWave = Math.sin(travelPhase + this.time * PRESSURE_WAVE_SPEED);

            // Gentle radial pulse
            const radialWave = Math.sin(this.time * PRESSURE_WAVE_SPEED * 0.8) * 0.3;

            // Smooth energy response (square root for gentler curve)
            const energyFactor = Math.min(1.0, Math.sqrt(energy * WAVE_ENERGY_SCALE));
            pressureWave = (directionalWave * 0.8 + radialWave * 0.2) * PRESSURE_WAVE_HEIGHT * energyFactor * depthFactor;
          }

          // === FLOW-BASED RIPPLES (subtle) ===
          const vx = this.water.velocityX[gridIdx];
          const vy = this.water.velocityY[gridIdx];
          const flowSpeed = Math.sqrt(vx * vx + vy * vy);
          let flowWave = 0;
          if (flowSpeed > 0.2) {  // Higher threshold
            const flowDir = Math.atan2(vy, vx);
            const flowPhase = (worldX * Math.cos(flowDir) + worldZ * Math.sin(flowDir)) * 0.15;
            flowWave = Math.sin(flowPhase + this.time * 2.0) * Math.min(1.0, flowSpeed * 0.3) * depthFactor;
          }

          // Combine all wave components
          const targetWave = ambientWave + pressureWave + flowWave;

          // Temporal smoothing to prevent flickering
          const prevWave = this.smoothedWaveHeights[gridIdx];
          const smoothedWave = prevWave + (targetWave - prevWave) * this.waveSmoothing;
          this.smoothedWaveHeights[gridIdx] = smoothedWave;

          positions[vertIdx + 1] = this.baseHeights[gridIdx] + smoothedWave;
        } else {
          this.smoothedWaveHeights[gridIdx] = 0;
          positions[vertIdx + 1] = this.baseHeights[gridIdx];
        }
      }
    }

    this.geometry.attributes.position.needsUpdate = true;
    this.geometry.computeVertexNormals();
  }

  update(dt = 1/60) {
    if (this.water.dirty || this.terrain.dirty) {
      this.updateGeometry();
    } else {
      // Just animate waves
      this.updateWaveAnimation(dt);
    }
  }

  get object() {
    return this.mesh;
  }
}
