/**
 * CascadeMode - Chain water through sequential rings for combos
 *
 * Gameplay:
 * - Rings are placed in sequence from high to low elevation
 * - Guide water to flow through rings in order (1 -> 2 -> 3 -> etc)
 * - Hitting rings in sequence builds combo multiplier
 * - Breaking sequence resets combo
 * - Game ends when time runs out
 */

import { BaseGameMode } from './BaseGameMode.js';
import * as THREE from 'three';
import { getWorldScale } from '../../simulation/constants.js';

// Ring states
const RingState = {
  WAITING: 'waiting',      // Waiting for water (not yet in sequence)
  READY: 'ready',          // Next ring in sequence
  TRIGGERED: 'triggered',  // Water just passed through
  COMPLETED: 'completed',  // Already scored
};

export class CascadeMode extends BaseGameMode {
  constructor(options) {
    super(options);

    // Mode configuration
    this.gameTime = 120; // 2 minutes per round
    this.numRings = 5;   // Rings per cascade
    this.ringRadius = 4; // Detection radius in grid cells
    this.ringVisualRadius = 3; // Visual size
    this.cascadeLifetime = 15; // Seconds for entire cascade chain

    // Ring tracking
    this.rings = [];
    this.currentRingIndex = 0; // Which ring is "ready" next
    this.cascadesCompleted = 0;
    this.cascadeTimeRemaining = 0; // Timer for current cascade

    // Points configuration
    this.basePoints = 50;
    this.sequenceBonus = 25; // Extra points per ring in sequence
    this.cascadeCompleteBonus = 500; // Bonus for completing full cascade

    // Timing
    this.flowCooldown = 0; // Prevent double-triggers
    this.ringResetDelay = 2; // Seconds before resetting completed cascade
    this.isRegenerating = false; // Flag during cascade regeneration

    // Hearts system
    this.maxHearts = 3;
    this.quarterHearts = 12;
    this.maxQuarterHearts = 12;
  }

  init() {
    super.init();

    // Reset state
    this.rings = [];
    this.currentRingIndex = 0;
    this.cascadesCompleted = 0;
    this.flowCooldown = 0;
    this.isRegenerating = false;
    this.quarterHearts = this.maxQuarterHearts;

    // Generate cascade of rings
    this.generateCascade();

    console.log('Cascade started! Guide water through the rings in order.');
  }

  generateCascade(retryCount = 0) {
    // Pause timer during generation
    this.isRegenerating = true;

    // Clear existing rings
    for (const ring of this.rings) {
      this.removeRing(ring);
    }
    this.rings = [];
    this.currentRingIndex = 0;

    // Find a good starting point (high ground)
    const startPos = this.findHighPoint();

    // Generate rings flowing downhill
    let currentX = startPos.x;
    let currentY = startPos.y;
    let lastHeight = startPos.height;
    let lastDirection = { x: 0, y: 1 }; // Default: flow towards positive Y

    for (let i = 0; i < this.numRings; i++) {
      // Find next position downhill
      const nextPos = this.findDownhillPosition(currentX, currentY, lastHeight, lastDirection);

      if (!nextPos) {
        // Limit retries to prevent infinite loops
        if (retryCount >= 3) {
          console.warn(`Could only place ${i} rings after ${retryCount} retries, using partial cascade`);
          break; // Use whatever rings we have
        }
        console.warn(`Could only place ${i} rings, retrying...`);
        setTimeout(() => this.generateCascade(retryCount + 1), 300);
        return;
      }

      const ring = {
        index: i,
        x: nextPos.x,
        y: nextPos.y,
        height: nextPos.height,
        radius: this.ringRadius,
        state: i === 0 ? RingState.READY : RingState.WAITING,
        flowDirection: nextPos.direction,
        triggered: false,
        triggerTime: 0,
        mesh: null,
        glowMesh: null,
      };

      this.createRingVisual(ring);
      this.rings.push(ring);

      currentX = nextPos.x;
      currentY = nextPos.y;
      lastHeight = nextPos.height;
      lastDirection = nextPos.direction;
    }

    // Start cascade timer and enable updates
    this.cascadeTimeRemaining = this.cascadeLifetime;
    this.isRegenerating = false;

    console.log(`Cascade generated with ${this.rings.length} rings`);
  }

  getTerrainHeightPercentile(percentile) {
    // Sample terrain to find height at given percentile
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

  findHighPoint() {
    const margin = 25;
    const minHeightThreshold = this.getTerrainHeightPercentile(0.6); // Top 40% of terrain

    let bestPos = null;
    let bestScore = -Infinity;

    // Sample many points to find good high ground
    for (let i = 0; i < 50; i++) {
      const x = margin + Math.random() * (this.terrain.width - margin * 2);
      const y = margin + Math.random() * (this.terrain.height - margin * 2);
      const height = this.zoneDetector.getTerrainHeight(x, y);

      // Skip if too low (below 60th percentile)
      if (height < minHeightThreshold) continue;

      // Skip if flooded (has water)
      const waterDepth = this.water.getDepth(Math.floor(x), Math.floor(y));
      if (waterDepth > 0.5) continue;

      // Score based on height and dryness
      let score = height;
      score -= waterDepth * 50; // Penalize wet areas

      if (score > bestScore) {
        bestScore = score;
        bestPos = { x, y, height };
      }
    }

    // Fallback if no dry high ground found
    if (!bestPos) {
      const x = this.terrain.width / 2;
      const y = this.terrain.height / 4;
      bestPos = {
        x,
        y,
        height: this.zoneDetector.getTerrainHeight(x, y)
      };
    }

    return bestPos;
  }

  findDownhillPosition(fromX, fromY, fromHeight, lastDirection) {
    const stepDistance = 12 + Math.random() * 8; // Distance between rings
    const attempts = 40;
    const margin = 20;
    let bestPos = null;
    let bestScore = -Infinity;

    // Base angle from last direction, with some variation
    const baseAngle = Math.atan2(lastDirection.y, lastDirection.x);

    for (let i = 0; i < attempts; i++) {
      // Vary angle from the base direction
      const angleVariation = (Math.random() - 0.5) * Math.PI * 0.6;
      const angle = baseAngle + angleVariation;

      const nx = fromX + Math.cos(angle) * stepDistance;
      const ny = fromY + Math.sin(angle) * stepDistance;

      // Check bounds with margin
      if (nx < margin || nx > this.terrain.width - margin ||
          ny < margin || ny > this.terrain.height - margin) continue;

      const height = this.zoneDetector.getTerrainHeight(nx, ny);

      // Must be downhill (at least a little)
      const heightDiff = fromHeight - height;
      if (heightDiff < 2) continue;

      // Check for water at this position
      const waterDepth = this.water.getDepth(Math.floor(nx), Math.floor(ny));

      // Score: prefer moderate drops, penalize extreme drops and wet areas
      let score = 10;
      score += Math.min(heightDiff, 30); // Reward downhill up to 30
      score -= Math.max(0, heightDiff - 40) * 0.5; // Penalize very steep
      score -= Math.abs(angleVariation) * 2; // Prefer straighter paths
      score -= waterDepth * 10; // Penalize flooded areas

      if (score > bestScore) {
        bestScore = score;
        bestPos = {
          x: nx,
          y: ny,
          height: height,
          direction: { x: Math.cos(angle), y: Math.sin(angle) },
        };
      }
    }

    // If no good downhill found, try to find ANY valid position
    if (!bestPos) {
      for (let i = 0; i < 20; i++) {
        const angle = Math.random() * Math.PI * 2;
        const dist = stepDistance * 0.8;
        const nx = fromX + Math.cos(angle) * dist;
        const ny = fromY + Math.sin(angle) * dist;

        if (nx < margin || nx > this.terrain.width - margin ||
            ny < margin || ny > this.terrain.height - margin) continue;

        const height = this.zoneDetector.getTerrainHeight(nx, ny);

        // Accept even flat or slightly uphill in desperation
        if (fromHeight - height > -10) {
          bestPos = {
            x: nx,
            y: ny,
            height: height,
            direction: { x: Math.cos(angle), y: Math.sin(angle) },
          };
          break;
        }
      }
    }

    return bestPos; // May be null if truly stuck
  }

  // Convert grid coordinates to world coordinates
  gridToWorld(gridX, gridY) {
    const worldScale = getWorldScale();
    return {
      x: (gridX - this.terrain.width / 2) * worldScale,
      z: (gridY - this.terrain.height / 2) * worldScale,
    };
  }

  createRingVisual(ring) {
    const worldScale = getWorldScale();
    const worldPos = this.gridToWorld(ring.x, ring.y);
    const ringWorldRadius = this.ringVisualRadius * worldScale;

    // Create torus (ring/hoop shape) - stands vertical
    const torusGeometry = new THREE.TorusGeometry(ringWorldRadius, ringWorldRadius * 0.15, 8, 24);
    const torusMaterial = new THREE.MeshBasicMaterial({
      color: ring.state === RingState.READY ? 0x44ff88 : 0x888888,
      transparent: true,
      opacity: 0.8,
      side: THREE.DoubleSide,
    });
    const torus = new THREE.Mesh(torusGeometry, torusMaterial);

    // Position and rotate to stand vertical, facing the flow direction
    const angle = Math.atan2(ring.flowDirection.y, ring.flowDirection.x);
    torus.rotation.y = -angle + Math.PI / 2;
    torus.rotation.x = Math.PI / 2;

    const terrainHeight = this.zoneDetector.getTerrainHeight(ring.x, ring.y);
    torus.position.set(worldPos.x, terrainHeight + ringWorldRadius * 1.2, worldPos.z);

    // Create number indicator
    const canvas = document.createElement('canvas');
    canvas.width = 64;
    canvas.height = 64;
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 48px Arial';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(String(ring.index + 1), 32, 32);

    const numberTexture = new THREE.CanvasTexture(canvas);
    const numberMaterial = new THREE.SpriteMaterial({
      map: numberTexture,
      transparent: true,
    });
    const numberSprite = new THREE.Sprite(numberMaterial);
    numberSprite.scale.set(4, 4, 1);
    numberSprite.position.set(worldPos.x, terrainHeight + ringWorldRadius * 2.5, worldPos.z);

    // Store references
    ring.mesh = torus;
    ring.numberSprite = numberSprite;
    ring.material = torusMaterial;
    ring.worldPos = worldPos;
    ring.ringWorldRadius = ringWorldRadius;

    // Add to scene
    this.addVisualElement(torus);
    this.addVisualElement(numberSprite);
  }

  update(dt) {
    super.update(dt);

    // Check game time
    const remainingTime = this.gameTime - this.elapsedTime;
    if (remainingTime <= 0) {
      this.gameOver = true;
      this.victory = true;
      return;
    }

    // Update cooldown
    if (this.flowCooldown > 0) {
      this.flowCooldown -= dt;
    }

    // Update cascade timer
    this.updateCascadeTimer(dt);

    // Check for water passing through rings
    this.checkRingFlow(dt);

    // Update ring visuals
    for (const ring of this.rings) {
      this.updateRingVisual(ring, dt);
    }
  }

  updateCascadeTimer(dt) {
    // Don't update timer during regeneration or if already expired
    if (this.isRegenerating) return;
    if (this.rings.length === 0) return;
    if (this.cascadeTimeRemaining <= 0) return;

    // Count down cascade timer
    this.cascadeTimeRemaining -= dt;

    // Check for expiration
    if (this.cascadeTimeRemaining <= 0) {
      this.cascadeTimeRemaining = 0;
      this.onCascadeExpired();
    }
  }

  onCascadeExpired() {
    // Prevent double-triggering
    if (this.isRegenerating) return;
    this.isRegenerating = true;

    // Count remaining rings (not yet triggered)
    const remainingRings = this.rings.length - this.currentRingIndex;

    // Lose (remainingRings - 1) quarter hearts
    // So getting 4 of 5 = no penalty, missing all 5 = lose 4 quarter hearts (1 full heart)
    const heartsLost = Math.max(0, remainingRings - 1);
    this.quarterHearts -= heartsLost;

    // Reset combo
    this.scoreManager.resetCombo();

    if (heartsLost > 0) {
      console.log(`Cascade expired! Missed ${remainingRings} rings. Lost ${heartsLost} quarter hearts. (${this.quarterHearts}/${this.maxQuarterHearts})`);
    } else {
      console.log(`Cascade expired! Only missed 1 ring - no penalty.`);
    }

    // Mark all remaining rings as completed (failed)
    for (let i = this.currentRingIndex; i < this.rings.length; i++) {
      if (this.rings[i]) {
        this.rings[i].state = RingState.COMPLETED;
      }
    }

    // Check for game over
    if (this.quarterHearts <= 0) {
      this.quarterHearts = 0;
      this.gameOver = true;
      this.victory = false;
      return;
    }

    // Generate new cascade after delay
    setTimeout(() => {
      if (!this.gameOver) {
        this.generateCascade();
      }
    }, this.ringResetDelay * 1000);
  }

  checkRingFlow(dt) {
    if (this.flowCooldown > 0) return;
    if (this.isRegenerating) return; // Don't check during regeneration

    // Make sure we have rings and a valid index
    if (this.rings.length === 0 || this.currentRingIndex >= this.rings.length) return;

    const readyRing = this.rings[this.currentRingIndex];
    if (!readyRing || readyRing.state !== RingState.READY) return;

    // Check for water flow through the ready ring
    const flow = this.zoneDetector.getFlowInCircle(readyRing.x, readyRing.y, readyRing.radius);
    const hasWater = this.zoneDetector.hasWater(readyRing.x, readyRing.y, readyRing.radius, 0.3);

    if (hasWater && flow.speed > 0.3) {
      // Ring triggered!
      this.onRingTriggered(readyRing);
    }
  }

  onRingTriggered(ring) {
    ring.state = RingState.TRIGGERED;
    ring.triggerTime = 0.5; // Flash duration

    // Calculate points
    const points = this.basePoints + (this.sequenceBonus * ring.index);
    this.addPoints(points);
    this.increaseCombo();

    console.log(`Ring ${ring.index + 1} triggered! +${points} points`);

    // Mark this ring as completed after flash
    setTimeout(() => {
      if (ring.state === RingState.TRIGGERED) {
        ring.state = RingState.COMPLETED;
      }
    }, 500);

    // Move to next ring
    this.currentRingIndex++;
    this.flowCooldown = 0.5; // Brief cooldown

    // Check if cascade complete
    if (this.currentRingIndex >= this.rings.length) {
      this.onCascadeComplete();
    } else {
      // Mark next ring as ready (cascade timer continues)
      const nextRing = this.rings[this.currentRingIndex];
      if (nextRing) {
        nextRing.state = RingState.READY;
      }
    }
  }

  onCascadeComplete() {
    this.cascadesCompleted++;
    this.addPoints(this.cascadeCompleteBonus);
    this.isRegenerating = true;
    this.cascadeTimeRemaining = 0; // Stop timer display

    console.log(`CASCADE COMPLETE! +${this.cascadeCompleteBonus} bonus!`);

    // Generate new cascade after delay
    setTimeout(() => {
      if (!this.gameOver) {
        this.generateCascade();
      }
    }, this.ringResetDelay * 1000);
  }

  updateRingVisual(ring, dt) {
    if (!ring.mesh || !ring.material) return;

    // Update terrain-following position
    const terrainHeight = this.zoneDetector.getTerrainHeight(ring.x, ring.y);
    ring.mesh.position.y = terrainHeight + ring.ringWorldRadius * 1.2;
    if (ring.numberSprite) {
      ring.numberSprite.position.y = terrainHeight + ring.ringWorldRadius * 2.5;
    }

    // Update trigger flash
    if (ring.triggerTime > 0) {
      ring.triggerTime -= dt;
      ring.material.color.setHex(0xffffff);
      ring.material.opacity = 0.5 + Math.sin(ring.triggerTime * 20) * 0.5;
      return;
    }

    // Color based on state
    switch (ring.state) {
      case RingState.READY:
        // Check for urgency (less than 30% cascade time remaining)
        const timeRatio = this.cascadeTimeRemaining / this.cascadeLifetime;
        if (timeRatio < 0.3) {
          // Urgent - flash red/orange like Flow Rush
          const urgent = Math.sin(this.elapsedTime * 10) > 0;
          ring.material.color.setHex(urgent ? 0xff3333 : 0xff6600);
          ring.material.opacity = 0.8 + Math.sin(this.elapsedTime * 8) * 0.2;
        } else {
          ring.material.color.setHex(0x44ff88); // Green - ready
          ring.material.opacity = 0.7 + Math.sin(this.elapsedTime * 4) * 0.2;
        }
        break;
      case RingState.WAITING:
        // Also show urgency on waiting rings when time is low
        const cascadeTimeRatio = this.cascadeTimeRemaining / this.cascadeLifetime;
        if (cascadeTimeRatio < 0.3) {
          const urgent = Math.sin(this.elapsedTime * 10) > 0;
          ring.material.color.setHex(urgent ? 0xaa2222 : 0x884400);
          ring.material.opacity = 0.5;
        } else {
          ring.material.color.setHex(0x666666); // Gray - waiting
          ring.material.opacity = 0.4;
        }
        break;
      case RingState.COMPLETED:
        ring.material.color.setHex(0x44aaff); // Blue - done
        ring.material.opacity = 0.3;
        break;
    }
  }

  removeRing(ring) {
    if (ring.mesh) {
      this.removeVisualElement(ring.mesh);
      ring.mesh.geometry.dispose();
      ring.material.dispose();
    }
    if (ring.numberSprite) {
      this.removeVisualElement(ring.numberSprite);
      ring.numberSprite.material.map.dispose();
      ring.numberSprite.material.dispose();
    }
  }

  cleanup() {
    for (const ring of this.rings) {
      this.removeRing(ring);
    }
    this.rings = [];
    super.cleanup();
  }

  getDisplayInfo() {
    const info = super.getDisplayInfo();
    info.timeRemaining = Math.max(0, this.gameTime - this.elapsedTime);
    info.cascadesCompleted = this.cascadesCompleted;
    info.currentRing = this.currentRingIndex + 1;
    info.totalRings = this.numRings;
    info.quarterHearts = this.quarterHearts;
    info.maxQuarterHearts = this.maxQuarterHearts;

    // Cascade timer for HUD display
    info.cascadeTimeRemaining = Math.max(0, this.cascadeTimeRemaining);
    info.cascadeLifetime = this.cascadeLifetime;
    return info;
  }

  getStats() {
    const stats = super.getStats();
    stats.cascadesCompleted = this.cascadesCompleted;
    return stats;
  }
}
