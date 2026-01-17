import * as THREE from 'three';
import {
  GRID_WIDTH,
  GRID_HEIGHT,
  WORLD_SCALE,
} from '../simulation/constants.js';

const MIN_RENDER_DEPTH = 0.3; // Minimum depth to render

export class WaterMesh {
  constructor(water, terrain) {
    this.water = water;
    this.terrain = terrain;

    // Create geometry (same resolution as terrain)
    this.geometry = new THREE.PlaneGeometry(
      GRID_WIDTH * WORLD_SCALE,
      GRID_HEIGHT * WORLD_SCALE,
      GRID_WIDTH - 1,
      GRID_HEIGHT - 1
    );

    // Rotate to horizontal
    this.geometry.rotateX(-Math.PI / 2);

    // Add vertex colors (RGBA) for per-vertex transparency
    const vertexCount = GRID_WIDTH * GRID_HEIGHT;
    const colors = new Float32Array(vertexCount * 4);
    this.geometry.setAttribute('color', new THREE.BufferAttribute(colors, 4));

    // Water material with vertex colors for alpha
    this.material = new THREE.MeshStandardMaterial({
      color: 0x2090ff,
      transparent: true,
      opacity: 1.0,  // Base opacity, modulated by vertex alpha
      roughness: 0.05,
      metalness: 0.3,
      side: THREE.FrontSide,
      vertexColors: true,
      depthWrite: false,  // Prevent z-fighting with terrain
    });

    // Override the material's onBeforeCompile to use vertex alpha
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

  updateGeometry() {
    const positions = this.geometry.attributes.position.array;
    const colors = this.geometry.attributes.color.array;
    const terrainHeights = this.terrain.heightMap;
    const waterDepths = this.water.depth;

    let hasWater = false;

    for (let y = 0; y < GRID_HEIGHT; y++) {
      for (let x = 0; x < GRID_WIDTH; x++) {
        const gridIdx = y * GRID_WIDTH + x;
        const vertIdx = gridIdx * 3;
        const colorIdx = gridIdx * 4;

        const depth = waterDepths[gridIdx];
        const terrainHeight = terrainHeights[gridIdx];

        if (depth > MIN_RENDER_DEPTH) {
          // Water surface = terrain + water depth
          positions[vertIdx + 1] = terrainHeight + depth;

          // Color: light blue with depth-based alpha
          const alpha = Math.min(1.0, depth / 5.0); // Fade in over depth
          colors[colorIdx] = 0.4;     // R
          colors[colorIdx + 1] = 0.6; // G
          colors[colorIdx + 2] = 1.0; // B
          colors[colorIdx + 3] = alpha * 0.8; // A

          hasWater = true;
        } else {
          // Push dry vertices below and make fully transparent
          positions[vertIdx + 1] = terrainHeight - 10;
          colors[colorIdx] = 0;
          colors[colorIdx + 1] = 0;
          colors[colorIdx + 2] = 0;
          colors[colorIdx + 3] = 0; // Fully transparent
        }
      }
    }

    this.geometry.attributes.position.needsUpdate = true;
    this.geometry.attributes.color.needsUpdate = true;
    this.geometry.computeVertexNormals();

    this.mesh.visible = hasWater;
    this.water.dirty = false;
  }

  update() {
    if (this.water.dirty || this.terrain.dirty) {
      this.updateGeometry();
    }
  }

  get object() {
    return this.mesh;
  }
}
