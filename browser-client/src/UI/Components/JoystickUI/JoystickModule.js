/**
 * UI/Components/JoystickUI/JoystickModule.js
 *
 * Main entry point for the Joystick component.
 * Manages the initialization (prepare) and cleanup (dispose)
 * of all joystick-related sub-services and polling loops.
 *
 * @author AoShinHo
 */

import Polling from './JoystickPollingLoop.js';
import InputService from './JoystickInputService.js';
import Interaction from './JoystickInteractionService.js';
import ShortcutMapper from './JoystickShortcutMapper.js';
import JoystickUIRenderer from './JoystickUIRenderer.js';

export default {
	prepare: function () {
		ShortcutMapper.prepare();
		InputService.prepare();
		Interaction.prepare();
		Polling.start();
		JoystickUIRenderer.hide();
	},
	dispose: function () {
		Polling.stop();
		Interaction.dispose();
		InputService.dispose();
		JoystickUIRenderer.dispose();
	}
};
