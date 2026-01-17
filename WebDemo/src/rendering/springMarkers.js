import * as THREE from 'three';
import { WORLD_SCALE } from '../simulation/constants.js';

export class SpringMarkers {
  constructor(water, terrain) {
    this.water = water;
    this.terrain = terrain;

    // Container for all spring markers
    this.group = new THREE.Group();

    // Simple subtle ring to mark spring location
    this.ringGeometry = new THREE.RingGeometry(3, 5, 16);
    this.ringMaterial = new THREE.MeshBasicMaterial({
      color: 0x44aacc,
      transparent: true,
      opacity: 0.4,
      side: THREE.DoubleSide,
    });

    this.markerPool = [];
    this.lastSpringCount = 0;
  }

  update(dt = 1/60) {
    const springs = this.water.springs;

    // Rebuild if spring count changed
    if (springs.length !== this.lastSpringCount) {
      this.rebuild();
      this.lastSpringCount = springs.length;
    }

    // Update marker positions
    for (let i = 0; i < springs.length; i++) {
      const spring = springs[i];
      const marker = this.markerPool[i];
      if (!marker) continue;

      const terrainHeight = this.terrain.getHeight(spring.x, spring.y);
      const waterDepth = this.water.getDepth(spring.x, spring.y);
      const worldX = (spring.x - this.terrain.width / 2) * WORLD_SCALE;
      const worldZ = (spring.y - this.terrain.height / 2) * WORLD_SCALE;

      // Position just above water surface (or terrain if no water yet)
      const surfaceY = terrainHeight + Math.max(0.5, waterDepth) + 0.2;
      marker.position.set(worldX, surfaceY, worldZ);
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
        marker = new THREE.Mesh(this.ringGeometry, this.ringMaterial);
        marker.rotation.x = -Math.PI / 2;  // Lay flat
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
