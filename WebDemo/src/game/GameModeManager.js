/**
 * GameModeManager - Handles game mode lifecycle and transitions
 *
 * Modes:
 * - sandbox: Free play, no objectives (default)
 * - flowRush: Guide water to targets for points
 * - cascade: Chain water through rings
 * - floodDefense: Protect zones from water
 */

export const GameModeType = {
  SANDBOX: 'sandbox',
  FLOW_RUSH: 'flowRush',
  CASCADE: 'cascade',
  FLOOD_DEFENSE: 'floodDefense',
};

export class GameModeManager {
  constructor(terrain, water, scene) {
    this.terrain = terrain;
    this.water = water;
    this.scene = scene;

    this.currentModeType = GameModeType.SANDBOX;
    this.lastPlayedMode = null; // Track last arcade mode for restart
    this.currentMode = null;
    this.isPlaying = false;

    // Registered mode classes
    this.modeClasses = new Map();

    // Callbacks
    this.onModeStart = null;
    this.onModeEnd = null;
    this.onScoreUpdate = null;
  }

  /**
   * Register a mode class for a mode type
   */
  registerMode(modeType, ModeClass) {
    this.modeClasses.set(modeType, ModeClass);
  }

  /**
   * Start a game mode
   */
  startMode(modeType) {
    // End current mode if playing
    if (this.isPlaying) {
      this.endMode(false);
    }

    // Check if mode is registered
    const ModeClass = this.modeClasses.get(modeType);
    if (!ModeClass) {
      console.warn(`Mode "${modeType}" not registered`);
      return false;
    }

    this.currentModeType = modeType;
    if (modeType !== GameModeType.SANDBOX) {
      this.lastPlayedMode = modeType;
    }

    // Create mode instance
    this.currentMode = new ModeClass({
      terrain: this.terrain,
      water: this.water,
      scene: this.scene,
      manager: this,
    });

    // Initialize the mode (generates map, places markers, etc.)
    this.currentMode.init();

    this.isPlaying = true;

    if (this.onModeStart) {
      this.onModeStart(modeType);
    }

    console.log(`Started mode: ${modeType}`);
    return true;
  }

  /**
   * End current game mode
   */
  endMode(completed = false) {
    if (!this.isPlaying || !this.currentMode) return;

    const finalScore = this.currentMode.getScore();
    const stats = this.currentMode.getStats();

    // Cleanup mode
    this.currentMode.cleanup();
    this.currentMode = null;

    this.isPlaying = false;

    if (this.onModeEnd) {
      this.onModeEnd(this.currentModeType, completed, finalScore, stats);
    }

    // Return to sandbox
    this.currentModeType = GameModeType.SANDBOX;

    console.log(`Ended mode. Score: ${finalScore}`);
    return { finalScore, stats };
  }

  /**
   * Update current mode (called each frame)
   */
  update(dt) {
    if (!this.isPlaying || !this.currentMode) return;

    // Update the mode
    this.currentMode.update(dt);

    // Check if mode has ended itself (game over condition)
    if (this.currentMode.isGameOver()) {
      this.endMode(this.currentMode.isVictory());
    }
  }

  /**
   * Report score update (called by modes)
   */
  reportScoreUpdate(score, combo) {
    if (this.onScoreUpdate) {
      this.onScoreUpdate(score, combo);
    }
  }

  /**
   * Get current mode info
   */
  getCurrentMode() {
    return {
      type: this.currentModeType,
      isPlaying: this.isPlaying,
      mode: this.currentMode,
    };
  }

  /**
   * Check if in sandbox mode
   */
  isSandbox() {
    return this.currentModeType === GameModeType.SANDBOX;
  }

  /**
   * Get available modes
   */
  getAvailableModes() {
    return Array.from(this.modeClasses.keys());
  }
}
