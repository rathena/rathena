/**
 * Controls/KeyEventHandler.js
 *
 * Key Manager
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Keys Constants
 */
const KEYS = {
	// Special Keys
	CTRL: false,
	SHIFT: false,
	ALT: false,

	MB_L: 1,
	MB_R: 2,
	MB_C: 4,
	BACK: 8,
	TAB: 9,
	CLEAR: 12,
	ENTER: 13,
	//SHIFT:    16,
	//CTRL:     17,
	//ALT:      18,
	PAUSE: 19,
	CAPS: 20,
	HANGUL: 21,
	HANJA: 25,
	ESCAPE: 27,
	SPACE: 32,
	PAGEDOWN: 33,
	PAGEUP: 34,
	HOME: 35,
	END: 36,
	LEFT: 37,
	UP: 38,
	RIGHT: 39,
	DOWN: 40,
	SELECT: 41,
	EXECUTE: 43,
	SNAPSHOT: 44,
	INSERT: 45,
	SUPR: 46,
	HELP: 47,

	// Numeric
	0: 48,
	1: 49,
	2: 50,
	3: 51,
	4: 52,
	5: 53,
	6: 54,
	7: 55,
	8: 56,
	9: 57,

	// Regular keys:
	A: 65,
	B: 66,
	C: 67,
	D: 68,
	E: 69,
	F: 70,
	G: 71,
	H: 72,
	I: 73,
	J: 74,
	K: 75,
	L: 76,
	M: 77,
	N: 78,
	O: 79,
	P: 80,
	Q: 81,
	R: 82,
	S: 83,
	T: 84,
	U: 85,
	V: 86,
	W: 87,
	X: 88,
	Y: 89,
	Z: 90,

	APPS: 93,

	// Num pad
	Num_0: 96,
	Num_1: 97,
	Num_2: 98,
	Num_3: 99,
	Num_4: 100,
	Num_5: 101,
	Num_6: 102,
	Num_7: 103,
	Num_8: 104,
	Num_9: 105,
	MULTIPLY: 106,
	ADD: 107,
	SEP: 108,
	SUB: 109,
	DECIMAL: 110,
	DEVIDE: 111,

	// Functions keys
	F1: 112,
	F2: 113,
	F3: 114,
	F4: 115,
	F5: 116,
	F6: 117,
	F7: 118,
	F8: 119,
	F9: 120,
	F10: 121,
	F11: 122,
	F12: 123,
	F13: 124,
	F14: 125,
	F15: 126,
	F16: 127,
	F17: 128,
	F18: 129,
	F19: 130,
	F20: 131,
	F21: 132,
	F22: 133,
	F23: 134,
	F24: 135,

	NUM: 144,
	SCROLL: 145,
	';': 186,
	'=': 187,
	',': 188,
	'-': 189,
	'.': 190,
	'/': 191,
	'`': 192,
	'[': 219,
	'\\': 220,
	']': 221,
	"'": 222
};

/**
 * Get Key name from id
 *
 * @param {number} key id
 * @param {string} readable key nam
 */
Object.defineProperty(KEYS, 'toReadableKey', {
	writable: false,
	enumerable: false,
	value: function toReadableKey(keyId) {
		let i;

		const keys = Object.keys(this);
		const count = keys.length;

		for (i = 0; i < count; ++i) {
			if (this[keys[i]] === keyId) {
				return keys[i];
			}
		}

		return '';
	}
});

/**
 * Get Key name from id
 *
 * @param {number} key id
 * @param {string} readable key nam
 */
Object.defineProperty(KEYS, 'getKeyIdString', {
	writable: false,
	enumerable: false,
	value: function getKeyIdString(shortcut) {
		const str = [];
		const tmp = this.toReadableKey(parseInt(shortcut.key, 10));

		if (shortcut.alt) {
			str.push('ALT');
		}

		if (shortcut.shift) {
			str.push('SHIFT');
		}

		if (shortcut.ctrl) {
			str.push('CTRL');
		}

		if (tmp) {
			str.push(tmp);
		}

		return str.join(' + ');
	}
});

/**
 * Keys CTRL/ALT/SHIFT Manager
 */
const onKeyEvent = event => {
	KEYS.SHIFT = !!event.shiftKey;
	KEYS.CTRL = !!event.ctrlKey;
	KEYS.ALT = !!event.altKey;
};
window.addEventListener('keydown', onKeyEvent);
window.addEventListener('keyup', onKeyEvent);

/**
 * Reset keys state when window got focused
 */
window.addEventListener('focus', () => {
	KEYS.SHIFT = false;
	KEYS.CTRL = false;
	KEYS.ALT = false;
});

/**
 * Get the actual focused element, piercing through Shadow DOM boundaries.
 * In Light DOM, document.activeElement returns the focused element directly.
 * In Shadow DOM, it returns the host element; this helper traverses into
 * shadow roots to find the real focused element (e.g., an <input> inside
 * a GUIComponent's shadow root).
 */
Object.defineProperty(KEYS, 'getDeepActiveElement', {
	writable: false,
	enumerable: false,
	value: function getDeepActiveElement() {
		let el = document.activeElement;
		while (el?.shadowRoot?.activeElement) {
			el = el.shadowRoot.activeElement;
		}
		return el;
	}
});

/**
 * Export
 */
export default KEYS;
