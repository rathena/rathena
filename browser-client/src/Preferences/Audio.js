/**
 * Preferences/Audio.js
 *
 * Audio preferences
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
	'Audio',
	{
		BGM: {
			play: true,
			volume: 0.5
		},

		Sound: {
			play: true,
			volume: 0.5
		}
	},
	1.0
);
