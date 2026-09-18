/**
 * UI/Components/JoystickUI/JoystickSetManager.js
 *
 * Manages the toggle state between different shortcut sets (e.g., Set 1 vs Set 2),
 * allowing the user to access multiple layers of mapped skills and items.
 *
 * @author AoShinHo
 */

let currentSet = 1;
export default {
	getCurrentSet: function () {
		return currentSet;
	},
	set: function (n) {
		currentSet = n === 1 || n === 2 ? n : currentSet;
	},
	toggle: function () {
		currentSet = currentSet === 1 ? 2 : 1;
	}
};
