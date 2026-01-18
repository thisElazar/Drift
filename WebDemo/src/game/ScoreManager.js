/**
 * ScoreManager - Handles scoring, combos, and high scores
 *
 * Features:
 * - Point accumulation
 * - Combo multiplier with decay
 * - High score persistence (localStorage)
 * - Per-mode high scores
 */

const STORAGE_KEY = 'drift-high-scores-v2';
const MAX_LEADERBOARD_ENTRIES = 5;

export class ScoreManager {
  constructor() {
    this.score = 0;
    this.combo = 1;
    this.maxCombo = 1;
    this.comboTimer = 0;
    this.comboDecayTime = 3.0; // Seconds before combo resets

    // Load high scores from localStorage
    // Structure: { modeType: [{ initials: "AAA", score: 1000 }, ...] }
    this.highScores = this.loadHighScores();

    // Callbacks
    this.onScoreChange = null;
    this.onComboChange = null;
    this.onNewHighScore = null;
  }

  /**
   * Reset score for new game
   */
  reset() {
    this.score = 0;
    this.combo = 1;
    this.maxCombo = 1;
    this.comboTimer = 0;
  }

  /**
   * Add points (multiplied by current combo)
   */
  addPoints(basePoints) {
    const points = Math.floor(basePoints * this.combo);
    this.score += points;

    // Refresh combo timer
    this.comboTimer = this.comboDecayTime;

    if (this.onScoreChange) {
      this.onScoreChange(this.score, points);
    }

    return points;
  }

  /**
   * Add raw points without combo multiplier
   */
  addRawPoints(points) {
    this.score += points;

    if (this.onScoreChange) {
      this.onScoreChange(this.score, points);
    }

    return points;
  }

  /**
   * Increase combo multiplier
   */
  increaseCombo() {
    this.combo += 1;
    this.comboTimer = this.comboDecayTime;

    if (this.combo > this.maxCombo) {
      this.maxCombo = this.combo;
    }

    if (this.onComboChange) {
      this.onComboChange(this.combo);
    }
  }

  /**
   * Reset combo to 1
   */
  resetCombo() {
    if (this.combo > 1) {
      this.combo = 1;
      if (this.onComboChange) {
        this.onComboChange(this.combo);
      }
    }
  }

  /**
   * Update combo timer (call each frame)
   */
  update(dt) {
    if (this.combo > 1) {
      this.comboTimer -= dt;
      if (this.comboTimer <= 0) {
        this.resetCombo();
      }
    }
  }

  /**
   * Get remaining combo time (0-1 for UI display)
   */
  getComboTimeRemaining() {
    return Math.max(0, this.comboTimer / this.comboDecayTime);
  }

  /**
   * Get current score info
   */
  getScoreInfo() {
    return {
      score: this.score,
      combo: this.combo,
      maxCombo: this.maxCombo,
      comboTimeRemaining: this.getComboTimeRemaining(),
    };
  }

  /**
   * Finalize score for a mode and check if it's a top 5 score
   */
  finalizeScore(modeType) {
    const leaderboard = this.getLeaderboard(modeType);
    const rank = this.getRankForScore(modeType, this.score);
    const isHighScore = rank <= MAX_LEADERBOARD_ENTRIES && this.score > 0;

    return {
      finalScore: this.score,
      maxCombo: this.maxCombo,
      isHighScore,
      rank: isHighScore ? rank : null,
      topScore: leaderboard.length > 0 ? leaderboard[0].score : 0,
    };
  }

  /**
   * Get what rank a score would be (1-based)
   */
  getRankForScore(modeType, score) {
    const leaderboard = this.getLeaderboard(modeType);
    let rank = 1;
    for (const entry of leaderboard) {
      if (score > entry.score) break;
      rank++;
    }
    return rank;
  }

  /**
   * Check if score qualifies for leaderboard
   */
  isTopScore(modeType, score) {
    const rank = this.getRankForScore(modeType, score);
    return rank <= MAX_LEADERBOARD_ENTRIES && score > 0;
  }

  /**
   * Add a score to the leaderboard with initials
   */
  addHighScore(modeType, score, initials) {
    if (!this.highScores[modeType]) {
      this.highScores[modeType] = [];
    }

    // Sanitize initials (3 uppercase letters)
    const cleanInitials = (initials || 'AAA')
      .toUpperCase()
      .replace(/[^A-Z]/g, '')
      .slice(0, 3)
      .padEnd(3, 'A');

    // Add new entry
    this.highScores[modeType].push({
      initials: cleanInitials,
      score: score,
      date: Date.now(),
    });

    // Sort by score descending
    this.highScores[modeType].sort((a, b) => b.score - a.score);

    // Keep only top 5
    this.highScores[modeType] = this.highScores[modeType].slice(0, MAX_LEADERBOARD_ENTRIES);

    this.saveHighScores();

    if (this.onNewHighScore) {
      this.onNewHighScore(modeType, score, cleanInitials);
    }

    return this.getRankForScore(modeType, score);
  }

  /**
   * Get leaderboard for a mode
   */
  getLeaderboard(modeType) {
    return this.highScores[modeType] || [];
  }

  /**
   * Get top high score for a mode (for display)
   */
  getHighScore(modeType) {
    const leaderboard = this.getLeaderboard(modeType);
    return leaderboard.length > 0 ? leaderboard[0].score : 0;
  }

  /**
   * Get all high scores
   */
  getAllHighScores() {
    return { ...this.highScores };
  }

  /**
   * Load high scores from localStorage
   */
  loadHighScores() {
    try {
      const stored = localStorage.getItem(STORAGE_KEY);
      if (stored) {
        return JSON.parse(stored);
      }
    } catch (e) {
      console.warn('Failed to load high scores:', e);
    }
    return {};
  }

  /**
   * Save high scores to localStorage
   */
  saveHighScores() {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(this.highScores));
    } catch (e) {
      console.warn('Failed to save high scores:', e);
    }
  }

  /**
   * Clear all high scores (for testing)
   */
  clearHighScores() {
    this.highScores = {};
    this.saveHighScores();
  }
}
