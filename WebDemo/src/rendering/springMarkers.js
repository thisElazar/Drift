import * as THREE from 'three';
import { WORLD_SCALE } from '../simulation/constants.js';

export class SpringMarkers {
  constructor(water, terrain) {
    this.water = water;
    this.terrain = terrain;

    // Container for all spring markers
    this.group = new THREE.Group();

    // Shared geometry and material for markers
    this.geometry = new THREE.ConeGeometry(3, 8, 8);
    this.geometry.rotateX(Math.PI); // Point down
    this.material = new THREE.MeshStandardMaterial({
      color: 0x00ffff,
      emissive: 0x004444,
    });

    this.markerPool = [];
    this.lastSpringCount = 0;
  }

  update() {
    const springs = this.water.springs;

    // Only rebuild if spring count changed
    if (springs.length !== this.lastSpringCount) {
      this.rebuild();
      this.lastSpringCount = springs.length;
    }

    // Update positions (springs stay in place, but terrain might change)
    for (let i = 0; i < springs.length; i++) {
      const spring = springs[i];
      const marker = this.markerPool[i];
      if (marker) {
        const terrainHeight = this.terrain.getHeight(spring.x, spring.y);
        const worldX = (spring.x - this.terrain.width / 2) * WORLD_SCALE;
        const worldZ = (spring.y - this.terrain.height / 2) * WORLD_SCALE;
        marker.position.set(worldX, terrainHeight + 15, worldZ);
      }
    }
  }

  rebuild() {
    const springs = this.water.springs;

    // Hide excess markers
    for (let i = springs.length; i < this.markerPool.length; i++) {
      this.markerPool[i].visible = false;
    }

    // Create/show needed markers
    for (let i = 0; i < springs.length; i++) {
      let marker = this.markerPool[i];

      if (!marker) {
        marker = new THREE.Mesh(this.geometry, this.material);
        this.markerPool.push(marker);
        this.group.add(marker);
      }

      marker.visible = true;
    }
  }

  get object() {
    return this.group;
  }
}
