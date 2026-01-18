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

// Check if device has touch capability
function isTouchDevice() {
  return (
    'ontouchstart' in window ||
    navigator.maxTouchPoints > 0 ||
    navigator.msMaxTouchPoints > 0
  );
}

// Auto-detect quality based on device type and screen size
export function autoDetectQuality() {
  const width = window.innerWidth;
  const height = window.innerHeight;
  const minDim = Math.min(width, height);

  // Touch devices default to LOW for better performance
  // Users can upgrade quality manually if their device handles it well
  if (isTouchDevice()) {
    return QualityLevel.LOW;
  }

  // Desktop: use screen size to determine quality
  if (minDim < 500) {
    return QualityLevel.LOW;
  } else if (minDim < 800) {
    return QualityLevel.MEDIUM;
  } else {
    return QualityLevel.HIGH;
  }
}
