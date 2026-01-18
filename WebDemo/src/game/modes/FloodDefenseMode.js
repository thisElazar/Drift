/**
 * FloodDefenseMode - Protect villages from flooding springs
 *
 * Gameplay:
 * - Aggressive springs spawn on high ground, flooding downhill
 * - Villages spawn in low areas every 15 seconds
 * - Villages flood and are destroyed after 5 seconds underwater
 * - Player builds barriers and redirects water to save villages
 * - Earn points for each village saved, lose hearts when villages flood
 */

import { BaseGameMode } from './BaseGameMode.js';
import * as THREE from 'three';
import { getWorldScale } from '../../simulation/constants.js';

export class FloodDefenseMode extends BaseGameMode {
  constructor(options) {
    super(options);

    // Mode configuration
    this.gameTime = 120; // 2 minutes to survive
    this.villageRadius = 5; // Detection radius in grid cells
    this.villageVisualRadius = 3; // Visual size

    // Spring configuration
    this.numStartingSprings = 3; // Initial springs
    this.springFlowRate = 800; // Extremely aggressive flow rate
    this.springs = [];

    // Village spawning
    this.numStartingVillages = 5; // Start with 5 villages
    this.villagesPerSpawn = 2; // Add 2 villages each spawn
    this.villageSpawnInterval = 15; // Every 15 seconds
    this.villageSpawnTimer = 15; // First spawn after 15 seconds (we start with 5)
    this.maxVillages = 20; // Higher cap for scaling difficulty

    // Flood detection
    this.floodThreshold = 0.5; // Water depth to count as flooded
    this.floodDamageInterval = 3; // Lose quarter heart every 3 seconds flooded
    this.floodDestroyTime = 12; // After 12 seconds, village is destroyed and respawns

    // Villages
    this.villages = [];
    this.villagesSaved = 0;
    this.villagesLost = 0;

    // Points configuration
    this.villageSavePoints = 200; // Points when village survives long enough
    this.villageSurviveTime = 30; // Seconds a village must survive to count as "saved"
    this.survivalPoints = 5; // Points per second

    // Hearts system
    this.maxHearts = 3;
    this.quarterHearts = 12;
    this.maxQuarterHearts = 12;
  }

  init() {
    super.init();

    // Reset state
    this.villages = [];
    this.springs = [];
    this.villageSpawnTimer = this.villageSpawnInterval;
    this.villagesSaved = 0;
    this.villagesLost = 0;
    this.quarterHearts = this.maxQuarterHearts;

    // Generate initial aggressive springs on high ground
    this.generateSprings(this.numStartingSprings);

    // Spawn starting villages
    for (let i = 0; i < this.numStartingVillages; i++) {
      this.spawnVillage();
    }

    console.log(`Flood Defense started! Protect ${this.villages.length} villages from the floods!`);
  }

  generateSprings(count) {
    // Springs always spawn at the highest available points
    for (let i = 0; i < count; i++) {
      this.addSpring();
    }

    console.log(`Generated ${count} springs at highest points (${this.springs.length} total)`);
  }

  addSpring() {
    try {
      const pos = this.findHighestSpringPosition();
      if (!pos) {
        console.warn('Could not find position for spring');
        return null;
      }

      // Store grid positions (floored)
      const gridX = Math.floor(pos.x);
      const gridY = Math.floor(pos.y);

      // Add spring to water system
      this.water.addSpring(gridX, gridY, this.springFlowRate);

      const spring = {
        x: gridX,
        y: gridY,
        height: pos.height,
      };

      this.springs.push(spring);
      return spring;
    } catch (e) {
      console.warn('Error adding spring:', e);
      return null;
    }
  }

  findHighestSpringPosition() {
    const margin = 20;
    const minDistanceFromOthers = 25;
    const step = 8; // Larger step = fewer samples = safer

    // Safety check
    if (!this.terrain || !this.terrain.width || !this.terrain.height) {
      return null;
    }

    // Take a snapshot of heights to avoid race conditions with terrain editing
    const heightMap = [];
    const width = this.terrain.width;
    const height = this.terrain.height;

    try {
      for (let y = margin; y < height - margin; y += step) {
        for (let x = margin; x < width - margin; x += step) {
          const h = this.zoneDetector.getTerrainHeight(x, y);
          if (typeof h === 'number' && !isNaN(h)) {
            heightMap.push({ x, y, height: h });
          }
        }
      }
    } catch (e) {
      console.warn('Error sampling terrain for springs:', e);
      return null;
    }

    if (heightMap.length === 0) {
      return null;
    }

    // Sort by height descending
    heightMap.sort((a, b) => b.height - a.height);

    // Find valid position from highest points
    for (const candidate of heightMap) {
      // Check distance from existing springs
      let tooClose = false;
      for (const spring of this.springs) {
        const dist = Math.sqrt((candidate.x - spring.x) ** 2 + (candidate.y - spring.y) ** 2);
        if (dist < minDistanceFromOthers) {
          tooClose = true;
          break;
        }
      }

      if (!tooClose) {
        return candidate;
      }
    }

    // Fallback: just return the highest point even if close to another spring
    return heightMap[0];
  }

  getTerrainHeightPercentile(percentile) {
    const samples = [];
    const step = 8;

    for (let y = 0; y < this.terrain.height; y += step) {
      for (let x = 0; x < this.terrain.width; x += step) {
        samples.push(this.zoneDetector.getTerrainHeight(x, y));
      }
    }

    samples.sort((a, b) => a - b);
    const index = Math.floor(samples.length * percentile);
    return samples[Math.min(index, samples.length - 1)];
  }

  spawnVillage() {
    if (this.villages.length >= this.maxVillages) return;

    // Find LOW ground for village (bottom 40%)
    const maxHeight = this.getTerrainHeightPercentile(0.4);
    const pos = this.findVillagePosition(maxHeight);

    if (!pos) {
      console.warn('Could not find position for new village');
      return;
    }

    const village = {
      index: this.villagesSaved + this.villagesLost + this.villages.length,
      x: pos.x,
      y: pos.y,
      height: pos.height,
      radius: this.villageRadius,
      isFlooded: false,
      floodTime: 0,
      lastDamageTick: 0, // Track when we last dealt flood damage
      aliveTime: 0, // How long this village has survived
      mesh: null,
      buildings: [],
    };

    this.createVillageVisual(village);
    this.villages.push(village);

    console.log(`New village spawned! (${this.villages.length} active)`);
  }

  findVillagePosition(maxHeight) {
    const margin = 20;
    const minDistanceFromVillages = 25;
    const minDistanceFromSprings = 35;
    let bestPos = null;
    let bestScore = -Infinity;

    for (let attempt = 0; attempt < 50; attempt++) {
      const x = margin + Math.random() * (this.terrain.width - margin * 2);
      const y = margin + Math.random() * (this.terrain.height - margin * 2);
      const height = this.zoneDetector.getTerrainHeight(x, y);

      // Must be on LOW ground
      if (height > maxHeight) continue;

      // Check current water level - don't spawn in already flooded areas
      const waterDepth = this.water.getDepth(Math.floor(x), Math.floor(y));
      if (waterDepth > 0.3) continue;

      // Check distance from other villages
      let tooClose = false;
      for (const other of this.villages) {
        const dist = Math.sqrt((x - other.x) ** 2 + (y - other.y) ** 2);
        if (dist < minDistanceFromVillages) {
          tooClose = true;
          break;
        }
      }
      if (tooClose) continue;

      // Check distance from springs (not TOO close)
      for (const spring of this.springs) {
        const dist = Math.sqrt((x - spring.x) ** 2 + (y - spring.y) ** 2);
        if (dist < minDistanceFromSprings) {
          tooClose = true;
          break;
        }
      }
      if (tooClose) continue;

      // Score: prefer areas not yet flooded, some distance from water
      let score = 100 - waterDepth * 50;
      score -= height * 0.5; // Slightly prefer lower ground (more dramatic)

      if (score > bestScore) {
        bestScore = score;
        bestPos = { x, y, height };
      }
    }

    return bestPos;
  }

  gridToWorld(gridX, gridY) {
    const worldScale = getWorldScale();
    return {
      x: (gridX - this.terrain.width / 2) * worldScale,
      z: (gridY - this.terrain.height / 2) * worldScale,
    };
  }

  createVillageVisual(village) {
    const worldScale = getWorldScale();
    const worldPos = this.gridToWorld(village.x, village.y);
    const terrainHeight = this.zoneDetector.getTerrainHeight(village.x, village.y);

    // Create simple buildings (boxes)
    const buildings = [];
    const buildingPositions = [
      { x: 0, z: 0, size: 1.2, height: 2.5 }, // Center building
      { x: 1.5, z: 0.8, size: 0.8, height: 1.8 },
      { x: -1, z: 1.2, size: 0.9, height: 2 },
      { x: 0.5, z: -1.2, size: 0.7, height: 1.5 },
    ];

    for (const bp of buildingPositions) {
      const buildingGeometry = new THREE.BoxGeometry(bp.size, bp.height, bp.size);
      const buildingMaterial = new THREE.MeshBasicMaterial({
        color: 0xDDCC99, // Tan/beige
        transparent: true,
        opacity: 0.9,
      });
      const building = new THREE.Mesh(buildingGeometry, buildingMaterial);
      building.position.set(
        worldPos.x + bp.x * worldScale * 0.4,
        terrainHeight + bp.height / 2,
        worldPos.z + bp.z * worldScale * 0.4
      );
      buildings.push(building);
      this.addVisualElement(building);
    }

    // Create protection zone indicator (ring) - raised high for visibility
    const ringRadius = this.villageRadius * worldScale;
    const beaconHeight = 12; // Height for ring and beacons
    const ringGeometry = new THREE.TorusGeometry(ringRadius, 0.4, 8, 32);
    const ringMaterial = new THREE.MeshBasicMaterial({
      color: 0x44ff88, // Green = safe
      transparent: true,
      opacity: 0.7,
    });
    const ring = new THREE.Mesh(ringGeometry, ringMaterial);
    ring.rotation.x = Math.PI / 2; // Lay flat
    ring.position.set(worldPos.x, terrainHeight + beaconHeight, worldPos.z);

    // Create beacon pillars at cardinal points for visibility in valleys
    const beaconGroup = new THREE.Group();
    const beaconGeom = new THREE.CylinderGeometry(0.4, 0.8, beaconHeight, 8);
    const beaconMat = new THREE.MeshBasicMaterial({
      color: 0x44ff88,
      transparent: true,
      opacity: 0.8,
    });

    for (let i = 0; i < 4; i++) {
      const angle = (i / 4) * Math.PI * 2;
      const bx = village.x + Math.cos(angle) * village.radius;
      const by = village.y + Math.sin(angle) * village.radius;
      const bHeight = this.zoneDetector.getTerrainHeight(bx, by);

      const beaconWorldPos = this.gridToWorld(bx, by);
      const beacon = new THREE.Mesh(beaconGeom, beaconMat.clone());
      beacon.position.set(beaconWorldPos.x, bHeight + beaconHeight / 2, beaconWorldPos.z);
      beaconGroup.add(beacon);
    }

    // Store references
    village.buildings = buildings;
    village.ring = ring;
    village.ringMaterial = ringMaterial;
    village.buildingMaterials = buildings.map(b => b.material);
    village.beacons = beaconGroup;
    village.beaconGeom = beaconGeom;
    village.beaconMat = beaconMat;
    village.beaconHeight = beaconHeight;
    village.worldPos = worldPos;
    village.worldScale = worldScale;

    // Add to scene
    this.addVisualElement(ring);
    this.addVisualElement(beaconGroup);
  }

  update(dt) {
    // Don't update if game already over
    if (this.gameOver) return;

    super.update(dt);

    // Check game time
    const remainingTime = this.gameTime - this.elapsedTime;
    if (remainingTime <= 0) {
      this.onGameEnd();
      return;
    }

    // Check for game over (no hearts)
    if (this.quarterHearts <= 0) {
      return;
    }

    // Survival points (every second)
    if (Math.floor(this.elapsedTime) > Math.floor(this.elapsedTime - dt)) {
      this.scoreManager.addRawPoints(this.survivalPoints);
      this.manager.reportScoreUpdate(this.scoreManager.score, this.scoreManager.combo);
    }

    // Village + spring spawning - pressure increases over time
    this.villageSpawnTimer -= dt;
    if (this.villageSpawnTimer <= 0) {
      // Spawn multiple villages
      for (let i = 0; i < this.villagesPerSpawn; i++) {
        this.spawnVillage();
      }
      // Add a new spring to increase flooding pressure
      this.addSpring();
      console.log(`Wave! Added ${this.villagesPerSpawn} villages and 1 spring. Total springs: ${this.springs.length}`);
      this.villageSpawnTimer = this.villageSpawnInterval;
    }

    // Update villages (flooding, survival, etc.)
    this.updateVillages(dt);
  }

  updateVillages(dt) {
    if (this.gameOver) return;

    const villagesToRemove = [];

    for (const village of this.villages) {
      if (this.gameOver) break; // Stop if game ended mid-loop

      // Track survival time
      village.aliveTime += dt;

      // Check if village has survived long enough to be "saved"
      if (!village.saved && village.aliveTime >= this.villageSurviveTime) {
        village.saved = true;
        this.villagesSaved++;
        this.addPoints(this.villageSavePoints);
        this.increaseCombo();
        console.log(`Village saved! +${this.villageSavePoints} points`);
      }

      // Check water level at village
      const waterDepth = this.getAverageWaterDepth(village.x, village.y, village.radius);

      if (waterDepth > this.floodThreshold) {
        village.isFlooded = true;
        village.floodTime += dt;

        // Deal damage every 3 seconds of flooding (at 3, 6, 9, 12 seconds)
        const damageTicks = Math.floor(village.floodTime / this.floodDamageInterval);
        if (damageTicks > village.lastDamageTick) {
          // New damage tick reached
          const ticksToApply = damageTicks - village.lastDamageTick;
          village.lastDamageTick = damageTicks;

          for (let i = 0; i < ticksToApply; i++) {
            this.onFloodDamage(village);
            if (this.gameOver) break;
          }
        }

        // After 12 seconds, village is destroyed and respawns
        if (!this.gameOver && village.floodTime >= this.floodDestroyTime) {
          villagesToRemove.push(village);
          this.onVillageDestroyed(village);
        }
      } else {
        village.isFlooded = false;
        // Slowly recover if water recedes - reset damage ticks too
        village.floodTime = Math.max(0, village.floodTime - dt * 0.5);
        village.lastDamageTick = Math.floor(village.floodTime / this.floodDamageInterval);
      }

      // Update visuals only if game still running
      if (!this.gameOver) {
        this.updateVillageVisual(village, dt);
      }
    }

    // Don't process removals/respawns if game ended
    if (this.gameOver) return;

    // Remove destroyed villages
    for (const village of villagesToRemove) {
      this.removeVillage(village);
      const idx = this.villages.indexOf(village);
      if (idx !== -1) {
        this.villages.splice(idx, 1);
      }
    }

    // Respawn destroyed villages (they relocate)
    for (const village of villagesToRemove) {
      this.spawnVillage();
    }
  }

  getAverageWaterDepth(centerX, centerY, radius) {
    let totalDepth = 0;
    let samples = 0;

    for (let dy = -radius; dy <= radius; dy += 2) {
      for (let dx = -radius; dx <= radius; dx += 2) {
        if (dx * dx + dy * dy > radius * radius) continue;

        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (x >= 0 && x < this.terrain.width && y >= 0 && y < this.terrain.height) {
          totalDepth += this.water.getDepth(x, y);
          samples++;
        }
      }
    }

    return samples > 0 ? totalDepth / samples : 0;
  }

  onFloodDamage(village) {
    // Don't process if game already over or no hearts left
    if (this.gameOver || this.quarterHearts <= 0) return;

    // Lose 1 quarter heart for every 3 seconds flooded
    this.quarterHearts--;

    // Clamp to 0 and trigger game over immediately if negative
    if (this.quarterHearts <= 0) {
      this.quarterHearts = 0;
      this.onGameEnd();
      return;
    }

    try {
      this.scoreManager.resetCombo();
    } catch (e) {
      // Ignore combo reset errors
    }

    const tickNumber = village ? village.lastDamageTick : 0;
    console.log(`Village flooding! (${tickNumber * 3}s) Lost 1 quarter heart. (${this.quarterHearts}/${this.maxQuarterHearts})`);
  }

  onVillageDestroyed(village) {
    if (this.gameOver) return;

    // Village has been flooded for 12 seconds - it's destroyed and will respawn
    this.villagesLost++;
    console.log(`Village destroyed after 12s flooding! Relocating survivors... (Lost: ${this.villagesLost})`);
  }

  onGameEnd() {
    if (this.gameOver) return; // Prevent double-calling

    // Mark game over FIRST to stop all other processing
    this.gameOver = true;
    this.victory = this.quarterHearts > 0;

    try {
      // Award points for all surviving villages (whether or not they reached 30s)
      const villagesCopy = [...this.villages]; // Copy to avoid iteration issues
      for (const village of villagesCopy) {
        if (village && !village.saved) {
          this.villagesSaved++;
          // Partial points based on how long they survived
          const survivalRatio = Math.min(1, (village.aliveTime || 0) / this.villageSurviveTime);
          const partialPoints = Math.floor(this.villageSavePoints * survivalRatio);
          if (partialPoints > 0) {
            this.scoreManager.addRawPoints(partialPoints);
          }
        }
      }

      // Single score update at the end
      if (this.manager && this.manager.reportScoreUpdate) {
        this.manager.reportScoreUpdate(this.scoreManager.score, this.scoreManager.combo);
      }

      console.log(`Game ended! Saved ${this.villagesSaved} villages, lost ${this.villagesLost}. Final score: ${this.scoreManager.score}`);
    } catch (e) {
      console.warn('Error in onGameEnd:', e);
    }
  }

  updateVillageVisual(village, dt) {
    // Safety check
    if (!village || !village.ring) return;

    try {
      // Update terrain-following position
      const centerHeight = this.zoneDetector.getTerrainHeight(village.x, village.y) || 0;
      const beaconHeight = village.beaconHeight || 12;
      const bobOffset = Math.sin(this.elapsedTime * 1.5) * 1;

      // Find the highest point among center and beacon positions
      let maxTerrainHeight = centerHeight;
      for (let i = 0; i < 4; i++) {
        const angle = (i / 4) * Math.PI * 2;
        const bx = village.x + Math.cos(angle) * village.radius;
        const by = village.y + Math.sin(angle) * village.radius;
        const bHeight = this.zoneDetector.getTerrainHeight(bx, by);
        if (typeof bHeight === 'number' && bHeight > maxTerrainHeight) {
          maxTerrainHeight = bHeight;
        }
      }

    // Ring floats above the highest terrain point
    if (village.ring) {
      village.ring.position.y = maxTerrainHeight + beaconHeight + 2 + bobOffset;
    }

    const buildingHeights = [2.5, 1.8, 2, 1.5];
    for (let i = 0; i < village.buildings.length; i++) {
      const building = village.buildings[i];
      const baseHeight = buildingHeights[i] || 2;
      building.position.y = centerHeight + baseHeight / 2;
    }

    // Update beacon positions (terrain-following + bob animation)
    if (village.beacons) {
      village.beacons.children.forEach((beacon, i) => {
        const angle = (i / 4) * Math.PI * 2;
        const bx = village.x + Math.cos(angle) * village.radius;
        const by = village.y + Math.sin(angle) * village.radius;
        const bHeight = this.zoneDetector.getTerrainHeight(bx, by);
        const beaconWorldPos = this.gridToWorld(bx, by);
        beacon.position.x = beaconWorldPos.x;
        beacon.position.z = beaconWorldPos.z;
        // Beacon extends from terrain to ring height
        beacon.position.y = bHeight + beaconHeight / 2 + bobOffset;
      });
    }

    // Determine color based on state
    let ringColor, beaconColor, buildingColor;
    let ringOpacity = 0.7;
    let beaconOpacity = 0.8;

    if (village.isFlooded) {
      // Flooding - flash red urgently, more urgent as time runs out
      const urgency = village.floodTime / this.floodDestroyTime;
      const flashSpeed = 8 + urgency * 12;
      const flash = Math.sin(this.elapsedTime * flashSpeed) > 0;

      ringColor = flash ? 0xff3333 : 0xff6600;
      beaconColor = flash ? 0xff3333 : 0xff6600;
      buildingColor = flash ? 0xff8888 : 0xffaa88;
      ringOpacity = 0.9;
      beaconOpacity = 0.9;

      // Buildings fade as they're about to be destroyed
      for (const mat of village.buildingMaterials) {
        mat.color.setHex(buildingColor);
        mat.opacity = 0.9 - urgency * 0.4;
      }
    } else if (village.floodTime > 0) {
      // Recently flooded, recovering - yellow warning
      ringColor = 0xffcc44;
      beaconColor = 0xffcc44;
      buildingColor = 0xDDCC99;
      ringOpacity = 0.7;

      for (const mat of village.buildingMaterials) {
        mat.color.setHex(buildingColor);
        mat.opacity = 0.9;
      }
    } else if (village.saved) {
      // Saved! - bright gold
      ringColor = 0xffdd44;
      beaconColor = 0xffdd44;
      buildingColor = 0xEEDD99;
      ringOpacity = 0.8;

      for (const mat of village.buildingMaterials) {
        mat.color.setHex(buildingColor);
      }
    } else {
      // Safe - green, pulsing gently
      const pulse = 0.6 + Math.sin(this.elapsedTime * 2) * 0.1;
      ringColor = 0x44ff88;
      beaconColor = 0x44ff88;
      buildingColor = 0xDDCC99;
      ringOpacity = pulse;
      beaconOpacity = pulse + 0.2;

      for (const mat of village.buildingMaterials) {
        mat.color.setHex(buildingColor);
        mat.opacity = 0.9;
      }
    }

    // Apply colors
    village.ringMaterial.color.setHex(ringColor);
    village.ringMaterial.opacity = ringOpacity;

    if (village.beacons) {
      village.beacons.children.forEach(beacon => {
        beacon.material.color.setHex(beaconColor);
        beacon.material.opacity = beaconOpacity;
      });
    }
    } catch (e) {
      // Ignore visual update errors during game end
      console.warn('Village visual update error:', e);
    }
  }

  removeVillage(village) {
    if (!village) return;

    try {
      if (village.ring) {
        this.removeVisualElement(village.ring);
        if (village.ring.geometry) village.ring.geometry.dispose();
        if (village.ringMaterial) village.ringMaterial.dispose();
        village.ring = null;
      }

      if (village.beacons) {
        this.removeVisualElement(village.beacons);
        // Dispose individual beacon materials (cloned), but NOT geometry (shared)
        village.beacons.children.forEach(beacon => {
          if (beacon.material) beacon.material.dispose();
        });
        village.beacons = null;
      }

      // Dispose the shared beacon geometry only once
      if (village.beaconGeom) {
        village.beaconGeom.dispose();
        village.beaconGeom = null;
      }

      if (village.buildings) {
        for (const building of village.buildings) {
          this.removeVisualElement(building);
          if (building.geometry) building.geometry.dispose();
          if (building.material) building.material.dispose();
        }
        village.buildings = [];
      }
    } catch (e) {
      console.warn('Error removing village:', e);
    }
  }

  cleanup() {
    // Mark as cleaning up to prevent any callbacks
    this.gameOver = true;

    // Remove villages safely
    try {
      const villagesCopy = [...this.villages];
      this.villages = []; // Clear first to prevent re-entry
      for (const village of villagesCopy) {
        this.removeVillage(village);
      }
    } catch (e) {
      console.warn('Error cleaning up villages:', e);
    }

    // Remove springs safely using removeSpringNear with exact coordinates
    try {
      const springsCopy = [...this.springs];
      this.springs = []; // Clear first to prevent re-entry
      for (const spring of springsCopy) {
        try {
          if (spring && typeof spring.x === 'number' && typeof spring.y === 'number') {
            // Use radius of 1 to remove only the exact spring
            this.water.removeSpringNear(spring.x, spring.y, 1);
          }
        } catch (e) {
          console.warn('Error removing spring:', e);
        }
      }
    } catch (e) {
      console.warn('Error cleaning up springs:', e);
    }

    try {
      super.cleanup();
    } catch (e) {
      console.warn('Error in base cleanup:', e);
    }
  }

  getDisplayInfo() {
    const info = super.getDisplayInfo();
    info.timeRemaining = Math.max(0, this.gameTime - this.elapsedTime);
    info.villagesSaved = this.villagesSaved;
    info.villagesLost = this.villagesLost;
    info.activeVillages = this.villages.length;
    info.quarterHearts = this.quarterHearts;
    info.maxQuarterHearts = this.maxQuarterHearts;
    info.nextVillageIn = Math.max(0, this.villageSpawnTimer);
    return info;
  }

  getStats() {
    const stats = super.getStats();
    stats.villagesSaved = this.villagesSaved;
    stats.villagesLost = this.villagesLost;
    return stats;
  }
}
