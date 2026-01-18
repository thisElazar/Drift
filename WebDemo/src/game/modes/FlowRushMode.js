/**
 * FlowRushMode - Guide water to target zones for points
 *
 * Gameplay:
 * - Targets spawn at random locations on the terrain
 * - Guide water to fill targets using terrain tools and springs
 * - Hit targets quickly to build combo multiplier
 * - Targets have a timer - let them expire and lose your combo
 * - Game ends when time runs out
 */

import { BaseGameMode } from './BaseGameMode.js';
import * as THREE from 'three';
import { getWorldScale } from '../../simulation/constants.js';

// Target states
const TargetState = {
  ACTIVE: 'active',
  FILLING: 'filling',
  COMPLETE: 'complete',
  EXPIRED: 'expired',
};

export class FlowRushMode extends BaseGameMode {
  constructor(options) {
    super(options);

    // Mode configuration
    this.gameTime = 120; // 2 minutes per round
    this.maxTargets = 3; // Max simultaneous targets
    this.targetLifetime = 15; // Seconds before target expires
    this.targetRadius = 6; // Grid cells
    this.fillThreshold = 50; // Water volume needed to complete target
    this.spawnDelay = 2; // Seconds between spawns

    // Target tracking
    this.targets = [];
    this.spawnTimer = 0;
    this.completedTargets = 0;

    // Points configuration
    this.basePoints = 100;
    this.timeBonus = 50; // Bonus points per second remaining on target

    // Hearts system (Zelda-style quarter hearts)
    this.maxHearts = 3;
    this.quarterHearts = 12; // 3 hearts * 4 quarters each
    this.maxQuarterHearts = 12;
  }

  init() {
    super.init();

    // Reset state
    this.targets = [];
    this.spawnTimer = 0;
    this.completedTargets = 0;
    this.quarterHearts = this.maxQuarterHearts;

    // Spawn initial targets
    for (let i = 0; i < 2; i++) {
      this.spawnTarget();
    }

    console.log('Flow Rush started! Guide water to the glowing targets.');
  }

  update(dt) {
    super.update(dt);

    // Check game time
    const remainingTime = this.gameTime - this.elapsedTime;
    if (remainingTime <= 0) {
      this.gameOver = true;
      this.victory = true; // Completing the time is a "win"
      return;
    }

    // Update spawn timer
    this.spawnTimer += dt;
    if (this.spawnTimer >= this.spawnDelay && this.targets.length < this.maxTargets) {
      this.spawnTarget();
      this.spawnTimer = 0;
    }

    // Update each target
    for (let i = this.targets.length - 1; i >= 0; i--) {
      const target = this.targets[i];
      this.updateTarget(target, dt);

      // Remove completed or expired targets
      if (target.state === TargetState.COMPLETE || target.state === TargetState.EXPIRED) {
        this.removeTarget(target);
        this.targets.splice(i, 1);
      }
    }
  }

  spawnTarget() {
    // Find a valid position (not too close to existing targets)
    const pos = this.findValidTargetPosition();
    if (!pos) return null;

    // Create target object
    const target = {
      x: pos.x,
      y: pos.y,
      height: pos.height,
      radius: this.targetRadius,
      state: TargetState.ACTIVE,
      lifetime: this.targetLifetime,
      timeRemaining: this.targetLifetime,
      fillAmount: 0,
      fillThreshold: this.fillThreshold,
      mesh: null,
      ring: null,
      pulsePhase: Math.random() * Math.PI * 2,
    };

    // Create visual
    this.createTargetVisual(target);

    this.targets.push(target);
    return target;
  }

  findValidTargetPosition() {
    const margin = 20;
    const minDistance = this.targetRadius * 3;
    const maxAttempts = 30;

    for (let i = 0; i < maxAttempts; i++) {
      const pos = this.zoneDetector.findRandomPosition(margin, -20, 30);

      // Check distance from other targets
      let valid = true;
      for (const target of this.targets) {
        const dx = pos.x - target.x;
        const dy = pos.y - target.y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist < minDistance) {
          valid = false;
          break;
        }
      }

      if (valid) return pos;
    }

    return null;
  }

  // Convert grid coordinates to world coordinates
  gridToWorld(gridX, gridY) {
    const worldScale = getWorldScale();
    return {
      x: (gridX - this.terrain.width / 2) * worldScale,
      z: (gridY - this.terrain.height / 2) * worldScale,
    };
  }

  createTargetVisual(target) {
    const worldScale = getWorldScale();
    const worldRadius = target.radius * worldScale;

    // Create terrain-following ring geometry
    const segments = 48;
    const ringWidthCells = 1.5; // Ring thickness in grid cells

    // Create custom geometry that follows terrain
    const ringGeometry = this.createTerrainFollowingRing(
      target.x, target.y, target.radius, ringWidthCells, segments
    );

    const ringMaterial = new THREE.MeshBasicMaterial({
      color: 0xffcc00, // Gold color
      transparent: true,
      opacity: 0.9,
      side: THREE.DoubleSide,
    });
    const ring = new THREE.Mesh(ringGeometry, ringMaterial);

    // Create inner glow/fill circle (also terrain-following)
    const fillGeometry = this.createTerrainFollowingDisc(
      target.x, target.y, target.radius * 0.9, segments
    );
    const fillMaterial = new THREE.MeshBasicMaterial({
      color: 0xffee88,
      transparent: true,
      opacity: 0.25,
      side: THREE.DoubleSide,
    });
    const fill = new THREE.Mesh(fillGeometry, fillMaterial);

    // Create animated beacon pillars at cardinal points
    const beaconGroup = new THREE.Group();
    const beaconHeight = 8;
    const beaconGeom = new THREE.CylinderGeometry(0.5, 1.0, beaconHeight, 8);
    const beaconMat = new THREE.MeshBasicMaterial({
      color: 0xffdd00,
      transparent: true,
      opacity: 0.8,
    });

    for (let i = 0; i < 4; i++) {
      const angle = (i / 4) * Math.PI * 2;
      const bx = target.x + Math.cos(angle) * target.radius;
      const by = target.y + Math.sin(angle) * target.radius;
      const bHeight = this.zoneDetector.getTerrainHeight(bx, by);

      const worldPos = this.gridToWorld(bx, by);
      const worldY = bHeight + beaconHeight / 2;

      const beacon = new THREE.Mesh(beaconGeom, beaconMat.clone());
      beacon.position.set(worldPos.x, worldY, worldPos.z);
      beaconGroup.add(beacon);
    }

    // Store references
    target.ring = ring;
    target.fill = fill;
    target.beacons = beaconGroup;
    target.material = ringMaterial;
    target.fillMaterial = fillMaterial;
    target.beaconMaterial = beaconMat;
    target.worldScale = worldScale;

    // Add to scene
    this.addVisualElement(ring);
    this.addVisualElement(fill);
    this.addVisualElement(beaconGroup);
  }

  createTerrainFollowingRing(centerX, centerY, radiusCells, ringWidthCells, segments) {
    const worldScale = getWorldScale();
    const innerRadiusCells = radiusCells - ringWidthCells / 2;
    const outerRadiusCells = radiusCells + ringWidthCells / 2;

    const geometry = new THREE.BufferGeometry();
    const vertices = [];
    const indices = [];

    for (let i = 0; i <= segments; i++) {
      const angle = (i / segments) * Math.PI * 2;
      const cos = Math.cos(angle);
      const sin = Math.sin(angle);

      // Sample terrain at inner and outer edge
      const innerGridX = centerX + cos * innerRadiusCells;
      const innerGridY = centerY + sin * innerRadiusCells;
      const outerGridX = centerX + cos * outerRadiusCells;
      const outerGridY = centerY + sin * outerRadiusCells;

      const innerHeight = this.zoneDetector.getTerrainHeight(innerGridX, innerGridY);
      const outerHeight = this.zoneDetector.getTerrainHeight(outerGridX, outerGridY);

      // Convert to world coords
      const innerWorld = this.gridToWorld(innerGridX, innerGridY);
      const outerWorld = this.gridToWorld(outerGridX, outerGridY);

      vertices.push(innerWorld.x, innerHeight + 0.5, innerWorld.z);
      vertices.push(outerWorld.x, outerHeight + 0.5, outerWorld.z);

      if (i < segments) {
        const base = i * 2;
        indices.push(base, base + 1, base + 2);
        indices.push(base + 1, base + 3, base + 2);
      }
    }

    geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    geometry.setIndex(indices);
    geometry.computeVertexNormals();

    return geometry;
  }

  updateTargetGeometry(target) {
    const segments = 48;
    const ringWidthCells = 1.5;

    // Update ring geometry vertices
    if (target.ring && target.ring.geometry) {
      const positions = target.ring.geometry.attributes.position.array;
      const innerRadiusCells = target.radius - ringWidthCells / 2;
      const outerRadiusCells = target.radius + ringWidthCells / 2;

      for (let i = 0; i <= segments; i++) {
        const angle = (i / segments) * Math.PI * 2;
        const cos = Math.cos(angle);
        const sin = Math.sin(angle);

        const innerGridX = target.x + cos * innerRadiusCells;
        const innerGridY = target.y + sin * innerRadiusCells;
        const outerGridX = target.x + cos * outerRadiusCells;
        const outerGridY = target.y + sin * outerRadiusCells;

        const innerHeight = this.zoneDetector.getTerrainHeight(innerGridX, innerGridY);
        const outerHeight = this.zoneDetector.getTerrainHeight(outerGridX, outerGridY);

        const innerWorld = this.gridToWorld(innerGridX, innerGridY);
        const outerWorld = this.gridToWorld(outerGridX, outerGridY);

        const baseIdx = i * 6; // 2 vertices * 3 components
        positions[baseIdx + 1] = innerHeight + 0.5;
        positions[baseIdx + 4] = outerHeight + 0.5;
      }
      target.ring.geometry.attributes.position.needsUpdate = true;
    }

    // Update fill geometry vertices
    if (target.fill && target.fill.geometry) {
      const positions = target.fill.geometry.attributes.position.array;
      const fillRadius = target.radius * 0.9;

      // Center vertex
      const centerHeight = this.zoneDetector.getTerrainHeight(target.x, target.y);
      positions[1] = centerHeight + 0.3;

      // Edge vertices
      for (let i = 0; i <= segments; i++) {
        const angle = (i / segments) * Math.PI * 2;
        const gridX = target.x + Math.cos(angle) * fillRadius;
        const gridY = target.y + Math.sin(angle) * fillRadius;
        const height = this.zoneDetector.getTerrainHeight(gridX, gridY);

        const idx = (i + 1) * 3; // +1 to skip center vertex
        positions[idx + 1] = height + 0.3;
      }
      target.fill.geometry.attributes.position.needsUpdate = true;
    }

    // Update beacon positions
    if (target.beacons) {
      target.beacons.children.forEach((beacon, i) => {
        const angle = (i / 4) * Math.PI * 2;
        const bx = target.x + Math.cos(angle) * target.radius;
        const by = target.y + Math.sin(angle) * target.radius;
        const bHeight = this.zoneDetector.getTerrainHeight(bx, by);
        const worldPos = this.gridToWorld(bx, by);
        beacon.position.x = worldPos.x;
        beacon.position.z = worldPos.z;
        // Y position is handled in the animation section
      });
    }
  }

  createTerrainFollowingDisc(centerX, centerY, radiusCells, segments) {
    const geometry = new THREE.BufferGeometry();
    const vertices = [];
    const indices = [];

    // Center vertex
    const centerHeight = this.zoneDetector.getTerrainHeight(centerX, centerY);
    const centerWorld = this.gridToWorld(centerX, centerY);
    vertices.push(centerWorld.x, centerHeight + 0.3, centerWorld.z);

    // Edge vertices
    for (let i = 0; i <= segments; i++) {
      const angle = (i / segments) * Math.PI * 2;
      const gridX = centerX + Math.cos(angle) * radiusCells;
      const gridY = centerY + Math.sin(angle) * radiusCells;
      const height = this.zoneDetector.getTerrainHeight(gridX, gridY);

      const world = this.gridToWorld(gridX, gridY);
      vertices.push(world.x, height + 0.3, world.z);

      if (i < segments) {
        indices.push(0, i + 1, i + 2);
      }
    }

    geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    geometry.setIndex(indices);
    geometry.computeVertexNormals();

    return geometry;
  }

  updateTarget(target, dt) {
    // Update timer
    target.timeRemaining -= dt;

    // Check for water in target zone
    const waterVolume = this.zoneDetector.getWaterInCircle(
      target.x,
      target.y,
      target.radius
    );

    // Update fill amount (water contributes to fill)
    if (waterVolume > 0) {
      target.fillAmount += waterVolume * dt * 0.5;
      target.state = TargetState.FILLING;
    }

    // Check completion
    if (target.fillAmount >= target.fillThreshold) {
      this.onTargetComplete(target);
      return;
    }

    // Check expiration
    if (target.timeRemaining <= 0) {
      this.onTargetExpired(target);
      return;
    }

    // Update visuals
    this.updateTargetVisual(target, dt);
  }

  updateTargetVisual(target, dt) {
    if (!target.ring || !target.material) return;

    // Update geometry to follow terrain changes
    this.updateTargetGeometry(target);

    // Pulse animation
    target.pulsePhase += dt * 4;
    const pulse = 0.5 + 0.5 * Math.sin(target.pulsePhase);

    // Color based on state and time remaining
    const timeRatio = target.timeRemaining / target.lifetime;
    const fillRatio = target.fillAmount / target.fillThreshold;

    if (timeRatio < 0.3) {
      // Urgent - flash red/orange
      const urgent = Math.sin(target.pulsePhase * 4) > 0;
      target.material.color.setHex(urgent ? 0xff3333 : 0xff6600);
      // Also flash beacons
      if (target.beacons) {
        target.beacons.children.forEach(beacon => {
          beacon.material.color.setHex(urgent ? 0xff3333 : 0xff6600);
        });
      }
    } else if (target.state === TargetState.FILLING) {
      // Filling - bright cyan/white
      target.material.color.setHex(0x66ffff);
      if (target.beacons) {
        target.beacons.children.forEach(beacon => {
          beacon.material.color.setHex(0x66ffff);
        });
      }
    } else {
      // Normal - gold
      target.material.color.setHex(0xffcc00);
      if (target.beacons) {
        target.beacons.children.forEach(beacon => {
          beacon.material.color.setHex(0xffdd00);
        });
      }
    }

    target.material.opacity = 0.6 + pulse * 0.4;

    // Update fill indicator
    if (target.fill) {
      target.fillMaterial.opacity = 0.15 + fillRatio * 0.4;
    }

    // Animate beacons - bob up and down
    if (target.beacons) {
      const bobOffset = Math.sin(target.pulsePhase * 1.5) * 2;
      target.beacons.children.forEach((beacon, i) => {
        const angle = (i / 4) * Math.PI * 2;
        const bx = target.x + Math.cos(angle) * target.radius;
        const by = target.y + Math.sin(angle) * target.radius;
        const bHeight = this.zoneDetector.getTerrainHeight(bx, by);
        const baseY = bHeight + 4; // Base height above terrain
        beacon.position.y = baseY + bobOffset + Math.sin(target.pulsePhase + i * 1.5) * 1;
        beacon.material.opacity = 0.5 + pulse * 0.5;
      });
    }
  }

  onTargetComplete(target) {
    target.state = TargetState.COMPLETE;

    // Calculate points
    const timeBonus = Math.floor(target.timeRemaining * this.timeBonus);
    const points = this.basePoints + timeBonus;

    // Award points (uses combo multiplier)
    this.addPoints(points);

    // Increase combo
    this.increaseCombo();

    this.completedTargets++;

    console.log(`Target complete! +${points} points (${timeBonus} time bonus)`);
  }

  onTargetExpired(target) {
    target.state = TargetState.EXPIRED;

    // Lose a quarter heart
    this.quarterHearts--;

    // Reset combo on missed target
    this.scoreManager.resetCombo();

    // Check for game over (no hearts left)
    if (this.quarterHearts <= 0) {
      this.gameOver = true;
      this.victory = false;
    }

    console.log(`Target expired - lost a quarter heart! (${this.quarterHearts}/${this.maxQuarterHearts} remaining)`);
  }

  removeTarget(target) {
    if (target.ring) {
      this.removeVisualElement(target.ring);
      target.ring.geometry.dispose();
      target.material.dispose();
    }
    if (target.fill) {
      this.removeVisualElement(target.fill);
      target.fill.geometry.dispose();
      target.fillMaterial.dispose();
    }
    if (target.beacons) {
      this.removeVisualElement(target.beacons);
      target.beacons.children.forEach(beacon => {
        beacon.geometry.dispose();
        beacon.material.dispose();
      });
    }
  }

  cleanup() {
    // Remove all target visuals
    for (const target of this.targets) {
      this.removeTarget(target);
    }
    this.targets = [];

    super.cleanup();
  }

  getDisplayInfo() {
    const info = super.getDisplayInfo();
    info.timeRemaining = Math.max(0, this.gameTime - this.elapsedTime);
    info.targetsCompleted = this.completedTargets;
    info.activeTargets = this.targets.length;
    info.quarterHearts = this.quarterHearts;
    info.maxQuarterHearts = this.maxQuarterHearts;
    return info;
  }

  getStats() {
    const stats = super.getStats();
    stats.targetsCompleted = this.completedTargets;
    stats.timeRemaining = Math.max(0, this.gameTime - this.elapsedTime);
    return stats;
  }
}
