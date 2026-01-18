/**
 * ZoneDetector - Detects water presence/volume in defined areas
 *
 * Used by game modes to check if water has reached targets,
 * flooded protected areas, or passed through checkpoints.
 */

export class ZoneDetector {
  constructor(water, terrain) {
    this.water = water;
    this.terrain = terrain;
  }

  /**
   * Get total water volume in a circular zone
   * @param {number} centerX - Grid X coordinate
   * @param {number} centerY - Grid Y coordinate
   * @param {number} radius - Radius in grid cells
   * @returns {number} Total water volume in zone
   */
  getWaterInCircle(centerX, centerY, radius) {
    let total = 0;
    const radiusSq = radius * radius;

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const distSq = dx * dx + dy * dy;
        if (distSq > radiusSq) continue;

        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (this.water.inBounds(x, y)) {
          total += this.water.getDepth(x, y);
        }
      }
    }

    return total;
  }

  /**
   * Get total water volume in a rectangular zone
   * @param {number} x1 - Min X (grid coords)
   * @param {number} y1 - Min Y (grid coords)
   * @param {number} x2 - Max X (grid coords)
   * @param {number} y2 - Max Y (grid coords)
   * @returns {number} Total water volume in zone
   */
  getWaterInRect(x1, y1, x2, y2) {
    let total = 0;

    const minX = Math.floor(Math.min(x1, x2));
    const maxX = Math.floor(Math.max(x1, x2));
    const minY = Math.floor(Math.min(y1, y2));
    const maxY = Math.floor(Math.max(y1, y2));

    for (let y = minY; y <= maxY; y++) {
      for (let x = minX; x <= maxX; x++) {
        if (this.water.inBounds(x, y)) {
          total += this.water.getDepth(x, y);
        }
      }
    }

    return total;
  }

  /**
   * Check if water volume in circle exceeds threshold
   */
  isCircleFilled(centerX, centerY, radius, threshold) {
    return this.getWaterInCircle(centerX, centerY, radius) >= threshold;
  }

  /**
   * Check if water volume in rect exceeds threshold
   */
  isRectFilled(x1, y1, x2, y2, threshold) {
    return this.getWaterInRect(x1, y1, x2, y2) >= threshold;
  }

  /**
   * Get average water depth in a circular zone
   */
  getAverageDepthInCircle(centerX, centerY, radius) {
    let total = 0;
    let count = 0;
    const radiusSq = radius * radius;

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const distSq = dx * dx + dy * dy;
        if (distSq > radiusSq) continue;

        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (this.water.inBounds(x, y)) {
          total += this.water.getDepth(x, y);
          count++;
        }
      }
    }

    return count > 0 ? total / count : 0;
  }

  /**
   * Check if zone has any significant water (for flow detection)
   */
  hasWater(centerX, centerY, radius, minDepth = 0.5) {
    const radiusSq = radius * radius;

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const distSq = dx * dx + dy * dy;
        if (distSq > radiusSq) continue;

        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (this.water.inBounds(x, y)) {
          if (this.water.getDepth(x, y) >= minDepth) {
            return true;
          }
        }
      }
    }

    return false;
  }

  /**
   * Get water flow velocity in a zone (for Cascade mode)
   */
  getFlowInCircle(centerX, centerY, radius) {
    let totalVx = 0;
    let totalVy = 0;
    let count = 0;
    const radiusSq = radius * radius;

    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const distSq = dx * dx + dy * dy;
        if (distSq > radiusSq) continue;

        const x = Math.floor(centerX + dx);
        const y = Math.floor(centerY + dy);

        if (this.water.inBounds(x, y)) {
          const idx = y * this.water.width + x;
          const depth = this.water.depth[idx];

          if (depth > 0.1) {
            totalVx += this.water.velocityX[idx];
            totalVy += this.water.velocityY[idx];
            count++;
          }
        }
      }
    }

    if (count === 0) return { vx: 0, vy: 0, speed: 0 };

    const vx = totalVx / count;
    const vy = totalVy / count;
    const speed = Math.sqrt(vx * vx + vy * vy);

    return { vx, vy, speed };
  }

  /**
   * Get terrain height at a point (for placing zones at proper heights)
   */
  getTerrainHeight(x, y) {
    return this.terrain.getHeight(Math.floor(x), Math.floor(y));
  }

  /**
   * Find a random valid position on the terrain
   * @param {number} margin - Distance from edges
   * @param {number} minHeight - Minimum terrain height
   * @param {number} maxHeight - Maximum terrain height (optional)
   */
  findRandomPosition(margin = 20, minHeight = -50, maxHeight = Infinity) {
    const maxAttempts = 50;

    for (let i = 0; i < maxAttempts; i++) {
      const x = margin + Math.random() * (this.terrain.width - margin * 2);
      const y = margin + Math.random() * (this.terrain.height - margin * 2);
      const height = this.terrain.getHeight(Math.floor(x), Math.floor(y));

      if (height >= minHeight && height <= maxHeight) {
        return { x, y, height };
      }
    }

    // Fallback to center
    const cx = this.terrain.width / 2;
    const cy = this.terrain.height / 2;
    return { x: cx, y: cy, height: this.terrain.getHeight(cx, cy) };
  }
}
