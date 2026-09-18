/**
 * Utils/Base62.js
 *
 * This is a plugin for ROBrowser to convert from and to Base62 format.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alisonrag
 */

const base62_alphabet = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';

// encode
function encode(number) {
	let result = '';
	while (number > 0) {
		result = base62_alphabet[number % 62] + result;
		number = Math.floor(number / 62);
	}
	return result;
}

// decode
function decode(string) {
	let result = 0;
	for (let i = 0; i < string.length; i++) {
		result = result * 62 + base62_alphabet.indexOf(string[i]);
	}
	return result;
}

/**
 * Export
 */
export default {
	encode: encode,
	decode: decode,
	base62_alphabet: base62_alphabet
};
