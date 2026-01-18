import * as THREE from 'three';
import {
  getWorldScale,
  MAX_HEIGHT,
  MIN_HEIGHT,
} from '../simulation/constants.js';

export class TerrainMesh {
  constructor(terrain) {
    this.terrain = terrain;
    const worldScale = getWorldScale();

    // Create geometry using terrain's dimensions
    this.geometry = new THREE.PlaneGeometry(
      terrain.width * worldScale,
      terrain.height * worldScale,
      terrain.width - 1,
      terrain.height - 1
    );

    // Rotate to be horizontal (XZ plane)
    this.geometry.rotateX(-Math.PI / 2);

    // Create material with vertex colors
    this.material = new THREE.MeshStandardMaterial({
      vertexColors: true,
      flatShading: false,
      roughness: 0.8,
      metalness: 0.1,
    });

    // Create mesh
    this.mesh = new THREE.Mesh(this.geometry, this.material);
    this.mesh.receiveShadow = true;

    // Initial update
    this.updateGeometry();
  }

  updateGeometry() {
    const positions = this.geometry.attributes.position.array;
    const colors = this.geometry.attributes.color?.array ||
      new Float32Array(positions.length);
    const gridWidth = this.terrain.width;
    const gridHeight = this.terrain.height;

    // Update vertex positions and colors
    for (let y = 0; y < gridHeight; y++) {
      for (let x = 0; x < gridWidth; x++) {
        const gridIdx = y * gridWidth + x;
        const vertIdx = gridIdx * 3;

        // Get height from terrain
        const h = this.terrain.heightMap[gridIdx];

        // Update Y position (which is up after rotation)
        positions[vertIdx + 1] = h;

        // Calculate color based on height
        const color = this.getHeightColor(h);
        colors[vertIdx] = color.r;
        colors[vertIdx + 1] = color.g;
        colors[vertIdx + 2] = color.b;
      }
    }

    // Set/update color attribute
    if (!this.geometry.attributes.color) {
      this.geometry.setAttribute('color', new THREE.BufferAttribute(colors, 3));
    }

    // Mark for update
    this.geometry.attributes.position.needsUpdate = true;
    this.geometry.attributes.color.needsUpdate = true;
    this.geometry.computeVertexNormals();

    this.terrain.dirty = false;
  }

  getHeightColor(height) {
    // Normalize height to 0-1 range
    const t = (height - MIN_HEIGHT) / (MAX_HEIGHT - MIN_HEIGHT);

    // Color stops for terrain
    if (t < 0.3) {
      // Deep water to shallow water (not used for terrain, but handles low points)
      return this.lerpColor({ r: 0.1, g: 0.2, b: 0.4 }, { r: 0.2, g: 0.4, b: 0.3 }, t / 0.3);
    } else if (t < 0.35) {
      // Beach/sand
      return this.lerpColor({ r: 0.76, g: 0.7, b: 0.5 }, { r: 0.6, g: 0.55, b: 0.4 }, (t - 0.3) / 0.05);
    } else if (t < 0.6) {
      // Grass
      return this.lerpColor({ r: 0.2, g: 0.5, b: 0.2 }, { r: 0.3, g: 0.45, b: 0.25 }, (t - 0.35) / 0.25);
    } else if (t < 0.75) {
      // Forest/darker green
      return this.lerpColor({ r: 0.15, g: 0.35, b: 0.15 }, { r: 0.3, g: 0.3, b: 0.25 }, (t - 0.6) / 0.15);
    } else if (t < 0.9) {
      // Rock/mountain
      return this.lerpColor({ r: 0.4, g: 0.38, b: 0.35 }, { r: 0.55, g: 0.53, b: 0.5 }, (t - 0.75) / 0.15);
    } else {
      // Snow caps
      return this.lerpColor({ r: 0.7, g: 0.7, b: 0.72 }, { r: 0.95, g: 0.95, b: 0.97 }, (t - 0.9) / 0.1);
    }
  }

  lerpColor(a, b, t) {
    t = Math.max(0, Math.min(1, t));
    return {
      r: a.r + (b.r - a.r) * t,
      g: a.g + (b.g - a.g) * t,
      b: a.b + (b.b - a.b) * t,
    };
  }

  update() {
    if (this.terrain.dirty) {
      this.updateGeometry();
    }
  }

  // Get the Three.js object to add to scene
  get object() {
    return this.mesh;
  }
}
