/**
 * Initials Filter - Client-side filtering for inappropriate 3-letter combinations
 *
 * Uses SHA-256 hashes so no plaintext bad words appear in source code.
 * For determined bad actors, you can always moderate via Firebase Console.
 */

// SHA-256 hashes (first 16 chars) of banned 3-letter combinations
// The actual words are not stored - only their hashes
const BANNED_HASHES = new Set([
  'c4ad8a7254f70ee4', 'b7484d3d7e4c9f01', '18474752a01a5a4f', '39d0110835048f76',
  '9fd24623bb92e9ef', 'eaab8e9d943582e5', '7d1139b784f03605', '1e767036a7f801c0',
  '03bc4735ec470681', '9422dc6bab18c4e5', '7557bd0494547ba9', 'f04736a51b45f491',
  '1b5d8169782cea1b', '902d44be57ca9ca6', '1999e8c3147794ba', '05305c8bd483f0b9',
  '0205ff7e26842c17', 'eb8f3e968adc6b41', '803859f5b9d6d170', '36a084fe6876a9c8',
  'eefc1369dbab3a92', '79f60db79c9619db', '7ff36ed30f4fa3aa', '84633a240977d118',
  '8ad8fd1c58decd94', '8436495162969e0e', 'cfa060f6b40ee052', 'ffaff1a2e6f36e3d',
  '9e916db353fb0e44', '7e2edab52cf2e979', 'd318b1ca4b5c25e8', '2fa210d13e5f0b01',
  '1ac236fbcd0ee716', '20f4f91e8242f223', 'c29a65e80c650d3e', '2e395b2cfad442f5',
  'f1e17cfb612841b0', 'e366fa6f48e1821b', 'd8f390b0a37bfbb9', '23de9b0a34210ace',
  '481327936d8a1555', '0b96ed52fc93d0ec', 'fbb516cf3421f712', 'd9869d3ebf1bb476',
  '7f890999e8a611f6', '21db519929fc5b59', '76b4e609df542f0a', '4bfdc6fd836975f3',
  '5424cb881e62854b', 'e7488880aedeeffd', '03db05f029ccf155', '94adad8da0405a63',
  'b8907d5593f577d0', 'c2d33913e60d2126', 'a6c1dbb7d91a3fdd', '0296276e01262f8f',
  '32c1c0b248f975d9', '3e07ff992c5faac9', '00809f7c597be78f', '212ec9cf3342454a',
  'ae325615091f3c8f', 'dcb42aa58b195599', '17503276f6480544', '5c31b1c1eda5a9b7',
  'b4bed028f83ea70b', '3d9c8f99f3e1eb6d', '9374bf57dd9175b9', '485ad4877b741fa5',
  '860260bb43785187', '9b38b8f5877f2395', '761eab44c73399a8', '8c070c91afd8b8c0',
  '1b506f6d1b840a02', '7fded0790922bf03', '7000f86bf7676fd4', '7811935386c250b0',
  'aae38437bb09c1ec', '16a22f11396e6e5d', 'c2a47e99b09c0b20', 'f9e4565116d58a61',
  '1bd4cc82ae481867', '228799331d2dc61e', '5a4faafed6c71a21', '8e38f23ddefe3e9d',
  'a6cc9afab88c4343', '1ac94ca8ca267cc6', '06de0416e5c5bdd5', '86e56f4bdfee48cb',
  'e06559c5367cb0de', 'd4b3526907e78576', 'e627ed6019c120f8', '6a780a2e122c630c',
  '2e169101f7bb780e', 'df084ddb40d35db8', '176159d61c93085f', 'd16d4a33fcb57f28',
  '194fbb80dfc13d80', 'c05643ba705154fb', '66f2eb694ffd614a', '63dc78c16cccb7cb',
  '3936af7f5716736b', '015ffb03d242fb94', '1e2b91e505aa21ce', '4a0f8e2115ae2a71',
  '0cbaa22a4a591310', '84cc42a10fa61ec6', '6a9d91436ad27071', '8ba328b91dc62476',
  '633473c683bdb561', 'ca95bb1202f047d1', 'ce18ab3497c461ea', 'd5e712956312ae10',
  '9485d541d591e3a6', 'ac8ab0f6ff3278da', 'daae09669f6ec837', 'ca36cd3eaf5a3a94',
  'ecdd3e4a16f4cd30', '1aefaf5cf220a087', '2d03cd5822cdf9f7', 'bc360b78fed60880',
  '6aa6b5e96601657f', '97a0fc6b1231a84c', 'ea04b3254ce3eb98', '86e73ded897be64b',
  '914c72379944278e', 'b680b32490540916', '2e276fb235ce3fe8', 'e192ccf435cf931d',
  '07912ee9ef3c8b92', 'ba245ec07310e44f', '55663e39e1052b02', 'c3487215b4b0a4c2',
  '2f59df49dab4f87a', '6d9c15e601ed23b5', 'f8efea9a9dbdfc88', '6b491378c01a44ea',
  '42391b910f4e2f5d', 'd5eccccb32ef1508', 'ea3c440bf8afc1a1', 'c9cdc457ebe981d5',
  'b31ca9b1e7e0a9a2', 'ac0b51236e63abc4', '9b17a298fa5efe8c', '933bb1103d6de889',
  '682aabd428fbbb55', 'ede54bb33278d3bf', 'bc00c05cd503791b', 'd5ac2a301efc5801',
  '0ceaa396e2134a99', 'a4281a7f5aa683fd'
]);

/**
 * Hash a string using SHA-256 (first 16 hex chars)
 */
async function hashInitials(initials) {
  const encoder = new TextEncoder();
  const data = encoder.encode(initials);
  const hashBuffer = await crypto.subtle.digest('SHA-256', data);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
  return hashHex.slice(0, 16);
}

/**
 * Check if initials are allowed (async due to hashing)
 * @param {string} initials - 3-letter initials to check
 * @returns {Promise<boolean>} - true if allowed, false if banned
 */
export async function isAllowedInitials(initials) {
  if (!initials || typeof initials !== 'string') {
    return false;
  }

  const cleaned = initials.toUpperCase().replace(/[^A-Z]/g, '');

  if (cleaned.length !== 3) {
    return false;
  }

  const hash = await hashInitials(cleaned);
  return !BANNED_HASHES.has(hash);
}

/**
 * Generate safe replacement initials
 * @returns {string} - Safe 3-letter initials
 */
export function getSafeInitials() {
  return 'AAA';
}

/**
 * Sanitize initials - returns cleaned initials or safe replacement if banned
 * @param {string} initials - Raw initials input
 * @returns {Promise<string>} - Safe 3-letter initials
 */
export async function sanitizeInitials(initials) {
  if (!initials || typeof initials !== 'string') {
    return getSafeInitials();
  }

  const cleaned = initials.toUpperCase().replace(/[^A-Z]/g, '').slice(0, 3).padEnd(3, 'A');

  if (!(await isAllowedInitials(cleaned))) {
    return getSafeInitials();
  }

  return cleaned;
}
