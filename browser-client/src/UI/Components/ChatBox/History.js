/**
 * UI/Components/ChatBox/History.js
 *
 * Helper to manage history in chatbox
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @Constructor
 *
 * @param {boolean} does key have to be unique ?
 */
function History(uniqueKey) {
	this.list = [];
	this.uniqueKey = !!uniqueKey;
}

/**
 * @const {number} max message in history
 */
const CACHE_SIZE = 50;

/**
 * @var {number} index in history
 */
History.prototype.index = 0;

/**
 * Push value in history
 *
 * @param {string} message
 */
History.prototype.push = function push(message) {
	let count = this.list.length;
	let pos;

	if (!count || this.list[count - 1] !== message) {
		// Remove duplicated key
		if (this.uniqueKey) {
			pos = this.list.indexOf(message);
			if (pos > -1) {
				this.list.splice(pos, 1);
				count--;
			}
		}

		// Remove first element if overflow
		if (count >= CACHE_SIZE) {
			this.list.shift();
		}

		// Store it
		this.list.push(message);
	}

	// Get new index
	this.index = this.list.length;
};

/**
 * Go back from history
 * @return {string}
 */
History.prototype.previous = function previous() {
	if (this.index > 0) {
		this.index--;
	}

	return this.list[this.index] || '';
};

/**
 * Move to history
 * @return {string}
 */
History.prototype.next = function next() {
	if (this.index < this.list.length) {
		this.index++;
	}

	return this.list[this.index] || '';
};

/**
 * Clean history message
 */
History.prototype.clear = function clear() {
	this.list.length = 0;
	this.index = 0;
};

export default History;
