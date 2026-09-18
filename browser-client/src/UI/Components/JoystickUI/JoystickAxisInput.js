/**
 * UI/Components/JoystickUI/JoystickAxisInput.js
 *
 * Processes analog stick movements (axes).
 * Responsible for translating stick coordinates into character
 * movement and mouse cursor positioning based on defined deadzones.
 *
 * @author AoShinHo
 */

import Interaction from './JoystickInteractionService.js';
import ControlsSettings from 'Preferences/Controls.js';
import JoystickUIRenderer from './JoystickUIRenderer.js';

export default {
	update: function (axes) {
		let active = false;

		// Left stick = movement
		let lx = axes[0];
		let ly = axes[1];

		if (ControlsSettings.joyReverseStick && axes.length >= 4) {
			lx = axes[2];
			ly = axes[3];
		}

		if (Math.abs(lx) > ControlsSettings.joyDeadline || Math.abs(ly) > ControlsSettings.joyDeadline) {
			Interaction.moveCharacter(lx, -ly);
			Interaction.cancelQuick = true;
			active = true;
		}

		// Right stick = cursor
		if (axes.length >= 4) {
			let rx = axes[2];
			let ry = axes[3];

			if (ControlsSettings.joyReverseStick) {
				rx = axes[0];
				ry = axes[1];
			}

			if (Math.abs(rx) > ControlsSettings.joyDeadline || Math.abs(ry) > ControlsSettings.joyDeadline) {
				Interaction.moveCursor(rx, ry);
				active = true;
			}
		}

		if (active) {
			JoystickUIRenderer.show();
		}

		return active;
	}
};
