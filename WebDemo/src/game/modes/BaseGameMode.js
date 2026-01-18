/**
 * BaseGameMode - Abstract base class for all game modes
 *
 * Provides common interface and utilities for game modes.
 * Extend this class to create specific modes like FlowRush, Cascade, etc.
 */

import { ZoneDetector } from '../ZoneDetector.js';
import { ScoreManager } from '../ScoreManager.js';

export class BaseGameMode {
  constructor({ terrain, water, scene, manager }) {
    this.terrain = terrain;
    this.water = water;
    this.scene = scene;
    this.manager = manager;

    // Create utilities
    this.zoneDetector = new ZoneDetector(water, terrain);
    this.scoreManager = new ScoreManager();

    // Game state
    this.gameOver = false;
    this.victory = false;
    this.elapsedTime = 0;

    // To be set by subclasses
    this.lives = 3;
    this.maxLives = 3;

    // Visual elements (to be cleaned up)
    this.visualElements = [];
  }

  /**
   * Initialize the mode (override in subclass)
   * Called when mode starts
   */
  init() {
    this.scoreManager.reset();
    this.gameOver = false;
    this.victory = false;
    this.elapsedTime = 0;
    this.lives = this.maxLives;

    // Reset terrain and water for fresh start
    this.terrain.generate();
    this.water.reset();
  }

  /**
   * Update the mode (override in subclass)
   * Called each frame
   */
  update(dt) {
    this.elapsedTime += dt;
    this.scoreManager.update(dt);
  }

  /**
   * Cleanup mode (override in subclass)
   * Called when mode ends
   */
  cleanup() {
    // Remove visual elements from scene
    for (const element of this.visualElements) {
      this.scene.remove(element);
    }
    this.visualElements = [];
  }

  /**
   * Get current score
   */
  getScore() {
    return this.scoreManager.score;
  }

  /**
   * Get mode statistics
   */
  getStats() {
    return {
      score: this.scoreManager.score,
      maxCombo: this.scoreManager.maxCombo,
      elapsedTime: this.elapsedTime,
      lives: this.lives,
    };
  }

  /**
   * Check if game is over
   */
  isGameOver() {
    return this.gameOver;
  }

  /**
   * Check if player won
   */
  isVictory() {
    return this.victory;
  }

  /**
   * Lose a life
   */
  loseLife() {
    this.lives--;
    if (this.lives <= 0) {
      this.gameOver = true;
      this.victory = false;
    }
    return this.lives;
  }

  /**
   * Add points and update manager
   */
  addPoints(basePoints) {
    const points = this.scoreManager.addPoints(basePoints);
    this.manager.reportScoreUpdate(this.scoreManager.score, this.scoreManager.combo);
    return points;
  }

  /**
   * Increase combo
   */
  increaseCombo() {
    this.scoreManager.increaseCombo();
    this.manager.reportScoreUpdate(this.scoreManager.score, this.scoreManager.combo);
  }

  /**
   * Get display info for UI
   */
  getDisplayInfo() {
    return {
      score: this.scoreManager.score,
      combo: this.scoreManager.combo,
      comboTimer: this.scoreManager.getComboTimeRemaining(),
      lives: this.lives,
      maxLives: this.maxLives,
      elapsedTime: this.elapsedTime,
    };
  }

  /**
   * Add a visual element to the scene (tracked for cleanup)
   */
  addVisualElement(element) {
    this.scene.add(element);
    this.visualElements.push(element);
    return element;
  }

  /**
   * Remove a visual element
   */
  removeVisualElement(element) {
    this.scene.remove(element);
    const idx = this.visualElements.indexOf(element);
    if (idx !== -1) {
      this.visualElements.splice(idx, 1);
    }
  }
}
