/**
 * Input Mode Detection and Management
 *
 * Handles detection of touch vs desktop input and user preference storage.
 * Preference is stored in localStorage alongside high scores.
 */

const STORAGE_KEY = 'drift-input-mode';

export const InputMode = {
  DESKTOP: 'desktop',
  TOUCH: 'touch',
};

/**
 * Detect if device has touch capability
 */
function hasTouchCapability() {
  return (
    'ontouchstart' in window ||
    navigator.maxTouchPoints > 0 ||
    navigator.msMaxTouchPoints > 0
  );
}

/**
 * Detect if device is likely a mobile/tablet based on screen size
 */
function isMobileScreenSize() {
  return window.innerWidth <= 1024 || window.innerHeight <= 768;
}

/**
 * Get saved input mode preference
 */
export function getSavedInputMode() {
  try {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (saved && (saved === InputMode.DESKTOP || saved === InputMode.TOUCH)) {
      return saved;
    }
  } catch (e) {
    console.warn('Failed to load input mode preference:', e);
  }
  return null;
}

/**
 * Save input mode preference
 */
export function saveInputMode(mode) {
  try {
    localStorage.setItem(STORAGE_KEY, mode);
  } catch (e) {
    console.warn('Failed to save input mode preference:', e);
  }
}

/**
 * Auto-detect the best input mode for this device
 */
export function detectInputMode() {
  // Check for saved preference first
  const saved = getSavedInputMode();
  if (saved) {
    console.log('Using saved input mode:', saved);
    return saved;
  }

  // Auto-detect based on device capabilities
  const hasTouch = hasTouchCapability();
  const isMobile = isMobileScreenSize();

  // If device has touch AND is mobile-sized, default to touch mode
  // Desktop with touchscreen (like Surface) defaults to desktop mode
  const detected = (hasTouch && isMobile) ? InputMode.TOUCH : InputMode.DESKTOP;

  console.log('Auto-detected input mode:', detected, { hasTouch, isMobile });
  return detected;
}

/**
 * Get current input mode (detect if not saved)
 */
export function getInputMode() {
  return getSavedInputMode() || detectInputMode();
}

/**
 * Switch input mode and reload page
 */
export function switchInputMode() {
  const current = getInputMode();
  const newMode = current === InputMode.DESKTOP ? InputMode.TOUCH : InputMode.DESKTOP;
  saveInputMode(newMode);
  window.location.reload();
}

/**
 * Set specific input mode and reload
 */
export function setInputMode(mode) {
  if (mode !== InputMode.DESKTOP && mode !== InputMode.TOUCH) {
    console.warn('Invalid input mode:', mode);
    return;
  }
  saveInputMode(mode);
  window.location.reload();
}

/**
 * Get display info for current mode
 */
export function getInputModeInfo(mode) {
  if (mode === InputMode.TOUCH) {
    return {
      icon: '🖥️',  // Show desktop icon to indicate "switch to desktop"
      label: 'Touch',
      description: 'Touch controls for mobile/tablet',
    };
  }
  return {
    icon: '📱',  // Show mobile icon to indicate "switch to mobile"
    label: 'Desktop',
    description: 'Mouse and keyboard controls',
  };
}
