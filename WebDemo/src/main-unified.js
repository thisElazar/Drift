/**
 * Unified App Entry Point
 *
 * Combines desktop and mobile functionality with automatic input detection.
 * Supports both mouse/keyboard and touch controls.
 */

import { Scene } from './rendering/scene.js';
import { Terrain, TerrainPreset } from './simulation/terrain.js';
import { Water } from './simulation/water.js';
import { TerrainMesh } from './rendering/terrainMesh.js';
import { WaterMesh } from './rendering/waterMesh.js';
import { SpringMarkers } from './rendering/springMarkers.js';
import { TerrainControls } from './ui/controls.js';
import { MobileControls } from './ui/mobileControls.js';
import {
  InputMode,
  getInputMode,
  switchInputMode,
  getInputModeInfo,
} from './ui/inputMode.js';
import {
  QualityLevel,
  QualityConfig,
  QualityPresets,
  setQuality,
  loadSavedQuality,
  autoDetectQuality
} from './simulation/quality.js';
import { getMaxSimSteps } from './simulation/constants.js';
import { GameModeManager, GameModeType } from './game/GameModeManager.js';
import { ScoreManager } from './game/ScoreManager.js';
import { FlowRushMode } from './game/modes/FlowRushMode.js';
import { CascadeMode } from './game/modes/CascadeMode.js';
import { FloodDefenseMode } from './game/modes/FloodDefenseMode.js';

// Initialize quality BEFORE creating app
function initQuality() {
  const saved = loadSavedQuality();
  if (saved) {
    console.log('Loaded saved quality:', saved);
    return saved;
  }
  // Auto-detect based on screen size
  const detected = autoDetectQuality();
  setQuality(detected);
  console.log('Auto-detected quality:', detected);
  return detected;
}

const currentQuality = initQuality();
const inputMode = getInputMode();

class UnifiedApp {
  constructor() {
    this.inputMode = inputMode;
    this.container = document.getElementById('app');

    // Create systems
    this.scene = new Scene(this.container);
    this.terrain = new Terrain();
    this.water = new Water(this.terrain);
    this.terrainMesh = new TerrainMesh(this.terrain);
    this.waterMesh = new WaterMesh(this.water, this.terrain);
    this.springMarkers = new SpringMarkers(this.water, this.terrain);

    // Add meshes to scene
    this.scene.add(this.terrainMesh.object);
    this.scene.add(this.waterMesh.object);
    this.scene.add(this.springMarkers.object);

    // Create game mode manager
    this.gameModeManager = new GameModeManager(this.terrain, this.water, this.scene.scene);
    this.gameModeManager.registerMode(GameModeType.FLOW_RUSH, FlowRushMode);
    this.gameModeManager.registerMode(GameModeType.CASCADE, CascadeMode);
    this.gameModeManager.registerMode(GameModeType.FLOOD_DEFENSE, FloodDefenseMode);
    this.setupGameModeCallbacks();

    // Create controls based on input mode
    if (this.inputMode === InputMode.TOUCH) {
      this.controls = new MobileControls(this.scene, this.terrain, this.terrainMesh);
      this.setupMobileControlCallbacks();
      document.body.classList.add('touch-mode');
    } else {
      this.controls = new TerrainControls(this.scene, this.terrain, this.terrainMesh);
      this.setupDesktopControlCallbacks();
      document.body.classList.add('desktop-mode');
    }

    // Override terrain reset to also regenerate springs
    const originalReset = this.terrain.reset.bind(this.terrain);
    this.terrain.reset = () => {
      originalReset();
      this.water.reset();
      this.water.autoGenerateSprings();
    };

    // Auto-generate springs on startup
    this.water.autoGenerateSprings();

    // Setup UI
    this.setupPresetButtons();
    this.setupQualitySelector();
    this.setupInputModeToggle();
    this.setupHomeButton();
    this.setupModeSelector();
    this.setupPregameModal();

    // Simulation timing
    this.lastTime = performance.now();
    this.simAccumulator = 0;
    this.simStep = 1 / 60;

    // Time control
    this.timeScale = 1.0;
    this.paused = false;

    // FPS tracking
    this.frameCount = 0;
    this.fpsTime = 0;
    this.currentFps = 60;

    // Adaptive performance - throttle simulation when FPS drops
    this.adaptiveMaxSteps = getMaxSimSteps();  // Current limit
    this.targetFps = 30;  // Minimum acceptable FPS
    this.fpsHistory = [];  // Track recent FPS values
    this.fpsHistorySize = 10;
    this.adaptiveCooldown = 0;  // Frames before we can adjust again

    // Start render loop
    this.animate();

    // Show help on first visit for touch mode
    if (this.inputMode === InputMode.TOUCH) {
      this.showHelpIfFirstVisit();
    }

    console.log('Drift Unified Demo initialized');
    console.log('Input mode:', this.inputMode);
    console.log('Quality:', QualityConfig.level, '- Grid:', this.terrain.width + 'x' + this.terrain.height);
  }

  setupDesktopControlCallbacks() {
    this.controls.updateToolDisplay();

    this.controls.onAddWater = (x, y) => {
      const amount = this.terrain.brushStrength * 50;
      const radius = this.terrain.brushRadius;
      this.water.addWater(x, y, amount, radius);
    };
    this.controls.onAddSpring = (x, y) => {
      const flowRate = this.terrain.brushStrength * this.terrain.brushRadius * 0.5;
      this.water.addSpring(x, y, flowRate);
    };
    this.controls.onRemoveSpring = (x, y) => {
      this.water.removeSpringNear(x, y, this.terrain.brushRadius);
    };
    this.controls.onClearWater = () => {
      this.water.depth.fill(0);
      this.water.waveEnergy.fill(0);
      this.water.dirty = true;
    };
    this.controls.onTimeScaleChange = (delta) => {
      this.timeScale = Math.max(0.25, Math.min(20.0, this.timeScale + delta));
    };
    this.controls.onTogglePause = () => {
      this.paused = !this.paused;
    };
    this.controls.onResetCamera = () => {
      this.scene.resetCamera();
    };
  }

  setupMobileControlCallbacks() {
    this.controls.onAddWater = (x, y) => {
      const amount = this.terrain.brushStrength * 50;
      const radius = this.terrain.brushRadius;
      this.water.addWater(x, y, amount, radius);
    };
    this.controls.onAddSpring = (x, y) => {
      const flowRate = this.terrain.brushStrength * this.terrain.brushRadius * 0.5;
      this.water.addSpring(x, y, flowRate);
    };
    this.controls.onTerrainReset = () => {
      this.water.reset();
      this.water.autoGenerateSprings();
    };
  }

  setupPresetButtons() {
    const buttons = document.querySelectorAll('.preset-btn');
    const presetDropdown = document.getElementById('preset-dropdown');

    buttons.forEach(btn => {
      const handlePreset = (e) => {
        if (e) {
          e.preventDefault();
          e.stopPropagation();
        }
        buttons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        const preset = btn.dataset.preset;
        this.terrain.generate(preset);
        this.water.reset();
        this.water.autoGenerateSprings();
        if (presetDropdown) presetDropdown.classList.remove('visible');
      };

      btn.addEventListener('click', handlePreset);
      btn.addEventListener('touchend', handlePreset);
    });
  }

  setupQualitySelector() {
    // Handle both desktop and touch quality buttons
    const qualityBtns = [
      document.getElementById('btn-quality'),
      document.getElementById('btn-quality-touch')
    ].filter(Boolean);
    // Use appropriate dropdown for current input mode
    const qualityDropdown = this.inputMode === InputMode.TOUCH
      ? document.getElementById('quality-dropdown-touch')
      : document.getElementById('quality-dropdown');
    const qualityButtons = document.querySelectorAll('.quality-btn');

    if (qualityBtns.length === 0 || !qualityDropdown) return;

    // Update button text to show current quality
    const preset = QualityPresets[QualityConfig.level];
    const labelText = preset ? preset.label : 'Quality';
    qualityBtns.forEach(btn => {
      // Touch button just shows "Q", desktop shows full label
      if (btn.id === 'btn-quality-touch') {
        btn.textContent = 'Q';
        btn.title = labelText;
      } else {
        btn.textContent = labelText;
      }
    });

    // Update active state in dropdown
    qualityButtons.forEach(btn => {
      const level = btn.dataset.quality;
      btn.classList.toggle('active', level === QualityConfig.level);
    });

    // Toggle dropdown
    qualityBtns.forEach(qualityBtn => {
      const toggleDropdown = (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById('mode-dropdown')?.classList.remove('visible');
        document.getElementById('mode-dropdown-touch')?.classList.remove('visible');
        document.getElementById('preset-dropdown')?.classList.remove('visible');
        qualityDropdown.classList.toggle('visible');
      };
      qualityBtn.addEventListener('click', toggleDropdown);
      qualityBtn.addEventListener('touchend', toggleDropdown);
    });

    // Handle quality selection
    qualityButtons.forEach(btn => {
      const handleQuality = (e) => {
        e.preventDefault();
        e.stopPropagation();
        const level = btn.dataset.quality;

        if (level !== QualityConfig.level) {
          this.changeQuality(level);
          const newPreset = QualityPresets[level];
          const newLabel = newPreset ? newPreset.label : 'Quality';
          qualityBtns.forEach(qb => {
            if (qb.id === 'btn-quality-touch') {
              qb.title = newLabel;
            } else {
              qb.textContent = newLabel;
            }
          });
          qualityButtons.forEach(b => {
            b.classList.toggle('active', b.dataset.quality === level);
          });
        }
        qualityDropdown.classList.remove('visible');
      };
      btn.addEventListener('click', handleQuality);
      btn.addEventListener('touchend', handleQuality);
    });

    // Close dropdown when clicking elsewhere
    document.addEventListener('click', () => qualityDropdown.classList.remove('visible'));
    document.addEventListener('touchend', (e) => {
      setTimeout(() => {
        const isQualityBtn = qualityBtns.some(btn => btn === e.target);
        if (!qualityDropdown.contains(e.target) && !isQualityBtn) {
          qualityDropdown.classList.remove('visible');
        }
      }, 50);
    });
  }

  setupInputModeToggle() {
    // Handle both desktop and touch toggle buttons
    const toggleBtns = [
      document.getElementById('btn-input-mode'),
      document.getElementById('btn-input-mode-touch')
    ].filter(Boolean);

    const modeInfo = getInputModeInfo(this.inputMode);

    toggleBtns.forEach(btn => {
      btn.textContent = modeInfo.icon;
      btn.title = `Switch to ${this.inputMode === InputMode.DESKTOP ? 'Touch' : 'Desktop'} mode`;

      const handleToggle = (e) => {
        e.preventDefault();
        e.stopPropagation();
        switchInputMode();
      };
      btn.addEventListener('click', handleToggle);
      btn.addEventListener('touchend', handleToggle);
    });
  }

  changeQuality(level) {
    // End any active game mode first
    if (this.gameModeManager.isPlaying) {
      this.gameModeManager.endMode(false);
      this.showArcadeHud(false);
    }

    // Take snapshots of current state
    const terrainSnapshot = this.terrain.getSnapshot();
    const waterSnapshot = this.water.getSnapshot();

    // Apply new quality settings
    setQuality(level);

    // Recreate terrain with new grid size
    this.terrain = new Terrain();
    this.terrain.loadFromSnapshot(terrainSnapshot);

    // Recreate water with new grid size
    this.water = new Water(this.terrain);
    this.water.loadFromSnapshot(waterSnapshot);

    // Rebuild meshes with new dimensions
    this.terrainMesh.terrain = this.terrain;
    this.terrainMesh.rebuild();

    this.waterMesh.water = this.water;
    this.waterMesh.terrain = this.terrain;
    this.waterMesh.rebuild();

    this.springMarkers.water = this.water;
    this.springMarkers.terrain = this.terrain;
    this.springMarkers.refreshScale();

    // Update controls with new terrain reference
    this.controls.terrain = this.terrain;
    this.controls.terrainMesh = this.terrainMesh;
    if (this.controls.refreshScale) {
      this.controls.refreshScale();
    }

    // Update game mode manager with new references
    this.gameModeManager.terrain = this.terrain;
    this.gameModeManager.water = this.water;

    console.log(`Quality changed to ${level}. Grid size: ${this.terrain.width}x${this.terrain.height}`);
  }

  setupHomeButton() {
    // Handle both desktop and touch home buttons
    const homeBtns = [
      document.getElementById('btn-home'),
      document.getElementById('btn-home-touch')
    ].filter(Boolean);

    homeBtns.forEach(btn => {
      const handleHome = (e) => {
        e.preventDefault();
        e.stopPropagation();
        this.scene.resetCamera();
      };
      btn.addEventListener('click', handleHome);
      btn.addEventListener('touchend', handleHome);
    });

    // Setup help overlay close button
    const closeHelpBtn = document.getElementById('close-help');
    if (closeHelpBtn) {
      const handleClose = (e) => {
        e.preventDefault();
        const helpOverlay = document.getElementById('help-overlay');
        if (helpOverlay) helpOverlay.classList.remove('visible');
      };
      closeHelpBtn.addEventListener('click', handleClose);
      closeHelpBtn.addEventListener('touchend', handleClose);
    }

    // Setup new map button for touch mode
    const newMapBtn = document.getElementById('btn-new-map');
    const presetDropdown = document.getElementById('preset-dropdown');
    if (newMapBtn && presetDropdown) {
      newMapBtn.onclick = (e) => {
        e.preventDefault();
        document.getElementById('quality-dropdown-touch')?.classList.remove('visible');
        document.getElementById('mode-dropdown-touch')?.classList.remove('visible');
        presetDropdown.classList.toggle('visible');
      };
    }
  }

  // Mode descriptions for pre-game modal
  getModeInfo(modeType) {
    const modeInfo = {
      flowRush: {
        icon: '🎯',
        title: 'Flow Rush',
        subtitle: '2 minutes to score big',
        description: 'Guide water to glowing targets scattered across the terrain. Build channels and redirect springs to maximize your score!',
        objectives: [
          'Water touching targets earns points',
          'Chain hits to build your combo multiplier',
          'Use terrain tools to guide water flow',
          'Find all targets before time runs out'
        ]
      },
      cascade: {
        icon: '💧',
        title: 'Cascade',
        subtitle: 'Chain reactions for massive combos',
        description: 'Water must flow through rings in sequence. Each ring you hit adds to your combo - miss the sequence and start over!',
        objectives: [
          'Guide water through the highlighted ring',
          'Rings must be hit in order (follow the sequence)',
          'Longer chains = bigger score multipliers',
          'Plan your terrain to create smooth water paths'
        ]
      },
      floodDefense: {
        icon: '🏘️',
        title: 'Flood Defense',
        subtitle: 'Protect the villages from rising water',
        description: 'Aggressive springs threaten villages in the valleys. Build barriers and redirect floods to keep settlements safe!',
        objectives: [
          'Villages spawn in low ground - protect them!',
          'Springs appear on high ground and flood downhill',
          'Flooded villages damage your health over time',
          'New villages and springs appear as you play'
        ]
      }
    };
    return modeInfo[modeType] || null;
  }

  showPregameModal(modeType) {
    const modal = document.getElementById('pregame-modal');
    const info = this.getModeInfo(modeType);

    if (!modal || !info) return;

    modal.querySelector('.mode-icon').textContent = info.icon;
    modal.querySelector('.mode-title').textContent = info.title;
    modal.querySelector('.mode-subtitle').textContent = info.subtitle;
    modal.querySelector('.mode-description').textContent = info.description;

    const objectivesList = modal.querySelector('.mode-objectives ul');
    objectivesList.innerHTML = info.objectives.map(obj => `<li>${obj}</li>`).join('');

    this.pendingGameMode = modeType;
    modal.classList.add('visible');
  }

  hidePregameModal() {
    const modal = document.getElementById('pregame-modal');
    if (modal) {
      modal.classList.remove('visible');
    }
  }

  setupPregameModal() {
    const modal = document.getElementById('pregame-modal');
    if (!modal) return;

    const startBtn = modal.querySelector('.start-btn');
    if (startBtn) {
      const handleStart = (e) => {
        e.preventDefault();
        if (this.pendingGameMode) {
          this.hidePregameModal();
          const started = this.gameModeManager.startMode(this.pendingGameMode);
          if (!started) {
            console.log(`Mode "${this.pendingGameMode}" not yet implemented`);
          }
          this.pendingGameMode = null;
        }
      };
      startBtn.addEventListener('click', handleStart);
      startBtn.addEventListener('touchend', handleStart);
    }
  }

  setupGameModeCallbacks() {
    this.gameModeManager.onModeStart = (modeType) => {
      if (modeType !== GameModeType.SANDBOX) {
        this.resetGameState();
        this.showArcadeHud(true);
      }
    };

    this.gameModeManager.onModeEnd = (modeType, completed, finalScore, stats) => {
      if (modeType !== GameModeType.SANDBOX) {
        this.showGameOver(finalScore, stats);
      }
    };

    this.gameModeManager.onScoreUpdate = (score, combo) => {
      this.updateArcadeHud({ score, combo });
    };
  }

  setupModeSelector() {
    // Handle both desktop and touch mode buttons
    const modeBtns = [
      document.getElementById('btn-mode'),
      document.getElementById('btn-mode-touch')
    ].filter(Boolean);
    // Use appropriate dropdown for current input mode
    const modeDropdown = this.inputMode === InputMode.TOUCH
      ? document.getElementById('mode-dropdown-touch')
      : document.getElementById('mode-dropdown');
    const modeButtons = document.querySelectorAll('.mode-btn');

    if (modeBtns.length === 0 || !modeDropdown) return;

    this.updateHighScoreDisplay();

    // Toggle dropdown from any mode button
    modeBtns.forEach(modeBtn => {
      const toggleDropdown = (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById('quality-dropdown')?.classList.remove('visible');
        document.getElementById('quality-dropdown-touch')?.classList.remove('visible');
        document.getElementById('preset-dropdown')?.classList.remove('visible');
        modeDropdown.classList.toggle('visible');
      };
      modeBtn.addEventListener('click', toggleDropdown);
      modeBtn.addEventListener('touchend', toggleDropdown);
    });

    // Handle mode selection
    modeButtons.forEach(btn => {
      const handleMode = (e) => {
        e.preventDefault();
        e.stopPropagation();
        const mode = btn.dataset.mode;

        modeButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        const modeName = btn.querySelector('.mode-name').textContent;
        // Update all mode buttons
        modeBtns.forEach(mb => mb.textContent = modeName);
        modeDropdown.classList.remove('visible');

        if (mode === 'sandbox') {
          this.gameModeManager.endMode(false);
          this.showArcadeHud(false);
        } else {
          this.showPregameModal(mode);
        }
      };
      btn.addEventListener('click', handleMode);
      btn.addEventListener('touchend', handleMode);
    });

    // Close dropdown when clicking elsewhere
    document.addEventListener('click', () => modeDropdown.classList.remove('visible'));
    document.addEventListener('touchend', (e) => {
      setTimeout(() => {
        const isModeBtn = modeBtns.some(btn => btn === e.target);
        if (!modeDropdown.contains(e.target) && !isModeBtn) {
          modeDropdown.classList.remove('visible');
        }
      }, 50);
    });

    // Setup game buttons
    const restartBtn = document.querySelector('#arcade-hud .restart-btn');
    if (restartBtn) {
      const handleRestart = (e) => {
        e.preventDefault();
        this.restartCurrentMode();
      };
      restartBtn.addEventListener('click', handleRestart);
      restartBtn.addEventListener('touchend', handleRestart);
    }

    const quitBtn = document.querySelector('#arcade-hud .quit-btn');
    if (quitBtn) {
      const handleQuit = (e) => {
        e.preventDefault();
        this.quitToSandbox();
      };
      quitBtn.addEventListener('click', handleQuit);
      quitBtn.addEventListener('touchend', handleQuit);
    }

    this.setupHighScoreEntry();
  }

  setupHighScoreEntry() {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const inputs = hud.querySelectorAll('.initials-input .initial');
    const submitBtn = hud.querySelector('.submit-score-btn');

    inputs.forEach((input, index) => {
      input.addEventListener('input', (e) => {
        e.target.value = e.target.value.replace(/[^a-zA-Z]/g, '').toUpperCase();
        if (e.target.value.length === 1 && index < inputs.length - 1) {
          inputs[index + 1].focus();
        }
      });

      input.addEventListener('keydown', (e) => {
        if (e.key === 'Backspace' && e.target.value === '' && index > 0) {
          inputs[index - 1].focus();
        }
        if (e.key === 'Enter') {
          submitBtn.click();
        }
      });
    });

    const handleSubmit = (e) => {
      e.preventDefault();
      const initials = Array.from(inputs).map(i => i.value || 'A').join('');
      this.submitHighScore(initials);
    };
    submitBtn.addEventListener('click', handleSubmit);
    submitBtn.addEventListener('touchend', handleSubmit);
  }

  async submitHighScore(initials) {
    const scoreManager = new ScoreManager();
    const modeType = this.gameModeManager.lastPlayedMode;
    const score = this.pendingHighScore;

    if (modeType && score > 0) {
      await scoreManager.addHighScore(modeType, score, initials);
      this.updateHighScoreDisplay();
      this.showLeaderboard(modeType, score);
    }

    const hud = document.getElementById('arcade-hud');
    if (hud) {
      hud.querySelector('.highscore-entry').classList.remove('visible');
      hud.querySelector('.game-buttons').style.display = 'flex';
    }
  }

  showLeaderboard(modeType, highlightScore = null) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const scoreManager = new ScoreManager();
    const localLeaderboard = scoreManager.getLeaderboard(modeType);
    const globalListEl = hud.querySelector('.leaderboard-list[data-tab-content="global"]');
    const localListEl = hud.querySelector('.leaderboard-list[data-tab-content="local"]');
    const statusEl = hud.querySelector('.leaderboard-status');
    const tabs = hud.querySelectorAll('.leaderboard-tab');

    // Render local leaderboard
    localListEl.innerHTML = this.renderLeaderboardList(localLeaderboard, highlightScore);

    // Set up tab switching
    tabs.forEach(tab => {
      tab.onclick = () => {
        tabs.forEach(t => t.classList.remove('active'));
        tab.classList.add('active');
        const tabName = tab.dataset.tab;
        globalListEl.style.display = tabName === 'global' ? '' : 'none';
        localListEl.style.display = tabName === 'local' ? '' : 'none';
      };
    });

    // Handle global leaderboard
    if (scoreManager.isGlobalLeaderboardAvailable()) {
      globalListEl.innerHTML = '<li style="color: #666; text-align: center;">Loading...</li>';
      statusEl.textContent = '';

      // Fetch global scores
      scoreManager.fetchGlobalLeaderboard(modeType).then(globalLeaderboard => {
        if (globalLeaderboard.length > 0) {
          globalListEl.innerHTML = this.renderLeaderboardList(globalLeaderboard, highlightScore);
        } else {
          globalListEl.innerHTML = '<li style="color: #666; text-align: center;">No scores yet - be the first!</li>';
        }
      }).catch(() => {
        globalListEl.innerHTML = '<li style="color: #666; text-align: center;">Could not load global scores</li>';
      });

      // Default to global tab
      tabs[0].classList.add('active');
      tabs[1].classList.remove('active');
      globalListEl.style.display = '';
      localListEl.style.display = 'none';
    } else {
      // Firebase not configured - show local only
      globalListEl.innerHTML = '<li style="color: #666; text-align: center;">Global scores unavailable</li>';
      statusEl.textContent = 'Local scores only';

      // Default to local tab when global unavailable
      tabs[0].classList.remove('active');
      tabs[1].classList.add('active');
      globalListEl.style.display = 'none';
      localListEl.style.display = '';
    }

    hud.querySelector('.leaderboard').classList.add('visible');
  }

  renderLeaderboardList(leaderboard, highlightScore = null) {
    if (leaderboard.length === 0) {
      return '<li style="color: #666; text-align: center;">No scores yet</li>';
    }

    return leaderboard.map((entry, index) => {
      const isHighlight = entry.score === highlightScore;
      return `
        <li class="${isHighlight ? 'highlight' : ''}">
          <span class="rank">${index + 1}.</span>
          <span class="initials">${entry.initials}</span>
          <span class="score">${entry.score.toLocaleString()}</span>
        </li>
      `;
    }).join('');
  }

  updateHighScoreDisplay() {
    const scoreManager = new ScoreManager();
    const modes = ['flowRush', 'cascade', 'floodDefense'];

    modes.forEach(mode => {
      const score = scoreManager.getHighScore(mode);
      const text = score > 0 ? `Best: ${score}` : '';
      // Update both desktop and touch high score displays
      const el = document.getElementById(`highscore-${mode}`);
      if (el) el.textContent = text;
      const elTouch = document.getElementById(`highscore-${mode}-touch`);
      if (elTouch) elTouch.textContent = text;
    });
  }

  showArcadeHud(visible) {
    const hud = document.getElementById('arcade-hud');
    if (hud) {
      hud.classList.toggle('visible', visible);
      if (visible) {
        hud.querySelector('.game-over').style.display = 'none';
        hud.querySelector('.final-score').style.display = 'none';
        hud.querySelector('.new-high-score').style.display = 'none';
        hud.querySelector('.game-buttons').style.display = 'none';
        hud.querySelector('.highscore-entry').classList.remove('visible');
        hud.querySelector('.leaderboard').classList.remove('visible');
        hud.querySelector('.score').textContent = '0';
        hud.querySelector('.combo').textContent = 'x1';
        hud.querySelector('.combo-bar-fill').style.width = '0%';
        hud.querySelector('.timer').textContent = '2:00';
        hud.querySelector('.timer').classList.remove('urgent');
        hud.querySelector('.lives').innerHTML = '';
        this.pendingHighScore = null;
      }
    }
  }

  updateArcadeHud(info) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    if (info.score !== undefined) {
      hud.querySelector('.score').textContent = info.score;
    }
    if (info.combo !== undefined) {
      hud.querySelector('.combo').textContent = `x${info.combo}`;
    }
    if (info.comboTimer !== undefined) {
      hud.querySelector('.combo-bar-fill').style.width = `${info.comboTimer * 100}%`;
    }
    if (info.quarterHearts !== undefined) {
      const livesEl = hud.querySelector('.lives');
      livesEl.innerHTML = this.renderQuarterHearts(info.quarterHearts, info.maxQuarterHearts);
    } else if (info.lives !== undefined) {
      const livesEl = hud.querySelector('.lives');
      livesEl.textContent = Array(info.lives).fill('\u2764').join(' ');
    }
    if (info.timeRemaining !== undefined) {
      const timerEl = hud.querySelector('.timer');
      const minutes = Math.floor(info.timeRemaining / 60);
      const seconds = Math.floor(info.timeRemaining % 60);
      timerEl.textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
      timerEl.classList.toggle('urgent', info.timeRemaining < 30);
    }
  }

  showGameOver(finalScore, stats) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const gameOverEl = hud.querySelector('.game-over');
    if (this.gameModeManager.currentMode && this.gameModeManager.currentMode.victory) {
      gameOverEl.textContent = "TIME'S UP!";
      gameOverEl.style.color = '#88ccff';
    } else {
      gameOverEl.textContent = 'GAME OVER';
      gameOverEl.style.color = '#ff6666';
    }
    gameOverEl.style.display = 'block';

    hud.querySelector('.final-score').style.display = 'block';
    hud.querySelector('.final-score').textContent = `Final Score: ${finalScore}`;

    const scoreManager = new ScoreManager();
    const modeType = this.gameModeManager.lastPlayedMode || this.gameModeManager.currentModeType;
    const isTopScore = scoreManager.isTopScore(modeType, finalScore);

    if (isTopScore) {
      hud.querySelector('.new-high-score').style.display = 'block';
      hud.querySelector('.highscore-entry').classList.add('visible');
      hud.querySelector('.game-buttons').style.display = 'none';

      this.pendingHighScore = finalScore;

      const inputs = hud.querySelectorAll('.initials-input .initial');
      inputs.forEach(i => i.value = '');
      inputs[0].focus();

      const rank = scoreManager.getRankForScore(modeType, finalScore);
      hud.querySelector('.rank-text').textContent = `#${rank} - Enter your initials:`;
    } else {
      hud.querySelector('.game-buttons').style.display = 'flex';
      if (scoreManager.getLeaderboard(modeType).length > 0) {
        this.showLeaderboard(modeType);
      }
    }
  }

  restartCurrentMode() {
    const modeToRestart = this.gameModeManager.lastPlayedMode;
    if (modeToRestart) {
      this.hideGameOverUI();
      this.showPregameModal(modeToRestart);
    }
  }

  hideGameOverUI() {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    hud.querySelector('.game-over').style.display = 'none';
    hud.querySelector('.final-score').style.display = 'none';
    hud.querySelector('.new-high-score').style.display = 'none';
    hud.querySelector('.highscore-entry').classList.remove('visible');
    hud.querySelector('.leaderboard').classList.remove('visible');
    hud.querySelector('.game-buttons').style.display = 'none';
    hud.classList.remove('visible');
  }

  quitToSandbox() {
    this.showArcadeHud(false);
    this.gameModeManager.endMode(false);

    // Update all mode buttons (desktop and touch)
    const modeBtns = [
      document.getElementById('btn-mode'),
      document.getElementById('btn-mode-touch')
    ].filter(Boolean);
    modeBtns.forEach(btn => btn.textContent = 'Sandbox');

    const modeButtons = document.querySelectorAll('.mode-btn');
    modeButtons.forEach(btn => {
      btn.classList.toggle('active', btn.dataset.mode === 'sandbox');
    });
  }

  resetGameState() {
    this.timeScale = 1.0;
    this.paused = false;
    this.terrain.brushRadius = 8;
    this.terrain.brushStrength = 2;

    if (this.inputMode === InputMode.DESKTOP) {
      this.controls.currentTool = 'raise';
      this.controls.updateToolDisplay();
    } else {
      this.controls.currentTool = 'raise';
      // Update touch mode tool button selection
      const toolBtns = document.querySelectorAll('.tool-btn');
      toolBtns.forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tool === 'raise');
      });
    }
  }

  renderQuarterHearts(quarterHearts, maxQuarterHearts) {
    const totalHearts = Math.ceil(maxQuarterHearts / 4);
    let html = '';

    for (let i = 0; i < totalHearts; i++) {
      const quartersForThisHeart = Math.max(0, Math.min(4, quarterHearts - i * 4));

      if (quartersForThisHeart === 4) {
        html += '<span class="heart full">\u2764</span>';
      } else if (quartersForThisHeart === 3) {
        html += '<span class="heart three-quarter">\u2764</span>';
      } else if (quartersForThisHeart === 2) {
        html += '<span class="heart half">\u2764</span>';
      } else if (quartersForThisHeart === 1) {
        html += '<span class="heart quarter">\u2764</span>';
      } else {
        html += '<span class="heart empty">\u2661</span>';
      }
    }

    return html;
  }

  showHelpIfFirstVisit() {
    const hasVisited = localStorage.getItem('drift-unified-visited');
    if (!hasVisited) {
      const helpOverlay = document.getElementById('help-overlay');
      if (helpOverlay) {
        helpOverlay.classList.add('visible');
      }
      localStorage.setItem('drift-unified-visited', 'true');
    }
  }

  animate() {
    requestAnimationFrame(() => this.animate());

    const now = performance.now();
    const rawDt = (now - this.lastTime) / 1000;
    this.lastTime = now;

    const dt = this.paused ? 0 : rawDt * this.timeScale;

    // FPS calculation
    this.frameCount++;
    this.fpsTime += rawDt;
    if (this.fpsTime >= 0.5) {
      this.currentFps = this.frameCount / this.fpsTime;
      this.frameCount = 0;
      this.fpsTime = 0;

      // Track FPS history for adaptive performance
      this.fpsHistory.push(this.currentFps);
      if (this.fpsHistory.length > this.fpsHistorySize) {
        this.fpsHistory.shift();
      }

      // Adaptive performance adjustment
      if (this.adaptiveCooldown > 0) {
        this.adaptiveCooldown--;
      } else if (this.fpsHistory.length >= 3) {
        const avgFps = this.fpsHistory.reduce((a, b) => a + b, 0) / this.fpsHistory.length;
        const minFps = Math.min(...this.fpsHistory.slice(-3));
        const baseMaxSteps = getMaxSimSteps();

        if (minFps < this.targetFps && this.adaptiveMaxSteps > 1) {
          // FPS too low - reduce simulation steps
          this.adaptiveMaxSteps = Math.max(1, this.adaptiveMaxSteps - 1);
          this.adaptiveCooldown = 4;  // Wait before adjusting again
          console.log(`Adaptive: FPS ${minFps.toFixed(0)} < ${this.targetFps}, reducing steps to ${this.adaptiveMaxSteps}`);
        } else if (avgFps > this.targetFps + 15 && this.adaptiveMaxSteps < baseMaxSteps) {
          // FPS recovered - gradually increase steps
          this.adaptiveMaxSteps = Math.min(baseMaxSteps, this.adaptiveMaxSteps + 1);
          this.adaptiveCooldown = 6;
          console.log(`Adaptive: FPS ${avgFps.toFixed(0)} good, increasing steps to ${this.adaptiveMaxSteps}`);
        }
      }

      // Update stats (desktop or mobile method)
      if (this.controls.updateStats) {
        if (this.inputMode === InputMode.DESKTOP) {
          this.controls.updateStats(
            this.currentFps,
            this.water.getTotalWater(),
            this.water.springs.length,
            this.timeScale,
            this.paused
          );
        } else {
          this.controls.updateStats(
            this.currentFps,
            this.water.getTotalWater(),
            this.water.springs.length
          );
        }
      }
    }

    // Simulation with adaptive step limiting
    this.simAccumulator += dt;
    let steps = 0;
    while (this.simAccumulator >= this.simStep && steps < this.adaptiveMaxSteps) {
      this.water.simulate(this.simStep);
      this.simAccumulator -= this.simStep;
      steps++;
    }
    // Prevent accumulator from growing unbounded when throttled
    if (this.simAccumulator > this.simStep * 4) {
      this.simAccumulator = this.simStep * 2;
    }

    // Update game mode
    this.gameModeManager.update(dt);

    // Update arcade HUD
    if (this.gameModeManager.isPlaying && this.gameModeManager.currentMode) {
      const info = this.gameModeManager.currentMode.getDisplayInfo();
      this.updateArcadeHud(info);
    }

    // Update camera (desktop only has fly controls)
    if (this.inputMode === InputMode.DESKTOP && this.controls.updateCamera) {
      this.controls.updateCamera(rawDt);
    }

    // Update meshes
    this.terrainMesh.update();
    this.waterMesh.update(dt);
    this.springMarkers.update(dt);

    // Update scene
    this.scene.update(dt);

    // Render
    this.scene.render();
  }
}

// Start app when DOM is ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => new UnifiedApp());
} else {
  new UnifiedApp();
}
