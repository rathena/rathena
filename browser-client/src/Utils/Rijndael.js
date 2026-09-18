/**
 * Utils/Rijndael.js
 *
 * Rijndael Wrapper
 *
 * Helper to encrypt data using the game's encryption algorithm.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alisonrag
 */

import RijndaelJS from 'rijndael-js';

/**
 * default key = [6, 169, 33, 64, 54, 184, 161, 91, 81, 46, 3, 213, 52, 18, 0, 6, 6, 169, 33, 64, 54, 184, 161, 91, 81, 46, 3, 213, 52, 18, 0, 6]
 * default chain = [61, 175, 186, 66, 157, 158, 180, 48, 180, 34, 218, 128, 44, 159, 172, 65, 61, 175, 186, 66, 157, 158, 180, 48, 180, 34, 218, 128, 44, 159, 172, 65]
 *
 * bRO key = [6, 169, 33, 64, 54, 184, 161, 91, 81, 46, 3, 213, 52, 18, 0, 6, 61, 175, 186, 66, 157, 158, 180, 48]
 * bRO chain = [61, 175, 186, 66, 157, 158, 180, 48, 180, 34, 218, 128, 44, 159, 172, 65, 1, 2, 4, 8, 16, 32, 128]
 *
 * for general use (XX must be 16, 24 or 32):
 * if you want to use a XX bytes key just get the first XX bytes of the default key
 * if you want to use a XX bytes chain just get the first XX bytes of the default chain
 */

class Rijndael {
	/**
	 * Encrypt data
	 * @param {Uint8Array|Array} input
	 * @param {Uint8Array|Array} key
	 * @param {Uint8Array|Array} chain (IV)
	 * @param {number} size Block size (must be 16, 24 or 32, default 16)
	 * @param {string} mode Mode (must be 'ecb', 'cbc' or 'cfb', default 'ecb')
	 * @returns {Uint8Array} Encrypted data
	 */
	static encrypt(input, key, chain, size = 16, mode = 'ecb') {
		// validate block size
		if (size !== 16 && size !== 24 && size !== 32) {
			console.error('Invalid block size:', size);
			return null;
		}

		// validate mode
		if (mode !== 'ecb' && mode !== 'cbc' && mode !== 'cfb') {
			console.error('Invalid mode:', mode);
			return null;
		}

		// validate key
		if (!key || key.length !== size) {
			console.error('Invalid key length:', key?.length, 'size:', size);
			return null;
		}

		// validate chain (optional)
		if (chain && chain.length !== size) {
			console.error('Invalid chain length:', chain?.length);
			return null;
		}

		// validate input
		if (!input || input.length % size !== 0) {
			console.error('Invalid input length:', input?.length);
			return null;
		}

		const cipher = new RijndaelJS(Array.from(key), mode);
		const iv = chain ? Array.from(chain) : undefined;
		const result = cipher.encrypt(Array.from(input), size, iv);
		return new Uint8Array(result);
	}

	/**
	 * Decrypt data
	 * @param {Uint8Array|Array} input
	 * @param {Uint8Array|Array} key
	 * @param {Uint8Array|Array} chain (IV)
	 * @param {number} size Block size (must be 16, 24 or 32, default 16)
	 * @param {string} mode Mode (must be 'ecb', 'cbc' or 'cfb', default 'ecb')
	 * @returns {Uint8Array} Decrypted data
	 */
	static decrypt(input, key, chain, size = 16, mode = 'ecb') {
		// validate block size
		if (size !== 16 && size !== 24 && size !== 32) {
			console.error('Invalid block size:', size);
			return null;
		}

		// validate mode
		if (mode !== 'ecb' && mode !== 'cbc' && mode !== 'cfb') {
			console.error('Invalid mode:', mode);
			return null;
		}

		// validate key
		if (!key || key.length !== size) {
			console.error('Invalid key length:', key?.length);
			return null;
		}

		// validate chain (optional)
		if (chain && chain.length !== size) {
			console.error('Invalid chain length:', chain?.length);
			return null;
		}

		// validate input
		if (!input || input.length % size !== 0) {
			console.error('Invalid input length:', input?.length);
			return null;
		}

		const cipher = new RijndaelJS(Array.from(key), mode);
		const iv = chain ? Array.from(chain) : undefined;
		const result = cipher.decrypt(Array.from(input), size, iv);
		return new Uint8Array(result);
	}
}

export default Rijndael;
