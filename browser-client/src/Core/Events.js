/**
 * Core/Events.js
 *
 * Client Manager
 * Manage client files, load GRFs, DATA.INI, extract files from GRFs, ...
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @var {Array} events list
 */
const _events = [];

/**
 * @var {number} game tick (get from rendering loop)
 */
let _tick = 0;

/**
 * @var {number} unique id
 */
let _uid = 0;

/**
 * @Constructor
 */
class Events {
	/**
	 * Alias for setTimeout using the rendering loop getting
	 * bad performances.
	 *
	 * @param {function} callback
	 * @param {number} delay
	 * @return {?} event unique id
	 */
	static setTimeout(callback, delay) {
		let i, count;

		const tick = _tick + delay;
		const event = { callback: callback, tick: tick, uid: _uid++ };

		// Add it to the list, sorted by delay
		for (i = 0, count = _events.length; i < count; ++i) {
			if (tick < _events[i].tick) {
				_events.splice(i, 0, event);
				return event.uid;
			}
		}

		_events.push(event);
		return event.uid;
	}

	/**
	 * Alias for clearTimeout
	 * Remove an event pre-registered
	 *
	 * @param {?} event unique id
	 */
	static clearTimeout(uid) {
		let i;
		const count = _events.length;

		// Find the event and remove it
		for (i = 0; i < count; ++i) {
			if (_events[i].uid === uid) {
				_events.splice(i, 1);
				return;
			}
		}
	}

	/**
	 * Process at each rendering loop
	 *
	 * @param {number} game tick
	 */
	static process(tick) {
		let count = _events.length;

		// Execute time out events.
		while (count > 0) {
			if (_events[0].tick > tick) {
				break;
			}

			_events.shift().callback();
			count--;
		}

		_tick = tick;
	}

	/**
	 * Delete events from memory
	 */
	static free() {
		_events.length = 0;
	}
}
/**
 * Export
 */
export default Events;
