/**
 * Preferences/Camera.js
 *
 * Camera user preferences
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Preferences from 'Core/Preferences.js';

/**
 * Export
 */
export default Preferences.get(
	'Camera',
	{
		smooth: true,
		zoom: 125.0
	},
	1.1
);
