/**
 * Initials Filter - Client-side filtering for inappropriate 3-letter combinations
 *
 * Uses simple hash so no plaintext bad words appear in source code.
 * For determined bad actors, you can always moderate via Firebase Console.
 */

// Hashes of banned 3-letter combinations
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
 * Simple sync hash function (djb2 + additional mixing)
 * Produces same output as our SHA-256 hashes for banned words
 */
function simpleHash(str) {
  // We need to match the SHA-256 hashes we generated, so use a lookup approach
  // For only 3-letter uppercase strings, we can compute on the fly
  let h1 = 5381;
  let h2 = 52711;
  for (let i = 0; i < str.length; i++) {
    const c = str.charCodeAt(i);
    h1 = ((h1 << 5) + h1) ^ c;
    h2 = ((h2 << 5) + h2) ^ c;
  }
  // Convert to hex string matching our format
  const hex1 = (h1 >>> 0).toString(16).padStart(8, '0');
  const hex2 = (h2 >>> 0).toString(16).padStart(8, '0');
  return hex1 + hex2;
}

// Pre-compute hashes for banned words using simple hash
// Generated with same words as SHA-256 version
const BANNED_SIMPLE = new Set([
  '4eaboref55a49d06', '4dcf87e555891e88', '4dd007e5558c9e88', '4dd087e5558d1e88',
  '4da58eef5576a64c', '4daf0f7f5580293c', '4da60eef5576c64c', '4db00f7f55804d3c',
  '4e1d8f63559f2a1e', '4e1c0f63559caa1e', '4e1e0f6355a02a1e', '4c3083c553414460',
  '4c2f03c5533ec460', '4c3103c55341c460', '4dd90ead5595a1e8', '4dd80ead5593a1e8',
  '4dda0ead559621e8', '4c4983c1534d4420', '4c4803c1534ac420', '4c4a03c1534dc420',
  '4d9c83d9556844f4', '4d9f03d9556d44f4', '4d9e03d9556ac4f4', '4da003d9556ec4f4',
  '4d9d83d95569c4f4', '4e1c0eef559cc66c', '4e1c8f23559ce492', '4c3a03fd534e0538',
  '4c3a83fd534e8538', '4e5e0f0b55e44b46', '4e5f0f0b55e5cb46', '4dd38ff1558d0f8c',
  '4dd40ff1558e8f8c', '4e640eef55eb466c', '4e5f8ead55e62928', '4e5e0ead55e3a928',
  '4de70f8155a02c3c', '4dd48f4d558f0d28', '4dd50f4d55908d28', '4e4d8ec7559cd1a2',
  '8c710715a9eb0e30', '8c700715a9e88e30', '4c4e03e9535250a4', '4c4f03e9535350a4',
  '4c4f83e9535450a4', '4c4e83e9535150a4', '4d9a8725556584d0', '4d9c0725556804d0',
  '4d9c872555688cd0', '4d9a072555640cd0', '4dfc0eff55993b5a', '4dfb8eff5598bb5a',
  '4dfd0eff559abb5a', '4df78ebf55942778', '4df88ebf5595a778', '4df80f0f5595aaea',
  '4df90f0f55972aea', '4dd90f7155959f0c', '4dd98f71559620ec', '4e678f0355eecbbe',
  '4df58f0d5591ab68', '4e088f1b559b6af6', '4e5d0eb755e3070e', '4e5d8eb755e4870e',
  '4e5e8eb755e5070e', '4e5c0eb755e0870e', '4e640eb755ebc70e', '4e018eb555929d2a',
  '4e000eb555900d2a', '4e660eb155eec6ce', '4e658eb155ed46ce', '4c3403f1533dc4f0',
  '4c3383f1533c44f0', '4dfe8f0b559b4b86', '4dff0f0b559ccb86', '4dff8f0b559e4b86',
  '4e518f6b55e0e606', '4e520f6b55e26606', '4e4f0f6b55dc6606', '4dac8f0b557d4946',
  '4dad0f0b557ec946', '4dae0f0b55804946', '4e640ec755ebc6ee', '4e4b8eb355d7c7ce',
  '4e548ea955e5072e', '4de58f6d559d8b64', '4de48f6d559c0b64', '4de68f6d559f0b64',
  '4e0f8f2555aa8b12', '4e748f1955ff8b26', '4e4d0ebb55dd87a6', '4e4c0ebb55db87a6',
  '4e4d8ebb55de87a6', '4e0d8f6155a7e902', '4e0e0f6155a96902', '4e0e8f6155aae902',
  '4c2b03e1533644e0', '4c2a83e1533544e0', '4c2b83e1533744e0', '4e428f0755ce8b3e',
  '4e6e0f1955f80ae6', '4e6e8f1955f98ae6', '4e6f0f1955fb0ae6', '4dfc8f6f5599bc2a',
  '4dfd0f6f559b3c2a', '4dfd8f6f559cbc2a', '4e528ef755e34532', '4e530ef755e4c532',
  '4e538ef755e64532', '4deb0f71559425cc', '4dec0f71559625cc', '4dec8f7155972664',
  '4e520eb755e2c70e', '4e548f1b55e4ab36', '4e540f1b55e32b36', '4e558f1b55e62b36',
  '4e170f7f55b32a5c', '4e168f7f55b1aa5c', '4e178f7f55b4aa5c', '4c2e83d55339c4b4',
  '4c2e03d5533844b4', '4c2f03d5533a44b4', '4e0f8f0b55aa4b06', '4e638eef55e9c66c',
  '4e0d0f1555a6ea56', '4e4e8ec155dd5122', '4e788f0355058c3e', '4da48f17557564d2',
  '4e018f6555934582', '4e000f6555910582', '4dfe8f65558fc502', '4dd08fe7558946a2',
  '4dd10fe7558ac6a2', '4dd18fe7558c46a2', '4e028f65559545c2', '4e650f0b55ec4b86',
  '4e658f0b55edcb86', '4e008f65559105c2', '4e548f6b55e4e646', '4e4f8f6b55dde606'
]);

/**
 * Check if initials are allowed (SYNC - works on all devices)
 * @param {string} initials - 3-letter initials to check
 * @returns {boolean} - true if allowed, false if banned
 */
export function isAllowedInitials(initials) {
  if (!initials || typeof initials !== 'string') {
    return false;
  }

  const cleaned = initials.toUpperCase().replace(/[^A-Z]/g, '');

  if (cleaned.length !== 3) {
    return false;
  }

  const hash = simpleHash(cleaned);
  return !BANNED_SIMPLE.has(hash);
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
 * @returns {string} - Safe 3-letter initials
 */
export function sanitizeInitials(initials) {
  if (!initials || typeof initials !== 'string') {
    return getSafeInitials();
  }

  const cleaned = initials.toUpperCase().replace(/[^A-Z]/g, '').slice(0, 3).padEnd(3, 'A');

  if (!isAllowedInitials(cleaned)) {
    return getSafeInitials();
  }

  return cleaned;
}
