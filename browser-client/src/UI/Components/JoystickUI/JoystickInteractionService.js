/**
 * UI/Components/JoystickUI/JoystickInteractionService.js
 *
 * Acts as the bridge between input signals and game actions.
 * Orchestrates shortcut execution, mouse emulation, camera controls,
 * and character interactions.
 *
 * @author AoShinHo
 */

import ShortCut from 'UI/Components/ShortCut/ShortCut.js';
import InventoryUI from 'UI/Components/Inventory/Inventory.js';
import ItemType from 'DB/Items/ItemType.js';
import Character from './JoystickCharacterControl.js';
import Target from './JoystickTargetService.js';
import Cursor from './JoystickMouseCursorAdapter.js';

import ControlsSettings from 'Preferences/Controls.js';
import SelectionUI from './JoystickSelectionUI.js';
import Input from './JoystickInputService.js';
import DB from 'DB/DBManager.js';
import SkillInfo from 'DB/Skills/SkillInfo.js';
import ShortcutMapper from './JoystickShortcutMapper.js';

export default {
	prepare: function () {},

	dispose: function () {},
	cancelQuick: false,
	executeShortcut: function (index, group) {
		const shortcut = ShortCut.getList()[index];
		if (!shortcut) {
			return;
		}

		if (!shortcut.isSkill) {
			const item = InventoryUI.getUI().getItemById(shortcut.ID);
			if (!item || item.count === 0) {
				return;
			}
		} // Move mouse to target entity position
		else if (ControlsSettings.attackTargetMode) {
			const targetEntity = Target.getEntity();
			if (targetEntity) {
				Cursor.moveMouseToEntity(targetEntity);
			}
		}

		ShortCut.onShortCut({
			cmd: 'EXECUTE' + index
		});

		if (ControlsSettings.joyQuick === 2) {
			Cursor.quickCastClick();
		} else if (ControlsSettings.joyQuick === 1) {
			this.cancelQuick = false;

			const waitforRelease = () => {
				setTimeout(() => {
					const buttons = Input.buttonStates;
					if (ShortcutMapper.getGroup(buttons) !== group) {
						Cursor.quickCastClick();
					} else if (!this.cancelQuick) {
						waitforRelease();
					}
				}, 50);
			};
			waitforRelease();
		}
	},

	openSelectionWindow: function (draggableElement) {
		const index = parseInt(draggableElement.getAttribute('data-index'), 10);
		const isSkill = draggableElement.closest('.skill');
		let itemData;

		if (!isSkill) {
			const item = InventoryUI.getUI().getItemByIndex(index);
			if (item) {
				if (
					item.type === ItemType.UNKNOWN ||
					item.type === ItemType.ETC ||
					item.type === ItemType.CARD ||
					item.type === ItemType.PETEGG ||
					item.type === ItemType.PETARMOR
				) {
					return false;
				}

				itemData = {
					isSkill: false,
					ID: item.ITID,
					value: item.count,
					name: DB.getItemName(item)
				};
			}
		} else {
			const skill = ShortCut.getSkillById(index);
			if (skill) {
				itemData = {
					isSkill: true,
					ID: skill.SKID,
					value: skill.selectedLevel ? skill.selectedLevel : skill.level,
					name: SkillInfo[skill.SKID].SkillName
				};
			}
		}

		if (itemData) {
			SelectionUI.showSelection(itemData);
			return true;
		}

		return false;
	},

	leftClick: function (click) {
		Cursor.leftClick(click);
	},

	rightClick: function (holding) {
		Cursor.rightClick(holding);
	},

	pickUpItem: function () {
		Character.pickUp();
	},

	attackTargeted: function () {
		Character.attack();
	},

	moveCursor: function (dx, dy) {
		Cursor.move(dx, dy);
	},

	cameraZoom: function (zoom) {
		Cursor.changeCameraZoom(zoom);
	},

	cameraAngle: function (angle) {
		Cursor.changeCameraAngle(angle);
	},

	escape: function () {
		Cursor.esc();
	},

	enter: function () {
		Cursor.enter();
	},

	showinfo: function () {
		return Cursor.contextMenu();
	},

	navigateDpad: function (direction) {
		return Cursor.navigateDraggableItems(direction);
	},

	moveCharacter: function (x, y) {
		Character.move(x, y);
	}
};
