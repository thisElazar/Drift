// Quality presets for different devices
export const QualityLevel = {
  LOW: 'low',
  MEDIUM: 'medium',
  HIGH: 'high',
};

export const QualityPresets = {
  [QualityLevel.LOW]: {
    gridSize: 128,
    worldScale: 16,      // Larger scale to keep similar world size
    maxSimSteps: 2,      // Fewer sim steps per frame
    label: 'Low',
  },
  [QualityLevel.MEDIUM]: {
    gridSize: 192,
    worldScale: 11,
    maxSimSteps: 3,
    label: 'Medium',
  },
  [QualityLevel.HIGH]: {
    gridSize: 256,
    worldScale: 8,
    maxSimSteps: 4,
    label: 'High',
  },
};

// Current quality settings (mutable)
export const QualityConfig = {
  level: QualityLevel.HIGH,
  gridWidth: 256,
  gridHeight: 256,
  gridSize: 256 * 256,
  worldScale: 8,
  maxSimSteps: 4,
};

// Apply a quality preset
export function setQuality(level) {
  const preset = QualityPresets[level];
  if (!preset) return false;

  QualityConfig.level = level;
  QualityConfig.gridWidth = preset.gridSize;
  QualityConfig.gridHeight = preset.gridSize;
  QualityConfig.gridSize = preset.gridSize * preset.gridSize;
  QualityConfig.worldScale = preset.worldScale;
  QualityConfig.maxSimSteps = preset.maxSimSteps;

  // Save preference
  try {
    localStorage.setItem('drift-quality', level);
  } catch (e) {}

  return true;
}

// Load saved quality preference
export function loadSavedQuality() {
  try {
    const saved = localStorage.getItem('drift-quality');
    if (saved && QualityPresets[saved]) {
      setQuality(saved);
      return saved;
    }
  } catch (e) {}
  return null;
}

// Auto-detect quality based on screen size
export function autoDetectQuality() {
  const width = window.innerWidth;
  const height = window.innerHeight;
  const minDim = Math.min(width, height);

  if (minDim < 500) {
    return QualityLevel.LOW;      // Phone
  } else if (minDim < 800) {
    return QualityLevel.MEDIUM;   // Small tablet
  } else {
    return QualityLevel.HIGH;     // Large tablet / desktop
  }
}
