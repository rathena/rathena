/**
 * Preferences/UI.js
 *
 * UI user preferences
 *
 */

import Preferences from 'Core/Preferences.js';

/**
 * Export
 */
export default Preferences.get(
	'UI',
	{
		windowmagnet: true
	},
	1.0
);
