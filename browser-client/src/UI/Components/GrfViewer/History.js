/**
 * UI/Components/GrfViewer/History.js
 *
 * Intro Manager
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @var {Array} cache history
 */
const _history = [];

/**
 * @var {number} position in history
 */
let _index = -1;

/**
 * @var {HTMLElement} previous button
 */
let _previous;

/**
 * @var {HTMLElement} next button
 */
let _next;

/**
 * Initialize history with buttons
 *
 * @param {HTMLElement} previousBtn
 * @param {HTMLElement} nextBtn
 */
function init(previousBtn, nextBtn) {
	_previous = previousBtn;
	_next = nextBtn;

	_previous.classList.remove('on');
	_next.classList.remove('on');
}

/**
 * Set a new link to cache
 *
 * @param {string} link
 */
function push(link) {
	_history.length = ++_index;
	_history.push(link);

	_next.classList.remove('on');

	if (_history.length > 1) {
		_previous.classList.add('on');
	}
}

/**
 * Move forward in history
 *
 * @return {string} url
 */
function next() {
	if (_index + 1 >= _history.length) {
		return null;
	}

	_previous.classList.add('on');
	if (_index + 2 >= _history.length) {
		_next.classList.remove('on');
	}

	return _history[++_index];
}

/**
 * Move back to history
 *
 * @return {string} url
 */
function previous() {
	if (_index - 1 < 0) {
		return null;
	}

	_next.classList.add('on');
	if (_index - 2 < 0) {
		_previous.classList.remove('on');
	}

	return _history[--_index];
}

/**
 * Export
 */
export default {
	push,
	next,
	previous,
	init
};
