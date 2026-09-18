/**
 * UI/Components/JoystickUI/JoystickPollingLoop.js
 *
 * Provides a high-frequency interval loop to poll the Gamepad API
 * and trigger input state updates at a consistent rate.
 *
 * @author AoShinHo
 */

import InputService from './JoystickInputService.js';

let timeoutHandle = null;
const POLL_RATE_ACTIVE = 100; // 10 FPS
const POLL_RATE_IDLE = 1000; // 1 FPS
export default {
	start: function () {
		if (timeoutHandle) {
			return;
		}
		this.run();
	},
	run: function () {
		const isConnected = InputService.update();

		const nextDelay = isConnected ? POLL_RATE_ACTIVE : POLL_RATE_IDLE;
		const self = this;
		timeoutHandle = setTimeout(function () {
			self.run();
		}, nextDelay);
	},
	stop: function () {
		if (timeoutHandle) {
			clearTimeout(timeoutHandle);
			timeoutHandle = null;
		}
	}
};
