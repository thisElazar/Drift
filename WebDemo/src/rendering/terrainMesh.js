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

    // Color stops for terrain (saturated for contrast)
    if (t < 0.1) {
      // Dark bedrock (neutral gray)
      return this.lerpColor({ r: 0.12, g: 0.11, b: 0.10 }, { r: 0.22, g: 0.21, b: 0.19 }, t / 0.1);
    } else if (t < 0.15) {
      // Gray to dark desaturated green (blue pulled out first)
      return this.lerpColor({ r: 0.22, g: 0.21, b: 0.19 }, { r: 0.12, g: 0.22, b: 0.12 }, (t - 0.1) / 0.05);
    } else if (t < 0.3) {
      // Dark green to teal-green (blue introduced gradually)
      return this.lerpColor({ r: 0.12, g: 0.22, b: 0.12 }, { r: 0.15, g: 0.35, b: 0.25 }, (t - 0.15) / 0.15);
    } else if (t < 0.35) {
      // Beach/sand - warm golden
      return this.lerpColor({ r: 0.82, g: 0.72, b: 0.45 }, { r: 0.65, g: 0.55, b: 0.35 }, (t - 0.3) / 0.05);
    } else if (t < 0.6) {
      // Grass - vibrant green
      return this.lerpColor({ r: 0.18, g: 0.55, b: 0.15 }, { r: 0.25, g: 0.48, b: 0.18 }, (t - 0.35) / 0.25);
    } else if (t < 0.75) {
      // Forest/darker green - rich deep green
      return this.lerpColor({ r: 0.1, g: 0.38, b: 0.1 }, { r: 0.22, g: 0.32, b: 0.18 }, (t - 0.6) / 0.15);
    } else if (t < 0.9) {
      // Rock/mountain - cooler gray with slight contrast
      return this.lerpColor({ r: 0.35, g: 0.34, b: 0.32 }, { r: 0.52, g: 0.50, b: 0.48 }, (t - 0.75) / 0.15);
    } else {
      // Snow caps - bright white
      return this.lerpColor({ r: 0.85, g: 0.85, b: 0.9 }, { r: 0.98, g: 0.98, b: 1.0 }, (t - 0.9) / 0.1);
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

  /**
   * Rebuild geometry when grid size changes
   */
  rebuild() {
    const worldScale = getWorldScale();

    // Dispose old geometry
    this.geometry.dispose();

    // Create new geometry with new dimensions
    this.geometry = new THREE.PlaneGeometry(
      this.terrain.width * worldScale,
      this.terrain.height * worldScale,
      this.terrain.width - 1,
      this.terrain.height - 1
    );

    // Rotate to be horizontal (XZ plane)
    this.geometry.rotateX(-Math.PI / 2);

    // Update mesh geometry reference
    this.mesh.geometry = this.geometry;

    // Update geometry with terrain data
    this.updateGeometry();
  }

  // Get the Three.js object to add to scene
  get object() {
    return this.mesh;
  }
}
