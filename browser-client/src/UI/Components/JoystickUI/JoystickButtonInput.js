/**
 * UI/Components/JoystickUI/JoystickButtonInput.js
 *
 * Manages the logic for digital button presses.
 * Handles input states (pressed, holding, unpressed),
 * button combination mapping for shortcuts, and world interactions
 * like attacking or interacting with NPCs.
 *
 * @author AoShinHo
 */

import ShortcutMapper from './JoystickShortcutMapper.js';
import Interaction from './JoystickInteractionService.js';
import SetManager from './JoystickSetManager.js';
import JoystickUIRenderer from './JoystickUIRenderer.js';
import SelectionUI from './JoystickSelectionUI.js';

let clickLock = false;
const lockTimeout = 200;

function setClickLock() {
	clickLock = true;
	setTimeout(function () {
		clickLock = false;
	}, lockTimeout);
}

const ButtonInput = {
	update: function (buttons) {
		if (clickLock) {
			return false;
		}

		if (SelectionUI.active()) {
			SelectionUI.handleGamepadInput(buttons);
			return false;
		}

		let pressed = false;

		JoystickUIRenderer.updateVisuals(buttons);

		// special buttons
		pressed |= this._handleSpecial(buttons);

		// set switching
		pressed |= this._handleSetChange(buttons);

		if (!pressed) {
			// basic actions (Left Click, Attack, etc)
			pressed |= this._handleWorldActions(buttons);

			// skills/items
			pressed |= this._handleShortcuts(buttons);
		}

		return pressed;
	},

	_handleWorldActions: function (btn) {
		let pressed = false;

		if (ShortcutMapper.getGroup(btn) !== '') {
			return false;
		}

		// A → left click
		if (btn[0] !== 'unpressed') {
			Interaction.leftClick(btn[0] === 'holding');
			pressed = true;
		}

		// B → right click
		if (btn[1] !== 'unpressed') {
			Interaction.rightClick(btn[1] === 'holding');
			pressed = true;
		}

		// X → attack
		if (btn[2] !== 'unpressed') {
			Interaction.attackTargeted();
			pressed = true;
		}

		// Y → pickup item
		if (btn[3] !== 'unpressed') {
			Interaction.pickUpItem();
			pressed = true;
		}

		if (pressed) {
			setClickLock();
		}

		return pressed;
	},

	_handleSetChange: function (btn) {
		const l2 = btn[6] === 'holding';
		const r2 = btn[7] === 'holding';

		if (l2 && r2) {
			SetManager.toggle();
			JoystickUIRenderer.updateSetIndicator();
			JoystickUIRenderer.sync();
			setClickLock();
			return true;
		}
		return false;
	},

	_handleSpecial: function (buttons) {
		let pressed = false;
		const selectPressed = buttons[8] === 'holding';

		if (selectPressed) {
			if (buttons[12] !== 'unpressed') {
				// D-pad Up
				Interaction.cameraZoom(-2);
				pressed = true;
			} else if (buttons[13] !== 'unpressed') {
				// D-pad Down
				Interaction.cameraZoom(2);
				pressed = true;
			} else if (buttons[14] !== 'unpressed') {
				// D-pad Left
				Interaction.cameraAngle(-5);
				pressed = true;
			} else if (buttons[15] !== 'unpressed') {
				// D-pad Right
				Interaction.cameraAngle(5);
				pressed = true;
			} else if (buttons[9] !== 'unpressed') {
				// Start button
				Interaction.escape();
				pressed = true;
			} else {
				pressed = Interaction.showinfo();
			}

			if (pressed) {
				setClickLock();
				return pressed;
			}
		}

		// D-Pad
		if (buttons[12] !== 'unpressed') {
			// D-pad Up
			Interaction.navigateDpad('up');
			pressed = true;
		} else if (buttons[13] !== 'unpressed') {
			// D-pad Down
			Interaction.navigateDpad('down');
			pressed = true;
		} else if (buttons[14] !== 'unpressed') {
			// D-pad Left
			Interaction.navigateDpad('left');
			pressed = true;
		} else if (buttons[15] !== 'unpressed') {
			// D-pad Right
			Interaction.navigateDpad('right');
			pressed = true;
		} else if (buttons[9] !== 'unpressed') {
			// Start button
			Interaction.enter();
			pressed = true;
		}

		if (pressed) {
			setClickLock();
		}

		return pressed;
	},

	_handleShortcuts: function (btn) {
		const idx = ShortcutMapper.getShortcutIndex(btn);
		if (idx !== -1) {
			Interaction.executeShortcut(idx, ShortcutMapper.getGroup(btn));
			setClickLock();
			return true;
		}
		return false;
	}
};

export default ButtonInput;
