/**
 * Core/Thread.js
 *
 * Client Thread
 * Manage the Client Thread to send data to it (let another Thread do the hard job : loading files, ...)
 *
 * This file is part of ROBrowser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Memory to get back data
 * @var List
 */
const _memory = {};

/**
 * List of hook callback
 * @var List
 */
const _hook = {};

/**
 * @var {number} uid
 */
let _uid = 0;

/**
 * @var {mixed} origin for security
 */
let _origin = [];

/**
 * @var {window|Worker} context to send data to
 */
let _source = null;

class Thread {
	/**
	 * Send data to thread
	 *
	 * @param {string} type
	 * @param {mixed} data
	 * @param {function} callback
	 */
	static send = (type, data, callback) => {
		let uid = 0;

		if (callback) {
			uid = ++_uid;
			_memory[uid] = callback;
		}

		_source.postMessage({ type, data, uid }, _origin);
	};

	/**
	 * Receive data from Thread
	 * Get back the data, find the caller and execute it
	 *
	 * @param {object} event
	 */
	static receive = event => {
		const uid = event.data.uid;
		const type = event.data.type;

		// Direct callback
		if (uid in _memory) {
			_memory[uid].apply(null, event.data.arguments);
			delete _memory[uid];
		}

		// Hook Feature
		if (type && _hook[type]) {
			_hook[type].call(null, event.data.data);
		}
	};

	/**
	 * Hook receive data
	 *
	 * @param {string} type
	 * @param {function} callback
	 */
	static hook = (type, callback) => {
		_hook[type] = callback;
	};

	/**
	 * Modify where to send informations
	 *
	 * @param {Window} source
	 * @param {string} origin
	 */
	static delegate = (source, origin) => {
		_source = source;
		_origin = origin;
	};

	/**
	 * Initialize Thread
	 */
	static init = () => {
		if (!_source) {
			_source = new Worker(new URL('./ThreadEventHandler.js', import.meta.url), { type: 'module' });
		}

		// Worker context
		if (_source instanceof Worker) {
			_source.addEventListener('message', Thread.receive, false);
		}

		// Other frame worker
		else {
			window.addEventListener('message', Thread.receive, false);
			_source.postMessage({ type: 'SYNC' }, _origin);
		}
	};
}
/**
 * Export
 */
export default Thread;
